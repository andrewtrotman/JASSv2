/*
	BEAP.H
	------
	Copyright (c) 2020 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Bi-parental heap over a fixed length array (min-beap).
	@author Andrew Trotman
	@copyright 2020 Andrew Trotman
*/
#pragma once

#include <math.h>

#include <vector>
#include <random>
#include <iostream>
#include <algorithm>

#include "asserts.h"

namespace JASS
	{
	/*
		CLASS BEAP
		----------
	*/
	/*!
		@brief Bi-parental heap over a fixed length array (min-beap).
		@details see:
		J.I. Munro, H. Suwanda (1980), Implicit data structures for fast search and update, Journal of Computer and System Sciences, 21(2):236-250, https://doi.org/10.1016/0022-0000(80)90037-9. (http://www.sciencedirect.com/science/article/pii/0022000080900379)
	*/
	template <typename TYPE>
	class beap
		{
		template <typename T>
		friend std::ostream &operator<<(std::ostream &stream, const beap<T> &object);

		private:
			TYPE *array;					///< the array that is the beap
			int64_t elements;				///< elements in the beap
			int64_t height;				///< height of the beap

		private:
			/*
				BEAP::GET_HEIGHT()
				------------------
			*/
			/*!
				@brief Return the row number of the element array[element].  Counting from 0, so get_height(0) == 0, get_height(1) == 1, get_height(2) == 1, etc.
				@param element [in] The index into the array (counting from 0).
				@return The row of the tree that holds array[element].
			*/
			static int64_t get_height(int64_t element)
				{
				return (int64_t)(ceil((sqrt(8 * element + 2) - 1) / 2) - 1);
				}

			/*
				BEAP::GET_FIRST()
				-----------------
			*/
			/*!
				@brief Return the first element of the row at the row given by height.  Both count from  0 (so get_first(0) == 0), get_first(1) == 1).
				@param height [in] The row number.
				@return The index into array[] of the first element of the row given by height.
			*/
			static int64_t get_first(int64_t height)
				{
				return height * (height + 1) / 2;
				}

			/*
				BEAP::GET_LAST()
				----------------
				both count from  0 (so get_last(0) == 0), get_first(1) == 2);
			*/
			/*!
				@brief Return the last element of the row at the row given by height.  Both count from  0 ((so get_last(0) == 0), get_first(1) == 2))
				@param height [in] The row number.
				@return The index into array[] of the last element of the row given by height
			*/
			static int64_t get_last(int64_t height)
				{
				return (height + 1) * (height + 2) / 2 - 1;
				}

			/*
				BEAP::LEFT_PUSH()
				-----------------
			*/
			/*!
				@brief Re-shuffle the beap from current_location towards the root, where current_location is on the left hand edge of the tree.
				@param key [in] the key we're moving up the tree (is not array[current_location] as this is being called from beap_up() which might be partially completed.
				@param current_location [in] The locaiton to reshuffle from.
				@param current_height [in] The row number of current_location.
				@return The new location in array[] of the element that was at current_location.
			*/
			int64_t left_push(const TYPE &key, int64_t current_location, int64_t current_height)
				{
				int64_t parent = current_location - current_height;

				while (current_height > 0 && array[parent] > key)
					{
					array[current_location] = array[parent];
					current_height--;
					current_location = parent;
					parent -= current_height;
					}
				array[current_location] = key;
				return current_location;
				}

			/*
				BEAP::RIGHT_PUSH()
				------------------
			*/
			/*!
				@brief Re-shuffle the beap from current_location towards the root, where current_location is on the right hand edge of the tree.
				@param key [in] the key we're moving up the tree (is not array[current_location] as this is being called from beap_up() which might be partially completed.
				@param current_location [in] The locaiton to reshuffle from.
				@param current_height [in] The row number of current_location.
				@return The new location in array[] of the element that was at current_location.
			*/
			int64_t right_push(const TYPE &key, int64_t current_location, int64_t current_height)
				{
				int64_t parent = current_location - current_height - 1;
				while (current_height > 0 && array[parent] > key)
					{
					array[current_location] = array[parent];
					current_height--;
					current_location = parent;
					parent -= current_height + 1;
					}
				array[current_location] = key;
				return current_location;
				}

		public:
			/*
				BEAP::BEAP()
				------------
				min beap (the lowest value is at array[0]
			*/
			/*!
				@brief Constructor of min-beap (array[0] is the smallest element and values increase as you go down the tree).
				@param array [in] The array to use as the beap.  This is assumed to be pre-populated, full, and out of order
				@param elements [in] The number of elements in the array (and therefore the beap) to use
			*/
			beap(TYPE *array, int64_t elements):
				array(array),
				elements(elements),
				height(get_height(elements - 1))
				{
				/* Nothing */
				}

			/*
				BEAP::SET_TOP_K()
				-----------------
			*/
			/*!
				@brief Set the new size of the beap.  Useful when constructed
			*/
			void set_top_k(int64_t elements)
				{
				this->elements = elements;
				height = get_height(elements - 1);
				}

			/*
				BEAP::MAKE_BEAP()
				-----------------
			*/
			/*!
				@brief Construct the beap over array[]
			*/
			void make_beap()
				{
				std::sort(array, array + elements);
				}

			/*
				BEAP::ISBEAP()
				--------------
			*/
			/*!
				@brief Check the beap to make sure it is, indeed, a beap.
				@return true if array[] is a beap, false otherwise.
			*/
			bool isbeap(void) const
				{
				int64_t end_of_row = 0;
				for (int64_t current_height = 0; current_height < height; current_height++)
					{
					end_of_row += get_last(current_height + 1);
					int64_t end_of_current_row = get_last(current_height);
					for (int64_t current_location = get_first(current_height); current_location <= end_of_current_row; current_location++)
						{
						int64_t child1 = current_location + current_height + 1;
						int64_t child2 = std::min(current_location + current_height + 2, elements - 1);

						if (child1 >= elements)
							return true;
						if (array[current_location] > array[child1] || array[current_location] > array[child2])
							return false;
						}
					}
				return true;
				}
				
			/*
				BEAP::FIND()
				------------
			*/
			/*!
				@brief Find an instance of key in the beap and return its index into array[].
				@param key [in] The key to look for.
				@return The index into array[] of an instance of key, or -1 if key cannot be found in the beap.
			*/
			int64_t find(const TYPE &key) const
				{
				/*
					Start in the bottom right corner of the beap
				*/
				int64_t current_height = height;
				int64_t current_location = get_first(height);
				int64_t end_of_row = get_last(height);

				do
					{
					if (array[current_location] > key)
						{
						/*
							If key is less then the current location them move up one level of the beap
						*/
						current_location -= current_height;
						current_height--;
						end_of_row -= current_height + 2;
						if (current_location < 0)
							return -1;
						}
					else if (key > array[current_location])
						{
						/*
							If key is greater then either go down (if we can) or across (if we're on the bottom row)
						*/
						if (current_height == height || current_location + current_height + 2 >= elements)
							{
							/*
								Can't go down so go across
							*/
							current_location++;
							if (current_location > end_of_row)
								return -1;
							if (current_location >= elements)
								{
								/*
									In this case we can't go across because we're on the bottom row and have a partially full row that we're at the edge of, so
									the next largest value might be up and to the right (i.e. the bottom row of the previous full row).  In other words, the
									full row above are the leaves and we need to go across a leaf, but that leaf isn't on the bottom ro because bottom row isn't full.
								*/
								current_location -= current_height;
								current_height--;
								end_of_row -= current_height + 2;
								// in this case we went accross, but we went past the end of the array so we now need to go up
								}
							}
						else
							{
							/*
								Go down because we can.
							*/
							current_location += current_height + 2;
							current_height++;
							end_of_row += current_height + 1;
							if (current_location > end_of_row)
								return -1;
							}
						}
					else
						{
						/*
							Not less than and not greater than so we've found the key in the beap
						*/
						return current_location;
						}
					}
				while (1);

				return -1;
				}

			/*
				BEAP::BEAP_DOWN()
				-----------------
			*/
			/*!
				@brief Re-shuffle the beap from current_location towards the leaves.
				@param current_location [in] The location to reshuffle from.
				@return The new location in array[] of the element that was at current_location.
			*/
			int64_t beap_down(const TYPE &key, int64_t current_location)
				{
				int64_t current_height = get_height(current_location);

				do
					{
					int64_t child1 = current_location + current_height + 1;
					int64_t child2 = current_location + current_height + 2;

					if (child1 >= elements)
						{
						/*
							On the second to last row, but the last row in incomplete, so we can't move down, so we're in the right place.
						*/
						array[current_location] = key;
						return current_location;
						}
					else if (key > array[child1])
						{
						/*
							Move down, swap with the largest child.
						*/
						if (child2 >= elements)
							{
							/*
								On the second to last row, but the last row in incomplete, so we can't move down, so we're in the right place.
							*/
							array[current_location] = array[child1];
							array[child1] = key;
							return child1;
							}

						/*
							Swap with the smallest child
						*/
						if (array[child2] > array[child1])
							{
							array[current_location] = array[child1];
							current_location = child1;
							}
						else
							{
							array[current_location] = array[child2];
							current_location = child2;
							}
						}
					else if (child2 < elements && key > array[child2])
						{
						/*
							We're between child 1 and child 2
						*/
						array[current_location] = array[child2];
						current_location = child2;
						}
					else
						{
						/*
							We're in the correct place
						*/
						array[current_location] = key;
						return current_location;
						}

					current_height++;
					}
				while (1);

				return -1;		// cannot happen
				}

			/*
				BEAP::BEAP_UP()
				---------------
			*/
			/*!
				@brief Re-shuffle the beap from current_location towards the root.
				@param current_location [in] The locaiton to reshuffle from.
				@return The new location in array[] of the element that was at current_location.
			*/
			int64_t beap_up(const TYPE &key, int64_t current_location)
				{
				int64_t current_height = get_height(current_location);
				int64_t end_of_row = get_last(current_height);

				do
					{
					int64_t start_of_row = end_of_row - current_height;

					/*
						If we're at and edge then we only need to do a linear search, special handeling is done in left_push() and right_push()
					*/
					if (current_location == start_of_row)
						return left_push(key, current_location, current_height);
					else if (current_location == end_of_row)
						return right_push(key, current_location, current_height);
					else
						{
						/*
							We're in the middle of the tree
						*/
						int64_t parent1 = current_location - current_height - 1;
						int64_t parent2 = current_location - current_height;

						if (current_location <= 0)
							{
							/*
								We're at the root of the beap
							*/
							array[0] = key;
							return 0;
							}
						else if (array[parent1] > key)
							{
							/*
								We need to shuffle up towards the root, so swap with the largest parent.
							*/
							if (array[parent1] > array[parent2])
								{
								array[current_location] = array[parent1];
								current_location = parent1;
								}
							else
								{
								array[current_location] = array[parent2];
								current_location = parent2;
								}
							}
						else if (array[parent2] > key)
							{
							/*
								We're between each parent so swap with the largest parent.
							*/
							array[current_location] = array[parent2];
							current_location = parent2;
							}
						else
							{
							/*
								Not smaller than either partnt so in the right place.
							*/
							array[current_location] = key;
							return current_location;
							}
						}

					current_height--;
					end_of_row -= current_height + 2;
					}
				while (1);

				return -1;
				}

			/*
				BEAP::GUARANTEED_REPLACE_WITH_SMALLER()
				---------------------------------------
			*/
			/*!
				@brief Find an instance of old_key (which the caller guarantees to exist), replace it with guaranteed smaller key, new_key, and reshuffle the beap to maintain the beap property
				@param old_key [in] The old key (the one to find)
				@param new_key [in] The new key (an instance old_key is replaced by new_key).
				@return -1 if old_key cannot be found, or the index into array of the new location of new_key
			*/
			int64_t guaranteed_replace_with_smaller(const TYPE &old_key, const TYPE &new_key)
				{
				return beap_up(new_key, find(old_key));
				}

 			/*
				BEAP::REPLACE_WITH_SMALLER()
				----------------------------
			*/
			/*!
				@brief Find an instance of old_key, replace it with guaranteed smaller key, new_key, and reshuffle the beap to maintain the beap property
				@param old_key [in] The old key (the one to find)
				@param new_key [in] The new key (an instance old_key is replaced by new_key).
				@return -1 if old_key cannot be found, or the index into array of the new location of new_key
			*/
			int64_t replace_with_smaller(const TYPE &old_key, const TYPE &new_key)
				{
				/*
					Find the old key
				*/
				int64_t location = find(old_key);
				if (location < 0)
					return -1;

				/*
					Insert and rebuild the beap
				*/
				return beap_up(new_key, location);
				}

			/*
				BEAP::REPLACE_SMALLEST_WITH()
				-----------------------------
			*/
			/*!
				@brief Replace the smallest element in the beap with guaranteed larger key, new_key, and reshuffle the beap to maintain the beap property
				@param new_key [in] The new key (an instance old_key is replaced by new_key).
				@return -1 if old_key cannot be found, or the index into array of the new location of new_key
			*/
			int64_t replace_smallest_with(const TYPE &new_key)
				{
				return beap_down(new_key, 0);
				}


			/*
				BEAP::GUARANTEED_REPLACE_WITH_LARGER()
				--------------------------------------
			*/
			/*!
				@brief Find an instance of old_key (which the caller guarantees to exist), replace it with guaranteed larger key, new_key, and reshuffle the beap to maintain the beap property
				@param old_key [in] The old key (the one to find)
				@param new_key [in] The new key (an instance old_key is replaced by new_key).
				@return -1 if old_key cannot be found, or the index into array of the new location of new_key
			*/
			int64_t guaranteed_replace_with_larger(const TYPE &old_key, const TYPE &new_key)
				{
				return beap_down(new_key, find(old_key));
				}

			/*
				BEAP::REPLACE_WITH_LARGER()
				---------------------------
			*/
			/*!
				@brief Find an instance of old_key, replace it with guaranteed larger key, new_key, and reshuffle the beap to maintain the beap property
				@param old_key [in] The old key (the one to find)
				@param new_key [in] The new key (an instance old_key is replaced by new_key).
				@return -1 if old_key cannot be found, or the index into array of the new location of new_key
			*/
			int64_t replace_with_larger(const TYPE &old_key, const TYPE &new_key)
				{
				/*
					Find the old key
				*/
				int64_t location = find(old_key);
				if (location < 0)
					return -1;

				/*
					Insert and rebuild the beap
				*/
				return beap_down(new_key, location);
				}

			/*
				BEAP::REPLACE()
				---------------
			*/
			/*!
				@brief Find an instance of old_key, replace it with new_key, and reshuffle the beap to maintain the beap property
				@param old_key [in] The old key (the one to find)
				@param new_key [in] The new key (an instance old_key is replaced by new_key).
				@return -1 if old_key cannot be found, or the index into array of the new location of new_key
			*/
			int64_t replace(const TYPE &old_key, const TYPE &new_key)
				{
				return old_key > new_key ? replace_with_smaller(old_key, new_key) : replace_with_larger(old_key, new_key);
				}

			/*
				BEAP::UNITTEST()
				----------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void)
				{
				/*
					Start with aa random order of integers.
				*/
				std::vector<int> sequence(100);
				std::iota(sequence.begin(), sequence.end(), 1400);
				std::random_device random_number_generator;
				std::shuffle(sequence.begin(), sequence.end(), std::knuth_b(random_number_generator()));

				/*
					Turn that list of integers into a beap
				*/
				JASS::beap<int> my_beap(sequence.data(), sequence.size());
				my_beap.make_beap();

				/*
					Check the helper functions.
				*/
				JASS_assert(my_beap.get_height(0) == 0);
				JASS_assert(my_beap.get_first(0) == 0);
				JASS_assert(my_beap.get_last(0) == 0);

				JASS_assert(my_beap.get_height(3) == 2);
				JASS_assert(my_beap.get_height(5) == 2);
				JASS_assert(my_beap.get_first(2) == 3);
				JASS_assert(my_beap.get_last(2) == 5);

				JASS_assert(my_beap.get_height(406) == 28);
				JASS_assert(my_beap.get_height(434) == 28);
				JASS_assert(my_beap.get_first(28) == 406);
				JASS_assert(my_beap.get_last(28) == 434);

				/*
					Randomly change elements, makeing sure we have a beap at the end.
				*/
				for (size_t x = 0; x < 1000000; x++)
					{
					size_t location = random_number_generator() % sequence.size();
					my_beap.replace(sequence[location], sequence[location] + (random_number_generator() % 100) - 50);
					JASS_assert(my_beap.isbeap());
					}

				puts("beap::PASS");
				}
		};

	/*
		OPERATOR<<()
		------------
	*/
	template <typename TYPE>
	std::ostream &operator<<(std::ostream &stream, const beap<TYPE> &object)
		{
		for (int64_t current_height = 0; current_height <= object.height; current_height++)
			{
			stream.width(3);
			stream << current_height << ":";
			for (int64_t element = object.get_first(current_height); element <= object.get_last(current_height); element++)
				{
				stream.width(3);
				if (element < object.elements)
					stream << object.array[element] << " ";
				}
			stream << "\n";
			}
		return stream;
		}
	}
