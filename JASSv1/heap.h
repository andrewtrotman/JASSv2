/*
	HEAP.H
	------
*/
#ifndef HEAP_H
#define HEAP_H

#include <stdio.h>
#include <math.h>
#include <stdlib.h>

/*
	class ANT_PRIMARY_CMP
	---------------------
*/
template <typename T> class ANT_primary_cmp
{
public:
	int operator()(const T &a, const T &b) { return a - b; }
};


/*
	class ANT_HEAP
	--------------
*/
template <typename T, typename _Compare = ANT_primary_cmp<T> >
class ANT_heap
{
private:
	T *array;
	long long size;
	inline void swap (T *a, T *b)
		{
		T temp = *a;
		*a = *b;
		*b = temp;
		}
	_Compare compare;

public:
	ANT_heap(T &arry, long long size, const _Compare& __c = _Compare()) : array(&arry), size(size), compare(__c) {};

	#define parent_pos(i) (((i)-1) >> 1)
	#define left_pos(i) (((i) << 1) + 1)
	#define right_pos(i) (((i) << 1) + 2)

	inline void set_size(long long size) { this->size = size; }
	inline long long get_size(void) { return this->size; }

	void max_heapify(long long pos);
	void max_heapify(long long pos, long long hsize);
	void build_max_heap(void);
	int max_update_maximum(T key);
	void max_heapsort(void);
	void text_render(long long i);

	void min_heapify(long long pos);
	void min_heapify(long long pos, long long hsize);
	void build_min_heap(void);
	void min_heapsort(void);
	int min_insert(T key);
	void min_update(T key);
	T get_second_smallest(void);
};

/*
	ANT_HEAP::MAX_UPDATE_MAXIMUM()
	------------------------------
*/
template <typename T, typename _Compare> int ANT_heap<T, _Compare>::max_update_maximum(T key)
{
long long i = 0, lpos, rpos;

while (i < this->size)
	{
	lpos = left_pos(i);
	rpos = right_pos(i);

	// check array out of bound, it's also the stopping condition
 	if (lpos < this->size && rpos < this->size)
		{
	 	if (compare(key, array[lpos]) >= 0 && compare(key, array[rpos]) >= 0)
			break;			// we're smaller then the left and the right so we're done
		else if (compare(array[lpos], array[rpos]) > 0)
			{
			array[i] = array[lpos];
			i = lpos;
			}
		else
			{
			array[i] = array[rpos];
			i = rpos;
			}
		}
	else if (lpos < this->size)			// and rpos > this->size (because this is an else)
		{
		if (compare(key, array[lpos]) < 0)
			{
			array[i] = array[lpos];
			i = lpos;
			}
		else
			break;
		}
	else
		break;			// both lpos and lpos exceed end of array
	}

array[i] = key;
return 1;
}

/*
	ANT_HEAP::MIN_INSERT()
	----------------------
*/
template <typename T, typename _Compare>
int ANT_heap<T, _Compare>::min_insert(T key)
{
long long i = 0, lpos, rpos;

// if key is less than the minimum in heap, then do nothing
if (compare(key, array[0]) < 0)
	return 0;

while (i < this->size)
	{
	lpos = left_pos(i);
	rpos = right_pos(i);

	// check array out of bound, it's also the stopping condition
 	if (lpos < this->size && rpos < this->size)
		{
	 	if (compare(key, array[lpos]) <= 0 && compare(key, array[rpos]) <= 0)
			break;			// we're smaller then the left and the right so we're done
		else if (compare(array[lpos], array[rpos]) < 0)
			{
			array[i] = array[lpos];
			i = lpos;
			}
		else
			{
			array[i] = array[rpos];
			i = rpos;
			}
		}
	else if (lpos < this->size)			// and rpos > this->size (because this is an else)
		{
		if (compare(key, array[lpos]) > 0)
			{
			array[i] = array[lpos];
			i = lpos;
			}
		else
			break;
		}
	else
		break;			// both lpos and lpos exceed end of array
	}

array[i] = key;
return 1;
}


/*
	ANT_HEAP::MIN_UPDATE()
	----------------------
*/
template <typename T, typename _Compare> void ANT_heap<T, _Compare>::min_update(T key)
{
long long i, lpos, rpos;

for (i = 0; i < this->size; i++)
	if (array[i] == key)
		break;

while (i < this->size)
	{
	lpos = left_pos(i);
	rpos = right_pos(i);
	if ((lpos < this->size) && (rpos < this->size))
		{
		if ((compare(key, array[lpos]) <= 0) && (compare(key, array[rpos]) <= 0))
			break;
		else if (compare(array[lpos], array[rpos]) > 0)
			{
			array[i] = array[rpos];
			i = rpos;
			}
		else
			{
			array[i] = array[lpos];
			i = lpos;
			}
		}
	else if (lpos < this->size)
		{
		if (compare(key, array[lpos]) > 0)
			{
			array[i] = array[lpos];
			i = lpos;
			}
		else
			break;	// we know it's the last one, so just exit the loop
		}
	else
		break;
	}

array[i] = key;
}

/*
	ANT_HEAP::MAX_HEAPIFY()
	-----------------------
*/
template <typename T, typename _Compare> void ANT_heap<T, _Compare>::max_heapify(long long pos)
{
long long left = left_pos(pos);
long long right = right_pos(pos);
long long largest;

if ((left < size) && (compare(array[left], array[pos]) > 0))
	largest = left;
else
	largest = pos;

if ((right < size) && (compare(array[right], array[largest]) > 0))
	largest = right;

if (largest != pos)
	{
	swap(&array[pos], &array[largest]);
	max_heapify(largest);
	}
}

/*
	ANT_HEAP::MAX_HEAPIFY()
	-----------------------
*/
template <typename T, typename _Compare> void ANT_heap<T, _Compare>::max_heapify(long long pos, long long hsize)
{
long long left = left_pos(pos);
long long right = right_pos(pos);
long long largest;

if ((left < hsize) && (compare(array[left], array[pos]) > 0))
	largest = left;
else
	largest = pos;

if ((right < hsize) && (compare(array[right], array[largest]) > 0))
	largest = right;

if (largest != pos)
	{
	swap(&array[largest], &array[pos]);
	max_heapify(largest, hsize);
	}
}

/*
	ANT_HEAP::BUILD_MAX_HEAP()
	--------------------------
*/
template <typename T, typename _Compare> void ANT_heap<T, _Compare>::build_max_heap()
{
for (long long i = size/2-1; i >= 0; i--)
	max_heapify(i);
}

/*
	ANT_HEAP::TEXT_RENDER()
	-----------------------
*/
template <typename T, typename _Compare> void ANT_heap<T, _Compare>::text_render(long long i)
{
#ifdef NEVER
	printf("i: %lld, ", i);
	for (long long i = 0; i < size; i++)
		printf("%lld ", array[i]);
	printf("\n");
#endif
}

/*
	ANT_HEAP::MAX_HEAPSORT()
	------------------------
	Sort in ascending order
*/
template <typename T, typename _Compare> void ANT_heap<T, _Compare>::max_heapsort()
{
build_max_heap();
for (long long i = size-1; i >= 1; i--)
	{
	swap(&array[0], &array[i]);
	max_heapify(0, i);
	}
}

/*
	ANT_HEAP::MIN_HEAPIFY()
	-----------------------
*/
template <typename T, typename _Compare> void ANT_heap<T, _Compare>::min_heapify(long long pos)
{
long long left = left_pos(pos);
long long right = right_pos(pos);
long long smallest;

if ((left < size) && (compare(array[left], array[pos]) < 0))
	smallest = left;
else
	smallest = pos;

if ((right < size) && (compare(array[right], array[smallest]) < 0))
	smallest = right;

if (smallest != pos)
	{
	swap(&array[pos], &array[smallest]);
	min_heapify(smallest);
	}
}

/*
	ANT_HEAP::MIN_HEAPIFY()
	-----------------------
*/
template <typename T, typename _Compare> void ANT_heap<T, _Compare>::min_heapify(long long pos, long long hsize)
{
long long left = left_pos(pos);
long long right = right_pos(pos);
long long smallest;

if ((left < hsize) && (compare(array[left], array[pos])) < 0)
	smallest = left;
else
	smallest = pos;

if ((right < hsize) && (compare(array[right], array[smallest]) < 0))
	smallest = right;

if (smallest != pos)
	{
	swap(&array[pos], &array[smallest]);
	min_heapify(smallest, hsize);
	}
}

/*
	ANT_HEAP::BUILD_MIN_HEAP()
	--------------------------
*/
template <typename T, typename _Compare> void ANT_heap<T, _Compare>::build_min_heap()
{
for (long long i = size/2 - 1; i >= 0; i--)
	min_heapify(i);
}

/*
	ANT_HEAP::MIN_HEAPSORT()
	------------------------
	Sort in descending order
*/
template <typename T, typename _Compare> void ANT_heap<T, _Compare>::min_heapsort()
{
build_min_heap();
for (long long i = size-1; i >= 1; i--)
	{
	swap(&array[0], &array[i]);
	min_heapify(0, i);
	}
}

/*
	ANT_HEAP::get_second_smallest()
	-------------------------------
*/
template <typename T, typename _Compare> T ANT_heap<T, _Compare>::get_second_smallest() {
	T the_left, the_right;
	if (size < 2) {
		perror("error calling heapk::get_second_smallest\n");
		exit(2);
	} else if (size == 2) {
		//printf("0 left pos[%ld]: %ld\n", left_pos(0), array[left_pos(0)]);
		return array[left_pos(0)];
	} else {
		//printf("0 left pos[%ld]: %ld\n", left_pos(0), array[left_pos(0)]);
		//printf("0 right pos[%ld]: %ld\n", right_pos(0), array[right_pos(0)]);
		the_left = array[left_pos(0)];
		the_right = array[right_pos(0)];
		return the_left <= the_right ? the_left : the_right;
	}
}

#endif
