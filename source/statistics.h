/*
	STATISTICS.H
	------------
	Copyright (c) 2019 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Implementaiton of various statistics usefile for Information Retrieval (such as t-test)
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
				NORMAL_CUMULATIVE_DISTRIBUTION_FUNCTION()
				-----------------------------------------
			*/
			/*!
				@brief This subroutine computes the cumulative distribution function value for the normal (gaussian) distribution with mean = 0 and standard deviation = 1.
				@details This distribution is defined for all x and has the probability density function f(x) = (1/sqrt(2*pi))*exp(-x*x/2).
				Converted from the fortan at: https://www.nist.gov/sites/default/files/documents/itl/sed/NORCDF.f

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
			static double normal_cumulative_distribution_function(double x)
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
C     FROM: https://www.nist.gov/sites/default/files/documents/itl/sed/TCDF.f
C
      SUBROUTINE TCDF(X,NU,CDF)
C
C     PURPOSE--THIS SUBROUTINE COMPUTES THE CUMULATIVE DISTRIBUTION
C              FUNCTION VALUE FOR STUDENT'S T DISTRIBUTION
C              WITH INTEGER DEGREES OF FREEDOM PARAMETER = NU.
C              THIS DISTRIBUTION IS DEFINED FOR ALL X.
C              THE PROBABILITY DENSITY FUNCTION IS GIVEN
C              IN THE REFERENCES BELOW.
C     INPUT  ARGUMENTS--X      = THE SINGLE PRECISION VALUE AT
C                                WHICH THE CUMULATIVE DISTRIBUTION
C                                FUNCTION IS TO BE EVALUATED.
C                                X SHOULD BE NON-NEGATIVE.
C                     --NU     = THE INTEGER NUMBER OF DEGREES
C                                OF FREEDOM.
C                                NU SHOULD BE POSITIVE.
C     OUTPUT ARGUMENTS--CDF    = THE SINGLE PRECISION CUMULATIVE
C                                DISTRIBUTION FUNCTION VALUE.
C     OUTPUT--THE SINGLE PRECISION CUMULATIVE DISTRIBUTION
C             FUNCTION VALUE CDF FOR THE STUDENT'S T DISTRIBUTION
C             WITH DEGREES OF FREEDOM PARAMETER = NU.
C     PRINTING--NONE UNLESS AN INPUT ARGUMENT ERROR CONDITION EXISTS.
C     RESTRICTIONS--NU SHOULD BE A POSITIVE INTEGER VARIABLE.
C     OTHER DATAPAC   SUBROUTINES NEEDED--NORCDF.
C     FORTRAN LIBRARY SUBROUTINES NEEDED--DSQRT, DATAN.
C     MODE OF INTERNAL OPERATIONS--DOUBLE PRECISION.
C     LANGUAGE--ANSI FORTRAN.
C     REFERENCES--NATIONAL BUREAU OF STANDARDS APPLIED MATHMATICS
C                 SERIES 55, 1964, PAGE 948, FORMULAE 26.7.3 AND 26.7.4.
C               --JOHNSON AND KOTZ, CONTINUOUS UNIVARIATE
C                 DISTRIBUTIONS--2, 1970, PAGES 94-129.
C               --FEDERIGHI, EXTENDED TABLES OF THE
C                 PERCENTAGE POINTS OF STUDENT'S
C                 T-DISTRIBUTION, JOURNAL OF THE
C                 AMERICAN STATISTICAL ASSOCIATION,
C                 1959, PAGES 683-688.
C               --OWEN, HANDBOOK OF STATISTICAL TABLES,
C                 1962, PAGES 27-30.
C               --PEARSON AND HARTLEY, BIOMETRIKA TABLES
C                 FOR STATISTICIANS, VOLUME 1, 1954,
C                 PAGES 132-134.
C     WRITTEN BY--JAMES J. FILLIBEN
C                 STATISTICAL ENGINEERING LABORATORY (205.03)
C                 NATIONAL BUREAU OF STANDARDS
C                 WASHINGTON, D. C. 20234
C                 PHONE:  301-921-2315
C     ORIGINAL VERSION--JUNE      1972.
C     UPDATED         --MAY       1974.
C     UPDATED         --SEPTEMBER 1975.
C     UPDATED         --NOVEMBER  1975.
C     UPDATED         --OCTOBER   1976.
C
C---------------------------------------------------------------------
C
      DOUBLE PRECISION DX,DNU,PI,C,CSQ,S,SUM,TERM,AI
      DOUBLE PRECISION DSQRT,DATAN
      DOUBLE PRECISION DCONST
      DOUBLE PRECISION TERM1,TERM2,TERM3
      DOUBLE PRECISION DCDFN
      DOUBLE PRECISION DCDF
      DOUBLE PRECISION B11
      DOUBLE PRECISION B21,B22,B23,B24,B25
      DOUBLE PRECISION B31,B32,B33,B34,B35,B36,B37
      DOUBLE PRECISION D1,D3,D5,D7,D9,D11
      DATA NUCUT/1000/
      DATA PI/3.14159265358979D0/
      DATA DCONST/0.3989422804D0/
      DATA B11/0.25D0/
      DATA B21/0.01041666666667D0/
      DATA B22,B23,B24,B25/3.0D0,-7.0D0,-5.0D0,-3.0D0/
      DATA B31/0.00260416666667D0/
      DATA B32,B33,B34,B35,B36,B37/1.0D0,-11.0D0,14.0D0,6.0D0,
     1                            -3.0D0,-15.0D0/
C
      IPR=6
C
C     CHECK THE INPUT ARGUMENTS FOR ERRORS
C
      IF(NU.LE.0)GOTO50
      GOTO90
   50 WRITE(IPR,15)
      WRITE(IPR,47)NU
      CDF=0.0
      RETURN
   90 CONTINUE
   15 FORMAT(1H , 91H***** FATAL ERROR--THE SECOND INPUT ARGUMENT TO THE
     1 TCDF   SUBROUTINE IS NON-POSITIVE *****)
   47 FORMAT(1H , 35H***** THE VALUE OF THE ARGUMENT IS ,I8   ,6H *****)
C
C-----START POINT-----------------------------------------------------
C
      DX=X
      ANU=NU
      DNU=NU
C
C     IF NU IS 3 THROUGH 9 AND X IS MORE THAN 3000
C     STANDARD DEVIATIONS BELOW THE MEAN,
C     SET CDF = 0.0 AND RETURN.
C     IF NU IS 10 OR LARGER AND X IS MORE THAN 150
C     STANDARD DEVIATIONS BELOW THE MEAN,
C     SET CDF = 0.0 AND RETURN.
C     IF NU IS 3 THROUGH 9 AND X IS MORE THAN 3000
C     STANDARD DEVIATIONS ABOVE THE MEAN,
C     SET CDF = 1.0 AND RETURN.
C     IF NU IS 10 OR LARGER AND X IS MORE THAN 150
C     STANDARD DEVIATIONS ABOVE THE MEAN,
C     SET CDF = 1.0 AND RETURN.
C
      IF(NU.LE.2)GOTO109
      SD=SQRT(ANU/(ANU-2.0))
      Z=X/SD
      IF(NU.LT.10.AND.Z.LT.-3000.0)GOTO107
      IF(NU.GE.10.AND.Z.LT.-150.0)GOTO107
      IF(NU.LT.10.AND.Z.GT.3000.0)GOTO108
      IF(NU.GE.10.AND.Z.GT.150.0)GOTO108
      GOTO109
  107 CDF=0.0
      RETURN
  108 CDF=1.0
      RETURN
  109 CONTINUE
C
C     DISTINGUISH BETWEEN THE SMALL AND MODERATE
C     DEGREES OF FREEDOM CASE VERSUS THE
C     LARGE DEGREES OF FREEDOM CASE
C
      IF(NU.LT.NUCUT)GOTO110
      GOTO250
C
C     TREAT THE SMALL AND MODERATE DEGREES OF FREEDOM CASE
C     METHOD UTILIZED--EXACT FINITE SUM
C     (SEE AMS 55, PAGE 948, FORMULAE 26.7.3 AND 26.7.4).
C
  110 CONTINUE
      C=DSQRT(DNU/(DX*DX+DNU))
      CSQ=DNU/(DX*DX+DNU)
      S=DX/DSQRT(DX*DX+DNU)
      IMAX=NU-2
      IEVODD=NU-2*(NU/2)
      IF(IEVODD.EQ.0)GOTO120
C
      SUM=C
      IF(NU.EQ.1)SUM=0.0D0
      TERM=C
      IMIN=3
      GOTO130
C
  120 SUM=1.0D0
      TERM=1.0D0
      IMIN=2
C
  130 IF(IMIN.GT.IMAX)GOTO160
      DO100I=IMIN,IMAX,2
      AI=I
      TERM=TERM*((AI-1.0D0)/AI)*CSQ
      SUM=SUM+TERM
  100 CONTINUE
C
  160 SUM=SUM*S
      IF(IEVODD.EQ.0)GOTO170
      SUM=(2.0D0/PI)*(DATAN(DX/DSQRT(DNU))+SUM)
  170 CDF=0.5D0+SUM/2.0D0
      RETURN
C
C     TREAT THE LARGE DEGREES OF FREEDOM CASE.
C     METHOD UTILIZED--TRUNCATED ASYMPTOTIC EXPANSION
C     (SEE JOHNSON AND KOTZ, VOLUME 2, PAGE 102, FORMULA 10;
C     SEE FEDERIGHI, PAGE 687).
C
  250 CONTINUE
      CALL NORCDF(X,CDFN)
      DCDFN=CDFN
      D1=DX
      D3=DX**3
      D5=DX**5
      D7=DX**7
      D9=DX**9
      D11=DX**11
      TERM1=B11*(D3+D1)/DNU
      TERM2=B21*(B22*D7+B23*D5+B24*D3+B25*D1)/(DNU**2)
      TERM3=B31*(B32*D11+B33*D9+B34*D7+B35*D5+B36*D3+B37*D1)/(DNU**3)
      DCDF=TERM1+TERM2+TERM3
      DCDF=DCDFN-(DCONST*(DEXP(-DX*DX/2.0D0)))*DCDF
      CDF=DCDF
      RETURN
C
      END
*/

/*
			C     INPUT  ARGUMENTS--X      = THE SINGLE PRECISION VALUE AT
			C                                WHICH THE CUMULATIVE DISTRIBUTION
			C                                FUNCTION IS TO BE EVALUATED.
			C                                X SHOULD BE NON-NEGATIVE.
			C                     --NU     = THE INTEGER NUMBER OF DEGREES
			C                                OF FREEDOM.
			C                                NU SHOULD BE POSITIVE.
			C     OUTPUT ARGUMENTS--CDF    = THE SINGLE PRECISION CUMULATIVE
			C                                DISTRIBUTION FUNCTION VALUE.
*/
static double TCDF(double X, int NU)
	{
	double  DX,DNU,C,CSQ,S,SUM,TERM,AI;
	double  TERM1,TERM2,TERM3;
	double  DCDFN;
	double  DCDF;
	double  D1,D3,D5,D7,D9,D11;

int NUCUT = 1000;
double PI = 3.14159265358979;
double DCONST = 0.3989422804;
double B11 = 0.25;
double B21 = 0.01041666666667;
double B22 = 3.0;
double B23 = -7.0;
double B24 = -5.0;
double B25 = -3.0;
double B31 = 0.00260416666667;
double B32 = 1.0;
double B33 = -11.0;
double B34 = 14.0;
double B35 = 6.0;
double B36 = -3.0;
double B37 = -15.0;


/*
			C
			C     CHECK THE INPUT ARGUMENTS FOR ERRORS
			C
*/
					if (NU <= 0)
						return -1;

/*
			C
			C-----START POINT-----------------------------------------------------
			C
*/
					DX = X;
					double ANU = NU;
					DNU = NU;
/*
			C
			C     IF NU IS 3 THROUGH 9 AND X IS MORE THAN 3000
			C     STANDARD DEVIATIONS BELOW THE MEAN,
			C     SET CDF = 0.0 AND RETURN.
			C     IF NU IS 10 OR LARGER AND X IS MORE THAN 150
			C     STANDARD DEVIATIONS BELOW THE MEAN,
			C     SET CDF = 0.0 AND RETURN.
			C     IF NU IS 3 THROUGH 9 AND X IS MORE THAN 3000
			C     STANDARD DEVIATIONS ABOVE THE MEAN,
			C     SET CDF = 1.0 AND RETURN.
			C     IF NU IS 10 OR LARGER AND X IS MORE THAN 150
			C     STANDARD DEVIATIONS ABOVE THE MEAN,
			C     SET CDF = 1.0 AND RETURN.
			C
*/

					if (NU > 2)
						{
						double SD = sqrt(ANU / (ANU - 2.0));
						double Z = X / SD;

						if (NU < 10 && Z < -3000.0)
							return 0;
						if (NU >= 10 && Z < -150.0)
							return 0;
						if (NU < 10 && Z > 3000.0)
							return 1;
						if (NU >= 10 && Z > 150.0)
							return 1;
						}
/*
			C
			C     DISTINGUISH BETWEEN THE SMALL AND MODERATE
			C     DEGREES OF FREEDOM CASE VERSUS THE
			C     LARGE DEGREES OF FREEDOM CASE
			C
*/
					if (NU < NUCUT)
						{
/*
			C
			C     TREAT THE SMALL AND MODERATE DEGREES OF FREEDOM CASE
			C     METHOD UTILIZED--EXACT FINITE SUM
			C     (SEE AMS 55, PAGE 948, FORMULAE 26.7.3 AND 26.7.4).
			C
*/
					C = sqrt(DNU / (DX * DX + DNU));
					CSQ = DNU / (DX * DX + DNU);
					S = DX / sqrt(DX * DX + DNU);
					int IMAX = NU - 2;
					int IEVODD = NU - 2 * (NU / 2);

					int IMIN;

					if (IEVODD == 0)
						{
						SUM = 1.0;
						TERM = 1.0;
						IMIN = 2;
						}
					else
						{
						SUM = C;
						if (NU == 1)
							SUM = 0.0;
						TERM = C;
						IMIN = 3;
						}

			      if (IMIN <= IMAX)
						for (int I = IMIN; I <= IMAX; I += 2)
							{
							AI = I;
							TERM = TERM * ((AI - 1.0) / AI) * CSQ;
							SUM = SUM + TERM;
							}



			      SUM = SUM * S;
					if (IEVODD != 0)
						SUM = (2.0 / PI) * (atan(DX / sqrt(DNU)) + SUM);
			      double CDF = 0.5 + SUM / 2.0;
					return CDF;
			}
			else
			{
			/*
			C
			C     TREAT THE LARGE DEGREES OF FREEDOM CASE.
			C     METHOD UTILIZED--TRUNCATED ASYMPTOTIC EXPANSION
			C     (SEE JOHNSON AND KOTZ, VOLUME 2, PAGE 102, FORMULA 10;
			C     SEE FEDERIGHI, PAGE 687).
			C
			*/
					double CDFN = normal_cumulative_distribution_function(X);
					DCDFN = CDFN;
					D1 = DX;
					D3 = pow(DX, 3);
					D5 = pow(DX, 5);
					D7 = pow(DX, 7);
					D9 = pow(DX, 9);
					D11 = pow(DX, 11);
					TERM1 = B11 * (D3 + D1) / DNU;
					TERM2 = B21 * (B22 * D7 + B23 * D5 + B24 * D3 + B25 * D1) / pow(DNU, 2);
					TERM3 = B31 * (B32 * D11 + B33 * D9 + B34 * D7 + B35 * D5 + B36 * D3 + B37 * D1) / pow(DNU, 3);
					DCDF = TERM1 + TERM2 + TERM3;
					DCDF = DCDFN - (DCONST * (exp(-DX * DX / 2.0))) * DCDF;
					double CDF = DCDF;
					return CDF;
				}
}


			static double ttest_paired(std::vector<double> &one, std::vector<double> &two, uint8_t tails)
				{
				/*
					Validate that the two samples are the same size
				*/
				size_t number_of_samples = one.size();
				if (two.size() != number_of_samples)
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
					Compute the standard deviation of the differences
				*/
				double variance = 0;
				for (double value : differences)
					variance += (value - mean) * (value - mean);
				double standard_deviation = sqrt(fabs(variance / (number_of_samples - 1)));

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

				return 1 - TCDF(t_value, number_of_samples - 1);

				/*
				degrees of freedom = number_of_samples - 1
				From Boost, but we need to compute it without Boost.

				students_t dist(v);
				double q = cdf(complement(dist, fabs(t_stat)));
				cout << setw(55) << left << "Probability that difference is due to chance" << "=  "
					<< setprecision(3) << scientific << 2 * q << "\n\n";

				*/

				}

			static void unittest(void)
				{
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

				double p = ttest_paired(distribution_one, distribution_two, 2);
				// p should be 0.1980
				std::cout << "P value:" << p << '\n';
				}
		};
	}
