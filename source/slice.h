/*
	SLICE.H
	-------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Slices (also known as string-descriptors) for C++.
	@author Andrew Trotman
	@copyright 2016 Andrew Trotman
*/
#pragma once

#include <string.h>

#include <iostream>

#include "asserts.h"
#include "allocator_pool.h"

namespace JASS
	{
	/*!
		@brief C++ slices (string-descriptors)
	*/
	class slice
		{
		protected:
			void *pointer;			///< The start of the data.
			size_t length;					///< The length of the data (in bytes).
			
		public:
			/*
				SLICE::SLICE()
				--------------
			*/
			/*!
				@brief Constructor
				@param pointer [in] The start address of the memory this slice represents.
				@param length [in] The length (in bytes) of this slice.
			*/
			slice(void *pointer = nullptr, size_t length = 0) :
				pointer(pointer),
				length(length)
				{
				/* Nothing */
				}

			/*
				SLICE::SLICE()
				--------------
			*/
			/*!
				@brief Constructor
				@param start [in] The start address of the memory this slice represents.
				@param end [in] The end address of the memory this slice represents.
			*/
			slice(void *start, void *end) :
				pointer(start),
				length((uint8_t *)end - (uint8_t *)start)
				{
				/* Nothing */
				}
			/*

				SLICE::SLICE()
				--------------
			*/
			/*!
				@brief Constructor.  Construct as a pointer to message and with length strlen(message).  Does not copy, points to.
				@param message [in] A pointer to a '\0' terminated 'C' string.
			*/
			slice(const char *message) :
				pointer((void *)message),
				length(strlen(message))
				{
				/* Nothing */
				}

			/*
				SLICE::SLICE()
				--------------
			*/
			/*!
				@brief Construct a slice by copying the data into allocator's pool of memory.  This does NOT ever delete the memory, that is the allocator's task.
				@param pool [in] An allocator to allocate the memory from.
				@param start [in] The start address of the memory this slice represents.
				@param end [in] The end address of the memory this slice represents.
			*/
			slice(allocator &pool, void *start, void *end)
				{
				length = (uint8_t *)end - (uint8_t *)start;
				pointer = (void *)pool.malloc(length);
				memcpy(pointer, start, length);
				}
			
			/*
				SLICE::SLICE()
				--------------
			*/
			/*!
				@brief Construct a slice by copying and '\0' termainating a string, using the allocator's pool of memory.  This does NOT ever delete the memory, that is the allocator's task.
				@param pool [in] An allocator to allocate the memory from.
				@param start [in] The start address of the C string.
				@param end [in] The end address of the C string.
			*/
			slice(allocator &pool, const char *start, const char *end)
				{
				length = end - start;
				pointer = (void *)pool.malloc(length + 1);
				memcpy(pointer, start, length);
				((char *)pointer)[length] = '\0';
				}

			/*
				SLICE::SLICE()
				--------------
			*/
			/*!
				@brief Construct a slice by copying and '\0' termainating a string, using the allocator's pool of memory.  This does NOT ever delete the memory, that is the allocator's task.
				@param pool [in] An allocator to allocate the memory from.
				@param start [in] The start address of the C string.
				@param end [in] The end address of the C string.
			*/
			slice(allocator &pool, const unsigned char *start, const unsigned char *end)
				{
				length = end - start;
				pointer = (void *)pool.malloc(length + 1);
				memcpy(pointer, start, length);
				((char *)pointer)[length] = '\0';
				}

			/*
				SLICE::SLICE()
				--------------
			*/
			/*!
				@brief Construct a slice by copying and '\0' termainating a string, using the allocator's pool of memory.  This does NOT ever delete the memory, that is the allocator's task.
				@param pool [in] An allocator to allocate the memory from.
				@param start [in] The start address of the C string.
			*/
			slice(allocator &pool, const char *start)
				{
				length = strlen(start);
				pointer = (void *)pool.malloc(length + 1);
				memcpy(pointer, start, length);
				((char *)pointer)[length] = '\0';
				}

			/*
				SLICE::SLICE()
				--------------
			*/
			/*!
				@brief Construct a slice by copying and '\0' termainating a string, using the allocator's pool of memory.  This does NOT ever delete the memory, that is the allocator's task.
				@param pool [in] An allocator to allocate the memory from.
				@param start [in] The start address of the C string.
			*/
			slice(allocator &pool, const unsigned char *start)
				{
				length = strlen(reinterpret_cast<const char *>(start));
				pointer = (void *)pool.malloc(length + 1);
				memcpy(pointer, start, length);
				((char *)pointer)[length] = '\0';
				}

			/*
				SLICE::SLICE()
				--------------
			*/
			/*!
				@brief Construct a slice by copying its contents into the pool allocator.
				@param pool [in] An allocator to allocate the memory from.
				@param from [in] The slice to copy.
			*/
			slice(allocator &pool, const slice &from)
				{
				length = from.size();
				pointer = (void *)pool.malloc(length);
				memcpy(pointer, from.address(), length);
				}

			/*
				SLICE::SLICE()
				--------------
			*/
			/*!
				@brief Construct a slice by allocating bytes of memory from a pool allocator
				@param pool [in] An allocator to allocate from.
				@param bytes [in] The number of bytes of memory to allocate from the pool.
			*/
			slice(allocator &pool, size_t bytes) :
				pointer((void *)pool.malloc(bytes)),
				length(bytes)
				{
				/* Nothing */
				}

			/*
				SLICE::SLICE()
				--------------
			*/
			/*!
				@brief Construct an empty slice with a pool allocator
				@param pool [in] An allocator to allocate from.
			*/
			slice(allocator &pool):
				pointer(nullptr),
				length(0)
				{
				/* Nothing */
				}

			/*
				SLICE::CLEAR()
				--------------
			*/
			/*!
				@brief Construct an empty slice with a pool allocator
				@param pool [in] An allocator to allocate from.
			*/
			void clear(void)
				{
				length = 0;
				pointer = nullptr;
				}



			/*
				SLICE::OPERATOR=()
				-------------------
			*/
			/*!
				@brief Operator = (asignment operator)
				@param with [in] The slice to copy to.
				@return A reference to this slice
			*/
			slice &operator=(const slice &with)
				{
				pointer = with.pointer;
				length = with.length;

				return *this;
				}

			/*
				SLICE::SIZE()
				-------------
			*/
			/*!
				@brief Return the length of this slice.
				@return Slice length.
			*/
			size_t size(void) const
				{
				return length;
				}
			
			/*
				SLICE::ADDRESS()
				----------------
			*/
			/*!
				@brief Extract the pointer value from the slice
				@return The slice's internal pointer
			*/
			void *address(void) const
				{
				return pointer;
				}
			
			/*
				SLICE::RESIZE()
				---------------
			*/
			/*!
				@brief Change the length of the slice.
				@param new_size [in] The new size (in bytes) of the slice.
			*/
			void resize(size_t new_size)
				{
				length = new_size;
				}
			
			/*
				SLICE::OPERATOR[]()
				-------------------
			*/
			/*!
				@brief Return a reference to the n'th byte past the start of the slice.
				@param index [in] Which byte to return.
				@details  No safety comparison is performed.  Out of range indexing will result in undefined behaviour
				@return A reference to the byte at position specified in index.
			*/
			uint8_t &operator[](size_t index) const
				{
				return ((uint8_t *)pointer)[index];
				}
			
			/*
				SLICE::STRICT_WEAK_ORDER_LESS_THAN()
				------------------------------------
			*/
			/*!
				@brief Return true if this < with.
				@details The colaiting sequence is memcmp().
				@param me [in] The slice to compare.
				@param with [in] The slice to compare to.
				@return true if this < with, else false.
			*/
			static bool strict_weak_order_less_than(const slice &me, const slice &with)
				{
				if (me.size() == with.size())
					return memcmp(me.address(), with.address(), me.size()) < 0;
				else if (me.size() < with.size())
					{
					auto cmp = memcmp(me.address(), with.address(), me.size());
					if (cmp == 0)
						return true;
					else
						return cmp < 0 ? true : false;
					}
				else
					{
					auto cmp = memcmp(me.address(), with.address(), with.size());
					if (cmp == 0)
						return false;
					else
						return cmp < 0 ? true : false;
					}
				}

			/*
				SLICE::OPERATOR<()
				------------------
			*/
			/*!
				@brief Return true if this < with.
				@details The colaiting sequence is: shorter strings are  "less" then longer strings.  For strings of equal length memcmp() us used.
				@param with [in] The slice to compare to.
				@return true if this < with, else false.
			*/
			bool operator<(const slice &with) const
				{
				if (size() < with.size())
					return true;
				if (size() > with.size())
					return false;
				return memcmp(address(), with.address(), size()) < 0;
				}

			/*
				SLICE::OPERATOR>()
				------------------
			*/
			/*!
				@brief Return true if this > with.
				@param with [in] The slice to compare to.
				@details The colaiting sequence is: shorter strings are  "less" then longer strings.  For strings of equal length memcmp() us used.
				@return true if this > with, else false.
			*/
			bool operator>(const slice &with) const
				{
				if (size() > with.size())
					return true;
				if (size() < with.size())
					return false;
				return memcmp(address(), with.address(), size()) > 0;
				}

			/*
				SLICE::OPERATOR==()
				-------------------
			*/
			/*!
				@brief Return true if this == with.
				@param with [in] The slice to compare to.
				@return true if this == with, else false.
			*/
			bool operator==(const slice &with) const
				{
				if (size() != with.size())
					return false;
				return memcmp(address(), with.address(), size()) == 0;
				}

			/*
				SLICE::UNITTEST()
				-----------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void)
				{
				slice chunk1;
				JASS_assert(chunk1.address() == nullptr && chunk1.size() == 0);
				
				slice chunk2((char *)unittest, (char *)unittest + 5);
				JASS_assert(chunk2.address() == unittest && chunk2.size() == 5);
				
				allocator_pool pool;
				char data[6];
				slice chunk3(pool, (void *)data, (void *)((uint8_t *)data + 5));
				JASS_assert(pool.size() == 5 && chunk3.size() == 5);
				
				const char *message = "here there and everywhere";
				slice chunk4(pool, message, message + 4);
				JASS_assert(pool.size() == 10 && chunk4.size() == 4);
				
				JASS_assert(chunk4[1] == 'e' && &chunk4[1] == (unsigned char *)chunk4.address() + 1);

				slice chunk5(pool, message);
				JASS_assert(pool.size() == 36 && strcmp((char *)chunk5.address(), message) == 0);

				slice chunk6(pool, 10);
				JASS_assert(pool.size() == 46 && chunk6.size() == 10);
				
				chunk6.resize(5);
				JASS_assert(chunk6.size() == 5);

				JASS_assert(slice("a") < slice("b"));
				JASS_assert(!(slice("b") < slice("a")));

				JASS_assert(!(slice("aa") < slice("b")));
				JASS_assert(slice("b") < slice("aa"));

				JASS_assert(!(slice("a") > slice("b")));
				JASS_assert(slice("b") > slice("a"));

				JASS_assert(slice("aa") > slice("b"));
				JASS_assert(!(slice("b") > slice("aa")));

				JASS_assert(slice("a") == slice("a"));
				JASS_assert(!(slice("aa") == slice("a")));

				/*
					exhaustively test strict_weak_order_less_than() which emilates a strcmp()
				*/
				JASS_assert(strict_weak_order_less_than(slice("aa"), slice("aa")) == false);
				JASS_assert(strict_weak_order_less_than(slice("a"), slice("aa")) == true);
				JASS_assert(strict_weak_order_less_than(slice("b"), slice("aa")) == false);
				JASS_assert(strict_weak_order_less_than(slice("a"), slice("bb")) == true);
				JASS_assert(strict_weak_order_less_than(slice("aa"), slice("a")) == false);
				JASS_assert(strict_weak_order_less_than(slice("aa"), slice("b")) == true);
				JASS_assert(strict_weak_order_less_than(slice("bb"), slice("a")) == false);

				puts("slice::PASSED");
				}
		};

	/*
		OPERATOR<<()
		------------
	*/
	/*!
		@brief Dump the contents of a slice (as binary data) down an output stream.
		@param stream [in] The stream to write to.
		@param data [in] The slice to write.
		@return The stream once the slice has been written.
	*/
	inline std::ostream &operator<<(std::ostream &stream, const slice &data)
		{
		stream.write((char *)data.address(), data.size());
		return stream;
		}

	}
