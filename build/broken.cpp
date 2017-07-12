/*
	BROKEN.CPP
	---------
*/
#include <stdio.h>
#include <iostream>
#include <string>

template <class TYPE> class my_allocator
	{
	public:
		int instance;
		
	public:
		using value_type = TYPE;

		my_allocator(int val) : instance(val) { }
		my_allocator(const my_allocator<TYPE> &other) : instance(other.instance) { }

		bool operator==(const my_allocator<TYPE> &that) const { return instance == that.instance; }
		bool operator!=(const my_allocator<TYPE> &that) const { return instance != that.instance; }
		
		TYPE *allocate(const size_t number)
			{
			if (number == 0)
				return nullptr;
				
			if (number > (size_t)-1 / sizeof(TYPE))
				throw std::bad_array_new_length();
					
			TYPE *pointer = (TYPE *)::malloc(number * sizeof(TYPE));
				
			if (pointer == nullptr)
				throw std::bad_alloc();

			return pointer;
			}

		void deallocate(TYPE * const pointer, size_t number) const { free(pointer); }
	};

typedef std::basic_string<char, std::char_traits<char>, my_allocator<char>>my_string;

int main(void)
	{
	my_allocator<char> allocator(1);
	
	my_string str(allocator);
	str = "one";
	std::cout << str;
	
	return 0;
	}

