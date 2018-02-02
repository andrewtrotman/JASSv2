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

#include <pthread.h>

namespace JASS
	{
	/*!
		@brief JASS implementation of C++ threads (for more control)
	*/
	class thread
		{
		private:
			const size_t DEFAULT_STACK_SIZE = 8 * 1024 * 1024;			///< default tread stack size

		private:
			pthread_t thread_id;						///< The Posix threads thread id
			pthread_attr_t attributes;				///< The Posix threads attributes

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
		static void *bootstrap(void *param)
			{
			/*
				extract the binding and call it.
			*/
			FUNCTOR functor = (FUNCTOR)param;
			(*functor)();

			return nullptr;
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
			thread(FUNCTION &&function, PARAMETERS &&... parameters)
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

				/*
					set any thread attributes we need
				*/
				pthread_attr_init(&attributes);

				/*
					give it a good-sized stack
				*/
				pthread_attr_setstacksize(&attributes, DEFAULT_STACK_SIZE);

				/*
					start the thread
				*/
				pthread_create(&thread_id, &attributes, bootstrap<decltype(functor)>, functor);
				}

			/*
				THREAD::JOIN()
				--------------
			*/
			/*!
				@brief block until this thead finishes
			*/
			void join(void)
				{
				void *return_value;
				pthread_join(thread_id, &return_value);
				}

			/*
				THREAD::UNITTEST_CALLBACK()
				---------------------------
			*/
			/*!
				@brief Callback function used by unittest().
				@param x [in/out] a reference to an integer to thread-unsafe alter.
			*/
			static void unittest_callback(int &x)
				{
				x++;
				}

			/*
				THREAD::UNITTEST()
				------------------
			*/
			/*!
				@brief Unit test this class.
			*/
			static void unittest(void)
				{
				int param = 1;
				auto x = thread(unittest_callback, std::ref(param));
				auto y = thread(unittest_callback, std::ref(param));

				x.join();
				y.join();

				JASS_assert(param != 1);

				puts("thread::PASSED");
				}
		};
	}
