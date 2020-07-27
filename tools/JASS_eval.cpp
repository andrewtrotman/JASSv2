/*
	JASS_EVAL.CPP
	-------------
	Copyright (c) 2019 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <math.h>
#include <string.h>

#include <map>
#include <string>
#include <iomanip>
#include <iostream>

#include "file.h"
#include "maths.h"
#include "asserts.h"
#include "statistics.h"
#include "evaluate_f.h"
#include "commandline.h"
#include "evaluate_map.h"
#include "evaluate_recall.h"
#include "evaluate_precision.h"
#include "evaluate_buying_power.h"
#include "evaluate_selling_power.h"
#include "evaluate_buying_power4k.h"
#include "evaluate_cheapest_precision.h"
#include "evaluate_mean_reciprocal_rank.h"
#include "evaluate_rank_biased_precision.h"
#include "evaluate_mean_reciprocal_rank4k.h"
#include "evaluate_expected_search_length.h"

#ifdef _MSC_VER
	#define strtok_r strtok_s
#endif

/*
	PARAMETERS
	----------
*/
std::string parameter_assessments_filename;							///< Name of file assessment file
std::string parameter_run_filenames;									///< Name of the run file to evaluate
bool parameter_help = false;												///< Output the usage() help
bool parameter_output_per_query_scores = false;						///< Shopuld we output per-query scores or not?
size_t parameter_depth = (std::numeric_limits<size_t>::max)();	///< How far down the results list to look (i.e. n in precision@n)
std::string parameters_errors;											///< Any errors as a result of command line parsing
size_t parameter_k = 10;													///< The n parameter to a metric (for example k in buying_power_at_k).
bool parameter_ttest = false;												///< Conduct t-tests over multiple runs.
bool parameter_spearman = false;											///< Conduct Spearman's Correlation over multiple metrics (requires multiple runs).
bool parameter_zeroless_averaging = false;							///< if true then conduct averaging over the number of queries in the run. If false the the numner of queries in the assessments.
double parameter_probability = 0.95;										///< used in metrics like rbp.

auto parameters = std::make_tuple										///< The  command line parameter block
	(
	JASS::commandline::parameter("-?", "--help",               "Print this help.", parameter_help),
	JASS::commandline::parameter("-a", "--assesmentfile",      "<filename> Name of the file containing the assessments", parameter_assessments_filename),
	JASS::commandline::parameter("-k", "--k_equals",           "<n> the K parameter in any parmetric metric (e.g. k in buying_power4k) [default=10]", parameter_k),
	JASS::commandline::parameter("-n", "--n_equals",           "<n> How far down the resuls list to look (i.e. n in P@n) [default=inf]", parameter_depth),
	JASS::commandline::parameter("-p", "--perquery",           "Output per-query statistics", parameter_output_per_query_scores),
	JASS::commandline::parameter("-P", "--probability",        "Probability of looking to the next result in metric such as RBP [default=0.95]", parameter_probability),
	JASS::commandline::parameter("-r", "--runfile",            "<filename[,filename[,...]]> Names of run files to evaluate", parameter_run_filenames),
	JASS::commandline::parameter("-s", "--spearman",           "Conduct Spearman's Correlation over multple metrics (required multiple runs)", parameter_spearman),
	JASS::commandline::parameter("-t", "--ttest",              "Conduct t-test (required multiple runs)", parameter_ttest),
	JASS::commandline::parameter("-Z", "--Zeroless-averaging", "Average over the number of queries in the run [default: number of queries in assessments]", parameter_zeroless_averaging)
	);

enum metric_index
	{
	NUMBER_OF_QUERIES,						///< the number of queries that this object represets (the numner of += ops called).
	RELEVANT_COUNT,							///< the number of relevant assessments for this query (assessments with a non-zero score).
	RETURNED,									///< the number of RESULTS IN the rsults list.
	RELEVANT_RETURNED,						///< the number of relevant results in the results list.
	METRICS_SENTINAL_ORDINALS,	// ALL INTEGER NON-AVERAGES METRICS ARE ABOVE THIS, REAL AND AVERAGED METRICS ARE BELOW.  DO NOT MOVE
	MEAN_RECIPROCAL_RANK,					///< the mean reciprocal rank.
	MEAN_RECIPROCAL_RANK_4K,				///< the mean reciprocal rank for k items.
	PRECISION,									///< set wise precision OF THE results list.
	RECALL,										///< set wise recall.
	ESL,											///< Expected search length.
	F1,											///< set wise f1 score.
	P_AT_5,										///< set wise precision of the results list.
	P_AT_10,										///< set wise precision of the results list.
	P_AT_15,										///< set wise precision of the results list.
	P_AT_20,										///< set wise precision of the results list.
	P_AT_30,										///< set wise precision of the results list.
	P_AT_100,									///< set wise precision of the results list.
	P_AT_200,									///< set wise precision of the results list.
	P_AT_500,									///< set wise precision of the results list.
	P_AT_1000,									///< set wise precision of the results list.
	R_AT_5,										///< set wise recall of the results list.
	R_AT_10,										///< set wise recall of the results list.
	R_AT_15,										///< set wise recall of the results list.
	R_AT_20,										///< set wise recall of the results list.
	R_AT_30,										///< set wise recall of the results list.
	R_AT_100,									///< set wise recall of the results list.
	R_AT_200,									///< set wise recall of the results list.
	R_AT_500,									///< set wise recall of the results list.
	R_AT_1000,									///< set wise recall of the results list.
	F1_AT_5,										///< set wise f1 of the results list.
	F1_AT_10,									///< set wise f1 of the results list.
	F1_AT_15,									///< set wise f1 of the results list.
	F1_AT_20,									///< set wise f1 of the results list.
	F1_AT_30,									///< set wise f1 of the results list.
	F1_AT_100,									///< set wise f1 of the results list.
	F1_AT_200,									///< set wise f1 of the results list.
	F1_AT_500,									///< set wise f1 of the results list.
	F1_AT_1000,									///< set wise f1 of the results list.
	MEAN_AVERAGE_PRECISION,					///< mean average precision (MAP).
	RANK_BIASED_PRECISION,					///< Rank Biased Precision (RBP).
	GEOMETRIC_MEAN_AVERAGE_PRECISION,	///< geometric_mean average precision (GMAP).
	CHEAPEST_PRECISION,						///< set wise precision of the cheapest items (if we are an eCommerce metric).
	SELLING_POWER,								///< selling power (if we are an eCommerce metric).
	BUYING_POWER,								///< buying power (if we are an eCommerce metric).
	BUYING_POWER4K,							///< buying power for K results (if we are an eCommerce metric).
	METRICS_SENTINAL 		// MUST BE LAST. DO NOT MOVE
	};

const std::vector<std::string> metric_name =
	{
	"Number of Queries",
	"Number Relevant",
	"Number of Results Returned",
	"Number Relevant Returned",
	"SENTINAL",
	"Mean Reciprocal Rank (MRR)",
	"Mean Reciprocal Rank for k(MRR4K)",
	"Precision (P)",
	"Recall (R)",
	"Expected Search Length (ESL)",
	"F1 (F1)",
	"Precision at 5 (P@5)",
	"Precision at 10 (P@10)",
	"Precision at 15 (P@15)",
	"Precision at 20 (P@20)",
	"Precision at 30 (P@30)",
	"Precision at 100 (P@100)",
	"Precision at 200 (P@200)",
	"Precision at 500 (P@500)",
	"Precision at 1000 (P@1000)",
	"Recall at 5 (R@5)",
	"Recall at 10 (R@10)",
	"Recall at 15 (R@15)",
	"Recall at 20 (R@20)",
	"Recall at 30 (R@30)",
	"Recall at 100 (R@100)",
	"Recall at 200 (R@200)",
	"Recall at 500 (R@500)",
	"Recall at 1000 (R@1000)",
	"F1 at 5 (F1@5)",
	"F1 at 10 (F1@10)",
	"F1 at 15 (F1@15)",
	"F1 at 20 (F1@20)",
	"F1 at 30 (F1@30)",
	"F1 at 100 (F1@100)",
	"F1 at 200 (F1@200)",
	"F1 at 500 (F1@500)",
	"F1 at 1000 (F1@1000)",
	"Mean Average Precision (MAP)",
	"Rank Biased Precision (RBP)",
	"Geometric MAP (GMAP)",
	"Cheapest Precision (PC)",
	"Selling Power (SP)",
	"Buying Power (BP)",
	"Buying Power for k (BP4K)",
	"SENTINAL"
	};

/*
	CLASS METRIC_SET
	----------------
*/
/*!
	@brief Object to store all the metrics for a given query
*/
class metric_set
	{
	public:
		std::string run_id;								///< the ID of the run (comes from the first result in the file)
		std::string query_id;							///< the ID of the query this result set represents
		double metric[METRICS_SENTINAL];				///< the set of metrics, indexes using the enum

	public:
		/*
			METRIC_SET::METRIC_SET()
			------------------------
		*/
		/*!
			@brief Constructor
		*/
		metric_set() = delete;

		/*
			METRIC_SET::METRIC_SET()
			------------------------
		*/
		/*!
			@brief Constructor
			@param query_id [in] The id of the query that these results represent.
			@param relevant_count [in] The number of relevant assessments.
			@param returned [in] The length of the results list.
			@param relevant_returned [in] The number of relevant documents in the results list.
			@param mean_reciprocal_rank [in] The mean reciprocal rank (MRR)
			@param mean_reciprocal_rank4k [in] The mean reciprocal rank (MRR)
			@param precision [in] The precision of this query.
			@param recall [in] Setwise recall.
			@param expected_search_length [in] Cooper's expected search length (ESL)
			@param f1 [in] Setwise f1.
			@param p_at_5 [in] Setwise precision at n
			@param p_at_10 [in] Setwise precision at n
			@param p_at_15 [in] Setwise precision at n
			@param p_at_20 [in] Setwise precision at n
			@param p_at_30 [in] Setwise precision at n
			@param p_at_100 [in] Setwise precision at n
			@param p_at_200 [in] Setwise precision at n
			@param p_at_500 [in] Setwise precision at n
			@param p_at_1000 [in] Setwise precision at n
			@param r_at_5 [in] Setwise recall at n
			@param r_at_10 [in] Setwise recall at n
			@param r_at_15 [in] Setwise recall at n
			@param r_at_20 [in] Setwise recall at n
			@param r_at_30 [in] Setwise recall at n
			@param r_at_100 [in] Setwise recall at n
			@param r_at_200 [in] Setwise recall at n
			@param r_at_500 [in] Setwise recall at n
			@param r_at_1000 [in] Setwise recall at n
			@param f1_at_5 [in] Setwise f1 at n
			@param f1_at_10 [in] Setwise f1 at n
			@param f1_at_15 [in] Setwise f1 at n
			@param f1_at_20 [in] Setwise f1 at n
			@param f1_at_30 [in] Setwise f1 at n
			@param f1_at_100 [in] Setwise f1 at n
			@param f1_at_200 [in] Setwise f1 at n
			@param f1_at_500 [in] Setwise f1 at n
			@param f1_at_1000 [in] Setwise f1 at n
			@param cheapest_precision [in] The cheapest precision of this query.
			@param selling_power [in] The selling power of this query.
			@param buying_power [in] The buying power of this query.
			@param buying_power4k [in] The buying power for k relevant of this query.
		*/
		metric_set(const std::string &run_id, const std::string &query_id, size_t relevant_count, size_t returned, size_t relevant_returned, double mean_reciprocal_rank, double mean_reciprocal_rank_4k, double precision, double recall, double expected_search_length, double f1,
			double p_at_5, double p_at_10, double p_at_15, double p_at_20, double p_at_30, double p_at_100, double p_at_200, double p_at_500, double p_at_1000,
			double r_at_5, double r_at_10, double r_at_15, double r_at_20, double r_at_30, double r_at_100, double r_at_200, double r_at_500, double r_at_1000,
			double f1_at_5, double f1_at_10, double f1_at_15, double f1_at_20, double f1_at_30, double f1_at_100, double f1_at_200, double f1_at_500, double f1_at_1000,
			double mean_average_precision, double rank_biased_precision, double cheapest_precision, double selling_power, double buying_power, double buying_power4k) :
			run_id(run_id),
			query_id(query_id)
			{
			metric[NUMBER_OF_QUERIES] = 1;
			metric[RELEVANT_COUNT] = relevant_count;
			metric[RETURNED] = returned;
			metric[RELEVANT_RETURNED] = relevant_returned;
			metric[MEAN_RECIPROCAL_RANK] = mean_reciprocal_rank;
			metric[MEAN_RECIPROCAL_RANK_4K] = mean_reciprocal_rank_4k;
			metric[PRECISION] = precision;
			metric[RECALL] = recall;
			metric[ESL] = expected_search_length;
			metric[F1] = f1;
			metric[P_AT_5] = p_at_5;
			metric[P_AT_10] = p_at_10;
			metric[P_AT_15] = p_at_15;
			metric[P_AT_20] = p_at_20;
			metric[P_AT_30] = p_at_30;
			metric[P_AT_100] = p_at_100;
			metric[P_AT_200] = p_at_200;
			metric[P_AT_500] = p_at_500;
			metric[P_AT_1000] = p_at_1000;
			metric[R_AT_5] = r_at_5;
			metric[R_AT_10] = r_at_10;
			metric[R_AT_15] = r_at_15;
			metric[R_AT_20] = r_at_20;
			metric[R_AT_30] = r_at_30;
			metric[R_AT_100] = r_at_100;
			metric[R_AT_200] = r_at_200;
			metric[R_AT_500] = r_at_500;
			metric[R_AT_1000] = r_at_1000;
			metric[F1_AT_5] = f1_at_5;
			metric[F1_AT_10] = f1_at_10;
			metric[F1_AT_15] = f1_at_15;
			metric[F1_AT_20] = f1_at_20;
			metric[F1_AT_30] = f1_at_30;
			metric[F1_AT_100] = f1_at_100;
			metric[F1_AT_200] = f1_at_200;
			metric[F1_AT_500] = f1_at_500;
			metric[F1_AT_1000] = f1_at_1000;
			metric[MEAN_AVERAGE_PRECISION] = mean_average_precision;
			metric[RANK_BIASED_PRECISION] = rank_biased_precision;
			metric[GEOMETRIC_MEAN_AVERAGE_PRECISION] = mean_average_precision == 0 ? 0 : log(mean_average_precision);
			metric[CHEAPEST_PRECISION] = cheapest_precision;
			metric[SELLING_POWER] = selling_power;
			metric[BUYING_POWER] = buying_power;
			metric[BUYING_POWER4K] = buying_power4k;
			}

		/*
			METRIC_SET::METRIC_SET()
			------------------------
		*/
		/*!
			@brief Constructor
		*/
		metric_set(const std::string &run_id, const std::string &query_id) :
			run_id(run_id),
			query_id(query_id)
			{
			memset(metric, 0, sizeof(metric));
			}

		 /*
			 METRIC_SET::OPERATOR+=()
			 ------------------------
		 */
		/*!
			@brief add the results of one query to the results of another (used for cumulative totals)
			@param other [in] the new data being added
		*/
		metric_set &operator+=(const metric_set &other)
			{
			/*
				Make sure we are only adding the results from one other query, otherwise the results for GMAP will be wrong.
			*/
			JASS_assert(other.metric[NUMBER_OF_QUERIES] <= 1);

			for (size_t which = 0; which < METRICS_SENTINAL; which++)
				metric[which] += other.metric[which];

			return *this;
			}

	/*
		METRIC_SET::BAKE()
		------------------
	*/
	/*!
		@brief Finalise any metrics by, for example, averaging over the number of queries
	*/
	void bake(size_t queres_in_assessments)
		{
		size_t number_of_queries = queres_in_assessments;

		/*
			save the number of queries to put back later
		*/
		if (parameter_zeroless_averaging)
			number_of_queries = metric[NUMBER_OF_QUERIES];

		/*
			do the averaging
		*/
		for (size_t which = METRICS_SENTINAL_ORDINALS + 1; which < METRICS_SENTINAL; which++)
			metric[which] /= number_of_queries;

		/*
			GMAP is a geometric mean not a "regular" mean.
		*/
		metric[GEOMETRIC_MEAN_AVERAGE_PRECISION] = exp(metric[GEOMETRIC_MEAN_AVERAGE_PRECISION] );

		/*
			put the number of queries back
		*/
		if (parameter_zeroless_averaging)
			metric[NUMBER_OF_QUERIES] = number_of_queries;
		}

	/*
		METRIC_SET::T_TEST()
		--------------------
	*/
	/*!
		@brief Compute the one tailed paired t-test between each run and for one metric.
		@details Only computed over the queries (topics) that are in both runs.  Topics in only one run are ignored.
		@param metric [in] Which metric (must be a member of enum metric_index).
		@param run_set [in] The set of runs.
	*/
	static void t_test(metric_index metric, std::map<std::string, std::map<std::string, metric_set>> &run_set)
		{
		/*
			Output the metric name
		*/
		std::cout << "t-test (1-tail, paired) matrix of p values for " << metric_name[metric] << '\n';
		/*
			Get the width of the widest column (the length of the longest run name
		*/
		size_t width = 0;
		for (const auto &[first_name, first_set] : run_set)
			{
			(void)first_name;		// unused, supress warning.
			width = JASS::maths::maximum(width, first_set.begin()->second.run_id.size());
			}

		width = JASS::maths::maximum(width + 1, static_cast<size_t>(7));			// account for a space and account for the size of a p value.
		std::cout.width(width);

		/*
			Output the column names
		*/
		std::cout << std::setw(width) << " ";
		for (const auto &[first_name, first_set] : run_set)
			{
			(void)first_name;		// unused, supress warning.
			std::cout << std::right << std::setw(width) << first_set.begin()->second.run_id;
			}
		std::cout << '\n';

		for (const auto &[first_name, first_set] : run_set)
			{
			(void)first_name;		// unused, supress warning.
			/*
				Output the row name
			*/
			std::cout << std::setw(width) << first_set.begin()->second.run_id;

			/*
				Output the p-value
			*/
			for (const auto &[second_name, second_set] : run_set)
					{
					(void)second_name;		// unused, supress warning.
					std::vector<double> first_scores;
					std::vector<double> second_scores;
					for (const auto &[first_query, first_metric_set] : first_set)
						{
						auto second_metric_set = second_set.find(first_query);
						if (second_metric_set != second_set.end())
							{
							first_scores.push_back(first_metric_set.metric[metric]);
							second_scores.push_back(second_metric_set->second.metric[metric]);
							}
						}
					double p = JASS::statistics::ttest_paired(first_scores, second_scores, JASS::statistics::ONE_TAILED);
					if (isnan(p))
						{
						if (first_scores == second_scores)
							std::cout << std::setw(width) << '-';
						else
							std::cout << std::setw(width) << '!';
						}
					else
						std::cout << std::setw(width) << p;
					}
				std::cout << '\n';
				}
		}

	/*
		METRIC_SET::SPEARMAN_CORRELATE()
		--------------------------------
	*/
	/*!
		@brief Compute Spearman Correlation between sets of metrics.
		@brief over [in] The set of metrics to correlate over.
		@param average_set [in] The set of averages for all the runs.
	*/
	static void spearman_correlate(std::vector<metric_index> over, std::map<std::string, metric_set> &average_set)
		{
		/*
			Output the metric name
		*/
		std::cout << "Spearman Correlation across metrics\n";

		/*
			Get the width of the widest column (the length of the longest run name
		*/
		size_t width = 0;
		for (const auto metric : over)
			width = JASS::maths::maximum(width, metric_name[metric].size());

		width = JASS::maths::maximum(width + 1, static_cast<size_t>(7));			// account for a space and account for the size of a p value.
		std::cout.width(width);

		/*
			Output the column names
		*/
		std::cout << std::setw(width) << " ";
		for (const auto metric : over)
			std::cout << std::right << std::setw(width) << metric_name[metric];
		std::cout << '\n';

		for (const auto metric_one : over)
			{
			/*
				Output the row name
			*/
			std::cout << std::setw(width) << metric_name[metric_one];

			/*
				Compute the first set of scores
			*/
			std::vector<double> scores_one;
			for (const auto &[name, values] : average_set)
				{
				(void)name;
				scores_one.push_back(values.metric[metric_one]);
				}

			/*
				Compute the second set of scores and the Spearman Correlation
			*/
			for (const auto metric_two : over)
				{
				std::vector<double> scores_two;
				for (const auto &[name, values] : average_set)
					{
					(void)name;
					scores_two.push_back(values.metric[metric_two]);
					}

				double rho = JASS::statistics::spearman_correlation(scores_one, scores_two);
				std::cout << std::setw(width) << rho;
				}
			std::cout << '\n';
			}
		}
	};

/*
	OPERATOR<<()
	------------
*/
/*!
	@brief Dump the contents of an object down an output stream.
	@param stream [in] The stream to write to.
	@param tree [in] The object to write.
	@return The stream once the tree has been written.
*/
std::ostream &operator<<(std::ostream &stream, const metric_set &object)
	{
	int width = 35;
	std::cout << "QUERY ID                     : " << object.query_id << '\n';
	std::cout << "RUN ID                       : " << object.run_id << '\n';

	for (size_t which = 0; which < METRICS_SENTINAL_ORDINALS; which++)
		std::cout << std::setw(width) << std::left << metric_name[which] << ": " << (int)(object.metric[which]) << '\n';

	for (size_t which = METRICS_SENTINAL_ORDINALS + 1; which < METRICS_SENTINAL; which++)
		std::cout << std::setw(width) << std::left << metric_name[which] << ": " << object.metric[which] << '\n';

	return stream;
	}

/*
	CLASS RUN_RESULT
	----------------
*/
/*!
	@brief a single parsed line from a TREC run file
*/
class run_result
	{
	public:
		std::string query_id;
		std::string document_id;
		double rank;
		double score;
		std::string tag;

	public:
		/*
			RUN_RESULT::RUN_RESULT()
			------------------------
		*/
		/*!
			@brief Constructor
			@details
				The TREC run format is:
					query_idid Q0 document_id rank score tag
				e.g.
					51 Q0 WSJ871218-0126 1 208 JASSv2
			@param source [in] a single line from a TREC run file in trec_eval format.
		*/
		explicit run_result(uint8_t *source)
			{
			if (char *column_1 = (char *)source)
				{
				while (isspace(*column_1))
					column_1++;

				if (char *column_2 = strchr(column_1, ' '))
					{
					*column_2++ = '\0';
					while (isspace(*column_2))
						column_2++;

					if (char *column_3 = strchr(column_2, ' '))
						{
						*column_3++ = '\0';
						while (isspace(*column_3))
							column_3++;

						if (char *column_4 = strchr(column_3, ' '))
							{
							*column_4++ = '\0';
							while (isspace(*column_4))
								column_4++;

							if (char *column_5 = strchr(column_4, ' '))
								{
								*column_5++ = '\0';
								while (isspace(*column_5))
									column_5++;

								if (char *column_6 = strchr(column_5, ' '))
									{
									*column_6++ = '\0';
									while (isspace(*column_6))
										column_6++;

									query_id = column_1;
									document_id = column_3;
									rank = atof(column_4);
									score = atof(column_5);
									tag = column_6;
									}
								}
							}
						}
					}
				}
			}
		/*
			RUN_RESULT::OPERATOR<()
			-----------------------
		*/
		/*!
			@brief Less than comparison operator used in sort().
			@param second [in] a second object to compare to.
			@return true if this is strictly less than second.
		*/
		bool operator<(run_result &second)
			{
			/*
				query id first
			*/
			auto compare = query_id.compare(second.query_id);
			if (compare < 0)
				return true;
			else if (compare > 0)
				return false;

			/*
				then rank
			*/
			if (rank < second.rank)
				return true;
			else if (rank > second.rank)
				return false;

			/*
				then score
			*/
			if (score < second.score)
				return true;
			else if (score > second.score)
				return false;

			/*
				finally dopcument id
			*/
			compare = document_id.compare(second.document_id);
			if (compare < 0)
				return true;
			else if (compare > 0)
				return false;

			/*
				Else the two are the same (so not strictly less than)
			*/
			return false;
			}

	};

/*
	OPERATOR<<()
	------------
*/
/*!
	@brief Dump the contents of an object down an output stream.
	@param stream [in] The stream to write to.
	@param tree [in] The object to write.
	@return The stream once the tree has been written.
*/
std::ostream &operator<<(std::ostream &stream, const run_result &object)
	{
	stream << "[" << object.query_id << "][" << object.document_id << "][" << object.rank << "][" << object.score << "][" << object.tag << "]\n";
	return stream;
	}

/*
	LOAD_RUN()
	----------
*/
/*!
	@brief Load and parse a run in trec_eval format
	@param run_filename [in] the name of the run file to load
	@param parsed_run [out] the run once parsed
*/
void load_run(const std::string &run_filename, std::vector<run_result> &parsed_run)
	{
	/*
		Load the run
	*/
	std::string run;
	JASS::file::read_entire_file(run_filename, run);
	if (run == "")
		exit(printf("Cannot read run file: %s\n", run_filename.c_str()));

	/*
		Turn the run into a list of primary keys so that we can evaluate it.
		Note that there is no requirement that the run file is sorted on anyting so first
		we turn it into a vector of objects and then sort on the query id and the rank ordering
		then the score.
	*/
	std::vector<uint8_t *> run_list;
	JASS::file::buffer_to_list(run_list, run);

	for (uint8_t *line : run_list)
		parsed_run.push_back(run_result(line));
	}

/*
	LOAD_ASSESSMENTS()
	------------------
*/
 /*!
	@brief Load and parse a set of assessments in trec_eval format (with JASS PRICE metric extensions)
	@param assessments_filename [in] the name of the assessments file to load
	@param gold_standard_price [out] the PRICEs of the documents, or empty if there are no prices
	@param gold_standard_assessments [out] the assessments
 */

void load_assessments(std::string &assessments_filename, JASS::evaluate &gold_standard_price, JASS::evaluate &gold_standard_assessments)
	{
	/*
		Load the assessments
	*/
	std::string assessments;
	JASS::file::read_entire_file(assessments_filename, assessments);
	if (assessments == "")
		exit(printf("Cannot read run file: %s\n", assessments_filename.c_str()));

	/*
		If we're an eCommerce assessment file then break it into two parts.  The first part is the
		prices of the items, the second part is the regular TREC eval formatted assessments.  The
		first part is needed so that we can determine the amount of spending before the first relevant
		item is found.  For example,

		PRICE 0 AP1 2.34
		PRICE 0 AP2 5.67
		1 0 AP1 0
		1 0 AP2 1

		would indicate that item AP1 is worth $2.34 while item AP2 is worth $5.67.  Then item AP1 is know
		not relevant to query 1, but AP2 is relevant to query 1.  The format is:  Q_ID IT DOC_ID REL
		where Q_ID is the query id, it is ignored (should be 0), DOC_ID is the document id and rel is
		the relevance of this document to this query (0 = not relevant and 1 = relevant).
	*/
	if (assessments.compare(0, 5, "PRICE") == 0)
		{
		/*
			The actual assessments start at the first line that does not start "PRICE"
		*/
		const char *ch = assessments.c_str();
		while (1)
			{
			if ((ch = strchr(ch + 1, '\n')) == NULL)
				break;

			while (isspace(*ch))
				ch++;

			if (*ch == '\0' || strncmp(ch, "PRICE", 5) != 0)
				break;
			}

		size_t break_point = ch - assessments.c_str();
		std::string prices = std::string(assessments, 0, break_point);
		gold_standard_price.decode_assessments_trec_qrels(prices);

		assessments = assessments.substr(break_point);
		}

	gold_standard_assessments.decode_assessments_trec_qrels(assessments);
	}

/*
	RELEVANCE_COUNT()
	-----------------
*/
/*!
	@brief Count the numner of relevant results for this query
	@param query_id [in] the ID of the query
	@param gold_standard_assessments [in] the assessments
*/
size_t relevance_count(const std::string &query_id, JASS::evaluate &gold_standard_assessments)
	{
	size_t relevant = 0;

	for (auto assessment = gold_standard_assessments.find_first(query_id); assessment != gold_standard_assessments.assessments.end(); assessment++)
		if (query_id == assessment->query_id && assessment->score != 0)
			relevant++;

	return relevant;
	}

/*
	EVALUATE_QUERY()
	----------------
*/
/*!
	@brief Evaluate a single query
	@param query_id [in] the ID of the query
	@param results_list [in] the results list for this query
	@param gold_standard_price [in] the PRICEs of the documents, or empty if there are no prices
	@param gold_standard_assessments [in] the assessments
	@param depth [in] The how far down the results list to measure (counting from 1)
	@return A metric_set object with the score using several metrics
*/
metric_set evaluate_query(const std::string &run_id, const std::string &query_id, std::vector<std::string> &results_list, std::shared_ptr<JASS::evaluate> gold_standard_price, std::shared_ptr<JASS::evaluate>gold_standard_assessments, size_t depth)
	{
	size_t returned = results_list.size();

	JASS::evaluate_map evaluate_map_computer(gold_standard_assessments);
	double map = evaluate_map_computer.compute(query_id, results_list, depth);

	JASS::evaluate_rank_biased_precision evaluate_rbp_computer(parameter_probability, gold_standard_assessments);
	double rbp = evaluate_map_computer.compute(query_id, results_list, depth);

	size_t number_of_relvant_assessments = evaluate_map_computer.relevance_count(query_id);

	JASS::evaluate_relevant_returned evaluate_relevant_returned_computer(gold_standard_assessments);
	size_t relevant_returned = static_cast<size_t>(evaluate_relevant_returned_computer.compute(query_id, results_list));

	JASS::evaluate_mean_reciprocal_rank evaluate_mean_reciprocal_rank_computer(gold_standard_assessments);
	double mrr = evaluate_mean_reciprocal_rank_computer.compute(query_id, results_list, depth);

	JASS::evaluate_mean_reciprocal_rank4k evaluate_mean_reciprocal_rank_computer_4k(parameter_k, gold_standard_assessments);
	double mrr4k = evaluate_mean_reciprocal_rank_computer_4k.compute(query_id, results_list, depth);

	JASS::evaluate_precision precision_computer(gold_standard_assessments);
	double precision = precision_computer.compute(query_id, results_list);

	double p5 = precision_computer.compute(query_id, results_list, 5);
	double p10 = precision_computer.compute(query_id, results_list, 10);
	double p15 = precision_computer.compute(query_id, results_list, 15);
	double p20 = precision_computer.compute(query_id, results_list, 20);
	double p30 = precision_computer.compute(query_id, results_list, 30);
	double p100 = precision_computer.compute(query_id, results_list, 100);
	double p200 = precision_computer.compute(query_id, results_list, 200);
	double p500 = precision_computer.compute(query_id, results_list, 500);
	double p1000 = precision_computer.compute(query_id, results_list, 1000);

	JASS::evaluate_recall recall_computer(gold_standard_assessments);
	double recall = recall_computer.compute(query_id, results_list);

	double r5 = recall_computer.compute(query_id, results_list, 5);
	double r10 = recall_computer.compute(query_id, results_list, 10);
	double r15 = recall_computer.compute(query_id, results_list, 15);
	double r20 = recall_computer.compute(query_id, results_list, 20);
	double r30 = recall_computer.compute(query_id, results_list, 30);
	double r100 = recall_computer.compute(query_id, results_list, 100);
	double r200 = recall_computer.compute(query_id, results_list, 200);
	double r500 = recall_computer.compute(query_id, results_list, 500);
	double r1000 = recall_computer.compute(query_id, results_list, 1000);

	JASS::evaluate_expected_search_length esl_computer(gold_standard_assessments);
	double esl = esl_computer.compute(query_id, results_list, depth);

	JASS::evaluate_f f1_computer(gold_standard_assessments);
	double f1 = f1_computer.compute(query_id, results_list);

	double f1_5 = f1_computer.compute(query_id, results_list, 5);
	double f1_10 = f1_computer.compute(query_id, results_list, 10);
	double f1_15 = f1_computer.compute(query_id, results_list, 15);
	double f1_20 = f1_computer.compute(query_id, results_list, 20);
	double f1_30 = f1_computer.compute(query_id, results_list, 30);
	double f1_100 = f1_computer.compute(query_id, results_list, 100);
	double f1_200 = f1_computer.compute(query_id, results_list, 200);
	double f1_500 = f1_computer.compute(query_id, results_list, 500);
	double f1_1000 = f1_computer.compute(query_id, results_list, 1000);

	double cheapest_precision = 0;
	double buying_power = 0;
	double buying_power4k = 0;
	double selling_power = 0;

	if (gold_standard_price->assessments.size() != 0)
		{
		JASS::evaluate_cheapest_precision cheapest_precision_computer(gold_standard_price, gold_standard_assessments);
		cheapest_precision = cheapest_precision_computer.compute(query_id, results_list, depth);

		JASS::evaluate_buying_power buying_power_computer(gold_standard_price, gold_standard_assessments);
		buying_power = buying_power_computer.compute(query_id, results_list, depth);

		JASS::evaluate_buying_power4k buying_power4k_computer(parameter_k, gold_standard_price, gold_standard_assessments);
		buying_power4k = buying_power4k_computer.compute(query_id, results_list, depth);

		JASS::evaluate_selling_power selling_power_computer(gold_standard_price, gold_standard_assessments);
		selling_power = selling_power_computer.compute(query_id, results_list, depth);
		}

	return metric_set(run_id, query_id, number_of_relvant_assessments, returned, relevant_returned, mrr, mrr4k, precision, recall, esl, f1,
		p5, p10, p15, p20, p30, p100, p200, p500, p1000,
		r5, r10, r15, r20, r30, r100, r200, r500, r1000,
		f1_5, f1_10, f1_15, f1_20, f1_30, f1_100, f1_200, f1_500, f1_1000,
		map, rbp, cheapest_precision, selling_power, buying_power, buying_power4k);
	}

/*
	EVALUATE_RUN()
	--------------
*/
/*!
	@brief Evaluate a set of queries (a run)
	@param per_query_scores [out] the metric scores for each query
	@param parsed_run [in] the run to evaluate
	@param gold_standard_price [in] the PRICEs of the documents, or empty if there are no prices
	@param gold_standard_assessments [in] the assessments
*/
void evaluate_run(std::map<std::string, metric_set> &per_query_scores, std::vector<run_result> &parsed_run, std::shared_ptr<JASS::evaluate> gold_standard_price, std::shared_ptr<JASS::evaluate> gold_standard_assessments, size_t depth)
	{
	/*
		Break the run into a set of queries and evaluate each query
	*/
	std::vector<std::string> query_result;
	std::string current_query_id = parsed_run.begin()->query_id;
	std::string run_id = parsed_run.begin()->tag;

	for (const auto &one : parsed_run)
		{
		if (one.query_id == current_query_id)
			query_result.push_back(one.document_id);
		else
			{
			per_query_scores.emplace(current_query_id, evaluate_query(run_id, current_query_id, query_result, gold_standard_price, gold_standard_assessments, depth));
			query_result.clear();
			query_result.push_back(one.document_id);
			current_query_id = one.query_id;
			}
		}
	per_query_scores.emplace(current_query_id, evaluate_query(run_id, current_query_id, query_result, gold_standard_price, gold_standard_assessments, depth));
	}


/*
	LOAD_AND_EVALUATE_RUN()
	-----------------------
*/
void load_and_evaluate_run(const std::string &filename, std::map<std::string, metric_set> &per_query_scores, metric_set &averages, std::shared_ptr<JASS::evaluate> gold_standard_price, std::shared_ptr<JASS::evaluate> gold_standard_assessments, size_t depth)
	{
	/*
		Load the run.
	*/
	std::vector<run_result> parsed_run;
	load_run(filename, parsed_run);

	/*
		Evaluate the run.
	*/
	evaluate_run(per_query_scores, parsed_run, gold_standard_price, gold_standard_assessments, depth);

	/*
		Set up the averages
	*/
	averages.run_id = per_query_scores.begin()->second.run_id;
	double query_count = 0;
	for (const auto &[query_id, scores] : per_query_scores)
		{
		(void)query_id;
		query_count++;
		averages += scores;
		}

	/*
		Generate the averages including geometric means.
	*/
	averages.bake(gold_standard_assessments->query_count());
	}

/*
	USAGE()
	-------
*/
uint8_t usage(const std::string &exename)
	{
	std::cout << JASS::commandline::usage(exename, parameters) << "\n";
	return 1;
	}

/*
	MAIN()
	------
*/
int main(int argc, const char *argv[])
	{
	/*
		Set up the number of decimal places to print
	*/
	std::cout << std::fixed << std::setprecision(4);

	/*
		Parse the command line parameters
	*/
	auto success = JASS::commandline::parse(argc, argv, parameters, parameters_errors);
	if (!success)
		{
		std::cout << parameters_errors;
		exit(1);
		}

	/*
		Make sure we have a run filename and an assessment filename
	*/
	if (parameter_help)
		exit(usage(argv[0]));
	if (parameter_run_filenames == "" || parameter_assessments_filename == "")
		exit(usage(argv[0]));

	/*
		Load the assessments
	*/
	std::shared_ptr<JASS::evaluate> gold_standard_price(new JASS::evaluate);
	std::shared_ptr<JASS::evaluate>  gold_standard_assessments(new JASS::evaluate);
	load_assessments(parameter_assessments_filename, *gold_standard_price, *gold_standard_assessments);

	/*
		Load and evaluate each run.
	*/
	std::map<std::string, std::map<std::string, metric_set>> all_the_runs;
	std::map<std::string, metric_set> all_the_averages;
	char *filenames = strdup(parameter_run_filenames.c_str());
	char *save_pointer;
	for (char *name = strtok_r(filenames, ",", &save_pointer); name != NULL; name = strtok_r(NULL, ",", &save_pointer))
		{
		metric_set averages(name, "Averages");
		std::map<std::string, metric_set> per_query_scores;
		load_and_evaluate_run(name, per_query_scores, averages, gold_standard_price, gold_standard_assessments, parameter_depth);
		all_the_averages.emplace(name, averages);
		all_the_runs.emplace(name, per_query_scores);
		}

	/*
		Output the per query scores and the averages.
	*/
	if (parameter_output_per_query_scores)
		for (const auto &[run_name, per_query_scores] : all_the_runs)
			{
			(void)run_name;
			for (const auto &[query_id, scores] : per_query_scores)
				{
				(void)query_id;
				std::cout << scores << '\n';
				}
			}

	for (const auto &[run_name, averages] : all_the_averages)
		{
		(void)run_name;
		std::cout << averages << '\n';
		}

	/*
		For each metric, conduct the t-test
	*/
	if (parameter_ttest && all_the_averages.size() >= 2)
		{
		metric_set::t_test(MEAN_RECIPROCAL_RANK, all_the_runs);
		std::cout << '\n';
		metric_set::t_test(F1, all_the_runs);
		std::cout << '\n';
		metric_set::t_test(MEAN_AVERAGE_PRECISION, all_the_runs);
		std::cout << '\n';

		if (gold_standard_price->assessments.size() != 0)
			{
			metric_set::t_test(CHEAPEST_PRECISION, all_the_runs);
			std::cout << '\n';
			metric_set::t_test(SELLING_POWER, all_the_runs);
			std::cout << '\n';
			metric_set::t_test(BUYING_POWER, all_the_runs);
			std::cout << '\n';
			metric_set::t_test(BUYING_POWER4K, all_the_runs);
			std::cout << '\n';
			}
		}
	/*
		Conduct the spearman's correlation over the (useful) metrics
	*/
	if (parameter_spearman && all_the_averages.size() >= 2)
		{
		std::vector<metric_index> spearman_set = {F1, MEAN_AVERAGE_PRECISION};
		switch (parameter_depth)
			{
			case 5:
				spearman_set.push_back(P_AT_5);
				spearman_set.push_back(R_AT_5);
				spearman_set.push_back(F1_AT_5);
				break;
			case 10:
				spearman_set.push_back(P_AT_10);
				spearman_set.push_back(R_AT_10);
				spearman_set.push_back(F1_AT_10);
				break;
			case 15:
				spearman_set.push_back(P_AT_15);
				spearman_set.push_back(R_AT_15);
				spearman_set.push_back(F1_AT_15);
				break;
			case 20:
				spearman_set.push_back(P_AT_20);
				spearman_set.push_back(R_AT_20);
				spearman_set.push_back(F1_AT_20);
				break;
			case 30:
				spearman_set.push_back(P_AT_30);
				spearman_set.push_back(R_AT_30);
				spearman_set.push_back(F1_AT_30);
				break;
			case 100:
				spearman_set.push_back(P_AT_100);
				spearman_set.push_back(R_AT_100);
				spearman_set.push_back(F1_AT_100);
				break;
			case 200:
				spearman_set.push_back(P_AT_200);
				spearman_set.push_back(R_AT_200);
				spearman_set.push_back(F1_AT_200);
				break;
			case 500:
				spearman_set.push_back(P_AT_500);
				spearman_set.push_back(R_AT_500);
				spearman_set.push_back(F1_AT_500);
				break;
			case 1000:
				spearman_set.push_back(P_AT_1000);
				spearman_set.push_back(R_AT_1000);
				spearman_set.push_back(F1_AT_1000);
				break;
			default:
				spearman_set.push_back(PRECISION);
				spearman_set.push_back(RECALL);
				spearman_set.push_back(F1);
			}

		if (gold_standard_price->assessments.size() != 0)
			{
			spearman_set.push_back(CHEAPEST_PRECISION);
			spearman_set.push_back(SELLING_POWER);
			spearman_set.push_back(BUYING_POWER);
			spearman_set.push_back(BUYING_POWER4K);

			/*
				For metrics paper
			*/
			spearman_set.push_back(MEAN_RECIPROCAL_RANK);
			spearman_set.push_back(MEAN_RECIPROCAL_RANK_4K);
			spearman_set.push_back(ESL);
			spearman_set.push_back(RANK_BIASED_PRECISION);
			}

		metric_set::spearman_correlate(spearman_set, all_the_averages);
		}

	/*
		Done.
	*/
	return 0;
	}
