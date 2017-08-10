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
#include <string>
#include <utility>
#include <stdlib.h>

#include "string.h"

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
				@brief Extract a boolean value of the parameter from the command line parameters
				@param arg [in] The unparsed paramter as given by the user
				@param element [out] where to put the value.
				@return true if it was possible to extract a value, false on error.
			*/
			static bool extract(std::ostringstream &messages, const char *arg, command<bool> element)
				{
				element.parameter = true;
				return true;
				}

			/*
				COMMANDLINE::EXTRACT
				--------------------
			*/
			/*!
				@brief Extract an integer value of the parameter from the command line parameters
				@param arg [in] The unparsed paramter as given by the user
				@param element [out] where to put the value.
				@return true if it was possible to extract a value, false on error.
			*/
			template <typename TYPE,
				typename std::enable_if <std::is_same<TYPE, short>::value ||
							 std::is_same<TYPE, int>::value ||
							 std::is_same<TYPE, long>::value ||
							 std::is_same<TYPE, long long>::value>::type* = nullptr>
			static bool extract(std::ostringstream &messages, const char *arg, command<TYPE> element)
				{
				long long answer = strtoll(arg, NULL, 0);
				if (answer > std::numeric_limits<TYPE>::max())
					messages << arg << " Numeric overflow on parameter\n";
				else if (answer < std::numeric_limits<TYPE>::min())
					messages << arg << " Numeric underflow on parameter\n";
				else
					{
					element.parameter = reinterpret_cast<TYPE>(answer);
					return true;
					}

				return false;
				}

			/*
				COMMANDLINE::EXTRACT
				--------------------
			*/
			/*!
				@brief Extract an unsigned integer value of the parameter from the command line parameters
				@param arg [in] The unparsed paramter as given by the user
				@param element [out] where to put the value.
				@return true if it was possible to extract a value, false on error.
			*/
			template <typename TYPE,
				typename std::enable_if <std::is_same<TYPE, unsigned short>::value ||
							 std::is_same<TYPE, unsigned int>::value ||
							 std::is_same<TYPE, unsigned long>::value ||
							 std::is_same<TYPE, unsigned long long>::value>::type* = nullptr>
			static bool extract(std::ostringstream &messages, const char *arg, command<TYPE> element)
				{
				unsigned long long answer = strtoull(arg, NULL, 0);
				if (answer > std::numeric_limits<TYPE>::max())
					{
					messages << arg << " Numeric overflow on parameter\n";
					return false;
					}
				element.parameter = reinterpret_cast<TYPE>(answer);
				return true;
				}

			/*
				COMMANDLINE::EXTRACT
				--------------------
			*/
			/*!
				@brief Extract a string value of the parameter from the command line parameters
				@param arg [in] The unparsed parameter as given by the user
				@param element [out] where to put the value.
				@return true if it was possible to extract a value, false on error.
			*/
			static bool extract(std::ostringstream &messages, const char *arg, command<std::string> element)
				{
				element.parameter = arg;
				return true;
				}

			/*
				COMMANDLINE::EXTRACT
				--------------------
			*/
			/*!
				@brief Catch all for unknown types
				@param arg [in] The unparsed parameter as given by the user
				@param element [out] where to put the value.
				@return true if it was possible to extract a value, false on error.
			*/
			template <typename TYPE>
			static bool extract(std::ostringstream &messages, const char *arg, TYPE element)
				{
				messages << element.shortname << " " << element.longname << arg << " Unknown parameter type\n";
				return false;
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
			*/
			template<std::size_t I = 0, typename... Tp>
			inline typename std::enable_if<I == sizeof...(Tp), void>::type
			static for_each_parameter(bool &success, std::ostringstream &messages, size_t &arg, const char *argv[], std::tuple<Tp...> &)
				{
				messages << argv[arg] << " Unknown parameter\n";
				arg++;
				success = false;
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
			static for_each_parameter(bool &success, std::ostringstream &messages, size_t &arg, const char *argv[], std::tuple<Tp...> &tuple)
				{
				if (strcmp(argv[arg], std::get<I>(tuple).shortname.c_str()) == 0)				// looking for "-a v"
					success |= !extract(messages, argv[++arg], std::get<I>(tuple));
				else if (strcmp(argv[arg], std::get<I>(tuple).longname.c_str()) == 0)		// looking for "--aaa v"
					success |= !extract(messages, argv[++arg], std::get<I>(tuple));
				else if (strncmp(argv[arg], std::get<I>(tuple).longname.c_str(), std::get<I>(tuple).longname.size()) == 0)			// looking for "-aav"
					{
					success |= !extract(messages, argv[arg] + std::get<I>(tuple).longname.size(), std::get<I>(tuple));
					arg++;
					}
				else if (strncmp(argv[arg], std::get<I>(tuple).shortname.c_str(), std::get<I>(tuple).shortname.size()) == 0)		// looking for "-av"
					{
					success |= !extract(messages, argv[arg] + std::get<I>(tuple).shortname.size(), std::get<I>(tuple));
					arg++;
					}
				else
					for_each_parameter<I + 1, Tp...>(success, messages, arg, argv, tuple);
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
			*/
			template <typename TYPE>
			static command<TYPE> parameter(const std::string &shortname, const std::string &longname, const std::string &description, TYPE &parameter)
				{
				return command<TYPE>(shortname, longname, description, parameter);
				}

			/*
				COMMANDLINE::PARAMETER()
				------------------------
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
				bool success = true;
				size_t argument = 0;

				while (argument < argc)
					for_each_parameter(success, messages, argument, argv, all_parameters);

				error = messages.str();
				return success;
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
				int argc = 2;
				const char *argv[] = {"-sj1", "-sj2another", "-sj3", };

				bool jass_v1_index = false;
				auto serialiser = commandline::parameter("-sj1", "--serialise_jass_v1", "Serialise the index as a JASS v1 index", jass_v1_index);

				std::string jass_v1_string = "Something";
				auto str = commandline::parameter("-sj2", "--serialise_jass_v2", "Serialise the index as a JASS v1 index", jass_v1_string);

				auto all_commands = std::make_tuple(serialiser, str);
				std::string error;
				auto success = commandline::parse(argc, argv, all_commands, error);

				if (!success)
					std::cout << error;
				else
					std::cout << jass_v1_index << " " << jass_v1_string << "\n";
				}
		};
	}

