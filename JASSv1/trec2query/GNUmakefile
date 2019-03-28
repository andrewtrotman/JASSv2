CC := g++
ATIRE_DIR := ../../atire
ATIRE_OBJECTS = $(wildcard $(ATIRE_DIR)/obj/*.o)

MAINS := atire atire_broker atire_client atire_dictionary atire_doclist atire_merge index
MAINS := $(MAINS:%=$(ATIRE_DIR)/obj/%.o)

ATIRE_OBJ = $(filter-out $(MAINS),$(ATIRE_OBJECTS))

all : trec2query

trec2query : trec2query.o
	$(CC) trec2query.o ${ATIRE_OBJ} ${ATIRE_DIR}/external/unencumbered/snowball/libstemmer.a ${ATIRE_DIR}/external/gpl/lzo/liblzo2.a ${ATIRE_DIR}/external/unencumbered/zlib/libz.a ${ATIRE_DIR}/external/unencumbered/bzip/libbz2.a ${ATIRE_DIR}/external/unencumbered/snappy/libsnappy.a -ldl -lpthread -o trec2query

%.o : %.c
	$(CC) -I${ATIRE_DIR} -g -x c++ -c -lpthread $<

clean :
	\rm -f *.o $(TARGETS)
