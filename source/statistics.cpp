/*
	STATISTICS.CPP
	--------------
	Copyright (c) 2019 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <math.h>
#include <stdio.h>
#include <stdint.h>

#include <vector>

#include "asserts.h"
#include "statistics.h"

namespace JASS
	{
	/*
		STATISTICS::NORMAL_CUMULATIVE_DISTRIBUTION_FUNCTION()
		-----------------------------------------------------
	*/
	double statistics::normal_cumulative_distribution_function(double x)
		{
		double b1 = .319381530;
		double b2 = -0.356563782;
		double b3 = 1.781477937;
		double b4 = -1.8212515978;
		double b5 = 1.330274429;
		double p = .2316419;

		double z = x;

		if (x < 0.0)
			z = -z;
		double t = 1.0 / (1.0 + p * z);
		double cdf = 1.0 - ((0.39894228040143) * exp(-0.5 * z * z)) * (b1 * t + b2 * pow(t, 2) + b3 * pow(t, 3) + b4 * pow(t, 4) + b5 * pow(t, 5));
		if (x < 0.0)
			cdf = 1.0 - cdf;
		return cdf;
		}

	/*
		STATISTICS::T_CUMULATIVE_DISTRIBUTION_FUNCTION()
		------------------------------------------------
	*/
	double statistics::t_cumulative_distribution_function(double x, uint32_t nu)
		{
		uint32_t nucut = 1000;

		double dx = x;
		double anu = nu;
		double dnu = nu;

		/*
			if nu is 3 through 9 and x is more than 3000 standard deviations below the mean,
				set cdf = 0.0 and return.
			if nu is 10 or larger and x is more than 150 standard deviations below the mean,
				set cdf = 0.0 and return.
			if nu is 3 through 9 and x is more than 3000 standard deviations above the mean,
				set cdf = 1.0 and return.
			if nu is 10 or larger and x is more than 150 standard deviations above the mean,
				set cdf = 1.0 and return.
		*/
		if (nu > 2)
			{
			double sd = sqrt(anu / (anu - 2.0));
			double z = x / sd;

			if (nu < 10 && z < -3000.0)
				return 0;
			if (nu >= 10 && z < -150.0)
				return 0;
			if (nu < 10 && z > 3000.0)
				return 1;
			if (nu >= 10 && z > 150.0)
				return 1;
			}
		/*
			distinguish between the small and moderate
			degrees of freedom case versus the
			large degrees of freedom case
		*/
		if (nu < nucut)
			{
			double term;
			double sum;
			/*
				treat the small and moderate degrees of freedom case
				method utilized--exact finite sum
				(see AMS 55, page 948, formulae 26.7.3 and 26.7.4).
			*/
			double c = sqrt(dnu / (dx * dx + dnu));
			double csq = dnu / (dx * dx + dnu);
			double s = dx / sqrt(dx * dx + dnu);
			int64_t imax = nu - 2;
			int64_t ievodd = nu - 2 * (nu / 2);

			int64_t imin;

			if (ievodd == 0)
				{
				sum = 1.0;
				term = 1.0;
				imin = 2;
				}
			else
				{
				sum = c;
				if (nu == 1)
					sum = 0.0;
				term = c;
				imin = 3;
				}

			if (imin <= imax)
				for (int64_t i = imin; i <= imax; i += 2)
					{
					double ai = i;
					term = term * ((ai - 1.0) / ai) * csq;
					sum = sum + term;
					}

			sum = sum * s;
			if (ievodd != 0)
				{
				const double pi = 3.14159265358979;
				sum = (2.0 / pi) * (atan(dx / sqrt(dnu)) + sum);
				}
			return 0.5 + sum / 2.0;
			}
		else
			{
			/*
				treat the large degrees of freedom case.
				method utilized--truncated asymptotic expansion
				(see Johnson and Kotz, volume 2, page 102, formula 10;
				see Federighi, page 687).
			*/
			double b11 = 0.25;
			double b21 = 0.01041666666667;
			double b22 = 3.0;
			double b23 = -7.0;
			double b24 = -5.0;
			double b25 = -3.0;
			double b31 = 0.00260416666667;
			double b32 = 1.0;
			double b33 = -11.0;
			double b34 = 14.0;
			double b35 = 6.0;
			double b36 = -3.0;
			double b37 = -15.0;
			double dconst = 0.3989422804;

			double cdfn = normal_cumulative_distribution_function(x);
			double dcdfn = cdfn;
			double d1 = dx;
			double d3 = pow(dx, 3);
			double d5 = pow(dx, 5);
			double d7 = pow(dx, 7);
			double d9 = pow(dx, 9);
			double d11 = pow(dx, 11);
			double term1 = b11 * (d3 + d1) / dnu;
			double term2 = b21 * (b22 * d7 + b23 * d5 + b24 * d3 + b25 * d1) / pow(dnu, 2);
			double term3 = b31 * (b32 * d11 + b33 * d9 + b34 * d7 + b35 * d5 + b36 * d3 + b37 * d1) / pow(dnu, 3);
			double dcdf = term1 + term2 + term3;
			return dcdfn - (dconst * (exp(-dx * dx / 2.0))) * dcdf;
			}
		}

	/*
		STATISTICS::TTEST_PAIRED()
		--------------------------
	*/
	double statistics::ttest_paired(const std::vector<double> &one, const std::vector<double> &two, statistics::tails tails)
		{
		/*
			Validate that the two samples are the same size
		*/
		int32_t number_of_samples = one.size();
		if (two.size() != one.size() || number_of_samples <= 1)
			return 1;

		/*
			Compute the differences between the two samples and the sum of differences
		*/
		std::vector<double> differences(number_of_samples);
		double mean = 0;
		for (size_t which = 0; which < one.size(); which++)
			{
			differences[which] = one[which] - two[which];
			mean += differences[which];
			}

		/*
			Compute the mean
		*/
		mean /= number_of_samples;

		/*
			Compute the number of degrees of freedom
		*/
		int32_t degrees_of_freedom = number_of_samples - 1;

		/*
			Compute the standard deviation of the differences
		*/
		double variance = 0;
		for (double value : differences)
			variance += (value - mean) * (value - mean);
		double standard_deviation = sqrt(fabs(variance / degrees_of_freedom));

		/*
			Compute the Sampling Error of the Mean Difference.
		*/
		double sample_error = standard_deviation / sqrt(number_of_samples);

		/*
			Computer the t score
		*/
		double t_value = mean / sample_error;

		/*
			Compute the p value of freedom
		*/

		double p = (1 - t_cumulative_distribution_function(fabs(t_value), degrees_of_freedom));
		if (tails == ONE_TAILED)
			return p;
		else
			return p * 2.0;
		}

	/*
		STATISTICS::PEARSON_CORRELATION
		-------------------------------
	*/
	double statistics::pearson_correlation(const std::vector<double> &one, const std::vector<double> &two)
		{
		double sum_x;
		double sum_xx;
		double sum_y;
		double sum_yy;
		double sum_xy;

		sum_x = sum_xx = sum_y = sum_yy = sum_xy = 0;
		size_t n = one.size();
		for (size_t which = 0; which < n; which++)
			{
			sum_x += one[which];
			sum_xx += one[which] * one[which];
			sum_y += two[which];
			sum_yy += two[which] * two[which];
			sum_xy += one[which] * two[which];
			}

		double r = (n * sum_xy  - sum_x * sum_y) / sqrt((n * sum_xx - sum_x * sum_x) * (n * sum_yy - sum_y * sum_y));

		return r;
		}

	/*
		STATISTICS::SPEARMAN_CORRELATION
		--------------------------------
	*/
	double statistics::spearman_correlation(const std::vector<double> &one, const std::vector<double> &two)
		{
		std::vector<double> ordinal_one;
		std::vector<double> ordinal_two;

		value_to_fractional_rank(ordinal_one, one);
		value_to_fractional_rank(ordinal_two, two);

		return pearson_correlation(ordinal_one, ordinal_two);
		}

	/*
		STATISTICS::UNITTEST
		--------------------
	*/
	void statistics::unittest(void)
		{
		/*
			Check the fractional ranks code
		*/
		std::vector<double> duplicate_ranks = {1.0, 1.0, 2.0, 3.0, 3.0, 4.0, 5.0, 5.0, 5.0};
		std::vector<double> ranks;
		value_to_fractional_rank(ranks, duplicate_ranks);

		std::vector<double> correct_ranks = {1.5, 1.5, 3.0, 4.5, 4.5, 6.0, 8.0, 8.0, 8.0};
		JASS_assert(ranks == correct_ranks);

		std::vector<double> distribution_one =
			{
			0.683005338,
			0.691112732, 
			0.007038754,
			0.374320082,
			0.873312345,
			0.365151536,
			0.812889045,
			0.229660635,
			0.071561512,
			0.926276185,
			0.315422657,
			0.171769328,
			0.525381885,
			0.495131092,
			0.809787691,
			0.650849305,
			0.675233717,
			0.604768548,
			0.222848758,
			0.713213154,
			0.172442017
			};

			std::vector<double> distribution_two =
			{
			0.501481838,
			0.054258913,
			0.373114368,
			0.888349007,
			0.608868164,
			0.299764323,
			0.599979771,
			0.262504344,
			0.386125902,
			0.583924624,
			0.293475478,
			0.932529537,
			0.017498429,
			0.026053669,
			0.027980164,
			0.818860004,
			0.668024613,
			0.107967740,
			0.100013727,
			0.985288779,
			0.289748983
			};

		/*
			Check that the values we see are those given by Excel
			First the t-test.
		*/
		double p = ttest_paired(distribution_one, distribution_two, ONE_TAILED);
		JASS_assert((int)(p * 10000) == 1980);
		double q = ttest_paired(distribution_one, distribution_two, TWO_TAILED);
		JASS_assert((int)(q * 10000) == 3961);

		/*
			Then the Pearson Correlation and Spearman correlation
		*/
		double r = pearson_correlation(distribution_one, distribution_two);
		JASS_assert((int)(r * 10000) == 1289);

		double rho = spearman_correlation(distribution_one, distribution_two);
		JASS_assert((int)(rho * 10000) == 1441);

		puts("statistics::PASSED");
		}
	}
