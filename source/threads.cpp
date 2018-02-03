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
			WaitForSingleObject((HANDLE)thread_id, INFINITE);
		#else
			void *return_value;
			pthread_join(thread_id, &return_value);
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
