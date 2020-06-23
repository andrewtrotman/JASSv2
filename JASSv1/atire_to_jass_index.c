/*
	ATIRE_TO_MAIN_HEAP.C
	--------------------
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sstream>
#include <stdint.h>
#include <sys/stat.h>
#include <limits.h>
#include "maths.h"
#include "memory.h"
#include "search_engine.h"
#include "btree_iterator.h"
#include "search_engine_btree_leaf.h"
#include "impact_header.h"
#include "compress_variable_byte.h"
#include "compress_simple8b.h"
#include "compress_qmx.h"
#include "compress_qmx_d4.h"
#include "compress_integer_elias_gamma_simd.h"
#include "compress_integer_elias_delta_simd.h"
#include "compress_integer_elias_gamma_simd_vb.h"

#ifndef IMPACT_HEADER
	#error "set IMPACT_HEADER in the ATIRE makefile and start all over again"
#endif
#ifndef SPECIAL_COMPRESSION
	#error "set SPECIAL_COMPRESSION in the ATIRE makefile and start all over again"
#endif

/*
	class CI_ATIRE_POSTINGS
	-----------------------
*/
class CI_ATIRE_postings
{
public:
	uint32_t tf;
	uint32_t docid;
};

/*
	class CI_ATIRE_POSTINGS_LIST
	----------------------------
*/
class CI_ATIRE_postings_list
{
public:
	uint32_t df;
	uint32_t cf;
	uint32_t postings_list_length;
	CI_ATIRE_postings *postings_list;
} raw_postings = {0,0,0,0};


#ifdef _MSC_VER
	#include <direct.h>

	#define atoll(x) _atoi64(x)
	#define mkdir(x,y) _mkdir(x)
#endif

using namespace std;

#define BUFFER_SIZE (1024*1024*1024)
static char *buffer;

/*
	struct CI_HEAP_QUANTUM_INDEXER
	------------------------------
*/
struct CI_heap_quantum
{
uint16_t impact;						// the quantum impact score
uint8_t *offset;						// where the data is
uint64_t length;						// length of the compressed postings list (in bytes)
uint64_t length_with_padding;		// length of the compressed postings list (in bytes) (plus any necessary SSE padding)
uint32_t quantum_frequency;		// number of integers in the quantum (needed by QMX and ant ATIRE based decompressors)
} ;

static CI_heap_quantum postings_offsets[0x100];

char *termlist[1024 * 1024];
uint32_t termlist_length = 0;

ostringstream *vocab_in_current_file = NULL;

uint8_t file_mode;					// which compressor we're using

/*
	STRING_COMPARE()
	----------------
*/
int string_compare(const void *a, const void *b)
{
char **one = (char **)a;
char **two = (char **)b;

return strcmp(*one, *two);
}

/*
	LOAD_TOPIC_FILE()
	-----------------
*/
void load_topic_file(char *filename)
{
static const char *SEPERATORS = " \t\n\r";
FILE *fp;
char *term;

if ((fp = fopen(filename, "rb")) == NULL)
	exit(printf("Cannot open ATIRE topic file '%s'\n", filename));

while (fgets(buffer, BUFFER_SIZE, fp) != NULL)
	if ((term = strtok(buffer, SEPERATORS)) != NULL)			// discard the first token as its the topic ID
		for (term = strtok(NULL, SEPERATORS); term != NULL; term = strtok(NULL, SEPERATORS))
			termlist[termlist_length++] = strdup(term);

qsort(termlist, termlist_length, sizeof(*termlist), string_compare);
}

/*
	USAGE()
	-------
*/
uint8_t usage(char *filename)
{
printf("Usage: %s <index.aspt> [<topicfile>] [-8|-c|-s|-q|-Q|-R|-G|-D|-g] [-SSE]\n", filename);
puts("Generate <topicfile> with trec2query <trectopicfile>");
puts("-8 compress the postings using simple 8b");
puts("-c compress the postings using Variable Byte Encoding (default)");
puts("-s 'static' do not compress the postings");
puts("-q compress the postings using QMX-D1 (QMX + 'regular' d-gaps)");
puts("-Q compress the postings using QMX-D4 (QMX + 'D4 d-gaps)");
puts("-R compress the postings using QMX-D0 (QMX  without d-gaps)");
puts("-SSE SSE algn postings lists (this is the default for QMX based schemes)");
puts("-G compress the postings using Group Elias Gamma SIMD (D1 gaps)");
puts("-g compress the postings using Group Elias Gamma SIMD with vByte (D1 gaps)");
puts("-D compress the postings using Group Elias Delta SIMD (D1 gaps)");

return 1;
}

#define MAX_DOCIDS_PER_IMPACT (1024 * 1024 * 60)

uint32_t remember_should_compress = true;
uint64_t remember_buffer_size, remember_compressed_buffer_size;
uint32_t *remember_buffer, *decompress_buffer;
uint32_t *remember_into = remember_buffer;
ANT_compress *compressor;
uint8_t *remember_compressed;
uint32_t sse_alignment = 0;

/*
	REMEMBER()
	----------
*/
void remember(uint32_t docid)
{
*remember_into++ = docid;

if (remember_into >= remember_buffer + remember_buffer_size)
	exit(printf("Too many integers per quantum, change MAX_DOCIDS_PER_IMPACT and re-build\n"));
}

/*
	REMEMBER_COMPRESS()
	-------------------
*/
ANT_compress_qmx QMX;
ANT_compress_qmx_d4 QMX_D4;

uint8_t *remember_compress(uint32_t *length, uint32_t *padded_length, uint32_t *integers_in_quantum)
{
uint32_t is, was, compressed_size;

*integers_in_quantum = remember_into - remember_buffer;

if (!remember_should_compress)
	{
	compressed_size = (sizeof(*remember_buffer) * (remember_into - remember_buffer));

	if (compressed_size > remember_compressed_buffer_size)
		exit(printf("Can't block copy in remember_compress(), change MAX_DOCIDS_PER_IMPACT and re-build\n"));

	memcpy(remember_compressed, remember_buffer, compressed_size);
	}
else
	{
	if (file_mode == 'Q' || file_mode == 'R')
		{
		/*
			We don't need to compute deltas because the scheme does it for us.
		*/
		}
	else
		{
		/*
			Use D1
		*/
		was = 0;
		for (uint32_t *current = remember_buffer; current < remember_into; current++)
			{
			is = *current;
			*current -= was;
			was = is;
			}
		}

	/*
		Now compress
	*/
	compressed_size = compressor->compress(remember_compressed, remember_compressed_buffer_size, remember_buffer, remember_into - remember_buffer);

#ifdef NEVER
							/*
								Check that the compressed sequence an decompress to the original
							*/
							if (file_mode == 'R' || file_mode == 'q')
								QMX.decodeArray((uint32_t *)remember_compressed, compressed_size, decompress_buffer, remember_into - remember_buffer);
							else if (file_mode == 'Q')
								QMX_D4.decodeArray((uint32_t *)remember_compressed, compressed_size, decompress_buffer, remember_into - remember_buffer);
							else
								compressor->decompress(decompress_buffer, remember_compressed, remember_into - remember_buffer);
							for (uint32_t ch = 0; ch < remember_into - remember_buffer; ch++)
								if (decompress_buffer[ch] != remember_buffer[ch])
									exit(printf("Decompressed doesn't match compressed at position %u\n", ch));
							/*
							*/
#endif

	if (compressed_size <= 0)
		exit(printf("cannot compress, change MAX_DOCIDS_PER_IMPACT and re-build\n"));
	}
	
/*
	If we need to SSE-word align then do so
*/
*length = compressed_size;
if (sse_alignment != 0)
	compressed_size = ((compressed_size + sse_alignment - 1) / sse_alignment) * sse_alignment;
*padded_length = compressed_size;

/*
	rewind the buffer
*/
remember_into = remember_buffer;

return remember_compressed;
}

/*
	Globals used by the main_heap code
*/
uint64_t unique_terms_in_index = 0;
FILE *fp, *vocab_dot_c, *postings_dot_bin, *vocab_terms_dot_bin, *vocab_dot_bin;
uint64_t max_docid = 0, max_q = 0;

/*
	GENERATE_DOCLIST()
	------------------
*/
void generate_doclist(char *index_aspt)
{
ANT_memory memory;
ANT_search_engine search_engine(&memory);
unsigned long buffer_length;
uint64_t start, end, doc;
char *buffer;
char **filenames;
long current;
FILE *doclist_dot_bin;
uint64_t *offset;

if (!search_engine.open(index_aspt))
	exit(printf("Can't open index file %s\n", index_aspt));

start = search_engine.get_variable((char *)"~documentfilenamesstart");
end = search_engine.get_variable((char *)"~documentfilenamesfinish");
if (start == 0 || end == 0)
	exit(printf("Document IDs aren't in the index\n"));

buffer_length = (unsigned long)(end - start);
buffer = new char [sizeof(*buffer) * buffer_length];
offset = new uint64_t [sizeof(*buffer) * buffer_length];				// this is way toooo big!

filenames = search_engine.get_document_filenames(buffer, &buffer_length);

/*
	do the doclist first as its fastest
*/
if ((doclist_dot_bin = fopen("CIdoclist.bin", "wb")) == NULL)
	exit(printf("Cannot open CIdoclist.bin output file"));

for (doc = 0; doc < search_engine.document_count(); doc++)
	{
	offset[doc] = ftell(doclist_dot_bin);
	fwrite(filenames[doc], strlen(filenames[doc]) + 1, 1, doclist_dot_bin);			// include the '\0' on the file to make the other end easier
	}
fwrite(offset, doc, sizeof(*offset), doclist_dot_bin);
fwrite(&doc, 1, sizeof(doc), doclist_dot_bin);

delete [] buffer;
delete [] offset;

fclose(doclist_dot_bin);
}

/*
	INITIALISE_MAIN_HEAP_STUFF()
	----------------------------
*/
void initialise_main_heap_stuff(int argc, char *argv[])
{
char *end_of_term, *buffer_address;
uint64_t first_time = true;
uint32_t parameter;

buffer = new char [1024 * 1024 * 1024];
buffer_address = buffer;

/*
	Default is Variable Byte
*/
file_mode = 'c';
remember_should_compress = true;
compressor = new ANT_compress_variable_byte;

/*
	Check the parametes to see if are anything else
*/
if (argc < 2)
	exit(usage(argv[0]));
for (parameter = 2; parameter < argc; parameter++)
	if (strcmp(argv[parameter], "-s") == 0)
		{
		file_mode = 's';
		remember_should_compress = false;
		}
	else if (strcmp(argv[parameter], "-8") == 0)
		{
		file_mode = '8';
		remember_should_compress = true;
		compressor = new ANT_compress_simple8b;
		}
	else if (strcmp(argv[parameter], "-q") == 0)
		{
		file_mode = 'q';
		remember_should_compress = true;
		compressor = new ANT_compress_qmx;
		sse_alignment = 16;
		}
	else if (strcmp(argv[parameter], "-Q") == 0)
		{
		file_mode = 'Q';
		remember_should_compress = true;
		compressor = new ANT_compress_qmx_d4;
		sse_alignment = 16;
		}
	else if (strcmp(argv[parameter], "-R") == 0)
		{
		file_mode = 'R';
		remember_should_compress = true;
		compressor = new ANT_compress_qmx;
		sse_alignment = 16;
		}
	else if (strcmp(argv[parameter], "-c") == 0)
		{
		file_mode = 'c';
		remember_should_compress = true;
		compressor = new ANT_compress_variable_byte;
		}
	else if (strcmp(argv[parameter], "-G") == 0)
		{
		file_mode = 'G';
		remember_should_compress = true;
		compressor = new JASS::compress_integer_elias_gamma_simd;
//		sse_alignment = 16;
		}
	else if (strcmp(argv[parameter], "-g") == 0)
		{
		file_mode = 'g';
		remember_should_compress = true;
		compressor = new compress_integer_elias_gamma_simd_vb;
//		sse_alignment = 16;
		}
	else if (strcmp(argv[parameter], "-D") == 0)
		{
		file_mode = 'D';
		remember_should_compress = true;
		compressor = new JASS::compress_integer_elias_delta_simd;
//		sse_alignment = 16;
		}
	else if (strcmp(argv[parameter], "-SSE") == 0)
		sse_alignment = 16;
	else
		load_topic_file(argv[parameter]);

/*
	do the doclist first as its fastest
*/
generate_doclist(argv[1]);

/*
	Now do the postings lists
*/
if ((vocab_dot_c = fopen("CIvocab_heap.c", "wb")) == NULL)
	exit(printf("Cannot open CIvocab_heap.c output file\n"));

if ((vocab_dot_bin = fopen("CIvocab.bin", "wb")) == NULL)
	exit(printf("Cannot open CIvocab.bin output file\n"));
if ((vocab_terms_dot_bin = fopen("CIvocab_terms.bin", "wb")) == NULL)
	exit(printf("Cannot open CIvocab_terms.bin output file\n"));

fprintf(vocab_dot_c, "#include <stdint.h>\n");
fprintf(vocab_dot_c, "#include \"CI.h\"\n");
fprintf(vocab_dot_c, "class CI_vocab_heap CI_dictionary[] = {\n");

if ((postings_dot_bin = fopen("CIpostings.bin", "wb")) == NULL)
	exit(printf("Cannot open CIpostings.h output file\n"));

/*
	Tell the postings list which compression strategy is being used
*/
fwrite(&file_mode, 1, 1, postings_dot_bin);
}

/*
	TERMINATE_MAIN_HEAP_STUFF()
	---------------------------
*/
void terminate_main_heap_stuff(void)
{
fprintf(vocab_dot_c, "};\n\n");
fprintf(vocab_dot_c, "uint32_t CI_unique_terms = %llu;\n", unique_terms_in_index);
fprintf(vocab_dot_c, "uint32_t CI_unique_documents = %llu;\n", max_docid + 1);			// +1 because we count from zero
fprintf(vocab_dot_c, "uint32_t CI_max_q = %llu;\n", max_q);

fclose(vocab_dot_c);
fclose(postings_dot_bin);
fclose(vocab_terms_dot_bin);
fclose(vocab_dot_bin);
}

/*
	CONVERT_TO_MAIN_HEAP()
	----------------------
*/
void convert_to_main_heap(char *term)
{
uint64_t previous_impact;
uint32_t docids_in_impact;
uint64_t cf, df, docid, impact;
uint64_t which_impact, end;
uint32_t data_length_in_bytes, data_length_in_bytes_with_padding, quantum;
uint8_t *data;
uint64_t current_quantum;

unique_terms_in_index++;
current_quantum = 0;
uint64_t sum_of_lengths = 0;
docids_in_impact = 0;
previous_impact = ULONG_MAX;
ostringstream term_method_list;
CI_ATIRE_postings *posting;

for (posting = raw_postings.postings_list; posting < raw_postings.postings_list + raw_postings.df; posting++)
	{
	docid = posting->docid;
	impact = posting->tf;

	if  (docid > max_docid)
		max_docid = docid;

	if (impact > max_q)
		max_q = impact;

	if (impact != previous_impact)
		{
		if (previous_impact != ULONG_MAX)
			{
			data = remember_compress(&data_length_in_bytes, &data_length_in_bytes_with_padding, &postings_offsets[current_quantum].quantum_frequency);
			postings_offsets[current_quantum].impact = previous_impact;
			postings_offsets[current_quantum].offset = new uint8_t [data_length_in_bytes];
			memcpy(postings_offsets[current_quantum].offset, data, data_length_in_bytes);
			postings_offsets[current_quantum].length = data_length_in_bytes;
			postings_offsets[current_quantum].length_with_padding = data_length_in_bytes_with_padding;
			sum_of_lengths += data_length_in_bytes;
			current_quantum++;
			}
		previous_impact = impact;
		}
	remember(docid);
	docids_in_impact++;
	}

/*
	Don't forget the final quantum
*/
data = remember_compress(&data_length_in_bytes, &data_length_in_bytes_with_padding, &postings_offsets[current_quantum].quantum_frequency);
postings_offsets[current_quantum].impact = impact;
postings_offsets[current_quantum].offset = new uint8_t [data_length_in_bytes];
memcpy(postings_offsets[current_quantum].offset, data, data_length_in_bytes);
postings_offsets[current_quantum].length = data_length_in_bytes;
postings_offsets[current_quantum].length_with_padding = data_length_in_bytes_with_padding;
sum_of_lengths += data_length_in_bytes;
current_quantum++;

/*
	Write out to disk
*/
/*
	Tell the vocab where we are
*/
uint64_t postings_list = ftell(postings_dot_bin);
uint64_t term_location = ftell(vocab_terms_dot_bin);
fwrite(term, strlen(term) + 1, 1, vocab_terms_dot_bin);		// include the '\0'
fwrite(&term_location, sizeof(term_location), 1, vocab_dot_bin);			// 64-bis				// the locaton of the term in the vocab_terms file
fwrite(&postings_list, sizeof(postings_list), 1, vocab_dot_bin);			// 64-bits
fwrite(&current_quantum, sizeof(current_quantum), 1, vocab_dot_bin);		// 32-bits

/*
	Write a pointer to each quantum header
*/
uint32_t header_size = sizeof(postings_offsets->impact) + sizeof(uint64_t) + sizeof(postings_offsets->length) + sizeof(postings_offsets->quantum_frequency);
uint64_t quantum_pointer = sizeof(quantum_pointer) * current_quantum;		// start at the end of the list of pointers

quantum_pointer += postings_list;			// offsets are relative to the start of the file
for (quantum = 0; quantum < current_quantum; quantum++)
	{
	fwrite(&quantum_pointer, sizeof(quantum_pointer), 1, postings_dot_bin);
	quantum_pointer += header_size;										// the next one is this many bytes further on
	}
/*
	Now write out the quantim headers
*/
uint64_t sse_offset, sse_padding;
uint64_t offset = header_size  * (current_quantum + 1) + sizeof(quantum_pointer) * current_quantum;		// start the data at the end of the quantum headers (which includes a zero termnator);
offset += postings_list;					// offset from the start of the file

if (sse_alignment != 0)
	{
	sse_offset = ((offset + sse_alignment - 1) / sse_alignment) * sse_alignment;
	sse_padding = sse_offset - offset;
	offset = sse_offset;
	}

for (quantum = 0; quantum < current_quantum; quantum++)
	{
	fwrite(&postings_offsets[quantum].impact, sizeof(postings_offsets[quantum].impact), 1, postings_dot_bin);
	fwrite(&offset, sizeof(offset), 1, postings_dot_bin);

	end = offset + postings_offsets[quantum].length;
	fwrite(&end, sizeof(end), 1, postings_dot_bin);

	fwrite(&postings_offsets[quantum].quantum_frequency, sizeof(postings_offsets[quantum].quantum_frequency), 1, postings_dot_bin);

	offset += postings_offsets[quantum].length_with_padding;
	}
	
/*
	Terminate the quantum header list with a bunch of zeros
*/
uint8_t zero[] = {0,0,  0,0,0,0,0,0,0,0,   0,0,0,0,0,0,0,0,   0,0,0,0};
fwrite(&zero, header_size, 1, postings_dot_bin);

if (sse_alignment != 0 && sse_padding != 0)
	{
	if (sse_padding > sizeof(zero))
		exit(printf("Padding is too large, fix the source code (sorry)\n"));
	fwrite(zero, sse_padding, 1, postings_dot_bin);
	}

/*
	Now write the compressed postings lists
*/
for (quantum = 0; quantum < current_quantum; quantum++)
	{
	fwrite(postings_offsets[quantum].offset, postings_offsets[quantum].length_with_padding, 1, postings_dot_bin);
	delete [] postings_offsets[quantum].offset;
	}
}

/*
	PROCESS()
	---------
*/
void process(ANT_compression_factory *factory, uint32_t quantum_count, ANT_compressable_integer *impact_header, ANT_compressable_integer *buffer, unsigned char *postings_list)
{
CI_ATIRE_postings *into = raw_postings.postings_list;
long long docid, sum;
ANT_compressable_integer *current, *end;
ANT_compressable_integer *impact_value_ptr, *doc_count_ptr, *impact_offset_ptr;
ANT_compressable_integer *impact_offset_start;

sum = 0;
impact_value_ptr = impact_header;
doc_count_ptr = impact_header + quantum_count;
impact_offset_start = impact_offset_ptr = impact_header + quantum_count * 2;

while (doc_count_ptr < impact_offset_start)
	{
	factory->decompress(buffer, postings_list + *impact_offset_ptr, *doc_count_ptr);

	docid = -1;
	current = buffer;
	end = buffer + *doc_count_ptr;
	while (current < end)
		{
		docid += *current++;

		into->tf = *impact_value_ptr;
		into->docid = docid;
		into++;
		}

	sum += *doc_count_ptr;

	impact_value_ptr++;
	impact_offset_ptr++;
	doc_count_ptr++;
	}
}

/*
	MAIN()
	------
*/
int main(int argc, char *argv[])
{
initialise_main_heap_stuff(argc, argv);

ANT_compressable_integer *raw;
long long postings_list_size;
long long raw_list_size;
unsigned char *postings_list = NULL;
char *term;
ANT_search_engine_btree_leaf leaf;
ANT_compression_factory factory;
ANT_memory memory;
ANT_search_engine search_engine(&memory);

remember_buffer_size = MAX_DOCIDS_PER_IMPACT;
remember_buffer = new uint32_t [remember_buffer_size];
decompress_buffer = new uint32_t [remember_buffer_size];
remember_into = remember_buffer;
remember_compressed_buffer_size = remember_buffer_size * sizeof(uint32_t);
remember_compressed = new uint8_t [remember_compressed_buffer_size];

search_engine.open(argv[1]);

ANT_btree_iterator iterator(&search_engine);

quantum_count_type the_quantum_count;
beginning_of_the_postings_type beginning_of_the_postings;
long long impact_header_info_size = ANT_impact_header::INFO_SIZE;
long long impact_header_size = ANT_impact_header::NUM_OF_QUANTUMS * sizeof(ANT_compressable_integer) * 3;
ANT_compressable_integer *impact_header_buffer = (ANT_compressable_integer *)malloc(impact_header_size);

postings_list_size = search_engine.get_postings_buffer_length();
raw_list_size = sizeof(*raw) * (search_engine.document_count() + ANT_COMPRESSION_FACTORY_END_PADDING);

postings_list = (unsigned char *)malloc((size_t)postings_list_size);
raw = (ANT_compressable_integer *)malloc((size_t)raw_list_size);

for (term = iterator.first(NULL); term != NULL; term = iterator.next())
	{
	iterator.get_postings_details(&leaf);
// puts(term);
	if (*term == '~')
		break;
	else
		{
		if ((termlist_length != 0) && (bsearch(&term, termlist, termlist_length, sizeof(*termlist), string_compare) == NULL))
			continue;										// only add to the output file if the term is in the term list (or we have no term list)
			
//printf("%s %lld %lld\n", term, leaf.local_collection_frequency, leaf.local_document_frequency);
		
		/*
			Make sure we have enough room to fit the postings into the intermediary
		*/
		raw_postings.cf = leaf.local_collection_frequency;
		raw_postings.df = leaf.local_document_frequency;
		if (raw_postings.postings_list_length < leaf.local_document_frequency)
			{
			delete [] raw_postings.postings_list;
			raw_postings.postings_list_length = leaf.local_document_frequency;
			raw_postings.postings_list = new CI_ATIRE_postings[raw_postings.postings_list_length];
			}
	
		/*
			load the postings list
		*/
		postings_list = search_engine.get_postings(&leaf, postings_list);

		/*
			decompress the header
		*/
		the_quantum_count = ANT_impact_header::get_quantum_count(postings_list);
		beginning_of_the_postings = ANT_impact_header::get_beginning_of_the_postings(postings_list);
		factory.decompress(impact_header_buffer, postings_list + impact_header_info_size, the_quantum_count * 3);

		/*
			print the postings
		*/
		process(&factory, the_quantum_count, impact_header_buffer, raw, postings_list + beginning_of_the_postings);

//for (uint32_t p = 0; p < raw_postings.df; p++)
//	printf("<%u,%u>", raw_postings.postings_list[p].docid, raw_postings.postings_list[p].tf);

		convert_to_main_heap(term);
		}
	}
terminate_main_heap_stuff();

return 0;
}
