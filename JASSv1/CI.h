/*
	CI.H
	----
*/
#ifndef CI_H_
#define CI_H_

#include <stdint.h>
#include <string.h>
#include "heap.h"

#if defined (__APPLE_) || defined (__GNUC__)
	#define __forceinline __inline__ __attribute__((always_inline))
#endif

class CI_vocab;

/*
	struct ADD_RSV_COMPARE
	----------------------
	compares on value and if they are the same then compares on address - i.e. tie break on docid
*/
struct add_rsv_compare
{
__forceinline int operator() (uint16_t *a, uint16_t *b) const { return *a > *b ? 1 : *a < *b ? -1 : (a > b ? 1 : a < b ? -1 : 0); }
};


extern uint16_t *CI_accumulators;				// the accumulators
extern uint16_t **CI_accumulator_pointers;	// an array of pointers into the accumulators (used to avoid computing docIDs)
extern uint32_t CI_top_k;							// the number of results to find (top-k)
extern uint32_t CI_results_list_length;		// the number of results we found (at most top-k)

extern uint8_t *CI_accumulator_clean_flags;	// is the "row" of the accumulator table
extern uint32_t CI_accumulators_shift;			// number of bits to shift (right) the docid by to get the CI_accumulator_clean_flags
extern uint32_t CI_accumulators_width;			// the "width" of the accumulator table
extern uint32_t CI_accumulators_height;		// the "height" of the accumulator table
extern ANT_heap<uint16_t *, add_rsv_compare> *CI_heap;

/*
	struct CI_IMPACT_METHOD
	-----------------------
*/
struct CI_impact_method
{
uint16_t impact;
void (*method)(void);
} ;

/*
	class CI_VOCAB
	--------------
*/
class CI_vocab
{
public:
	const char *term;
	void **methods;				// should be, but to make it compile faster it isn't : struct CI_impact_method **methods;
	uint64_t impacts;

public:
	static int compare(const void *a, const void *b) { return strcmp(((CI_vocab*)a)->term, ((CI_vocab*)b)->term);}
	static int compare_string(const void *a, const void *b) { return strcmp((char *)a, ((CI_vocab*)b)->term);}
};

/*
	class CI_VOCAB_HEAP
	-------------------
*/
class CI_vocab_heap
{
public:
	const char *term;
	uint64_t offset;
	uint64_t impacts;

public:
	static int compare(const void *a, const void *b) { return strcmp(((CI_vocab_heap*)a)->term, ((CI_vocab_heap*)b)->term);}
	static int compare_string(const void *a, const void *b) { return strcmp((char *)a, ((CI_vocab_heap*)b)->term);}
} ;

void top_k_qsort(uint16_t **a, long long n, long long top_k);

extern uint32_t CI_unique_terms;					// number of terms in the vocab
extern uint32_t CI_unique_documents;			// number of documents in the collection
extern const char *CI_doclist[];					// the list of document IDs (TREC document IDs)

/*
	ADD_RSV()
	---------
	This method cannot be forced inline because when I do so clang generates code that (sometimes) doesn't work!
*/
#ifdef CI_FORCEINLINE
	__forceinline void add_rsv(uint32_t docid, uint16_t score)
#else
	static void add_rsv(uint32_t docid, uint16_t score)
#endif
{
uint16_t old_value;
uint16_t *which = CI_accumulators + docid;
add_rsv_compare cmp;

/*
	Make sure the accumulator exists
*/
if (CI_accumulator_clean_flags[docid >> CI_accumulators_shift] == 0)
	{
	CI_accumulator_clean_flags[docid >> CI_accumulators_shift] = 1;
	memset(CI_accumulators + (CI_accumulators_width * (docid >> CI_accumulators_shift)), 0, CI_accumulators_width * sizeof(uint16_t));
	}

/*
	CI_top_k search so we maintain a heap
*/
if (CI_results_list_length < CI_top_k)
	{
	/*
		We haven't got enough to worry about the heap yet, so just plonk it in
	*/
	old_value = *which;
	*which += score;

	if (old_value == 0)
		CI_accumulator_pointers[CI_results_list_length++] = which;

	if (CI_results_list_length == CI_top_k)
		CI_heap->build_min_heap();
	}
else if (cmp(which, CI_accumulator_pointers[0]) >= 0)
	{
	/*
		We were already in the heap, so update
	*/
	*which +=score;
	CI_heap->min_update(which);
	}
else
	{
	/*
		We weren't in the heap, but we could get put there
	*/
	*which += score;
	if (cmp(which, CI_accumulator_pointers[0]) > 0)
		CI_heap->min_insert(which);
	}
}

#endif
