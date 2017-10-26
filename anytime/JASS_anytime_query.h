
#include <stdint.h>

#include <atomic>

class JASS_anytime_query
	{
	public:
		std::atomic<uint8_t> taken;
		std::string query;

	public:
		JASS_anytime_query() = delete;

		JASS_anytime_query(const std::string &query) :
			taken(0),
			query(query)
				{
				/* Nothing */
				}

		JASS_anytime_query(JASS_anytime_query &&original) :
			taken(original.taken.load()),
			query(original.query)
				{
				/* Nothing */
				}

		static std::string get_next_query(std::vector<JASS_anytime_query>&list, size_t &starting_from)
			{
			auto total_queries = list.size();
			while (starting_from < total_queries)
				{
				while (!list[starting_from].taken)					/// remember that compare_exchange_strong
					{
					uint8_t expected = 0;
					if (list[starting_from].taken.compare_exchange_strong(expected, 1))
						return list[starting_from].query;
					}
				starting_from++;
				}

			return std::string();
			}
	};
