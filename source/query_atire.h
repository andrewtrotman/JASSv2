
#incliude "query.h"

namespace JASS
	{
	class query_atire : public query
		{
		public:
			struct add_rsv_compare
			{
			__forceinline int operator() (ACCUMULATOR_TYPE *a, ACCUMULATOR_TYPE *b) const { return *a > *b ? 1 : *a < *b ? -1 : (a > b ? 1 : a < b ? -1 : 0); }
			};

		private:
			ANT_heap<ACCUMULATOR_TYPE *, add_rsv_compare> heap;
			ACCUMULATOR_TYPE *accumulators;
			ACCUMULATOR_TYPE **accumulator_pointers;

		public:
			query_atire(const std::vector<std::string> &primary_keys, size_t documents = 1024, size_t top_k = 10) :\
				query(primary_keys, documents, top_k),
				accumulators(new ACCUMULATOR_TYPE[documents]),
				accumulator_pointers(new ACCUMULATOR_TYPE*[top_k]
				{
				/* Nothing */
				}
			void add_rsv(size_t docid, ACCUMULATOR_TYPE score)
				{
				ACCUMULATOR_TYPE old_value;
				ACCUMULATOR_TYPE *which = accumulators + docid;
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

		};
	}
