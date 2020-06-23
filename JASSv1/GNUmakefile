ATIRE_DIR = ../../atire

ATIRE_OBJ = \
	$(ATIRE_DIR)/obj/stats.o			\
	$(ATIRE_DIR)/obj/ctypes.o					\
	$(ATIRE_DIR)/obj/compression_factory.o		\
	$(ATIRE_DIR)/obj/memory.o					\
	$(ATIRE_DIR)/obj/search_engine.o			\
	$(ATIRE_DIR)/obj/btree_iterator.o			\
	$(ATIRE_DIR)/obj/bitstring.o				\
	$(ATIRE_DIR)/obj/file.o						\
	$(ATIRE_DIR)/obj/file_internals.o			\
	$(ATIRE_DIR)/obj/critical_section.o			\
	$(ATIRE_DIR)/obj/file_memory.o				\
	$(ATIRE_DIR)/obj/search_engine_result_id_iterator.o				\
	$(ATIRE_DIR)/obj/search_engine_result.o		\
	$(ATIRE_DIR)/obj/stats_search_engine.o		\
	$(ATIRE_DIR)/obj/stats_time.o				\
	$(ATIRE_DIR)/obj/stem_s.o					\
	$(ATIRE_DIR)/obj/version.o					\
	$(ATIRE_DIR)/obj/stemmer_factory.o			\
	$(ATIRE_DIR)/obj/stemmer.o					\
	$(ATIRE_DIR)/obj/stemmer_term_similarity_weighted.o					\
	$(ATIRE_DIR)/obj/stemmer_term_similarity_threshold.o					\
	$(ATIRE_DIR)/obj/stemmer_term_similarity.o					\
	$(ATIRE_DIR)/obj/stem_paice_husk.o			\
	$(ATIRE_DIR)/obj/stem_snowball.o			\
	$(ATIRE_DIR)/obj/stem_krovetz.o				\
	$(ATIRE_DIR)/obj/stem_porter.o				\
	$(ATIRE_DIR)/obj/stem_otago.o				\
	$(ATIRE_DIR)/obj/stem_otago_v2.o			\
	$(ATIRE_DIR)/obj/compress_carryover12.o		\
	$(ATIRE_DIR)/obj/compress_relative10.o		\
	$(ATIRE_DIR)/obj/compress_simple8b_packed.o			\
	$(ATIRE_DIR)/obj/compress_simple9.o			\
	$(ATIRE_DIR)/obj/compress_simple9_packed.o			\
	$(ATIRE_DIR)/obj/compress_simple16.o		\
	$(ATIRE_DIR)/obj/compress_simple16_packed.o		\
	$(ATIRE_DIR)/obj/compress_four_integer_variable_byte.o			\
	$(ATIRE_DIR)/obj/compress_elias_gamma.o		\
	$(ATIRE_DIR)/obj/compress_golomb.o			\
	$(ATIRE_DIR)/obj/compress_elias_delta.o		\
	$(ATIRE_DIR)/obj/compress_sigma.o			\
	$(ATIRE_DIR)/obj/compress_none.o			\
	$(ATIRE_DIR)/obj/bitstream.o				\
	$(ATIRE_DIR)/obj/search_engine_accumulator.o	\
	$(ATIRE_DIR)/obj/compression_text_factory.o		\
	$(ATIRE_DIR)/obj/compress_text_snappy.o		\
	$(ATIRE_DIR)/obj/compress_text_none.o		\
	$(ATIRE_DIR)/obj/compress_text_deflate.o		\
	$(ATIRE_DIR)/obj/compress_text_bz2.o

ATIRE_LIBS = \
	$(ATIRE_DIR)/external/unencumbered/snappy/libsnappy.a							\
	$(ATIRE_DIR)/external/unencumbered/zlib/libz.a 					\
	$(ATIRE_DIR)/external/unencumbered/bzip/libbz2.a 				\
	$(ATIRE_DIR)/external/unencumbered/snappy/libsnappy.a 							\
	$(ATIRE_DIR)/external/unencumbered/snowball/libstemmer.a 		\
	$(ATIRE_DIR)/external/gpl/lzo/liblzo2.a

MINUS_D = -DHASHER=1 -DHEADER_HASHER=1
MINUS_D += -DSPECIAL_COMPRESSION=1
MINUS_D += -DTWO_D_ACCUMULATORS
MINUS_D += -DTOP_K_READ_AND_DECOMPRESSOR
MINUS_D += -DPARALLEL_INDEXING
MINUS_D += -DPARALLEL_INDEXING_DOCUMENTS
MINUS_D += -DANT_ACCUMULATOR_T="double"
MINUS_D += -DANT_PREGEN_T="unsigned long long"
MINUS_D += -DNOMINMAX
MINUS_D += -DIMPACT_HEADER
MINUS_D += -DFILENAME_INDEX

CI_FLAGS = -x c++ -DCI_FORCEINLINE -msse4 -std=c++11 -O3 -I$(ATIRE_DIR)/source $(MINUS_D)

all : atire_to_jass_index jass

jass : jass.c CI.c compress_qmx.c maths.c compress_qmx_d4.c process_postings.c compress_integer_elias_delta_simd.c compress_integer_elias_gamma_simd.c
	g++ -march=native $(CI_FLAGS) jass.c CI.c compress_simple8b.c compress_qmx.c compress_qmx_d4.c maths.c process_postings.c compress_integer_elias_gamma_simd.c compress_integer_elias_delta_simd.c compress_integer_elias_gamma_simd_vb.cpp compress_integer_variable_byte.cpp -o jass

atire_to_jass_index : atire_to_jass_index.c compress_integer_elias_delta_simd.c compress_integer_elias_gamma_simd.c
	g++ -march=native $(ATIRE_OBJ) $(ATIRE_LIBS) $(CI_FLAGS) compress_integer_elias_delta_simd.c compress_integer_elias_gamma_simd.c atire_to_jass_index.c compress_variable_byte.c compress_simple8b.c compress_qmx.c compress_qmx_d4.c maths.c compress_integer_elias_gamma_simd_vb.cpp compress_integer_variable_byte.cpp -o atire_to_jass_index
	

clean:
	-rm atire_to_jass_index jass *.o CIvocab.c CIpostings.h CIpostings.c CIdoclist.c CIvocab_heap.c CIpostings.bin CIdoclist.bin CIvocab.bin CIvocab_terms.bin
