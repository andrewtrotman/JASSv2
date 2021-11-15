/*
	THREADS.CPP
	-----------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#ifdef _MSC_VER
	#include <windows.h>
	#include <process.h>
#else
	#include <pthread.h>
#endif

#include "threads.h"

namespace JASS
	{
	/*
		THREAD::THREAD()
		----------------
	*/
	thread::thread(thread &&other) :
		thread_id(other.thread_id)
#ifndef _MSC_VER
		,
		attributes(other.attributes),
		valid(other.valid)
#endif
		{
		#ifdef _MSC_VER
			other.thread_id = -1;
		#else
			other.valid = false;
		#endif
		}

	/*
		THREAD::THREAD()
		----------------
	*/
	thread &thread::operator=(thread &&other)
		{  
		if (this != &other)
			{
			#ifdef _MSC_VER
				if (thread_id != -1)
					CloseHandle((HANDLE)thread_id);

				thread_id = other.thread_id;
				other.thread_id = -1;
			#else
				thread_id = other.thread_id;

				valid = other.valid;
				other.valid = false;

				attributes = other.attributes;
			#endif
			}

		return *this;
		}

	/*
		THREAD::~THREAD()
		-----------------
	*/
	/*!
		@brief Destructor.
	*/
	thread::~thread()
		{
		#ifdef _MSC_VER
			/*
				The thread_id must be valid because otherwise we exit() from the constructor.
			*/
			CloseHandle((HANDLE)thread_id);
		#endif
		}

	/*
		THREAD::JOIN()
		--------------
	*/
	void thread::join(void)
		{
		#ifdef _MSC_VER
			if (thread_id != -1)
				WaitForSingleObject((HANDLE)thread_id, INFINITE);
		#else
			if (valid)
				{
				void *return_value;
				pthread_join(thread_id, &return_value);
				}
		#endif
		}

	/*
		THREAD::UNITTEST_CALLBACK()
		---------------------------
	*/
	void thread::unittest_callback(int &x)
		{
		x++;
		}
		
	/*
		THREAD::UNITTEST()
		------------------
	*/
	void thread::unittest(void)
		{
		int param = 1;
		auto x = thread(unittest_callback, std::ref(param));
		auto y = thread(unittest_callback, std::ref(param));

		x.join();
		y.join();

		JASS_assert(param != 1);

		puts("thread::PASSED");
		}
	}
