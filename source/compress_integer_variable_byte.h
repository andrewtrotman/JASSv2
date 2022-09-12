/*
	COMPRESS_INTEGER_VARIABLE_BYTE.H
	--------------------------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Variable byte compression for integer sequences.
	@author Andrew Trotman
	@copyright 2016 Andrew Trotman
*/
#pragma once

#include "forceinline.h"
#include "compress_integer.h"

namespace JASS
	{
	/*
		CLASS COMPRESS_INTEGER_VARIABLE_BYTE
		------------------------------------
	*/
	/*!
		@brief Variable byte compression for integer sequences.
		@details Variable byte compression is a whole suite of different techniques, for details see:
		A. Trotman (2014), Compression, SIMD, and Postings Lists. In Proceedings of the 2014 Australasian Document Computing Symposium (ADCS 2014), Pages 50-58. DOI=http://dx.doi.org/10.1145/2682862.2682870
		This particular version uses a stop-bit in the high bit of the last byte of the encoded integer, 
		stores the integer big-endian (high byte first), and uses loop unwinding for decoding efficiency.
		The encoding is straight forward.  An integer is broken into 7-bit chunks with the top bit of each
		chunk being 0, except the last byte which has a 1 in the top bit.  So, the integer 1905 (0x771)
		is the binary sequence 011101110001, which broken into 7-bit chunks is 0001110 1110001.  These then
		get the high bits added, 0 for all except the last byte, [0]0001110 [1]1110001, then write out
		the byte sequence high byte first 0x0E 0xF1.
		This implementation works with 32-bit and 64-bit integers.  To encode 64-bit integers ensure 
		\#define JASS_COMPRESS_INTEGER_BITS_PER_INTEGER 64
		is set at compile time.
	*/
	class compress_integer_variable_byte : public compress_integer
		{
		public:
			/*
				COMPRESS_INTEGER_VARIABLE_BYTE::COMPRESS_INTEGER_VARIABLE_BYTE()
				----------------------------------------------------------------
			*/
			/*!
				@brief Constructor.
			*/
			compress_integer_variable_byte()
				{
				/* Nothing */
				}
			
			/*
				COMPRESS_INTEGER_VARIABLE_BYTE::~COMPRESS_INTEGER_VARIABLE_BYTE()
				-----------------------------------------------------------------
			*/
			/*!
				@brief Constructor.
			*/
			virtual ~compress_integer_variable_byte()
				{
				/* Nothing */
				}

			/*
				COMPRESS_INTEGER_VARIABLE_BYTE::ENCODE()
				----------------------------------------
			*/
			/*!
				@brief Encode a sequence of integers returning the number of bytes used for the encoding, or 0 if the encoded sequence doesn't fit in the buffer.
				@param encoded [out] The sequence of bytes that is the encoded sequence.
				@param encoded_buffer_length [in] The length (in bytes) of the output buffer, encoded.
				@param source [in] The sequence of integers to encode.
				@param source_integers [in] The length (in integers) of the source buffer.
				@return The number of bytes used to encode the integer sequence, or 0 on error (i.e. overflow).
			*/
			virtual size_t encode(void *encoded, size_t encoded_buffer_length, const integer *source, size_t source_integers);
			
			/*
				COMPRESS_INTEGER_VARIABLE_BYTE::DECODE()
				----------------------------------------
			*/
			/*!
				@brief Decode a sequence of integers encoded with this codex.
				@param decoded [out] The sequence of decoded integers.
				@param integers_to_decode [in] The minimum number of integers to decode (it may decode more).
				@param source [in] The encoded integers.
				@param source_length [in] The length (in bytes) of the source buffer.
			*/
			virtual void decode(integer *decoded, size_t integers_to_decode, const void *source, size_t source_length)
				{
				/*
					Call through to the static version of this function
				*/
				static_decode(decoded, integers_to_decode, source, source_length);
				}

			/*
				COMPRESS_INTEGER_VARIABLE_BYTE::STATIC_DECODE()
				-----------------------------------------------
			*/
			/*!
				@brief Decode a sequence of integers encoded with this codex.
				@param decoded [out] The sequence of decoded integers.
				@param integers_to_decode [in] The minimum number of integers to decode (it may decode more).
				@param source_as_void [in] The encoded integers.
				@param source_length [in] The length (in bytes) of the source buffer.
			*/
			static inline void static_decode(integer *decoded, size_t integers_to_decode, const void *source_as_void, size_t source_length)
				{
				const uint8_t *source = static_cast<const uint8_t *>(source_as_void);
				const uint8_t *end = source + source_length;		// compute the stopping condition

				while (source < end)
					{
					decompress_into(decoded, source);
					decoded++;
					}
				}

			/*
				COMPRESS_INTEGER_VARIABLE_BYTE::DECODE_WITH_WRITER()
				----------------------------------------------------
			*/
			/*!
				@brief Decode a sequence of integers encoded with this codex, calling add_rsv for each SIMD register
				@param integers_to_decode [in] The minimum number of integers to decode (it may decode more).
				@param source [in] The encoded integers.
				@param source_length [in] The length (in bytes) of the source buffer.
			*/
#ifdef SIMD_JASS
			virtual void decode_with_writer(size_t integers_to_decode, const void *source_as_void, size_t source_length)
				{
				const uint8_t *source = static_cast<const uint8_t *>(source_as_void);
				const uint8_t *end = source + source_length;		// compute the stopping condition

				while (source < end)
					{
					integer into;
					
					decompress_into(&into, source);
					add_rsv_d1(into);
					}
				}
#endif

			/*
				COMPRESS_INTEGER_VARIABLE_BYTE::BYTES_NEEDED_FOR()
				--------------------------------------------------
			*/
			/*!
				@brief Return the number of bytes necessary to encode the integer value.
				@param value [in] The value whose encoded size is being computed
				@return the numner of bytes needed to store the enoding of value.
			*/
			static inline size_t bytes_needed_for(uint32_t value)
				{
				/*
					The size can be computed by compairing to a bunch of constants.
				*/
				if (value < ((uint64_t)1 << 7))
					return 1;
				else if (value < ((uint64_t)1 << 14))
					return 2;
				else if (value < ((uint64_t)1 << 21))
					return 3;
				else if (value < ((uint64_t)1 << 28))
					return 4;
				else
					return 5;
				}

			/*
				COMPRESS_INTEGER_VARIABLE_BYTE::BYTES_NEEDED_FOR()
				--------------------------------------------------
			*/
			/*!
				@brief Return the number of bytes necessary to encode the integer value.
				@param value [in] The value whose encoded size is being computed
				@return the numner of bytes needed to store the enoding of value.
			*/
			static inline size_t bytes_needed_for(uint64_t value)
				{
				/*
					The size can be computed by compairing to a bunch of constants.
				*/
				if (value < ((uint64_t)1 << 7))
					return 1;
				else if (value < ((uint64_t)1 << 14))
					return 2;
				else if (value < ((uint64_t)1 << 21))
					return 3;
				else if (value < ((uint64_t)1 << 28))
					return 4;
				else if (value < ((uint64_t)1 << 35))
					return 5;
				else if (value < ((uint64_t)1 << 42))
					return 6;
				else if (value < ((uint64_t)1 << 49))
					return 7;
				else if (value < ((uint64_t)1 << 56))
					return 8;
				else if (value < ((uint64_t)1 << 63))
					return 9;
				else
					return 10;
				}

			/*
				COMPRESS_INTEGER_VARIABLE_BYTE::COMPRESS_INTO()
				-----------------------------------------------
			*/
			/*!
				@brief Encode the given integer placing the encoding into destination (whose size is not validated).
				@param destination [out] The buffer to write into.
				@param value [in] The value to encode.
			*/
			template <typename DESTINATION>
			static forceinline void compress_into(DESTINATION &destination, uint32_t value)
				{
				/*
					Work out how many bytes it'll take to encode
				*/
				if (value < ((uint64_t)1 << 7))
					goto one;
				else if (value < ((uint64_t)1 << 14))
					goto two;
				else if (value < ((uint64_t)1 << 21))
					goto three;
				else if (value < ((uint64_t)1 << 28))
					goto four;
				goto five;

				/*
					Now encode byte at a time with fall-through
				*/
				five:
					*destination = (value >> 28) & 0x7F;
					++destination;
				four:
					*destination = (value >> 21) & 0x7F;
					++destination;
				three:
					*destination = (value >> 14) & 0x7F;
					++destination;
				two:
					*destination = (value >> 7) & 0x7F;
					++destination;
				one:
					*destination = (value & 0x7F) | 0x80;
					++destination;
				}


			/*
				COMPRESS_INTEGER_VARIABLE_BYTE::COMPRESS_INTO()
				-----------------------------------------------
			*/
			/*!
				@brief Encode the given integer placing the encoding into destination (whose size is not validated).
				@param destination [out] The buffer to write into.
				@param value [in] The value to encode.
			*/
			template <typename DESTINATION>
			static forceinline void compress_into(DESTINATION &destination, uint64_t value)
				{
				/*
					Work out how many bytes it'll take to encode
				*/
				if (value < ((uint64_t)1 << 7))
					goto one;
				else if (value < ((uint64_t)1 << 14))
					goto two;
				else if (value < ((uint64_t)1 << 21))
					goto three;
				else if (value < ((uint64_t)1 << 28))
					goto four;
				else if (value < ((uint64_t)1 << 35))
					goto five;
				else if (value < ((uint64_t)1 << 42))
					goto six;
				else if (value < ((uint64_t)1 << 49))
					goto seven;
				else if (value < ((uint64_t)1 << 56))
					goto eight;
				else if (value < ((uint64_t)1 << 63))
					goto nine;
				else
					goto ten;

				/*
					Now encode byte at a time with fall-through
				*/
				ten:
					*destination = (value >> 63) & 0x7F;
					++destination;
				nine:
					*destination = (value >> 56) & 0x7F;
					++destination;
				eight:
					*destination = (value >> 49) & 0x7F;
					++destination;
				seven:
					*destination = (value >> 42) & 0x7F;
					++destination;
				six:
					*destination = (value >> 35) & 0x7F;
					++destination;
				five:
					*destination = (value >> 28) & 0x7F;
					++destination;
				four:
					*destination = (value >> 21) & 0x7F;
					++destination;
				three:
					*destination = (value >> 14) & 0x7F;
					++destination;
				two:
					*destination = (value >> 7) & 0x7F;
					++destination;
				one:
					*destination = (value & 0x7F) | 0x80;
					++destination;
				}

			/*
				COMPRESS_INTEGER_VARIABLE_BYTE::DECOMPRESS_INTO()
				-------------------------------------------------
			*/
			/*!
				@brief Decode the given integer placing the encoding into destination (whose size is not validated).
				@param decoded [out] The decoded integer.
				@param source [in] The buffer to decode from.
			*/
			template <typename SOURCE>
			static forceinline void decompress_into(uint16_t *decoded, SOURCE &source)
				{
				/*
					If the high bit is set the sequence is over, otherwise, in an unwound loop, decode the integers one at a time.
				*/
				if (*source & 0x80)
					{
					*decoded = *source & 0x7F;
					++source;
					}
				else
					{
					*decoded = *source;
					++source;
					if (*source & 0x80)
						{
						*decoded = (*decoded << 7) | (*source & 0x7F);
						++source;
						}
					else
						{
						*decoded = (*decoded << 7) | *source;
						++source;
						}
					}
				}

			/*
				COMPRESS_INTEGER_VARIABLE_BYTE::DECOMPRESS_INTO()
				-------------------------------------------------
			*/
			/*!
				@brief Decode the given integer placing the encoding into destination (whose size is not validated).
				@param decoded [out] The decoded integer.
				@param source [in] The buffer to decode from.
			*/
			template <typename SOURCE>
			static forceinline void decompress_into(uint32_t *decoded, SOURCE &source)
				{
				/*
					If the high bit is set the sequence is over, otherwise, in an unwound loop, decode the integers one at a time.
				*/
				if (*source & 0x80)
					{
					*decoded = *source & 0x7F;
					++source;
					}
				else
					{
					*decoded = *source;
					++source;
					if (*source & 0x80)
						{
						*decoded = (*decoded << 7) | (*source & 0x7F);
						++source;
						}
					else
						{
						*decoded = (*decoded << 7) | *source;
						++source;
						if (*source & 0x80)
							{
							*decoded = (*decoded << 7) | (*source & 0x7F);
							++source;
							}
						else
							{
							*decoded = (*decoded << 7) | *source;
							++source;
							if (*source & 0x80)
								{
								*decoded = (*decoded << 7) | (*source & 0x7F);
								++source;
								}
							else
								{
								*decoded = (*decoded << 7) | *source;
								++source;
								if (*source & 0x80)
									{
									*decoded = (*decoded << 7) | (*source & 0x7F);
									++source;
									}
								}
							}
						}
					}
				}


			/*
				COMPRESS_INTEGER_VARIABLE_BYTE::DECOMPRESS_INTO()
				-------------------------------------------------
			*/
			/*!
				@brief Decode the given integer placing the encoding into destination (whose size is not validated).
				@param decoded [out] The decoded integer.
				@param source [in] The buffer to decode from.
			*/
			template <typename SOURCE>
			static forceinline void decompress_into(uint64_t *decoded, SOURCE &source)
				{
				/*
					If the high bit is set the sequence is over, otherwise, in an unwound loop, decode the integers one at a time.
				*/
				if (*source & 0x80)
					{
					*decoded = *source & 0x7F;
					++source;
					}
				else
					{
					*decoded = *source;
					++source;
					if (*source & 0x80)
						{
						*decoded = (*decoded << 7) | (*source & 0x7F);
						++source;
						}
					else
						{
						*decoded = (*decoded << 7) | *source;
						++source;
						if (*source & 0x80)
							{
							*decoded = (*decoded << 7) | (*source & 0x7F);
							++source;
							}
						else
							{
							*decoded = (*decoded << 7) | *source;
							++source;
							if (*source & 0x80)
								{
								*decoded = (*decoded << 7) | (*source & 0x7F);
								++source;
								}
							else
								{
								*decoded = (*decoded << 7) | *source;
								++source;
								if (*source & 0x80)
									{
									*decoded = (*decoded << 7) | (*source & 0x7F);
									++source;
									}
								else
									{
									*decoded = (*decoded << 7) | *source;
									++source;
									if (*source & 0x80)
										{
										*decoded = (*decoded << 7) | (*source & 0x7F);
										++source;
										}
									else
										{
										*decoded = (*decoded << 7) | *source;
										++source;
										if (*source & 0x80)
											{
											*decoded = (*decoded << 7) | (*source & 0x7F);
											++source;
											}
										else
											{
											*decoded = (*decoded << 7) | *source;
											++source;
											if (*source & 0x80)
												{
												*decoded = (*decoded << 7) | (*source & 0x7F);
												++source;
												}
											else
												{
												*decoded = (*decoded << 7) | *source;
												++source;
												if (*source & 0x80)
													{
													*decoded = (*decoded << 7) | (*source & 0x7F);
													++source;
													}
												else
													{
													*decoded = (*decoded << 7) | *source;
													++source;
													if (*source & 0x80)
														{
														*decoded = (*decoded << 7) | (*source & 0x7F);
														++source;
														}
													else
														{
														*decoded = (*decoded << 7) | *source;
														++source;
														}
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}

			/*
				COMPRESS_INTEGER_VARIABLE_BYTE::UNITTEST()
				------------------------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void);
		} ;
}
