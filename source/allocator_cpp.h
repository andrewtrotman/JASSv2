/*
	ALLOCATOR_CPP.H
	---------------
	This is a C++11 allocator class.  It is based on the article here: https://msdn.microsoft.com/en-us/library/aa985953.aspx
*/
#pragma once

#include <stdlib.h>
#include <assert.h>

#include <new>
#include <memory>

#include "allocator.h"

namespace JASS
	{
	/*
		CLASS ALLOCATOR_CPP
		-------------------
	*/
	/*!
		@brief C++ allocator based on the C allocator
	*/
	template <class TYPE>
	class allocator_cpp
	{
	private:
		allocator pool;							///< This is the memory pool from which this C++ allocator allocated.
		
	public:
		typedef TYPE value_type;				///< This is an allocator for objects of type allocator_cpp::value_type.

		/*
			ALLOCATOR_CPP::ALLOCATOR_CPP()
			------------------------------
		*/
		/*!
			@brief Default constructor.
			@param c_allocator [in] A reference to C style allocator.
		*/
		allocator_cpp(allocator &pool) : pool(pool)
		 {
		 /*
		 	Nothing
		 */
		 }

		/*
			ALLOCATOR_CPP::ALLOCATOR_CPP()
			------------------------------
		*/
		/*!
			@brief Copy constructor
			@param other [in] the object to copy
		*/
		template<class OTHER_TYPE>
		allocator_cpp(const allocator_cpp<OTHER_TYPE> &other)
			{
			this->pool = other.pool;
			}
		
		/*
			ALLOCATOR_CPP::OPERATOR==()
			---------------------------
		*/
		/*!
			@brief Compare for equality two objects of this class type.
			@param that [in] The object to compare to.
			@return True if this == that, else false.
		*/
		template<class OTHER_TYPE>
		bool operator==(const allocator_cpp<OTHER_TYPE> &that) const
			{
			return this->pool == that.pool;
			}
	
		/*
			ALLOCATOR_CPP::OPERATOR!=()
			---------------------------
		*/
		/*!
			@brief Compare for inequlity two objects of this class type.
			@param that [in] The object to compare to.
			@return True if this != that, else false.
		*/
		template<class OTHER_TYPE>
		bool operator!=(const allocator_cpp<OTHER_TYPE> &that) const
			{
			return this->pool != that.pool;
			}
	
		/*
			ALLOCATOR_CPP::ALLOCATE()
			-------------------------
		*/
		/*!
			@brief Allocate space for number of objects of this type.
			@param number [in] The number of objects of this type to create space for.
			@return Pointer to enough memory to store number of objects, else throw an exception.
		*/
		TYPE *allocate(const size_t number)
			{
			if (number == 0)
				return nullptr;
				
			if (number > (size_t)-1 / sizeof(TYPE))
				throw std::bad_array_new_length();
				
			TYPE *pointer = (TYPE *)pool.malloc(number * sizeof(TYPE));
			
			if (pointer == NULL)
				throw std::bad_alloc();

			return pointer;
			}

		/*
			ALLOCATOR_CPP::DEALLOCATE()
			---------------------------
		*/
		/*!
			@brief Dealocate previously allocated space (not).  This method does nothing but is required for a C++11 allocator.
			@details This method does nothing because this class allocates from a pool allocator which manages all the deallocation itself.
			@param pointer [in] A pointer to memory previously allocated by this allocator and to be released.
			@param number [in] The number of objects of this type that were allocated on the call to allocate.
		*/

		void deallocate(TYPE * const pointer, size_t number) const
			{
//			pool.free(pointer);			// DO NOT DO THIS AS THERE IS NOT free() METHOD IN THE POOL ALLOCATOR.
			}
			
		/*
			ALLOCATOR_CPP::UNITTEST()
			-------------------------
		*/
		/*!
			Perform the unit test
		*/
		static void unittest(void)
			{
			allocator pool;
			allocator_cpp<int> memory_1(pool);
			std::vector<int, allocator_cpp<int>> collection_1(memory_1);
			collection_1.push_back(32);

			allocator_cpp<int> memory_2(pool);
			std::vector<int, allocator_cpp<int>> collection_2(memory_2);
			collection_2.push_back(64);
			
			collection_1[0] = collection_2[0];
			
			assert(collection_1[0] == collection_2[0]);
			
			puts("allocator_cpp::PASSED");
			}
	};
}
