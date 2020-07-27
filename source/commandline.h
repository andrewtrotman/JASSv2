/*
	COMMANDLINE.H
	-------------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Command line processor in a single stand-alone header file
	@author Andrew Trotman
	@copyright 2017 Andrew Trotman
*/
#pragma once
#include <tuple>
#include <limits>
#include <string>
#include <utility>
#include <sstream>
#include <iostream>
#include <typeinfo>

#include <stdlib.h>

#include "string.h"
#include "asserts.h"

namespace JASS
	{
	/*
		CLASS COMMANDLINE
		-----------------
	*/
	/*!
		@brief Command line processor
	*/
	class commandline
		{
		private:
			/*
				CLASS COMMANDLINE::TEXT_NOTE
				----------------------------
			*/
			/*!
				@brief used to specify text strings to appear in the help.
			*/
			class text_note
			 {
			 /* Nothing */
			 };

			/*
				CLASS COMMANDLINE::COMMAND
				--------------------------
			*/
			/*!
				@brief A single command line parameter
			*/
			template <typename TYPE>
			class command
				{
				public:
					std::string shortname;				///< The short name to match e.g. "-m".
					std::string longname;				///< The long name to mathc e.g. "-mood".
					std::string description;			///< The descriptrion to show when the user asks for help e.g. "The users' mood".
					TYPE &parameter;						///< A reference to the external variable to set based on the command line.

				public:

					/*
						CLASS COMMANDLINE::COMMAND::COMMAND
						-----------------------------------
					*/
					/*!
						@brief Build an object that represents a possible command line parameter.
						@param shortname [in] The short name to match e.g. "-m".
						@param longname [in] The long name to mathc e.g. "-mood".
						@param description [in] The descriptrion to show when the user asks for help e.g. "The users' mood".
						@param parameter [out] A reference to the external variable to set based on the command line.
					*/
					command(const std::string &shortname, const std::string &longname, const std::string &description, TYPE &parameter) :
						shortname(shortname),
						longname(longname),
						description(description),
						parameter(parameter)
						{
						/* Nothing */
						}
				};

		private:
			/*
				COMMANDLINE::EXTRACT
				--------------------
			*/
			/*!
				@brief Dummy for use with text embedded in the parameter lists.
				@param messages [out] Any errors are reported down this stream.
				@param parameter [in] The unparsed paramter as given by the user.
				@param element [out] where to put the value (unused).
				@return true if it was possible to extract a value, false on error.
			*/
			static void extract(std::ostringstream &messages, const char *parameter, command<text_note> element)
				{
				/* Nothing */
				}

			/*
				COMMANDLINE::EXTRACT
				--------------------
			*/
			/*!
				@brief Extract a boolean value of the parameter from the command line parameters.
				@param messages [out] Any errors are reported down this stream.
				@param parameter [in] The unparsed paramter as given by the user.
				@param element [out] where to put the value.
			*/
			static void extract(std::ostringstream &messages, const char *parameter, command<bool> element)
				{
				element.parameter = true;
				}

			/*
				COMMANDLINE::EXTRACT
				--------------------
			*/
			/*!
				@brief Extract an integer value of the parameter from the command line parameters
				@details All signed integer types can be decode in this one method by assuming the largest type and then checking for out of range
				@param messages [out] Any errors are reported down this stream.
				@param parameter [in] The unparsed paramter as given by the user
				@param element [out] where to put the value.
			*/
			template <typename TYPE,
				typename std::enable_if <std::is_same<TYPE, short>::value ||
							 std::is_same<TYPE, int>::value ||
							 std::is_same<TYPE, long>::value ||
							 std::is_same<TYPE, long long>::value>::type* = nullptr>
			static void extract(std::ostringstream &messages, const char *parameter, command<TYPE> element)
				{
				long long answer = strtoll(parameter, NULL, 0);

				if (answer > (std::numeric_limits<TYPE>::max)())
					messages << parameter << " Numeric overflow on parameter\n";
				else if (answer < (std::numeric_limits<TYPE>::min)())
					messages << parameter << " Numeric underflow on parameter\n";
				else
					element.parameter = static_cast<TYPE>(answer);
				}

			/*
				COMMANDLINE::EXTRACT
				--------------------
			*/
			/*!
				@brief Extract an unsigned integer value of the parameter from the command line parameters
				@details All unsigned integer types can be decode in this one method by assuming the largest type and then checking for out of range
				@param messages [out] Any errors are reported down this stream.
				@param parameter [in] The unparsed paramter as given by the user
				@param element [out] where to put the value.
			*/
			template <typename TYPE,
				typename std::enable_if <std::is_same<TYPE, unsigned short>::value ||
							 std::is_same<TYPE, unsigned int>::value ||
							 std::is_same<TYPE, unsigned long>::value ||
							 std::is_same<TYPE, unsigned long long>::value>::type* = nullptr>
			static void extract(std::ostringstream &messages, const char *parameter, command<TYPE> element)
				{
				unsigned long long answer = strtoull(parameter, NULL, 0);

				/*
					Coverity Scan complains about the line below when TYPE is a unisgned long long because it can never be true.
				*/
				if constexpr (!std::is_same<TYPE, unsigned long long>::value)
					if (answer > (std::numeric_limits<TYPE>::max)())
						messages << parameter << " Numeric overflow on parameter\n";

				element.parameter = static_cast<TYPE>(answer);
				}

			/*
				COMMANDLINE::EXTRACT
				--------------------
			*/
			/*!
				@brief Extract a floating point value of the parameter from the command line parameters
				@details manages floats and doubles and checks for out of range
				@param messages [out] Any errors are reported down this stream.
				@param parameter [in] The unparsed paramter as given by the user
				@param element [out] where to put the value.
			*/
			template <typename TYPE,
				typename std::enable_if <std::is_same<TYPE, float>::value ||
							 std::is_same<TYPE, double>::value>::type* = nullptr>
			static void extract(std::ostringstream &messages, const char *parameter, command<TYPE> element)
				{
				double answer = std::stod(parameter);

				if (answer > (std::numeric_limits<TYPE>::max)())
					messages << parameter << " Numeric overflow on parameter\n";
				else if (answer < (std::numeric_limits<TYPE>::min)())
					messages << parameter << " Numeric underflow on parameter\n";
				else
					element.parameter = static_cast<TYPE>(answer);
				}

			/*
				COMMANDLINE::EXTRACT
				--------------------
			*/
			/*!
				@brief Extract a string value of the parameter from the command line parameters
				@param messages [out] Any errors are reported down this stream.
				@param parameter [in] The unparsed parameter as given by the user
				@param element [out] where to put the value.
			*/
			static void extract(std::ostringstream &messages, const char *parameter, command<std::string> element)
				{
				element.parameter = parameter;
				}

			/*
				COMMANDLINE::EXTRACT
				--------------------
			*/
			/*!
				@brief Catch all for unknown types
				@param messages [out] Any errors are reported down this stream.
				@param parameter [in] The unparsed parameter as given by the user
				@param element [out] where to put the value.
			*/
			template <typename TYPE>
			static void extract(std::ostringstream &messages, const char *parameter, TYPE element)
				{
				messages << element.shortname << " (" << element.longname  << ") Unknown parameter type\n";
				}

			/*
				COMMANDLINE::FOR_EACH_PARAMETER()
				---------------------------------
				For information on interating over a tuple see here https://stackoverflow.com/questions/1198260/iterate-over-tuple
			*/
			/*!
				@brief Iterate over each parameter looking for one that matches
				@param messages [out] Any errors are reported down this stream.
				@param arg [in/out] The place in the command line parameter list that we're currenty looking at.
				@param argv [in] The command line parameter list.
			*/
			template<std::size_t I = 0, typename... Tp>
			inline typename std::enable_if<I == sizeof...(Tp), void>::type
			static for_each_parameter(std::ostringstream &messages, size_t &arg, const char *argv[], std::tuple<Tp...> &)
				{
				messages << argv[arg] << " Unknown parameter\n";
				arg++;
				}

			/*
				COMMANDLINE::FOR_EACH_PARAMETER()
				---------------------------------
				For information on interating over a tuple see here https://stackoverflow.com/questions/1198260/iterate-over-tuple
			*/
			/*!
				@brief Iterate over each parameter looking for one that matches
				@param arg [in/out] The place in the command line parameter list that we're currenty looking at.
				@param argv [in] The command line parameter list.
				@param tuple [in] The remainder of the command line parameters as specified as the programmer.
			*/
			template<std::size_t I = 0, typename... Tp>
			inline typename std::enable_if<I < sizeof...(Tp), void>::type
			static for_each_parameter(std::ostringstream &messages, size_t &arg, const char *argv[], std::tuple<Tp...> &tuple)
				{
				if (std::get<I>(tuple).shortname.size() == 0 || std::get<I>(tuple).longname.size() == 0)				// looking for non-parameters (i.e. notes_
					for_each_parameter<I + 1, Tp...>(messages, arg, argv, tuple);
				else if (strcmp(argv[arg], std::get<I>(tuple).shortname.c_str()) == 0)				// looking for "-a v"
					{
					extract(messages, argv[++arg], std::get<I>(tuple));
					if (typeid(std::get<I>(tuple).parameter) != typeid(bool))
						arg++;
					}
				else if (strcmp(argv[arg], std::get<I>(tuple).longname.c_str()) == 0)		// looking for "--aaa v"
					{
					extract(messages, argv[++arg], std::get<I>(tuple));
					if (typeid(std::get<I>(tuple).parameter) != typeid(bool))
						arg++;
					}
				else if (strncmp(argv[arg], std::get<I>(tuple).longname.c_str(), std::get<I>(tuple).longname.size()) == 0)			// looking for "-aav"
					{
					extract(messages, argv[arg] + std::get<I>(tuple).longname.size(), std::get<I>(tuple));
					arg++;
					}
				else if (strncmp(argv[arg], std::get<I>(tuple).shortname.c_str(), std::get<I>(tuple).shortname.size()) == 0)		// looking for "-av"
					{
					extract(messages, argv[arg] + std::get<I>(tuple).shortname.size(), std::get<I>(tuple));
					arg++;
					}
				else
					for_each_parameter<I + 1, Tp...>(messages, arg, argv, tuple);
				}

			/*
				COMMANDLINE::FOR_EACH_USAGE_FORMATTING()
				----------------------------------------
				For information on interating over a tuple see here https://stackoverflow.com/questions/1198260/iterate-over-tuple
			*/
			/*!
				@brief Iterate over each parameter computing the length of the longest shortname and longname
				@param width_of_shortname [out] the longest shortname here or below.
				@param width_of_longname [out] the longest longname here or below.
				@param tuple [in] The remainder of the command line parameters as specified as the programmer.
				@tparam I the iteration number.
				@tparam TP the tupple being iterated over
			*/
			template<std::size_t I = 0, typename... Tp>
			inline typename std::enable_if<I == sizeof...(Tp), void>::type
			static for_each_usage_formatting(size_t &width_of_shortname, size_t &width_of_longname, const std::tuple<Tp...> &)
				{
				/* Nothing */
				}

			/*
				COMMANDLINE::FOR_EACH_USAGE_FORMATTING()
				----------------------------------------
				For information on interating over a tuple see here https://stackoverflow.com/questions/1198260/iterate-over-tuple
			*/
			/*!
				@brief Iterate over each parameter computing the length of the longest shortname and longname
				@param width_of_shortname [out] the longest shortname here or below.
				@param width_of_longname [out] the longest longname here or below.
				@param tuple [in] The remainder of the command line parameters as specified as the programmer.
				@tparam I the iteration number.
				@tparam TP the tupple being iterated over
			*/
			template<std::size_t I = 0, typename... Tp>
			inline typename std::enable_if<I < sizeof...(Tp), void>::type
			static for_each_usage_formatting(size_t &width_of_shortname, size_t &width_of_longname, const std::tuple<Tp...> &tuple)
				{
				if (std::get<I>(tuple).shortname.size() > width_of_shortname)
					width_of_shortname = std::get<I>(tuple).shortname.size();

				if (std::get<I>(tuple).longname.size() > width_of_longname)
					width_of_longname = std::get<I>(tuple).longname.size();

				for_each_usage_formatting<I + 1, Tp...>(width_of_shortname, width_of_longname, tuple);
				}

			/*
				COMMANDLINE::FOR_EACH_USAGE_PRINT()
				-----------------------------------
				For information on interating over a tuple see here https://stackoverflow.com/questions/1198260/iterate-over-tuple
			*/
			/*!
				@brief Iterate over each parameter printing the shortname, longname, and description.
				@param out [in] The stream to write to.
				@param width_of_shortname [in] Space to take to print the shortname.
				@param width_of_longname [in] Space to take to print the shortname.
			*/
			template<std::size_t I = 0, typename... Tp>
			inline typename std::enable_if<I == sizeof...(Tp), void>::type
			static for_each_usage_print(std::ostream &out, size_t width_of_shortname, size_t width_of_longname, const std::tuple<Tp...> &)
				{
				/* Nothing */
				}

			/*
				COMMANDLINE::FOR_EACH_USAGE_PRINT()
				-----------------------------------
				For information on interating over a tuple see here https://stackoverflow.com/questions/1198260/iterate-over-tuple
			*/
			/*!
				@brief Iterate over each parameter printing the shortname, longname, and description.
				@param out [in] The stream to write to.
				@param width_of_shortname [in] Space to take to print the shortname.
				@param width_of_longname [in] Space to take to print the shortname.
				@param tuple [in] The remainder of the command line parameters as specified as the programmer.
			*/
			template<std::size_t I = 0, typename... Tp>
			inline typename std::enable_if<I < sizeof...(Tp), void>::type
			static for_each_usage_print(std::ostream &out, size_t width_of_shortname, size_t width_of_longname, const std::tuple<Tp...> &tuple)
				{
				/*
					Save the state of the stream (because we're going to manipulate it)
				*/
				std::ios state(NULL);
				state.copyfmt(out);
				std::ios_base::fmtflags fmt(std::cout.flags());
				/*
					Dump out the usage
				*/
				if (std::get<I>(tuple).shortname.size() == 0)
					out << std::left << std::get<I>(tuple).description << '\n';
				else
					{
					out.width(width_of_shortname + 1);
					out << std::left << std::get<I>(tuple).shortname;
					out.width(width_of_longname + 1);
					out << std::get<I>(tuple).longname;
					out << std::get<I>(tuple).description << '\n';
					}
					
				/*
					Return the state to how it was before we changed it
				*/
				std::cout.flags(fmt);
				out.copyfmt(state);

				for_each_usage_print<I + 1, Tp...>(out, width_of_shortname, width_of_longname, tuple);
				}

		public:
			/*
				COMMANDLINE::PARAMETER()
				------------------------
			*/
			/*!
				@brief Specify an allowable command line parameter
				@details To specify a possible command line parameter call this method and use the result in a std::tuple call to parse.
				For example:
				
				bool jass_v1_index = true;

				auto serialiser = commandline::parameter("-sj1", "--serialise_jass_v1", "Serialise the index as a JASS v1 index", jass_v1_index);

				@param shortname [in] The short name to match e.g. "-m".
				@param longname [in] The long name to mathc e.g. "-mood".
				@param description [in] The descriptrion to show when the user asks for help e.g. "The users' mood".
				@param parameter [out] A reference to the external variable to set based on the command line.
				@return A command object for use in a tuple.
			*/
			template <typename TYPE>
			static command<TYPE> parameter(const std::string &shortname, const std::string &longname, const std::string &description, TYPE &parameter)
				{
				return command<TYPE>(shortname, longname, description, parameter);
				}


			/*
				COMMANDLINE::NOTE()
				-------------------
			*/
			/*!
				@brief Specify an allowable command line parameter
				@details To specify a possible command line parameter call this method and use the result in a std::tuple call to parse.
				For example:
				
				bool jass_v1_index = true;

				auto serialiser = commandline::parameter("-sj1", "--serialise_jass_v1", "Serialise the index as a JASS v1 index", jass_v1_index);

				@param description [in] The descriptrion to show when the user asks for help e.g. "The users' mood".
			*/
			static command<text_note> note(const std::string &description)
				{
				text_note blank;

				return command<text_note>("", "", description, blank);
				}

			/*
				COMMANDLINE::PARSE()
				--------------------
			*/
			/*!
				@brief Perform the command line parsing.
				@param argc [in] argc as passed to main
				@param argv [in] argv as passed to main.
				@param all_parameters [in] a tuple of objects returned bu commandline::parameter.
				@param error [out] if there is an error then this parameter will recieve a textural description of that error.
				@return true on success, false on error.
			*/
			template <typename... TYPE>
			static bool parse(int argc, const char *argv[], std::tuple<TYPE...> &all_parameters, std::string &error)
				{
				std::ostringstream messages;
				size_t argument = 1;

				while (argument < static_cast<size_t>(argc))
					for_each_parameter(messages, argument, argv, all_parameters);

				error = messages.str();
				return error == "";
				}

			/*
				COMMANDLINE::USAGE()
				--------------------
			*/
			/*!
				@brief Produce command-line help from the parameters
				@param exename [in] The name of the executable
				@param all_parameters [in] a tuple of objects returned bu commandline::parameter.
			*/
			template <typename... TYPE>
			static std::string usage(const std::string &exename, const std::tuple<TYPE...> &all_parameters)
				{
				size_t width_of_shortname = 0;
				size_t width_of_longname = 0;
				std::ostringstream answer;

				/*
					Compute the length of the shortname and longname fields.
				*/
				for_each_usage_formatting(width_of_shortname, width_of_longname, all_parameters);

				/*
					Print the usage instructions.
				*/
				for_each_usage_print(answer, width_of_shortname, width_of_longname, all_parameters);

				return answer.str();
				}

			/*
				ALLOCATOR_CPP::UNITTEST()
				-------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void)
				{
				/*
					Declare some objects
				*/
				bool parameter_boolean = false;
				std::string parameter_string = "Something";
				int parameter_integer = 0;
				unsigned int parameter_unsigned = 0;
				unsigned long long parameter_unsigned_long_long;
				long long parameter_long_long;

				/* 
					Declare the parameter object to parse
				*/
				auto all_commands = std::make_tuple
					(
					commandline::note("PARAMETERS"),
					commandline::parameter("-b", "--boolean", "Extract a boolean", parameter_boolean),
					commandline::parameter("-s", "--string", "Extractr a string", parameter_string),
					commandline::parameter("-i", "--integer", "Extract an integer", parameter_integer),
					commandline::parameter("-u", "--unsigned", "Extract an unsigned integer", parameter_unsigned),
					commandline::parameter("-h", "--huge", "Extract an unsigned ilong long nteger", parameter_unsigned_long_long),
					commandline::parameter("-H", "--Huge", "Extract an long log integer", parameter_long_long)
					);

				/*
					Declare a command line like argc and argv[].
				*/
				int argc = 9;
				const char *argv[] = {"program", "-b", "-s", "string", "-i3", "-u", "4", "-h0", "-H0"};

				/*
					Call the parser
				*/
				std::string error;
				auto success = commandline::parse(argc, argv, all_commands, error);

				/*
					Make sure we got the right answer.
				*/
				JASS_assert(success);
				JASS_assert(error == "");
				std::ostringstream results;
				results << parameter_boolean << parameter_string << parameter_integer << parameter_unsigned << parameter_unsigned_long_long;
				JASS_assert(results.str() == "1string340");

				/*
					Check longnames
				*/
				parameter_boolean = false;
				argc = 7;
				const char *argv2[] = {"program", "--boolean", "--string", "four", "--integer5", "--unsigned", "6"};
				success = commandline::parse(argc, argv2, all_commands, error);
				JASS_assert(success);
				std::ostringstream results2;
				results2 << parameter_boolean << parameter_string << parameter_integer << parameter_unsigned;
				JASS_assert(results2.str() == "1four56");

				/*
					check for errors
				*/
				class sptang
					{
					/* Nothing */
					} funny_object;
				auto error_commands = std::make_tuple
					(
					commandline::note("PARAMETERS"),
					commandline::parameter("-b", "--boolean", "Extract a boolean", parameter_boolean),
					commandline::parameter("-s", "--string", "Extractr a string", parameter_string),
					commandline::parameter("-i", "--integer", "Extract an integer", parameter_integer),
					commandline::parameter("-u", "--unsigned", "Extract an unsigned integer", parameter_unsigned),
					commandline::parameter("-f", "--funny", "Extract an object", funny_object)
					);

				const char *argv3[] = {"program", "--integer", "2147483648", "--unsigned", "4294967296", "--integer", "-2147483649", "--nonexistant", "--funny"};
				success = commandline::parse(9, argv3, error_commands, error);
				JASS_assert(!success);
				std::string answer =
					"2147483648 Numeric overflow on parameter\n"
					"4294967296 Numeric overflow on parameter\n"
					"-2147483649 Numeric underflow on parameter\n"
					"--nonexistant Unknown parameter\n"
					"-f (--funny) Unknown parameter type\n";
				JASS_assert(error == answer);

				auto how_to = usage("exename", error_commands);
				std::string how_to_use =
					"PARAMETERS\n"
					"-b --boolean  Extract a boolean\n"
					"-s --string   Extractr a string\n"
					"-i --integer  Extract an integer\n"
					"-u --unsigned Extract an unsigned integer\n"
					"-f --funny    Extract an object\n";
				JASS_assert(how_to == how_to_use);

				puts("commandline:PASSED");
				}
		};
	}
