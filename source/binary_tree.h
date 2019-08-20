/*
	BINARY_TREE.H
	-------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Thread-safe binary tree.
	@author Andrew Trotman
	@copyright 2016 Andrew Trotman
*/
#pragma once

#include <stdio.h>

#include <atomic>
#include <sstream>
#include <iostream>
#include <utility>

#include "allocator_pool.h"

namespace JASS
	{
	/*
		CLASS BINARY_TREE
		-----------------
	*/
	/*!
		@brief Thread-safe unballanced binary tree that uses an allocator, and consequently never calls delete on elements or keys.
		@details Data is kept in nodes and in-order with low on the left and high on the right.
		Elements are addressed using the operator[key] syntax seen with a std::map.  Note that this syntax makes it impossible to have duplicate keys.
		There is no way to remove an element from the binary tree once added.  Note that KEY and ELEMENT must take the allocator in their constructors.
		@tparam KEY The type used as the key to the element (must include KEY(allocator, KEY) copy constructor)
		@tparam ELEMENT The element data returned given the key (must include ELEMENT(allocator) constructur)
	*/
	template <typename KEY, typename ELEMENT>
	class binary_tree
		{
		/*!
			@brief Output a human readable serialisation to an ostream
			@relates binary_tree
		*/
		template<typename A, typename B> friend std::ostream &operator<<(std::ostream &stream, const binary_tree<A, B> &tree);

		protected:
			/*
				CLASS BINARY_TREE::NODE
				-----------------------
			*/
			/*!
				@brief A node within the binary tree
			*/
			class node
				{
				public:
					const KEY key;												///< Data in the tree are keyed on this.
					ELEMENT element;											///< This is the data stored in the tree.
					std::atomic<node *>left;								///< Pointer to the left (smaller than) data.
					std::atomic<node *>right;								///< Pointer to the right (larger than) data.
					std::atomic<node *>parent;								///< Pointer to the parent node.
					
				public:
					/*
						BINARY_TREE::NODE()
						-------------------
					*/
					/*!
						@brief Constructor
						@param key [in] The key to the element data.
						@param pool [in] The pool allocator use for keys and elements.
					*/
					node(const KEY &key, allocator &pool) :
						key(pool, key),
						element(pool)
						{
						left = right = nullptr;
						}
				};
		public:
			/*
				CLASS BINARY_TREE::ITERATOR
				---------------------------
			*/
			/*!
				@brief Iterate over the binary tree.
				@details This is an adaptation of the algorithm given by polygenelubricants on stackoverflow here: http://stackoverflow.com/questions/2942517/how-do-i-iterate-over-binary-tree
				The details of which are given on that thread but repeated here:
				
				@verbatim
					What you're looking for is a successor algorithm.

					Here's how it can be defined:

					*First rule: The first node in the tree is the leftmost node in the tree.
					*Next rule: The successor of a node is:
						*Next-R rule: If it has a right subtree, the leftmost node in the right subtree.
						*Next-U rule: Otherwise, traverse up the tree
							*If you make a right turn (i.e. this node was a left child), then that parent node is the successor
							*If you make a left turn (i.e. this node was a right child), continue going up.
							*If you can't go up anymore, then there's no successor
							
					As you can see, for this to work, you need a parent node pointer.
				@endverbatim
			 
				Source code in Java is provided in that thread.
			*/
			class iterator
				{
				private:
					node *location;			///< The node that is currently being examined.
					
				public:
					/*
						BINARY_TREE::ITERATOR::ITERATOR()
						---------------------------------
					*/
					/*!
						@brief Constructor
					*/
					iterator(node *current) :
						location(current)
						{
						if (location != nullptr)
							get_left_most();
						}
					
					/*
						BINARY_TREE::ITERATOR::OPERATOR*()
						----------------------------------
					*/
					/*!
						@brief Return a reference to the object at the current location.
						@return The current object.
					*/
					const std::pair<const KEY &, const ELEMENT &> operator*() const
						{
						return std::make_pair(std::ref(location->key), std::ref(location->element));
						}
					
					/*
						BINARY_TREE::ITERATOR::OPERATOR!=()
						-----------------------------------
					*/
					/*!
						@brief Compare two iterator objects for non-equality.
						@param other [in] The iterator object to compare to.
						@return true if they differ, else false.
					*/
					bool operator!=(const iterator &other) const
						{
						return location != other.location;
						}
					
					/*
						BINARY_TREE::ITERATOR::GET_LEFT_MOST()
						--------------------------------------
					*/
					/*!
						@brief Set the current pointer to the left-most node beneath the current location
					*/
					void get_left_most(void)
						{
						while (location->left.load() != nullptr)
							location = location->left;
						}
		
					/*
						BINARY_TREE::ITERATOR::OPERATOR++()
						-----------------------------------
					*/
					/*!
						@brief Increment this iterator.
					*/
					iterator &operator++()
						{
						if (location->right.load() != nullptr)
							{
							/*
								If there is a right subtree then the left-most node in it is the next location (the Next-R rule)
							*/
							location = location->right;
							get_left_most();
							}
						else
							{
							/*
								Otherwise move up the tree (the Next-U rule)
							*/
							while (location->parent.load() != nullptr && location == location->parent.load()->right.load())
								location = location->parent;
								
							location = location->parent;
							}
						/*
							We must return something.
						*/
						return *this;
						}
				};
			
		protected:
			allocator &pool;							///< The pool allocator
			std::atomic<node *> root;				///< The root of the binary tree

		protected:
			/*
				BINARY_TREE::FIND_AND_ADD()
				---------------------------
			*/
			/*!
				@brief If the key exists in the tree then return the data associated with it, else create empty data for the key
				@param key [in] The key to search for.
				@param parent [in] A pointer to the parent node (used as the "up" pointer for a new node).
				@param current [in] A reference to the current node pointer.
				@param new_node [in] A pointer to the node to add to the tree (do not use, this is used internally to avoid memory wastage).
				@return The element associated with the key, or an empty element if a new node for the key was created.
			*/
			ELEMENT &find_and_add(const KEY &key, node *parent, std::atomic<node *> &current, node *new_node = nullptr)
				{
				if (current.load() == nullptr)
					{
					/*
						We have a NULL pointer so we've exhausted the search
					*/
					node *empty = nullptr;
					if (new_node == nullptr)
						new_node = new (pool.malloc(sizeof(node), sizeof(void *))) node(key, pool);
					new_node->parent = parent;
					/*
						If the Compare and Swap fails then there are two possible reasons: Either some other thread has created
						this node with the same key, or some other thread has created this node with a different key.  Either way,
						this can be resolved with a recursive call back into this method.  Note that if the tree is undergoing heavy
						change then this might fail several times before finally succeeding (one way or another).
						If the Compare and Swap was successful then the answer is current.load()->element.
					*/
					if (!current.compare_exchange_strong(empty, new_node))
						return find_and_add(key, parent, current, new_node);
					else
						return current.load()->element;
					}
				/*
					Search on the left or the right
				*/
				else if (key < current.load()->key)
					return find_and_add(key, current.load(), current.load()->right);
				else if (current.load()->key < key)
					return find_and_add(key, current.load(), current.load()->left);

				/*
					Found the element, or we created one.
				*/
				return current.load()->element;
				}
			
		protected:
			/*
				BINARY_TREE::TEXT_RENDER()
				--------------------------
			*/
			/*!
				@brief Write the contents of this object to the output steam.
				@param stream [in] The stream to write to.
			*/
			void text_render(std::ostream &stream) const
				{
				text_render(stream, root);
				}
			
			/*
				BINARY_TREE::TEXT_RENDER()
				--------------------------
			*/
			/*!
				@brief Helper function for writing to output streams.
				@param stream [in] The stream to write to.
				@param current [in] A reference to the node to write.
				@param depth [in] The level of recursion (used for spacing).
			*/

			void text_render(std::ostream &stream, const std::atomic<node *> &current, size_t depth = 0) const
				{
				if (current.load() != nullptr)
					{
					/*
						In-order traversal.
					*/
					text_render(stream, current.load()->left, depth + 1);
					stream << std::string(2 * depth, ' ');
					stream << current.load()->key << "->" << current.load()->element << '\n';
					text_render(stream, current.load()->right, depth + 1);
					}
				}
			

		public:
			/*
				BINARY_TREE::BINARY_TREE()
				--------------------------
			*/
			/*!
				@brief Constructor
				@param pool [in] The allocator used for all storage within this tree.
			*/
			binary_tree(allocator &pool) : pool(pool), root(nullptr)
				{
				/*
					Nothing
				*/
				}

			/*
				BINARY_TREE::BEGIN()
				--------------------
			*/
			/*!
				@brief Return an iterator pointing to left-most (smallest) node in the tree.
				@return Iterator pointing to smallest member of the tree.
			*/
			iterator begin() const
				{
				return iterator(root);
				}

			/*
				BINARY_TREE::END()
				------------------
			*/
			/*!
				@brief Return an iterator pointing past the end of the tree.
				@return Iterator pointing past the end of the tree.
			*/
			iterator end() const
				{
				return iterator(nullptr);
				}

			/*
				BINARY_TREE::OPERATOR[]()
				-------------------------
			*/
			/*!
				@brief Return a reference to the element stored for the given key.  If no element is stored for the key then a new empty element is made.
				@param key [in] They key to find the data for.
				@return The element associated with the key - or an empty element if no key previously existed.
			*/
			ELEMENT &operator[](const KEY &key)
				{
				return find_and_add(key, nullptr, root);
				}
			
			/*
				BINARY_TREE::UNITTEST()
				-----------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void)
				{
				/*
					Check adding to a tree and serialising it.
				*/
				allocator_pool pool;
				binary_tree<slice, slice> tree(pool);
				
				tree[slice("5")] = slice("five");
				tree[slice("3")] = slice("three");
				tree[slice("7")] = slice("seven");
				tree[slice("4")] = slice("four");
				tree[slice("2")] = slice("two");
				tree[slice("1")] = slice("one");
				tree[slice("9")] = slice("nine");
				tree[slice("6")] = slice("six");
				tree[slice("8")] = slice("eight");
				tree[slice("0")];

				const char *answer = "    9->nine\n      8->eight\n  7->seven\n    6->six\n5->five\n    4->four\n  3->three\n    2->two\n      1->one\n        0->\n";
				
				std::ostringstream serialised;
				serialised << tree;
				JASS_assert(strcmp(serialised.str().c_str(), answer) == 0);

				/*
					Check the tree iterator
				*/
				std::ostringstream output;
				for (const auto key : tree)
					output << key.first;
				
				JASS_assert(output.str() == "9876543210");
				
				puts("binary_tree::PASSED");
				}
	};

	/*
		OPERATOR<<()
		------------
	*/
	/*!
		@brief Dump the contents of a binary_tree down an output stream.
		@param stream [in] The stream to write to.
		@param tree [in] The tree to write.
		@tparam KEY The type used as the key to the element (must include KEY(allocator, KEY) copy constructor)
		@tparam ELEMENT The element data returned given the key (must include ELEMENT(allocator) constructur)
		@return The stream once the tree has been written.
	*/
	template<typename KEY, typename ELEMENT>
	inline std::ostream &operator<<(std::ostream &stream, const binary_tree<KEY, ELEMENT> &tree)
		{
		tree.text_render(stream);
		return stream;
		}
	}
