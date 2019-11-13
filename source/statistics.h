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

#include <vector>

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
			*/
			static double ttest_paired(const std::vector<double> &one, const std::vector<double> &two, statistics::tails tails);

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
