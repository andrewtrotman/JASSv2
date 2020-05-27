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
//#define HARDWARE_SUPPORT_STAND_ALONE

#ifndef HARDWARE_SUPPORT_STAND_ALONE
	#pragma once
#endif

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#ifdef HARDWARE_SUPPORT_STAND_ALONE
	#include <assert.h>
	#define JASS_assert(x) assert(x)
#endif

#ifdef _MSC_VER
	#include <intrin.h>
	#include <windows.h>
#else
	#include <cpuid.h>
	#include <unistd.h>
#endif

#include <sstream>
#include <iostream>

#if defined(_MSC_VER) || defined(HARDWARE_SUPPORT_STAND_ALONE)
	#define RUNNING_ON_VALGRIND (0)
#else
	#include "../external/valgrind/valgrind.h"
#endif

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
			bool FP;				///< Has Floating Point on chip
			bool MMX;			///< MMX instructions
			bool x64;			///< x64 i.e. 64-bit processor
			bool ABM;			///< Advanced bit manipulation
			bool RDRAND;		///< Hardware random number generator
			bool BMI1;			///< Bit manipulation instructions 1
			bool BMI2;			///< Bit manipulation instructions 2
			bool ADX;			///< Multi-precision add-carry instruction extensions
			bool PREFETCHWT1;	///< Prefetch instructions (Xeon Phi(
			bool CX8; 			///< CMPXCHG8B - compare exchange 8-byte values
			bool CMPXCHG16B;	///< compare exchange 16-byte values
			bool CMOV;			///< Conditional move CMOV instructions
			bool CLFSH;			///< CLFLUSH instruction
			bool PCLMULQDQ;	///< Carryless Multiplication
			bool MOVBE;			///< MOVBE instructon (convert endianness)
			bool POPCNT;		///< popcount.  Count set bits
			bool XSAVE;			///< Save processor state
			bool OSXSAVE;		///< Save processor state
			bool F16C;			///< 16-bit float conversion
			bool FSGSBASE;		///< F segment and a G segment
			bool SGX;			///< Software Guard Extensions
			bool HLE;			///< Hardware Lock Elision (Transactional Memory)
			bool RTM;			///< Restricted Transactional Memory
			bool RDSEED;		///< RDSEED
			bool CLFLUSHOPT;	///< Cache flush
			bool CLWB;			///< Cache flush
			bool RDPID;		    ///< RDPID
			bool CLDEMOTE;     ///< cache line demote
			bool MOVDIRI;		 ///< MOVDIRI
			bool MOVDIR64B;    ///< MOVDIR64B

			//  SIMD: 128-bit
			bool FXSR;		///< save and restore FP, MMX, SSE registers
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
			bool AVX512PF;   ///<  AVX512 Prefetch (Xeon Phi)
			bool AVX512ER;   ///<  AVX512 Exponential + Reciprocal (Xeon Phi)
			bool AVX512VL;   ///<  AVX512 Vector Length Extensions
			bool AVX512BW;   ///<  AVX512 Byte + Word
			bool AVX512DQ;   ///<  AVX512 Doubleword + Quadword
			bool AVX512IFMA; ///<  AVX512 Integer 52-bit Fused Multiply-Add
			bool AVX512VBMI; ///<  AVX512 Vector Byte Manipulation Instructions
			bool AVX512VBMI2;	    ///< AVX512 Vector Byte Manipulation Instructions 2
			bool AVX512GFNI;	    ///< AVX512 Galois field new instructions
			bool AVX512VAES;	    ///< AVX512 AES instructions
			bool VPCLMULQDQ;	    ///< VPCLMULQDQ
			bool AVX512VNNI;      ///< AVX512 Vector Neural Network Instructions
			bool AVX512BITALG;    ///< AVX512 Bit Algorithms
			bool AVX512VPOPCNTDQ;///< AVX512 Vector population count instruction
			bool AVX5124VNNIW;	///< AVX512 Vector Neural Network Instructions Word variable precision
			bool AVX5124FMAPS;	///< AVX512 Fused Multiply Accumulation Packed Single precision

			char brand[256];			///< The CPU model number (e.g. "Intel(R) Core(TM) i7-9800X CPU @ 3.80GHz")
			char manufacturer[14];	///< The CPU manufacturer (such as "GenuineIntel")

			uint64_t level_1_data_cache_size_in_bytes;
			uint64_t level_1_instruction_cache_size_in_bytes;
			uint64_t level_2_cache_size_in_bytes;
			uint64_t level_3_cache_size_in_bytes;

			uint32_t hyperthreads_per_core;
			uint32_t cores_per_die;
			uint32_t cpus_in_sockets;

			uint64_t ram_physical_in_bytes;

		protected:
			/*
				HARDWARE_SUPPORT::CPUID()
				-------------------------
			*/
			/*!
				@brief Execute a CPUID instruction.
				@param info [out] the CPUID data.
				@param info_type [in] the type of informaiton being sought.
				@param subfunction [in] the subfunction
			*/
			void cpuid(uint32_t info[4], uint32_t info_type, uint32_t subfunction = 0)
				{
				#ifdef _MSC_VER
					__cpuidex((int32_t *)info, info_type, subfunction);
				#else
					__cpuid_count(info_type, subfunction, info[0], info[1], info[2], info[3]);
				#endif
				}

			/*
				HARDWARE_SUPPORT::BITMASK32()
				------------------------------
			*/
			/*!
				@brief Compute a mask (all 1s) between h and l.
				@param h [in] the high bit number.
				@param l [in] the low bit number.
				@return a bit mask with 1 for h..l elsewhere 0.
			*/
			inline uint32_t bitmask32(uint32_t h, uint32_t l)
				{
				return (1U << h | ((1U << h) - 1)) & ~((1U << l ) - 1);
				}

			/*
				HARDWARE_SUPPORT::BITFIELD32()
				------------------------------
			*/
			/*!
				@brief Extract bitfield data from x
				@param x [in] get bits h..l out of this
				@param h [in] the high bit number
				@param l [in] the low bit number
				@return x[h..l]
			*/
			inline uint32_t bitfield32(uint32_t x, uint32_t h, uint32_t l)
				{
				return (x & bitmask32(h, l)) >> l;
				}

			/*
				HARDWARE_SUPPORT::INTEL_GET_CACHE_SIZES()
				-----------------------------------------
			*/
			/*!
				@brief Compute the sizes of the caches on an Intel CPU
			*/
			void intel_get_cache_sizes(void)
				{
				uint32_t info[4];

				for(uint32_t level = 0; level < 10; level++)		// never going to see 10 levels of cache (are we?)
					{
					cpuid(info, 4, level);

					if (info[0] == 0)
						break;

					uint64_t ways = bitfield32(info[1], 31, 22);
					uint64_t partitions = bitfield32(info[1], 21, 12);
					uint64_t line_size = bitfield32(info[1], 11, 0);
					uint64_t sets = info[2];
					uint64_t total_size_in_bytes = (ways + 1) * (partitions + 1) * (line_size + 1) * (sets + 1);

//					uint32_t cache_level = bitfield32(info[0], 7, 5);
//					uint32_t cache_type = bitfield32(info[0], 4, 0);
//					printf("L%c%c ",  cache_level + '0', cache_type == 1 ? 'd' : cache_type == 2 ? 'i' : cache_type == 3 ? 'u' : '?');
//					printf("%lluKB\n", total_size_in_bytes / 1024);

					uint32_t cache_type_level = bitfield32(info[0], 7, 0);
					if (cache_type_level == 0x21)
						level_1_data_cache_size_in_bytes = total_size_in_bytes;
					else if (cache_type_level == 0x22)
						level_1_instruction_cache_size_in_bytes = total_size_in_bytes;
					else if (cache_type_level == 0x43)
						level_2_cache_size_in_bytes = total_size_in_bytes;
					else if (cache_type_level == 0x63)
						level_3_cache_size_in_bytes = total_size_in_bytes;
					else
						JASS_assert(1);
					}
				}

			/*
				HARDWARE_SUPPORT::AMD_GET_CACHE_SIZES()
				---------------------------------------
			*/
			/*!
				@brief Compute the sizes of the caches on an AMD CPU
			*/
			void amd_get_chache_sizes(void)
				{
				uint32_t info[4];
				cpuid(info, 0x80000005);

				level_1_data_cache_size_in_bytes = bitfield32(info[2], 31, 24) * 1024;
				level_1_instruction_cache_size_in_bytes = bitfield32(info[3], 31, 24) * 1024;

				cpuid(info, 0x80000006);

				level_2_cache_size_in_bytes = bitfield32(info[2], 31, 16) * 1024;
				level_3_cache_size_in_bytes = bitfield32(info[3], 31, 18) * 512 * 1024;
				}

			/*
				HARDWARE_SUPPORT::INTEL_GET_CORE_COUNT()
				----------------------------------------
			*/
			/*!
				@brief Compute the core count and the hyperthreading count
			*/
			void intel_get_core_count(void)
				{
				uint32_t regs[4];

				/*
					Old school leaf 0x0B becaue I don't have a CPU with leaf 0x1F
				*/
				hyperthreads_per_core = cores_per_die = 1;

				for (uint32_t level = 0; level < 10; level++)
					{
					cpuid(regs, 0x0B, level);
					uint32_t type = bitfield32(regs[2], 15, 8);
					uint32_t count = bitfield32(regs[1], 15, 0);

					if (type == 0)
						break;
					else if (type == 1)
						hyperthreads_per_core = count;
					else if (type == 2)
						cores_per_die = count;
					}
				cores_per_die /= hyperthreads_per_core;
				}

			/*
				HARDWARE_SUPPORT::AMD_GET_CORE_COUNT()
				--------------------------------------
			*/
			/*!
				@brief Compute the core count and the hyperthreading count
			*/
			void amd_get_core_count(void)
				{
				uint32_t regs[4];

				cpuid(regs, 0x8000001E);
				hyperthreads_per_core = bitfield32(regs[1], 15, 8) + 1;

				cpuid(regs, 0x80000008);
				cores_per_die = bitfield32(regs[2], 7, 0) + 1;

				cores_per_die /= hyperthreads_per_core;
				}

			/*
				HARDWARE_SUPPORT::GET_CPUS_IN_SOCKETS_COUNT()
				---------------------------------------------
			*/
			/*!
				@brief Compute the number of physically seperate CPUs in sockets on the motherboard
			*/
			void get_cpus_in_sockets_count(void)
				{
				#ifdef _MSC_VER
					SYSTEM_INFO sysinfo;
					GetSystemInfo(&sysinfo);
					cpus_in_sockets = sysinfo.dwNumberOfProcessors;
				#else
					cpus_in_sockets = sysconf(_SC_NPROCESSORS_ONLN);
				#endif

				cpus_in_sockets /= cores_per_die * hyperthreads_per_core;
				}

			/*
				HARDWARE_SUPPORT::GET_PHYSICAL_RAM()
				------------------------------------
			*/
			/*!
				@brief Compute the total amount of physical RAM on the macbine.
			*/
			void get_physical_ram(void)
			{
			#ifdef _MSC_VER
				MEMORYSTATUSEX status;
				status.dwLength = sizeof(status);
				GlobalMemoryStatusEx(&status);
				ram_physical_in_bytes = status.ullTotalPhys;
			#else
				uint64_t pages = sysconf(_SC_PHYS_PAGES);
				uint64_t page_size = sysconf(_SC_PAGE_SIZE);
				ram_physical_in_bytes = pages * page_size;
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
			hardware_support() :
				FP(false),
				MMX(false),
				x64(false),
				ABM(false),
				RDRAND(false),
				BMI1(false),
				BMI2(false),
				ADX(false),
				PREFETCHWT1(false),
				CX8(false),
				CMPXCHG16B(false),
				CMOV(false),
				CLFSH(false),
				PCLMULQDQ(false),
				MOVBE(false),
				POPCNT(false),
				XSAVE(false),
				OSXSAVE(false),
				F16C(false),
				FSGSBASE(false),
				SGX(false),
				HLE(false),
				RTM(false),
				RDSEED(false),
				CLFLUSHOPT(false),
				CLWB(false),
				RDPID(false),
				CLDEMOTE(false),
				MOVDIRI(false),
				MOVDIR64B(false),
				FXSR(false),
				SSE(false),
				SSE2(false),
				SSE3(false),
				SSSE3(false),
				SSE41(false),
				SSE42(false),
				SSE4a(false),
				AES(false),
				SHA(false),
				AVX(false),
				XOP(false),
				FMA3(false),
				FMA4(false),
				AVX2(false),
				AVX512F(false),
				AVX512CD(false),
				AVX512PF(false),
				AVX512ER(false),
				AVX512VL(false),
				AVX512BW(false),
				AVX512DQ(false),
				AVX512IFMA(false),
				AVX512VBMI(false),
				AVX512VBMI2(false),
				AVX512GFNI(false),
				AVX512VAES(false),
				VPCLMULQDQ(false),
				AVX512VNNI(false),
				AVX512BITALG(false),
				AVX512VPOPCNTDQ(false),
				AVX5124VNNIW(false),
				AVX5124FMAPS(false),
				level_1_data_cache_size_in_bytes(0),
				level_1_instruction_cache_size_in_bytes(0),
				level_2_cache_size_in_bytes(0),
				level_3_cache_size_in_bytes(0),
				hyperthreads_per_core(1),
				cores_per_die(1),
				cpus_in_sockets(1),
				ram_physical_in_bytes(0)
				{
				uint32_t info[4];
				cpuid(info, 0);
				uint32_t nIds = info[0];

				cpuid(info, 0x80000000);
				unsigned nExIds = info[0];

				//  Detect Features
				if (nIds >= 0x00000001)
					{
					cpuid(info, 0x00000001);
					FP     = (info[3] & ((int)1 << 0)) != 0;
					CX8    = (info[3] & ((int)1 << 8)) != 0;
					CMOV   = (info[3] & ((int)1 << 15)) != 0;
					CLFSH  = (info[3] & ((int)1 << 19)) != 0;
					MMX    = (info[3] & ((int)1 << 23)) != 0;
					FXSR 	 = (info[3] & ((int)1 << 24)) != 0;
					SSE    = (info[3] & ((int)1 << 25)) != 0;
					SSE2   = (info[3] & ((int)1 << 26)) != 0;

					SSE3       = (info[2] & ((int)1 <<  0)) != 0;
					PCLMULQDQ  = (info[2] & ((int)1 <<  1)) != 0;
					SSSE3      = (info[2] & ((int)1 <<  9)) != 0;
					FMA3       = (info[2] & ((int)1 << 12)) != 0;
					CMPXCHG16B = (info[2] & ((int)1 << 13)) != 0;
					SSE41      = (info[2] & ((int)1 << 19)) != 0;
					SSE42      = (info[2] & ((int)1 << 20)) != 0;
					MOVBE      = (info[2] & ((int)1 << 22)) != 0;
					POPCNT     = (info[2] & ((int)1 << 23)) != 0;
					AES        = (info[2] & ((int)1 << 25)) != 0;
					XSAVE      = (info[2] & ((int)1 << 26)) != 0;
					OSXSAVE    = (info[2] & ((int)1 << 27)) != 0;
					AVX        = (info[2] & ((int)1 << 28)) != 0;
					F16C       = (info[2] & ((int)1 << 29)) != 0;
					RDRAND     = (info[2] & ((int)1 << 30)) != 0;
					}
				if (nIds >= 0x00000007)
					{
					cpuid(info,0x00000007);
					FSGSBASE    = (info[1] & ((int)1 <<  0)) != 0;
					SGX         = (info[1] & ((int)1 <<  2)) != 0;
					BMI1        = (info[1] & ((int)1 <<  3)) != 0;
					HLE         = (info[1] & ((int)1 <<  4)) != 0;
					BMI2        = (info[1] & ((int)1 <<  8)) != 0;
					RTM         = (info[1] & ((int)1 << 11)) != 0;
					RDSEED      = (info[1] & ((int)1 << 18)) != 0;
					ADX         = (info[1] & ((int)1 << 19)) != 0;
					CLFLUSHOPT  = (info[1] & ((int)1 << 23)) != 0;
					CLWB        = (info[1] & ((int)1 << 24)) != 0;
					SHA         = (info[1] & ((int)1 << 29)) != 0;

					PREFETCHWT1 = (info[2] & ((int)1 <<  0)) != 0;
					RDPID       = (info[2] & ((int)1 << 22)) != 0;
					CLDEMOTE    = (info[2] & ((int)1 << 25)) != 0;
					MOVDIRI     = (info[2] & ((int)1 << 27)) != 0;
					MOVDIR64B   = (info[2] & ((int)1 << 28)) != 0;

					/*
						At present AVX2 and above is not supported on vlagrind.
					*/
					if (!RUNNING_ON_VALGRIND)
						{
						AVX2        = (info[1] & ((int)1 <<  5)) != 0;

						AVX512F     = (info[1] & ((int)1 << 16)) != 0;
						AVX512DQ    = (info[1] & ((int)1 << 17)) != 0;
						AVX512IFMA  = (info[1] & ((int)1 << 21)) != 0;
						AVX512PF    = (info[1] & ((int)1 << 26)) != 0;
						AVX512ER    = (info[1] & ((int)1 << 27)) != 0;
						AVX512CD    = (info[1] & ((int)1 << 28)) != 0;
						AVX512BW    = (info[1] & ((int)1 << 30)) != 0;
						AVX512VL    = (info[1] & ((int)1 << 31)) != 0;

						AVX512VBMI  = (info[2] & ((int)1 <<  1)) != 0;
						AVX512VBMI2 = (info[2] & ((int)1 <<  6)) != 0;

						AVX512GFNI      = (info[2] & ((int)1 <<  8)) != 0;
						AVX512VAES      = (info[2] & ((int)1 <<  9)) != 0;
						VPCLMULQDQ      = (info[2] & ((int)1 << 10)) != 0;
						AVX512VNNI      = (info[2] & ((int)1 << 11)) != 0;
						AVX512BITALG    = (info[2] & ((int)1 << 12)) != 0;
						AVX512VPOPCNTDQ = (info[2] & ((int)1 << 14)) != 0;

						AVX5124VNNIW = (info[3] & ((int)1 << 2)) != 0;
						AVX5124FMAPS = (info[3] & ((int)1 << 3)) != 0;
						}
					}
				if (nExIds >= 0x80000001)
					{
					/*
						AMD flags, see: https://en.wikipedia.org/wiki/CPUID
					*/
					cpuid(info, 0x80000001);
					ABM   = (info[2] & ((int)1 <<  5)) != 0;
					SSE4a = (info[2] & ((int)1 <<  6)) != 0;
					FMA4  = (info[2] & ((int)1 << 16)) != 0;
					XOP   = (info[2] & ((int)1 << 11)) != 0;

					x64   = (info[3] & ((int)1 << 29)) != 0;
					}

				/*
					CPU manufacturer
				*/
				memset(manufacturer, 0, sizeof(manufacturer));
				cpuid(info, 0x00000000);
				*(uint32_t *)manufacturer = info[1];
				*(uint32_t *)(manufacturer + 4) = info[3];
				*(uint32_t *)(manufacturer + 8) = info[2];

				/*
					CPU model brand
				*/
				memset(brand, 0, sizeof(brand));
				if (nExIds >= 0x80000004)
					{
					cpuid((uint32_t *)brand, 0x80000002);
					cpuid((uint32_t *)brand + 4, 0x80000003);
					cpuid((uint32_t *)brand + 8, 0x80000004);
					}

				/*
					Get the cache and core count
				*/
				if (::strcmp(manufacturer, "GenuineIntel") == 0)
					{
					intel_get_cache_sizes();
					intel_get_core_count();
					}
				else if (::strcmp(manufacturer, "AuthenticAMD") == 0)
					{
					amd_get_chache_sizes();
					amd_get_core_count();
					}

				/*
					Get the number of CPUs (not cores or hyperthreads)
				*/
				get_cpus_in_sockets_count();

				/*
					Get the amount of physical RAM
				*/
				get_physical_ram();
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
		stream << "Manufacturer    :" << data.manufacturer << "\n";
		stream << "Model           :" << data.brand << "\n";

		stream << "L1d cache       :" << data.level_1_data_cache_size_in_bytes / 1024 << "KB\n";
		stream << "L1i cache       :" << data.level_1_instruction_cache_size_in_bytes / 1024 << "KB\n";
		stream << "L2  cache       :" << data.level_2_cache_size_in_bytes / 1024 << "KB\n";
		stream << "L3  cache       :" << data.level_3_cache_size_in_bytes / 1024 << "KB\n";
		stream << "Physical RAM    :" << data.ram_physical_in_bytes / (1024 * 1024 * 1023)<< "GB\n";

		stream << "CPUs            :" << data.cpus_in_sockets << "\n";
		stream << "CPU cores       :" << data.cores_per_die << "\n";
		stream << "Threads per core:" << data.hyperthreads_per_core << "\n";
		stream << "Total           :" << data.cpus_in_sockets * data.cores_per_die * data.hyperthreads_per_core << " Processing units\n";

		stream << "x64             :" << data.x64 << "\n";
		stream << "POPCNT          :" << data.POPCNT << "\n";
		stream << "CX8             :" << data.CX8 << "\n";
		stream << "CMPXCHG16B      :" << data.CMPXCHG16B << "\n";
		stream << "CMOV            :" << data.CMOV << "\n";
		stream << "ABM             :" << data.ABM << "\n";
		stream << "BMI1            :" << data.BMI1 << "\n";
		stream << "BMI2            :" << data.BMI2 << "\n";
		stream << "ADX             :" << data.ADX << "\n";
		stream << "PREFETCHWT1     :" << data.PREFETCHWT1 << "\n";
		stream << "PCLMULQDQ       :" << data.PCLMULQDQ << "\n";
		stream << "MOVBE           :" << data.MOVBE << "\n";
		stream << "XSAVE           :" << data.XSAVE << "\n";
		stream << "OSXSAVE         :" << data.OSXSAVE << "\n";
		stream << "F16C            :" << data.F16C << "\n";
		stream << "FSGSBASE        :" << data.FSGSBASE << "\n";
		stream << "SGX             :" << data.SGX << "\n";
		stream << "HLE             :" << data.HLE << "\n";
		stream << "RTM             :" << data.RTM << "\n";
		stream << "RDRAND          :" << data.RDRAND << "\n";
		stream << "RDSEED          :" << data.RDSEED << "\n";
		stream << "CLFLUSHOPT      :" << data.CLFLUSHOPT << "\n";
		stream << "CLWB            :" << data.CLWB << "\n";
		stream << "CLFSH           :" << data.CLFSH << "\n";

		stream << "RDPID           :" << data.RDPID << "\n";
		stream << "CLDEMOTE        :" << data.CLDEMOTE << "\n";
		stream << "MOVDIRI         :" << data.MOVDIRI << "\n";
		stream << "MOVDIR64B       :" << data.MOVDIR64B << "\n";

		stream << "FXSR            :" << data.FXSR << "\n";
		stream << "FP              :" << data.FP << "\n";
		stream << "MMX             :" << data.MMX << "\n";
		stream << "SSE             :" << data.SSE << "\n";
		stream << "SSE2            :" << data.SSE2 << "\n";
		stream << "SSE3            :" << data.SSE3 << "\n";
		stream << "SSSE3           :" << data.SSSE3 << "\n";
		stream << "SSE41           :" << data.SSE41 << "\n";
		stream << "SSE42           :" << data.SSE42 << "\n";
		stream << "SSE4a           :" << data.SSE4a << "\n";
		stream << "AES             :" << data.AES << "\n";
		stream << "SHA             :" << data.SHA << "\n";

		stream << "AVX             :" << data.AVX << "\n";
		stream << "XOP             :" << data.XOP << "\n";
		stream << "FMA3            :" << data.FMA3 << "\n";
		stream << "FMA4            :" << data.FMA4 << "\n";
		stream << "AVX2            :" << data.AVX2 << "\n";

		stream << "AVX512F         :" << data.AVX512F << "\n";
		stream << "AVX512CD        :" << data.AVX512CD << "\n";
		stream << "AVX512PF        :" << data.AVX512PF << "\n";
		stream << "AVX512ER        :" << data.AVX512ER << "\n";
		stream << "AVX512VL        :" << data.AVX512VL << "\n";
		stream << "AVX512BW        :" << data.AVX512BW << "\n";
		stream << "AVX512DQ        :" << data.AVX512DQ << "\n";
		stream << "AVX512IFMA      :" << data.AVX512IFMA << "\n";
		stream << "AVX512VBMI      :" << data.AVX512VBMI << "\n";

		stream << "AVX512VBMI2     :" << data.AVX512VBMI2 << "\n";
		stream << "AVX512GFNI      :" << data.AVX512GFNI << "\n";
		stream << "AVX512VAES      :" << data.AVX512VAES << "\n";
		stream << "VPCLMULQDQ      :" << data.VPCLMULQDQ << "\n";
		stream << "AVX512VNNI      :" << data.AVX512VNNI << "\n";
		stream << "AVX512BITALG    :" << data.AVX512BITALG << "\n";
		stream << "AVX512VPOPCNTDQ :" << data.AVX512VPOPCNTDQ << "\n";

		stream << "AVX5124VNNIW    :" << data.AVX5124VNNIW << "\n";
		stream << "AVX5124FMAPS    :" << data.AVX5124FMAPS << "\n";
		return stream;
		}
	}

#ifdef HARDWARE_SUPPORT_STAND_ALONE
	/*
		MAIN()
		------
		If you #define HARDWARE_SUPPORT_STAND_ALONE at the top then this becomes
		a stand alone program that dump stats about the machine it is running on.
	*/
	int main(void)
		{
		std::cout << JASS::hardware_support();
		}
#endif
