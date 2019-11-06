/*
	UNITTEST.CPP
	------------
	Copyright (c) 2016-2019 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include "file.h"
#include "heap.h"
#include "ascii.h"
#include "maths.h"
#include "query.h"
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
#include "decode_d0.h"
#include "decode_d1.h"
#include "bitstring.h"
#include "hash_table.h"
#include "run_export.h"
#include "top_k_heap.h"
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
#include "dynamic_array.h"
#include "allocator_cpp.h"
#include "instream_file.h"
#include "index_manager.h"
#include "allocator_pool.h"
#include "index_postings.h"
#include "accumulator_2d.h"
#include "instream_memory.h"
#include "run_export_trec.h"
#include "hardware_support.h"
#include "allocator_memory.h"
#include "ranking_function.h"
#include "serialise_jass_v1.h"
#include "serialise_integers.h"
#include "evaluate_precision.h"
#include "instream_file_star.h"
#include "compress_integer_all.h"
#include "evaluate_buying_power.h"
#include "compress_integer_none.h"
#include "index_postings_impact.h"
#include "compress_general_zlib.h"
#include "instream_document_trec.h"
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
#include "compress_integer_elias_gamma.h"
#include "compress_integer_elias_delta.h"
#include "compress_integer_bitpack_128.h"
#include "compress_integer_bitpack_256.h"
#include "compress_integer_relative_10.h"
#include "compress_integer_qmx_jass_v1.h"
#include "compress_integer_stream_vbyte.h"
#include "compress_integer_qmx_original.h"
#include "compress_integer_qmx_improved.h"
#include "compress_integer_carryover_12.h"
#include "compress_integer_variable_byte.h"
#include "compress_integer_simple_9_packed.h"
#include "compress_integer_elias_delta_simd.h"
#include "compress_integer_simple_8b_packed.h"
#include "compress_integer_simple_16_packed.h"
#include "compress_integer_elias_gamma_simd.h"
#include "compress_integer_bitpack_32_reduced.h"
#include "compress_integer_elias_gamma_bitwise.h"
#include "compress_integer_elias_delta_bitwise.h"
#include "compress_integer_gather_elias_gamma_simd.h"
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
		Test the JASS classes
	*/
	puts("PERFORM ALL UNIT TESTS");

	try
		{
		JASS::hardware_support hardware;
		if (hardware.AVX2)
			{
			puts("compress_integer_bitpack_256");
			JASS::compress_integer_bitpack_256::unittest();

			puts("compress_integer_elias_gamma_simd");
			JASS::compress_integer_elias_gamma_simd::unittest();

			puts("compress_integer_gather_elias_gamma_simd");
			JASS::compress_integer_gather_elias_gamma_simd::unittest();

			puts("compress_integer_elias_delta_simd");
			JASS::compress_integer_elias_delta_simd::unittest();
			}
		else
			{
	// LCOV_EXCL_START
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


		puts("bitstream");
		JASS::bitstream::unittest();

		puts("compress_integer_elias_gamma_bitwise");
		JASS::compress_integer_elias_gamma_bitwise::unittest();

		puts("compress_integer_elias_delta_bitwise");
		JASS::compress_integer_elias_delta_bitwise::unittest();

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

		puts("evaluate_map");
		JASS::evaluate_map::unittest();

		puts("evaluate_precision");
		JASS::evaluate_precision::unittest();

		puts("evaluate_buying_power");
		JASS::evaluate_buying_power::unittest();

		puts("evaluate_buying_power4k");
		JASS::evaluate_buying_power4k::unittest();

		puts("evaluate_selling_power");
		JASS::evaluate_selling_power::unittest();

		puts("evaluate_cheapest_precision");
		JASS::evaluate_cheapest_precision::unittest();

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

		puts("instream_document_fasta");
		JASS::instream_document_fasta::unittest();

		// JASS::channel does not have a unittest because it is a virtual base class

		puts("channel_file");
		JASS::channel_file::unittest();

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
		JASS::heap<int, int>::unittest();

		puts("top_k_heap");
		JASS::top_k_heap<int>::unittest();

		puts("query");
		JASS::query<uint16_t, 1, 1>::unittest();

		puts("decode_d0");
		JASS::decoder_d0::unittest();

		puts("decode_d1");
		JASS::decoder_d1::unittest();

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
