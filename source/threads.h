/*
	THREADS.H
	---------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief JASS implementation of C++ threads
	@details Based on https://stackoverflow.com/questions/8542591/c11-reverse-range-based-for-loop

	@author Andrew Trotman
	@copyright 2017 Andrew Trotman
*/

#pragma once

#ifdef _MSC_VER
	#include <process.h>
#else
	#include <pthread.h>
#endif

#include <stdio.h>

#include <functional> 

#include "asserts.h"

namespace JASS
	{
	/*!
		@brief JASS implementation of C++ threads (for more control)
	*/
	class thread
		{
		private:
			#ifdef _MSC_VER
				const unsigned int DEFAULT_STACK_SIZE = 8 * 1024 * 1024;			///< default tread stack size
			#else
				const size_t DEFAULT_STACK_SIZE = 8 * 1024 * 1024;			///< default tread stack size
			#endif

		private:
			#ifdef _MSC_VER
				uintptr_t thread_id;
			#else
				pthread_t thread_id;						///< The Posix threads thread id
				pthread_attr_t attributes;				///< The Posix threads attributes
			#endif

		/*
			THREAD::BOOTSTRAP()
			-------------------
		*/
		/*!
			@brief Callback function used to bootstrap from the Operating System thread library into C++.
			@param param[in] A C++ binding used to perform the call.
			@return Always returns nullptr.
		*/
			template<typename FUNCTOR>
			#ifdef _MSC_VER
				static unsigned bootstrap(void *param)
			#else
				static void *bootstrap(void *param)
			#endif
				{
				/*
					extract the binding and call it.
				*/
				FUNCTOR functor = (FUNCTOR)param;
				(*functor)();

				#ifdef _MSC_VER
					return 0;
				#else
					return nullptr;
				#endif
				}

		public:
			/*
				THREAD::THREAD()
				----------------
			*/
			/*!
				@brief Constructor.
				@param function [in] the function to call.
				@param parameters [in] the parameters to that function.
			*/
			template<typename FUNCTION, typename... PARAMETERS>
			thread(FUNCTION function, PARAMETERS ... parameters):
				thread_id()
				{
				/*
					first create a binding of the function and its parameters
				*/
				auto binding = std::bind(function, parameters...);

				/*
					resolve all rvalue references
				*/
				auto forwarding = std::forward<decltype(binding)>(binding);

				/*
					dynamically allocate the result
				*/
				auto functor = new decltype(forwarding)(forwarding);

				#ifdef _MSC_VER
					thread_id = _beginthreadex(NULL, DEFAULT_STACK_SIZE, bootstrap<decltype(functor)>, functor, 0, NULL);
					if (thread_id == 0)
						exit(printf("Can't start thread"));
				#else
					/*
						set any thread attributes we need
					*/
					if (pthread_attr_init(&attributes) != 0)
						exit(printf("Can't start thread"));

					/*
						give it a good-sized stack
					*/
					if (pthread_attr_setstacksize(&attributes, DEFAULT_STACK_SIZE) != 0)
						exit(printf("Can't start thread"));

					/*
						start the thread
					*/
					if (pthread_create(&thread_id, &attributes, bootstrap<decltype(functor)>, functor) != 0)
						exit(printf("Can't start thread"));
				#endif
				}

			/*
				THREAD::~THREAD()
				-----------------
			*/
			/*!
				@brief Destructor.
			*/
			~thread();

			/*
				THREAD::JOIN()
				--------------
			*/
			/*!
				@brief block until this thead finishes
			*/
			void join(void);

			/*
				THREAD::UNITTEST_CALLBACK()
				---------------------------
			*/
			/*!
				@brief Callback function used by unittest().
				@param x [in/out] a reference to an integer to thread-unsafe alter.
			*/
			static void unittest_callback(int &x);

			/*
				THREAD::UNITTEST()
				------------------
			*/
			/*!
				@brief Unit test this class.
			*/
			static void unittest(void);
		};
	}
