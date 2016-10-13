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
					const KEY key;							///< Data in the tree are keyed on this.
					ELEMENT element;						///< This is the data stored in the tree.
					std::atomic<node *>left;			///< Pointer to the left (smaller than) data.
					std::atomic<node *>right;			///< Pointer to the right (larger than) data.
					
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
				@param current [in] A reference to the current node pointer.
				@paramn ew_node [in] A pointer to the node to add to the tree (do not use, this is used internally to avoid memory wastage)
				@return The element associated with the key, or an empty element if a new node for the key was created.
			*/
			ELEMENT &find_and_add(const KEY &key, std::atomic<node *> &current, node *new_node = nullptr)
				{
				if (current == nullptr)
					{
					/*
						We have a NULL pointer so we've exhausted the search
					*/
					node *empty = nullptr;
					if (new_node == nullptr)
						new_node = new (pool.malloc(sizeof(node), sizeof(void *))) node(key, pool);
					/*
						If the Compare and Swap fails then there are two possible reasons: Either some other thread has created
						this node with the same key, or some other thread has created this node with a different key.  Either way,
						this can be resolved with a recursive call back into this method.  Note that if the tree is undergoing heavy
						change then this might fail several times before finally succeeding (one way or another).
						If the Compare and Swap was successful then the answer is current.load()->element.
					*/
					if (!current.compare_exchange_strong(empty, new_node))
						return find_and_add(key, current, new_node);
					else
						return current.load()->element;
					}
				/*
					Search on the left or the right
				*/
				else if (key < current.load()->key)
					return find_and_add(key, current.load()->right);
				else if (current.load()->key < key)
					return find_and_add(key, current.load()->left);

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
			*/

			void text_render(std::ostream &stream, const std::atomic<node *> &current, size_t depth = 0) const
				{
				if (current != nullptr)
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
				return find_and_add(key, root);
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
				
				puts("binary_tree::PASSED");
				}
	};

	/*
		OPERATOR<<()
		------------
		@brief Dump the contents of a binary_tree down an output stream.
		@param stream [in] The stream to write to.
		@param tree [in] The tree to write.
		@return The stream once the tree has been written.
	*/
	template<typename KEY, typename ELEMENT>
	inline std::ostream &operator<<(std::ostream &stream, const binary_tree<KEY, ELEMENT> &tree)
		{
		tree.text_render(stream);
		return stream;
		}

	}
