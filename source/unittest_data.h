/*
	UNITTEST_DATA.H
	---------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Data that can be used and re-used for unittests (and other purposes).
	@author Andrew Trotman
	@copyright 2016 Andrew Trotman
*/
#pragma once

#include <string>

namespace JASS
	{
	/*!
		@brief Global data used for unit testing.
	*/
	class unittest_data
		{
		public:
			static std::string ten_documents;					///< Ten TREC formatted documents with ten terms (ten .. one) where each term occurs it's count number of times.
			static std::string ten_document_1;					///< The first of the 10 ten_documents.
			static std::string ten_document_1_key;				///< The primary key of the first of the 10 ten_documents.
			static std::string ten_document_2;					///< The second of the 10 ten_documents.
			static std::string ten_document_2_key;				///< The primary bkey of the second of the 10 ten_documents.
			static std::string ten_document_3;					///< The third of the 10 ten_documents.
			static std::string ten_document_3_key;				///< The primary key of the third of the 10 ten_documents.
			static std::string ten_document_4;					///< The fourth of the 10 ten_documents.
			static std::string ten_document_4_key;				///< The primary key of the fourth of the 10 ten_documents.
			static std::string ten_document_5;					///< The fifth of the 10 ten_documents.
			static std::string ten_document_5_key;				///< The primary key of the fifth of the 10 ten_documents.
			static std::string ten_document_6;					///< The sixth of the 10 ten_documents.
			static std::string ten_document_6_key;				///< The primary key of the sixth of the 10 ten_documents.
			static std::string ten_document_7;					///< The seventh of the 10 ten_documents.
			static std::string ten_document_7_key;				///< The primary key of the seventh of the 10 ten_documents.
			static std::string ten_document_8;					///< The eightth of the 10 ten_documents.
			static std::string ten_document_8_key;				///< The primary key of the eightth of the 10 ten_documents.
			static std::string ten_document_9;					///< The ninth of the 10 ten_documents.
			static std::string ten_document_9_key;				///< The primary key of the ninth of the 10 ten_documents.
			static std::string ten_document_10;					///< The tenth of the 10 ten_documents.
			static std::string ten_document_10_key;			///< The primary key of the tenth of the 10 ten_documents.
			static std::string ten_document_11_broken;		///< a broken document that has a DOC and DOCNO but no close tags
			static std::string ten_document_12_broken;		///< a broken document that has a DOC and DOCNO but no close DOC tag
			static std::string ten_document_13_broken;		///< a broken document that has a DOC and DOCNO but no close DOCNO tag
			static std::string three_documents_asymetric;	///< Three documents with terms occuring an asymetric number of times

			static const std::string five_trec_assessments; 			///< Five example TREC assessments.
			static const std::string ten_price_assessments_prices;	///< The prices of ten example items.
			static const std::string ten_price_assessments;				///< The assessments of ten example items.
		};
	}
