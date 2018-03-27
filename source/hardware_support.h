/*
	HARDWARE_SUPPORT.H
	------------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Get and store details of the CPU's feature set.
	@author Andrew Trotman
	@copyright 2018 Andrew Trotman
*/
#pragma once

#ifndef _MSC_VER
	#include <cpuid.h>
#endif

#include <sstream>
#include <iostream>


namespace JASS
	{
	/*
		Forward declarations.
	*/
	class hardware_support;
	std::ostream &operator<<(std::ostream &stream, const hardware_support &data);

	/*
		CLASS HARDWARE_SUPPORT
		----------------------
	*/	
	/*!
		@brief Get and store details of the CPU's feature set.
		@details Addapted from: https://stackoverflow.com/questions/6121792/how-to-check-if-a-cpu-supports-the-sse3-instruction-set?rq=1
	*/
	class hardware_support
		{
		public:
			//  Misc.
			bool MMX;		///< MMX instructions
			bool x64;		///< x64 i.e. 64-bit processor
			bool ABM;      	///< Advanced bit manipulation
			bool RDRAND;	///< Hardware random number generator
			bool BMI1;		///< Bit manipulation instructions 1
			bool BMI2;		///< Bit manipulation instructions 2
			bool ADX;		///< Multi-precision add-carry instruction extensions
			bool PREFETCHWT1;	///< Prefetch instructions

			//  SIMD: 128-bit
			bool SSE;		///< SSE instructions
			bool SSE2;		///< SSE 2 instructions
			bool SSE3;		///< SSE 3 instructions
			bool SSSE3;		///< SSSE 3 instructions
			bool SSE41;		///< SSE 4.1 instructions
			bool SSE42;		///< SSE 4.2 instructions
			bool SSE4a;		///< SSE 4a  instructions
			bool AES;		///< Advanced Encryption Standard New Instructions
			bool SHA;		///< Secure Hash Algorithm (SHA-1 and SHA-256)

			//  SIMD: 256-bit
			bool AVX;		///< AVX instructions
			bool XOP;		///< SSE eXtended Operations
			bool FMA3;		///< Fused multiply add with 3 parameters
			bool FMA4;		///< Fused multiply add with 4 parameters
			bool AVX2;		///< AVX 2 instrucitons

			//  SIMD: 512-bit
			bool AVX512F;    ///<  AVX512 Foundation
			bool AVX512CD;   ///<  AVX512 Conflict Detection
			bool AVX512PF;   ///<  AVX512 Prefetch
			bool AVX512ER;   ///<  AVX512 Exponential + Reciprocal
			bool AVX512VL;   ///<  AVX512 Vector Length Extensions
			bool AVX512BW;   ///<  AVX512 Byte + Word
			bool AVX512DQ;   ///<  AVX512 Doubleword + Quadword
			bool AVX512IFMA; ///<  AVX512 Integer 52-bit Fused Multiply-Add
			bool AVX512VBMI; ///<  AVX512 Vector Byte Manipulation Instructions

		protected:

			/*
				HARDWARE_SUPPORT::CPUID()
				-------------------------
			*/
			/*!
				@brief Execute a CPUID instruction.
				@param info [out] the CPUID data.
				@param info_type [in] the type of informaiton being sought.
			*/
			void cpuid(int info[4], int info_type)
				{
				#ifdef _MSC_VER
					__cpuidex(info, info_type, 0);
				#else
					__cpuid_count(info_type, 0, info[0], info[1], info[2], info[3]);
				#endif
				}

		public:
			/*
				HARDWARE_SUPPORT::HARDWARE_SUPPORT()
				------------------------------------
			*/
			/*!
				@brief Constructor.  Ask the CPU for details of what it is.
			*/
			hardware_support()
				{
				int info[4];
				cpuid(info, 0);
				int nIds = info[0];

				cpuid(info, 0x80000000);
				unsigned nExIds = info[0];

				//  Detect Features
				if (nIds >= 0x00000001)
					{
					cpuid(info,0x00000001);
					MMX    = (info[3] & ((int)1 << 23)) != 0;
					SSE    = (info[3] & ((int)1 << 25)) != 0;
					SSE2   = (info[3] & ((int)1 << 26)) != 0;
					SSE3   = (info[2] & ((int)1 <<  0)) != 0;

					SSSE3  = (info[2] & ((int)1 <<  9)) != 0;
					SSE41  = (info[2] & ((int)1 << 19)) != 0;
					SSE42  = (info[2] & ((int)1 << 20)) != 0;
					AES    = (info[2] & ((int)1 << 25)) != 0;

					AVX    = (info[2] & ((int)1 << 28)) != 0;
					FMA3   = (info[2] & ((int)1 << 12)) != 0;

					RDRAND = (info[2] & ((int)1 << 30)) != 0;
					}
				if (nIds >= 0x00000007)
					{
					cpuid(info,0x00000007);
					AVX2   = (info[1] & ((int)1 <<  5)) != 0;

					BMI1        = (info[1] & ((int)1 <<  3)) != 0;
					BMI2        = (info[1] & ((int)1 <<  8)) != 0;
					ADX         = (info[1] & ((int)1 << 19)) != 0;
					SHA         = (info[1] & ((int)1 << 29)) != 0;
					PREFETCHWT1 = (info[2] & ((int)1 <<  0)) != 0;

					AVX512F     = (info[1] & ((int)1 << 16)) != 0;
					AVX512CD    = (info[1] & ((int)1 << 28)) != 0;
					AVX512PF    = (info[1] & ((int)1 << 26)) != 0;
					AVX512ER    = (info[1] & ((int)1 << 27)) != 0;
					AVX512VL    = (info[1] & ((int)1 << 31)) != 0;
					AVX512BW    = (info[1] & ((int)1 << 30)) != 0;
					AVX512DQ    = (info[1] & ((int)1 << 17)) != 0;
					AVX512IFMA  = (info[1] & ((int)1 << 21)) != 0;
					AVX512VBMI  = (info[2] & ((int)1 <<  1)) != 0;
					}
				if (nExIds >= 0x80000001)
					{
					cpuid(info,0x80000001);
					x64   = (info[3] & ((int)1 << 29)) != 0;
					ABM   = (info[2] & ((int)1 <<  5)) != 0;
					SSE4a = (info[2] & ((int)1 <<  6)) != 0;
					FMA4  = (info[2] & ((int)1 << 16)) != 0;
					XOP   = (info[2] & ((int)1 << 11)) != 0;
					}
				}


			/*
				HARDWARE_SUPPORT::UNITTEST()
				----------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void)
				{
				std::ostringstream data;
				hardware_support hardware;

				data << hardware;
								   
				JASS_assert(hardware.x64 == true);
				puts("hardware_support::PASSED");
				}
		};
	/*
		OPERATOR<<()
		------------
	*/
	/*!
		@brief Dump the contents of an object.
		@param stream [in] The stream to write to.
		@param data [in] The data to write.
		@return The stream once the data has been written.
	*/
	inline std::ostream &operator<<(std::ostream &stream, const hardware_support &data)
		{
		stream << "MMX        :" << data.MMX << "\n";
		stream << "x64        :" << data.x64 << "\n";
		stream << "ABM        :" << data.ABM << "\n";
		stream << "RDRAND     :" << data.RDRAND << "\n";
		stream << "BMI1       :" << data.BMI1 << "\n";
		stream << "BMI2       :" << data.BMI2 << "\n";
		stream << "ADX        :" << data.ADX << "\n";
		stream << "PREFETCHWT1:" << data.PREFETCHWT1 << "\n";

		stream << "SSE        :" << data.SSE << "\n";
		stream << "SSE2       :" << data.SSE2 << "\n";
		stream << "SSE3       :" << data.SSE3 << "\n";
		stream << "SSSE3      :" << data.SSSE3 << "\n";
		stream << "SSE41      :" << data.SSE41 << "\n";
		stream << "SSE42      :" << data.SSE42 << "\n";
		stream << "SSE4a      :" << data.SSE4a << "\n";
		stream << "AES        :" << data.AES << "\n";
		stream << "SHA        :" << data.SHA << "\n";

		stream << "AVX        :" << data.AVX << "\n";
		stream << "XOP        :" << data.XOP << "\n";
		stream << "FMA3       :" << data.FMA3 << "\n";
		stream << "FMA4       :" << data.FMA4 << "\n";
		stream << "AVX2       :" << data.AVX2 << "\n";

		stream << "AVX512F    :" << data.AVX512F << "\n";
		stream << "AVX512CD   :" << data.AVX512CD << "\n";
		stream << "AVX512PF   :" << data.AVX512PF << "\n";
		stream << "AVX512ER   :" << data.AVX512ER << "\n";
		stream << "AVX512VL   :" << data.AVX512VL << "\n";
		stream << "AVX512BW   :" << data.AVX512BW << "\n";
		stream << "AVX512DQ   :" << data.AVX512DQ << "\n";
		stream << "AVX512IFMA :" << data.AVX512IFMA << "\n";
		stream << "AVX512VBMI :" << data.AVX512VBMI << "\n";

		return stream;
		}
	}
