/*
	UNITTEST.CPP
	------------
	Copyright (c) 2016-2019 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include "beap.h"
#include "simd.h"
#include "file.h"
#include "heap.h"
#include "ascii.h"
#include "maths.h"
#include "slice.h"
#include "timer.h"
#include "parser.h"
#include "unicode.h"
#include "version.h"
#include "reverse.h"
#include "threads.h"
#include "evaluate.h"
#include "checksum.h"
#include "quantize.h"
#include "bitstream.h"
#include "bitstring.h"
#include "query_heap.h"
#include "statistics.h"
#include "evaluate_f.h"
#include "hash_table.h"
#include "run_export.h"
#include "top_k_heap.h"
#include "stem_porter.h"
#include "top_k_qsort.h"
#include "binary_tree.h"
#include "commandline.h"
#include "pointer_box.h"
#include "evaluate_map.h"
#include "serialise_ci.h"
#include "hash_pearson.h"
#include "parser_query.h"
#include "parser_fasta.h"
#include "channel_file.h"
#include "channel_trec.h"
#include "query_bucket.h"
#include "dynamic_array.h"
#include "allocator_cpp.h"
#include "instream_file.h"
#include "index_manager.h"
#include "query_maxblock.h"
#include "allocator_pool.h"
#include "index_postings.h"
#include "accumulator_2d.h"
#include "channel_buffer.h"
#include "instream_memory.h"
#include "run_export_trec.h"
#include "evaluate_recall.h"
#include "hardware_support.h"
#include "allocator_memory.h"
#include "ranking_function.h"
#include "serialise_jass_v1.h"
#include "serialise_integers.h"
#include "evaluate_precision.h"
#include "instream_file_star.h"
#include "parser_unicoil_json.h"
#include "query_maxblock_heap.h"
#include "accumulator_counter.h"
#include "compress_integer_all.h"
#include "evaluate_buying_power.h"
#include "compress_integer_none.h"
#include "index_postings_impact.h"
#include "compress_general_zlib.h"
#include "instream_document_trec.h"
#include "instream_document_warc.h"
#include "evaluate_selling_power.h"
#include "evaluate_buying_power4k.h"
#include "instream_document_fasta.h"
#include "serialise_forward_index.h"
#include "index_manager_sequential.h"
#include "compress_integer_carry_8b.h"
#include "compress_integer_simple_9.h"
#include "evaluate_relevant_returned.h"
#include "compress_integer_simple_8b.h"
#include "compress_integer_simple_16.h"
#include "evaluate_cheapest_precision.h"
#include "compress_integer_bitpack_64.h"
#include "ranking_function_atire_bm25.h"
#include "instream_directory_iterator.h"
#include "compress_integer_elias_gamma.h"
#include "compress_integer_elias_delta.h"
#include "compress_integer_bitpack_128.h"
#include "compress_integer_bitpack_256.h"
#include "compress_integer_relative_10.h"
#include "compress_integer_qmx_jass_v1.h"
#include "evaluate_mean_reciprocal_rank.h"
#include "compress_integer_stream_vbyte.h"
#include "compress_integer_qmx_original.h"
#include "compress_integer_qmx_improved.h"
#include "compress_integer_carryover_12.h"
#include "evaluate_rank_biased_precision.h"
#include "compress_integer_variable_byte.h"
#include "instream_document_unicoil_json.h"
#include "accumulator_counter_interleaved.h"
#include "evaluate_mean_reciprocal_rank4k.h"
#include "evaluate_expected_search_length.h"
#include "compress_integer_simple_9_packed.h"
#include "compress_integer_elias_delta_simd.h"
#include "compress_integer_simple_8b_packed.h"
#include "compress_integer_simple_16_packed.h"
#include "compress_integer_elias_gamma_simd.h"
#include "compress_integer_bitpack_32_reduced.h"
#include "compress_integer_elias_gamma_bitwise.h"
#include "compress_integer_elias_delta_bitwise.h"
#include "compress_integer_elias_gamma_simd_vb.h"

/*
	MAIN()
	------
*/
int main(void)
	{
	int failed = true;			// main() returns 0 on success

	/*
		Output basic statistics about JASS as it stands
	*/
	setvbuf(stdout, NULL, _IONBF, 0);
 	puts(JASS::version::credits().c_str());

 	/*
 		Output basic statistics about the machine we're running on.
 	*/
 	std::cout << JASS::hardware_support();

	/*
		Test the JASS classes
	*/
	puts("PERFORM ALL UNIT TESTS");

	try
		{
		JASS::hardware_support hardware;

		puts("query_term_list");
		JASS::query_term_list::unittest();

		puts("instream_directory_iterator");
		JASS::instream_directory_iterator::unittest();

		puts("parser_unicoil_json::unittest");
		JASS::parser_unicoil_json::unittest();

		puts("instream_document_unicoil_json");
		JASS::instream_document_unicoil_json::unittest();

		puts("compress_integer_elias_gamma_simd_vb");
		JASS::compress_integer_elias_gamma_simd_vb::unittest();

		puts("beap");
		JASS::beap<int>::unittest();
		
		if (hardware.AVX2)
			{
			puts("simd");
			JASS::simd::unittest();

			puts("compress_integer_bitpack_256");
			JASS::compress_integer_bitpack_256::unittest();

			puts("compress_integer_elias_gamma_simd");
			JASS::compress_integer_elias_gamma_simd::unittest();

			puts("compress_integer_elias_delta_simd");
			JASS::compress_integer_elias_delta_simd::unittest();
			}
		else
			{
// LCOV_EXCL_START
			puts("simd");
			puts("Cannot test as no 256-bit SIMD instructions on this CPU");

			puts("compress_integer_bitpack_256");
			puts("Cannot test as no 256-bit SIMD instructions on this CPU");

			puts("compress_integer_elias_gamma_simd");
			puts("Cannot test as no 256-bit SIMD instructions on this CPU");
// LCOV_EXCL_STOP
			}

		if (hardware.BMI1)
			{
			puts("compress_integer_elias_gamma");
			JASS::compress_integer_elias_gamma::unittest();

			puts("compress_integer_elias_delta");
			JASS::compress_integer_elias_delta::unittest();
			}
		else
			{
// LCOV_EXCL_START
			puts("compress_integer_elias_gamma");
			puts("Cannot test as no BMI1 instructions on this CPU");

			puts("compress_integer_elias_delta");
			puts("Cannot test as no BMI1 instructions on this CPU");
// LCOV_EXCL_STOP
			}

		puts("accumulator_counter");
		JASS::accumulator_counter<uint32_t, 1, 8>::unittest();

		puts("accumulator_counter_interleaved");
		JASS::accumulator_counter_interleaved<uint32_t, 1, 8>::unittest();

		puts("stem_porter");
		JASS::stem_porter::unittest();

		puts("statistics");
		JASS::statistics::unittest();
		
		puts("bitstream");
		JASS::bitstream::unittest();

		puts("ranking_function_atire_bm25");
		JASS::ranking_function_atire_bm25::unittest();

		puts("ranking_function");
		JASS::ranking_function<JASS::ranking_function_atire_bm25>::unittest();

		puts("hardware_support");
		JASS::hardware_support::unittest();

		puts("threads");
		JASS::thread::unittest();
		
		puts("top_k_sort");
		JASS::top_k_qsort::unittest();

		puts("compress_integer_all");
		JASS::compress_integer_all::unittest();

		puts("timer");
		JASS::timer::unittest();

		puts("reverse");
		JASS::reverse_adapter<int>::unittest();

		puts("commandline");
		JASS::commandline::unittest();
		
		//	JASS::string is a typedef so nothing to test.
		
		puts("checksum");
		JASS::checksum::unittest();

		puts("file");
		JASS::file::unittest();

		puts("evaluate");
		JASS::evaluate::unittest();

		puts("evaluate_relevant_returned");
		JASS::evaluate_relevant_returned::unittest();

		puts("evaluate_mean_reciprocal_rank");
		JASS::evaluate_mean_reciprocal_rank::unittest();

		puts("evaluate_mean_reciprocal_rank4k");
		JASS::evaluate_mean_reciprocal_rank4k::unittest();

		puts("evaluate_expected_search_length");
		JASS::evaluate_expected_search_length::unittest();

		puts("evaluate_map");
		JASS::evaluate_map::unittest();

		puts("evaluate_precision");
		JASS::evaluate_precision::unittest();

		puts("evaluate_recall");
		JASS::evaluate_recall::unittest();

		puts("evaluate_f");
		JASS::evaluate_f::unittest();

		puts("evaluate_buying_power");
		JASS::evaluate_buying_power::unittest();

		puts("evaluate_buying_power4k");
		JASS::evaluate_buying_power4k::unittest();

		puts("evaluate_selling_power");
		JASS::evaluate_selling_power::unittest();

		puts("evaluate_cheapest_precision");
		JASS::evaluate_cheapest_precision::unittest();

		puts("evaluate_rank_biased_precision");
		JASS::evaluate_rank_biased_precision::unittest();

		puts("bitstring");
		JASS::bitstring::unittest();

		puts("unicode");
		JASS::unicode::unittest();
		
		puts("ascii");
		JASS::ascii::unittest();
		
		puts("maths");
		JASS::maths::unittest();
		
		// JASS::allocator does not have a unittest because it is a virtual base class
		
		puts("allocator_pool");
		JASS::allocator_pool::unittest();
		
		puts("allocator_memory");
		JASS::allocator_memory::unittest();

		puts("document");
		JASS::document::unittest();
		
		puts("allocator_cpp");
		JASS::allocator_cpp<size_t>::unittest();
		
		puts("slice");
		JASS::slice::unittest();
		
		// instream does not have a unittest because it is a virtual base class
		
		puts("instream_file");
		JASS::instream_file::unittest();

		puts("instream_file_star");
		JASS::instream_file_star::unittest();

		puts("instream_memory");
		JASS::instream_memory::unittest();

		puts("instream_document_trec");
		JASS::instream_document_trec::unittest();

		puts("instream_document_warc");
		JASS::instream_document_warc::unittest();

		puts("instream_document_fasta");
		JASS::instream_document_fasta::unittest();

		// JASS::channel does not have a unittest because it is a virtual base class

		puts("channel_buffer");
		JASS::channel_buffer::unittest();

		puts("channel_file");
		JASS::channel_file::unittest();

		puts("channel_trec");
		JASS::channel_trec::unittest();

		puts("parser");
		JASS::parser::unittest();

		puts("parser_fasta");
		JASS::parser_fasta::unittest();

		puts("query_term");
		JASS::query_term::unittest();
		
		puts("parser_query");
		JASS::parser_query::unittest();
		
		puts("hash_pearson");
		JASS::hash_pearson::unittest();

		puts("binary_tree");
		JASS::binary_tree<size_t, size_t>::unittest();
		
		puts("hash_table");
		JASS::hash_table<JASS::slice, JASS::slice>::unittest();

		puts("dynamic_array");
		JASS::dynamic_array<JASS::slice>::unittest();

		puts("index_postings");
		JASS::index_postings::unittest();

		puts("impact_postings_impact");
		JASS::index_postings_impact::unittest();

		puts("index_manager");
		JASS::index_manager::unittest();

		puts("index_manager_sequential");
		JASS::index_manager_sequential::unittest();

		puts("serialise_ci");
		JASS::serialise_ci::unittest();

		puts("serialise_jass_v1");
		JASS::serialise_jass_v1::unittest();

		puts("serialise_integers");
		JASS::serialise_integers::unittest();

		puts("serialise_forward_index");
		JASS::serialise_forward_index::unittest();

		puts("compress_integer_elias_gamma_bitwise");
		JASS::compress_integer_elias_gamma_bitwise::unittest();

		puts("compress_integer_elias_delta_bitwise");
		JASS::compress_integer_elias_delta_bitwise::unittest();

		puts("compress_integer_none");
		JASS::compress_integer_none::unittest();

		puts("compress_integer_variable_byte");
		JASS::compress_integer_variable_byte::unittest();

		puts("compress_integer_stream_vbyte");
		JASS::compress_integer_stream_vbyte::unittest();

		puts("compress_integer_qmx_original");
		JASS::compress_integer_qmx_original::unittest();

		puts("compress_integer_qmx_improved");
		JASS::compress_integer_qmx_improved::unittest();

		puts("compress_integer_qmx_jass_v1");
		JASS::compress_integer_qmx_jass_v1::unittest();

		puts("compress_integer_simple_9");
		JASS::compress_integer_simple_9::unittest();

		puts("compress_integer_simple_9_packed");
		JASS::compress_integer_simple_9_packed::unittest();

		puts("compress_integer_simple_16");
		JASS::compress_integer_simple_16::unittest();

		puts("compress_integer_simple_16_packed");
		JASS::compress_integer_simple_16_packed::unittest();

		puts("compress_integer_simple_8b");
		JASS::compress_integer_simple_8b::unittest();

		puts("compress_integer_simple_8b_packed");
		JASS::compress_integer_simple_8b_packed::unittest();

		puts("compress_integer_relative_10");
		JASS::compress_integer_relative_10::unittest();

		puts("compress_integer_carryover_12");
		JASS::compress_integer_carryover_12::unittest();

		puts("compress_integer_carry_8b");
		JASS::compress_integer_carry_8b::unittest();

		puts("compress_integer_bitpack_32_reduced");
		JASS::compress_integer_bitpack_32_reduced::unittest();

		puts("compress_integer_bitpack_64");
		JASS::compress_integer_bitpack_64::unittest();

		puts("compress_integer_bitpack_128");
		JASS::compress_integer_bitpack_128::unittest();

		puts("accumulator_2d");
		JASS::accumulator_2d<uint32_t, 1>::unittest();

		puts("pointer_box");
		JASS::pointer_box<int>::unittest();

		puts("heap");
		JASS::heap<int>::unittest();

		puts("top_k_heap");
		JASS::top_k_heap<int>::unittest();

		puts("query_heap");
		JASS::query_heap::unittest();

		puts("query_maxblock");
		JASS::query_maxblock::unittest();

		puts("query_maxblock_heap");
		JASS::query_maxblock_heap::unittest();

		puts("query_bucket");
		JASS::query_bucket::unittest();

		puts("run_export_trec");
		JASS::run_export_trec::unittest();

		puts("run_export");
		JASS::run_export::unittest();

		puts("quantize");
		JASS::quantize<JASS::ranking_function_atire_bm25>::unittest();

		puts("compress_general_zlib");
		JASS::compress_general_zlib::unittest();

		puts("ALL UNIT TESTS HAVE PASSED");
		failed = false;
		}
	// LCOV_EXCL_START
	catch (std::bad_array_new_length &error)
		{
		printf("CAUGHT AN EXCEPTION OF TYPE std::bad_array_new_length (%s)\n", error.what());
		}
	catch (std::exception &error)
		{
		printf("CAUGHT AN EXCEPTION OF TYPE std::exception (%s)\n", error.what());
		}
	catch (...)
		{
		printf("CAUGHT AN EXCEPTION OF UNKNOEN TYPE)\n");
		}
	// LCOV_EXCL_STOP

	return failed;
	}
