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

#include <stdint.h>

#include <vector>

namespace JASS
	{
	class statistics
		{
		public:

/*
C     FROM: https://www.nist.gov/sites/default/files/documents/itl/sed/TCDF.f

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

			static double student_p_value(double t_value, double degrees_of_freedom)
				{
				return 0;
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
				return student_p_value(t_value, number_of_samples - 1);

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
