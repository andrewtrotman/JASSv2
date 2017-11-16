/*
	UNITTEST_DATA.CPP
	-----------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include "unittest_data.h"

namespace JASS
	{
	/*
		UNITTEST_DATA_TEN_DOCUMENTS
		---------------------------
	*/
	std::string unittest_data::ten_documents = "<DOC><DOCNO> 1 </DOCNO>ten</DOC><DOC><DOCNO>2</DOCNO>ten nine</DOC><DOC><DOCNO>3</DOCNO>ten nine eight</DOC><DOC><DOCNO>4</DOCNO>ten nine eight seven</DOC><DOC><DOCNO>5</DOCNO>ten nine eight seven six</DOC><DOC><DOCNO>6</DOCNO>ten nine eight seven six five</DOC><DOC><DOCNO>7</DOCNO>ten nine eight seven six five four</DOC><DOC><DOCNO>8</DOCNO>ten nine eight seven six five four three</DOC><DOC><DOCNO>9</DOCNO>ten nine eight seven six five four three two</DOC><DOC><DOCNO>10</DOCNO>ten nine eight seven six five four three two one</DOC>";

	/*
		UNITTEST_DATA_THREE_DOCUMENTS_ASYMETRIC
		---------------------------------------
	*/
	std::string unittest_data::three_documents_asymetric = "<DOC><DOCNO>1</DOCNO>one two</DOC><DOC><DOCNO>2</DOCNO>one two two</DOC><DOC><DOCNO>3</DOCNO>one two two</DOC>";

	/*
		UNITTEST_DATA_TEN_DOCUMENTS_X
		-----------------------------
	*/
	std::string unittest_data::ten_document_1 = "<DOC><DOCNO> 1 </DOCNO>ten</DOC>";
	std::string unittest_data::ten_document_1_key = "1";
	std::string unittest_data::ten_document_2 = "<DOC><DOCNO>2</DOCNO>ten nine</DOC>";
	std::string unittest_data::ten_document_2_key = "2";
	std::string unittest_data::ten_document_3 = "<DOC><DOCNO>3</DOCNO>ten nine eight</DOC>";
	std::string unittest_data::ten_document_3_key = "3";
	std::string unittest_data::ten_document_4 = "<DOC><DOCNO>4</DOCNO>ten nine eight seven</DOC>";
	std::string unittest_data::ten_document_4_key = "4";
	std::string unittest_data::ten_document_5 = "<DOC><DOCNO>5</DOCNO>ten nine eight seven six</DOC>";
	std::string unittest_data::ten_document_5_key = "5";
	std::string unittest_data::ten_document_6 = "<DOC><DOCNO>6</DOCNO>ten nine eight seven six five</DOC>";
	std::string unittest_data::ten_document_6_key = "6";
	std::string unittest_data::ten_document_7 = "<DOC><DOCNO>7</DOCNO>ten nine eight seven six five four</DOC>";
	std::string unittest_data::ten_document_7_key = "7";
	std::string unittest_data::ten_document_8 = "<DOC><DOCNO>8</DOCNO>ten nine eight seven six five four three</DOC>";
	std::string unittest_data::ten_document_8_key = "8";
	std::string unittest_data::ten_document_9 = "<DOC><DOCNO>9</DOCNO>ten nine eight seven six five four three two</DOC>";
	std::string unittest_data::ten_document_9_key = "9";
	std::string unittest_data::ten_document_10 ="<DOC><DOCNO>10</DOCNO>ten nine eight seven six five four three two one</DOC>";
	std::string unittest_data::ten_document_10_key = "10";
	std::string unittest_data::ten_document_11_broken ="<DOC><DOCNO>11";
	std::string unittest_data::ten_document_12_broken ="<DOC><DOCNO>12</DOCNO>twelve eleven ten nine eight seven six five four three two one";
	std::string unittest_data::ten_document_13_broken ="<DOC><DOCNO>13 thirteen twelve eleven ten nine eight seven six five four three two one</DOC>";
	}
