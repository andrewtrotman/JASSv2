/*
	COMPRESS_INTEGER_ALL.H
	----------------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief A data structure representing all the integer compression schemes known by JASS
	@author Andrew Trotman
	@copyright 2016 Andrew Trotman
*/
#pragma once

#include <tuple>
#include <string>

#include "commandline.h"
#include "compress_integer.h"

namespace JASS
	{
	class compress_integer_all
		{
		public:
			static std::vector<compress_integer_all> compress_integer_all_compressors;
			
		public:
			std::string shortname;
			std::string longname;
			std::string description;
			compress_integer *codex;
			
		private:
			// https://stackoverflow.com/questions/28410697/c-convert-vector-to-tuple

			template <typename TYPE, std::size_t... Indices>
			auto vectorToTupleHelper(const std::vector<TYPE>& v, std::index_sequence<Indices...>)
			{
			return std::make_tuple((v[Indices]+1)...);
			}

			template <std::size_t N, typename TYPE>
			auto vectorToTuple(const std::vector<TYPE>& v)
			{
			assert(v.size() >= N);
			return vectorToTupleHelper(v, std::make_index_sequence<N>());
			}
		
		public:
#ifdef NEVER
			static auto codexlist_as_parameterlist(std::vector<char> &option)
				{
				size_t which;
				auto command_line_parameters = std::make_tuple();
				
				for (const auto &row : compress_integer_all_compressors)
					{
					auto next_row = std::make_tuple(commandline::parameter(row.shortname, row.longname, std::string("Compress postings lists using:") + row.description, option[which]));
					command_line_parameters = std::tuple_cat(command_line_parameters, next_row);
					which++;
					}
				}
#endif
		};

	extern std::vector<compress_integer_all> compress_integer_all_compressors;
	}
