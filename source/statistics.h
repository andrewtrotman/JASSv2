/*
	STATISTICS.H
	------------
	Copyright (c) 2019 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Implementaiton of various statistics usefile for Information Retrieval (such as the t-test)
	@author Andrew Trotman
	@copyright 2019 Andrew Trotman
*/
#pragma once

#include <math.h>
#include <stdint.h>
#include <algorithm>

#include <vector>
#include <numeric>
#include <algorithm>

namespace JASS
	{
	class statistics
		{
		public:
			/*
				ENUM STATISTICS::TAILS
				----------------------
			*/
			/*!
				@enum tails
				@brief The number of tails to use in a test such as the t-test.
			*/
			enum tails
				{
				ONE_TAILED = 1,
				TWO_TAILED = 2,
				};

		public:
			/*
				STATISTICS::NORMAL_CUMULATIVE_DISTRIBUTION_FUNCTION()
				-----------------------------------------------------
			*/
			/*!
				@brief This subroutine computes the cumulative distribution function value for the normal (gaussian) distribution with mean = 0 and standard deviation = 1.
				@details This distribution is defined for all x and has the probability density function f(x) = (1/sqrt(2*pi))*exp(-x*x/2).
				Converted from the Fortan at: https://www.nist.gov/sites/default/files/documents/itl/sed/NORCDF.f

				References--National Nureau of Standards Applied Mathematics Series 55, 1964, page 932, formula 26.2.17.
				          --Johnson and Kotz, Continuous Univariate Distributions--1, 1970, pages 40-111.
				Written by--James J. Filliben
				            Statistical Engineering Laboratory (205.03)
				            National Bureau of Standards
				            Washington, D. C. 20234
				            Phone:  301-921-2315
				Original version--June      1972.
				Updated         --September 1975.
				Updated         --November  1975.
				C/C++ version   --November 2019 (by Andrew Trotman).

				@param x [in] The single precision value at which the cumulative distribution function is to be evaluated.
				@return The cumulative distribution function value.
			*/
			static double normal_cumulative_distribution_function(double x);


			/*
				STATISTICS::T_CUMULATIVE_DISTRIBUTION_FUNCTION()
				------------------------------------------------
			*/
			/*!
				@brief This subroutine computes the cumulative distribution function value for student's t distribution.
				@details This distribution is defined for all x.  The probability density function is given in the references below.
				Converted from the Fortan at: https://www.nist.gov/sites/default/files/documents/itl/sed/TCDF.f

				References--National Bureau of Standards Applied Mathmatics Series 55, 1964, page 948, formulae 26.7.3 and 26.7.4.
				          --Johnson and Kotz, Continuous Univariate Distributions--2, 1970, pages 94-129.
							 --Federighi, Extended Tables of the Percentage Points of Student's T-Distribution, Journal of the American Statistical Association, 1959, pages 683-688.
				          --Owen, Handbook of Statistical Tables, 1962, pages 27-30.
				          --Pearson and Hartley, Biometrika Tables for Statisticians, volume 1, 1954, pages 132-134.

				Written by--James J. Filliben
				            Statistical Engineering Laboratory (205.03)
				            National Bureau of Standards
				            Washington, D. C. 20234
				            Phone:  301-921-2315

				Original version--June      1972.
				Updated         --May       1974.
				Updated         --September 1975.
				Updated         --November  1975.
				Updated         --October   1976.
				C/C++ version   --November 2019 (by Andrew Trotman).

				@param x [in] The single precision value at which the cumulative distribution function is to be evaluated. x should be non-negative.
				@param nu [in] The integer number of degrees of freedom.
				@return The cumulative distribution function value cdf for the student's t distribution with degrees of freedom parameter = nu.
			*/
			static double t_cumulative_distribution_function(double x, uint32_t nu);

			/*
				STATISTICS::TTEST_PAIRED()
				--------------------------
			*/
			/*!
				@brief Compute the p value of the paired Student's t-test
				@param one [in] the first distribution
				@param two [in] the second distribution
				@return the t-test p value.
			*/
			static double ttest_paired(const std::vector<double> &one, const std::vector<double> &two, statistics::tails tails);


			/*
				STATISTICS::PEARSON_CORRELATION
				-------------------------------
			*/
			/*!
				@brief Compute the Pearson correlation of the two distributions.
				@details Also known as the Pearson correlation coefficient, PCC, Pearson's r, the
				Pearson product-moment correlation coefficient (PPMCC) or the bivariate correlation.
				For details see: https://en.wikipedia.org/wiki/Pearson_correlation_coefficient
				@param one [in] the first distribution
				@param two [in] the second distribution
				@return the Perarson r value.
			*/
			static double pearson_correlation(const std::vector<double> &one, const std::vector<double> &two);

			/*
				STATISTICS::VALUE_TO_RANK()
				---------------------------
			*/
			/*!
				@brief Turn an unordered vector into a vector of ranks.
				@details given a sequence (such as [4.5, 3.2, 9.5]) turn this into a list or ranks (such as [2,1,3]).  As this
				is templeted, the ordinal type of the ranks can be any integer or float type.  The source type is anything
				that can be sorted with sort().  The source type must be copyable.
				This method is used by spearman_correlation() turn scores into ranks before pearson_correlation() is called.
				sources from: https://stackoverflow.com/questions/41184561/how-to-sort-and-rank-a-vector-in-c-without-using-c11
				@param destination [out] the rank orders
				@param source [in] the original distribution
			*/
			template <typename ORDINAL_TYPE, typename TYPE>
			static void value_to_rank(std::vector<ORDINAL_TYPE> &destination, const std::vector<TYPE> &source)
				{
				/*
					Number each element and sort the data
				*/
				std::vector<size_t> index(source.size());
				std::iota(index.begin(), index.end(), 0);
				std::sort(index.begin(), index.end(), [&source](size_t i1, size_t i2){return source[i1] < source[i2];});

				/*
					Return the ranking
				*/
				destination.resize(source.size());
				for (ORDINAL_TYPE which = 0; which < source.size(); which++)
					destination[index[which]] = which;
				}

			/*
				STATISTICS::SPEARMAN_CORRELATION
				--------------------------------
			*/
			/*!
				@brief Compute the Spearmam correlation of the two distributions
				@details Compute Spearman's rank correlation coefficient, also known as Spearman's rho
				For details see: https://en.wikipedia.org/wiki/Spearman%27s_rank_correlation_coefficient
				@param one [in] the first distribution
				@param two [in] the second distribution
				@return the Spearman rho value
			*/
			static double spearman_correlation(const std::vector<double> &one, const std::vector<double> &two);

			/*
				STATISTICS::UNITTEST
				--------------------
			*/
			/*!
				@brief Unit test this class.
			*/
			static void unittest(void);
		};
	}
