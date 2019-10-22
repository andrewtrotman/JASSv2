/*
	JASS_EVAL.CPP
	-------------
	Copyright (c) 2019 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <string.h>

#include <map>
#include <string>

#include "file.h"
#include "commandline.h"
#include "evaluate_precision.h"
#include "evaluate_buying_power.h"
#include "evaluate_selling_power.h"
#include "evaluate_buying_power4k.h"
#include "evaluate_cheapest_precision.h"

/*
	PARAMETERS
	----------
*/
std::string parameter_assessments_filename;			///< Name of file assessment file
std::string parameter_run_filename;						///< Name of the run file to evaluate
bool parameter_help = false;								///< Output the usage() help
bool parameter_output_per_query_scores = false;		///< Shopuld we output per-query scores or not?

std::string parameters_errors;							///< Any errors as a result of command line parsing

auto parameters = std::make_tuple						///< The  command line parameter block
	(
	JASS::commandline::parameter("-?", "--help",          "Print this help.", parameter_help),
	JASS::commandline::parameter("-a", "--assesmentfile", "<filename> Name of the file containing the assessmengts", parameter_assessments_filename),
	JASS::commandline::parameter("-r", "--runfile",       "<filename> Name of run file to evaluatge", parameter_run_filename),
	JASS::commandline::parameter("-p", "--perquery",      " Output per-query statistics", parameter_output_per_query_scores)
	);

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
		double precision;					///< set wise precision of the results list.
		double cheapest_precision;		///< set wise precision of the cheapest items (if we are an eCommerce metric)
		double selling_power;			///< selling power (if we are an eCommerce metric)
		double buying_power;				///< buying power (if we are an eCommerce metric)

	public:
		/*
			METRIC_SET::METRIC_SET()
			------------------------
		*/
		/*!
			@brief Constructor
			@param precision [in] The precision of this query.
			@param cheapest_precision [in] The cheapest precision of this query.
			@param selling_power [in] The selling power of this query.
			@param buying_power [in] The buying power of this query.
		*/
		metric_set(double precision, double cheapest_precision, double selling_power, double buying_power) :
			precision(precision),
			cheapest_precision(cheapest_precision),
			selling_power(selling_power),
			buying_power(buying_power)
			{
			/* Nothing */
			}

		/*!
			@brief Constructor
		*/
		metric_set() :
			metric_set(0, 0, 0, 0)
			{
			/* Nothing */
			}
	};

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
		run_result(uint8_t *source)
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
void load_run(std::string &run_filename, std::vector<run_result> &parsed_run)
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
	EVALUATE_QUERY()
	----------------
*/
/*!
	@brief Evaluate a single query
	@param query_id [in] the ID of the query
	@param results_list [in] the results list for this query
	@param gold_standard_price [in] the PRICEs of the documents, or empty if there are no prices
	@param gold_standard_assessments [in] the assessments
	@return A metric_set object with the score using several metrics
*/
metric_set evaluate_query(const std::string &query_id, std::vector<std::string> &results_list, JASS::evaluate &gold_standard_price, JASS::evaluate &gold_standard_assessments)
	{
	JASS::evaluate_precision precision_computer(gold_standard_assessments);
	double precision = precision_computer.compute(query_id, results_list);

	JASS::evaluate_cheapest_precision cheapest_precision_computer(gold_standard_price, gold_standard_assessments);
	double cheapest_precision = cheapest_precision_computer.compute(query_id, results_list);

	JASS::evaluate_buying_power buying_power_computer(gold_standard_price, gold_standard_assessments);
	double buying_power = buying_power_computer.compute(query_id, results_list);

	JASS::evaluate_selling_power selling_power_computer(gold_standard_price, gold_standard_assessments);
	double selling_power = selling_power_computer.compute(query_id, results_list);

	return metric_set(precision, cheapest_precision, selling_power, buying_power);
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
void evaluate_run(std::map<std::string, metric_set> &per_query_scores, std::vector<run_result> &parsed_run, JASS::evaluate &gold_standard_price, JASS::evaluate &gold_standard_assessments)
	{
	/*
		Break the run into a set of queries and evaluate each query
	*/
	std::vector<std::string> query_result;
	std::string current_query_id = parsed_run.begin()->query_id;

	for (const auto &one : parsed_run)
		{
		if (one.query_id == current_query_id)
			query_result.push_back(one.document_id);
		else
			{
			per_query_scores.emplace(current_query_id, evaluate_query(current_query_id, query_result, gold_standard_price, gold_standard_assessments));
			query_result.clear();
			query_result.push_back(one.document_id);
			current_query_id = one.query_id;
			}
		}
	per_query_scores.emplace(current_query_id, evaluate_query(current_query_id, query_result, gold_standard_price, gold_standard_assessments));
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
		Parse the commane line parameters
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
	if (parameter_run_filename == "" || parameter_assessments_filename == "")
		exit(usage(argv[0]));

	/*
		Load the run and the assessments
	*/
	std::vector<run_result> parsed_run;
	load_run(parameter_run_filename, parsed_run);

	JASS::evaluate gold_standard_price;
	JASS::evaluate gold_standard_assessments;
	load_assessments(parameter_assessments_filename, gold_standard_price, gold_standard_assessments);

	/*
		Evaluate the run
	*/
	std::map<std::string, metric_set> per_query_scores;
	evaluate_run(per_query_scores, parsed_run, gold_standard_price, gold_standard_assessments);

	metric_set averages;
	double query_count = 0;
	for (const auto &[query_id, scores] : per_query_scores)
		{
		query_count++;
		averages.precision += scores.precision;
		averages.cheapest_precision += scores.cheapest_precision;
		averages.selling_power += scores.selling_power;
		averages.buying_power += scores.buying_power;
		if (parameter_output_per_query_scores)
			{
			std::cout << query_id << " Precision " << scores.precision << "\n";
			std::cout << query_id << " Cheapest Precision " << scores.cheapest_precision << "\n";
			std::cout << query_id << " Selling Power " << scores.selling_power << "\n";
			std::cout << query_id << " Buying Power " << scores.buying_power << "\n";
			}
		}

	/*
		Output the average (mean) scores.
	*/
	std::cout << "Average Precision " << averages.precision / query_count << "\n";
	std::cout << "Average Cheapest Precision " << averages.cheapest_precision / query_count << "\n";
	std::cout << "Average Selling Power " << averages.selling_power / query_count  << "\n";
	std::cout << "Average Buying Power " << averages.buying_power / query_count  << "\n";

	return 0;
	}
