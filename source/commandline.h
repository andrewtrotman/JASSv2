


#include <tuple>
#include <string>
#include <utility>

namespace JASS
	{
	class commandline
		{
		private:
			template <typename TYPE>
			class command
				{
				public:
					std::string shortname;
					std::string longname;
					std::string description;
					TYPE parameter;

				public:
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
			template <typename TYPE>
			static void extract(TYPE element)
				{
				std::cout << element.shortname << "=" << element.parameter << "\n";
				}

			static void extract(command<std::string> element)
				{
				std::cout << element.shortname << "->" << element.parameter << "\n";
				}

			static void extract(command<bool> element)
				{
				std::cout << element.shortname << "::" << element.parameter << "\n";
				}

			/*
				Iterating over a tuiple: see here:  https://stackoverflow.com/questions/1198260/iterate-over-tuple
			*/
			template<std::size_t I = 0, typename... Tp>
			inline typename std::enable_if<I == sizeof...(Tp), void>::type
			static for_each(std::tuple<Tp...> &)
				{
				/* Nothing */
				}

			template<std::size_t I = 0, typename... Tp>
			inline typename std::enable_if<I < sizeof...(Tp), void>::type
			static for_each(std::tuple<Tp...>& t)
				{
				extract(std::get<I>(t));
				for_each<I + 1, Tp...>(t);
				}

		public:
			template <typename TYPE>
			static command<TYPE> parameter(const std::string &shortname, const std::string &longname, const std::string &description, TYPE &parameter)
				{
				return command<TYPE>(shortname, longname, description, parameter);
				}

			template <typename... TYPE>
			static void parse(int argc, const char *argv[], std::tuple<TYPE...> &all_parameters)
				{
				for_each(all_parameters);
				}

			static void unittest(void)
				{
				int argc = 2;
				const char *argv[] = {"-sj1", "-sj2"};

				bool jass_v1_index = true;
				std::string jass_v1_string = "Something";
				auto serialiser = commandline::parameter("-sj1", "--serialise_jass_v1", "Serialise the index as a JASS v1 index", jass_v1_index);
				auto str = commandline::parameter("-sj2", "--serialise_jass_v2", "Serialise the index as a JASS v1 index", jass_v1_string);
				auto all_commands = std::make_tuple(serialiser, str);
				commandline::parse(argc, argv, all_commands);
				}
		};
	}

