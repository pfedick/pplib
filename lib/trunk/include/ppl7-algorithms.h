/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 56 $
 * $Date: 2011-04-30 12:15:56 +0000 (Sa, 30 Apr 2011) $
 * $Id: ppl7-types.h 56 2011-04-30 12:15:56Z pafe $
 * $URL: $
 *
 *******************************************************************************
 * Copyright (c) 2011, Patrick Fedick <patrick@pfp.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the copyright holder nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER AND CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************/


#ifndef PPL7ALGORITHMS_H_
#define PPL7ALGORITHMS_H_

namespace ppl7 {

// ***************************************************************************
// AVL-Trees
// ***************************************************************************
#ifndef AVL_MAX_HEIGHT
#define AVL_MAX_HEIGHT 32
#endif

class AVLTreeAlgorithm
{
	public:
		class Node
		{
			friend class AVLTreeAlgorithm;
			public:
				Node *left, *right, *parent;
				signed char balance;
			public:
				Node() {
					left=right=parent=NULL;
					balance=0;
				}
				virtual ~Node() {};
		};

		class Iterator
		{
			friend class AVLTreeAlgorithm;
			private:
			Node	*current;
			Node	*stack[AVL_MAX_HEIGHT];
			size_t	stack_height;
			public:
			Iterator();
		};

	private:
		Node	*root;
		size_t	numElements;
		bool	dupes;

		void	upInsert(Node *node);
		void	upDelete(Node *node);
		Node	*rotate(Node *kn);
		void	swapNodes(Node *item1, Node *item2);

	public:
		PPLNORMALEXCEPTION(MissingCompareOperator);

		AVLTreeAlgorithm();
		virtual ~AVLTreeAlgorithm();
		virtual int	compare(const Node &value1, const Node &value2) const;
		void	allowDupes(bool allow);
		size_t	num() const;
		size_t	count() const;
		void	clear();
		void	addNode(Node *item);
		void	eraseNode(Node *item);
		Node	*findNode(const Node *value) const;
		void	reset(Iterator &it) const;
		Node	*getFirst(Iterator &it) const;
		Node	*getNext(Iterator &it) const;
		Node	*getLast(Iterator &it) const;
		Node	*getPrevious(Iterator &it) const;
		Node	*getCurrent(Iterator &it) const;
		Node	*getRoot() const;

};

template <class K, class T> class AVLTree : private AVLTreeAlgorithm
{
	private:
		class TreeItem : private AVLTreeAlgorithm::Node {
			friend class AVLTree;
			K key;
			T value;
			public:
				void *operator new(size_t, void *p) { return p;}
		};
		Heap		MyHeap;
		void clearRecursive(TreeItem *item) {
			if (item->left) clearRecursive((TreeItem*)item->left);
			item->~TreeItem();
			if (item->right) clearRecursive((TreeItem*)item->right);
		}
	public:
		typedef AVLTreeAlgorithm::Iterator Iterator;
		class Pair {
			private:
				const K *myKey;
				T *myValue;
			public:
				Pair() {
					myKey=NULL; myValue=NULL;
				}
				Pair(const Pair &other) {
					myKey=other.myKey; myValue=other.myValue;
				}
				Pair(const K &key, T &value)
				{
					myKey=&key;
					myValue=&value;
				}
				const K& key() const { return *myKey; }
				T& value() const {return *myValue; }
		};
		AVLTree() {
			MyHeap.init(sizeof(TreeItem),0,100);
		};
		~AVLTree() {
			clear();
		}

		virtual int	compare(const Node &value1, const Node &value2) const
		{
			const TreeItem &v1=static_cast<const TreeItem&>(value1);
			const TreeItem &v2=static_cast<const TreeItem&>(value2);
			if (v1.key<v2.key) return 1;
			if (v1.key>v2.key) return -1;
			return 0;
		}
 		size_t capacity() const
 		{
 			return MyHeap.capacity();
 		}
 		size_t itemSize() const
 		{
 			return sizeof(TreeItem);
 		}
 		void reserve(size_t num)
 		{
 			MyHeap.reserve(num);
 		}

		void	add(const K &key, const T &value)
		{
			TreeItem *item=new (MyHeap.malloc())TreeItem;
			item->key=key;
			item->value=value;
			try {
				AVLTreeAlgorithm::addNode(item);
			} catch (...) {
				delete item;
				throw;
			}
			//printf ("Elements in Heap: %zu\n",MyHeap.count());
		}
		T& find(const K &key) const
		{
			TreeItem search, *found;
			search.key=key;
			found=(TreeItem*)AVLTreeAlgorithm::findNode(&search);
			if (!found) throw ItemNotFoundException();
			return found->value;
		}

		T& operator[](const K &key) const
		{
			TreeItem search, *found;
			search.key=key;
			found=(TreeItem*)AVLTreeAlgorithm::findNode(&search);
			if (!found) throw ItemNotFoundException();
			return found->value;
		}

		void	erase(const K &key)
		{
			TreeItem search, *found;
			search.key=key;
			found=(TreeItem*)AVLTreeAlgorithm::findNode(&search);
			if (!found) throw ItemNotFoundException();
			AVLTreeAlgorithm::eraseNode(found);
			found->~TreeItem();
			MyHeap.free(found);
		}
		void		clear()
		{
			TreeItem *root=(TreeItem*)AVLTreeAlgorithm::getRoot();
			// Destruktoren der einzelnen Elemente aufrufen
			if (root) clearRecursive(root);
			MyHeap.clear();		// kompletten Speicher freigeben
			AVLTreeAlgorithm::clear();

		}
		void		allowDupes(bool allow)
		{
			AVLTreeAlgorithm::allowDupes(allow);
		}
		size_t		num() const
		{
			return AVLTreeAlgorithm::num();
		}
		size_t		count() const
		{
			return AVLTreeAlgorithm::count();
		}

		void	reset(Iterator &it) const
		{
			AVLTreeAlgorithm::reset(it);
		}
		Pair getFirst(Iterator &it) const
		{
			TreeItem *item=(TreeItem*)AVLTreeAlgorithm::getFirst(it);
			if (!item) throw OutOfBoundsEception();
			return Pair(item->key, item->value);
		}
		Pair getNext(Iterator &it) const
		{
			TreeItem *item=(TreeItem*)AVLTreeAlgorithm::getNext(it);
			if (!item) throw OutOfBoundsEception();
			return Pair(item->key, item->value);
		}
		Pair getLast(Iterator &it) const
		{
			TreeItem *item=(TreeItem*)AVLTreeAlgorithm::getLast(it);
			if (!item) throw OutOfBoundsEception();
			return Pair(item->key, item->value);
		}
		Pair getPrevious(Iterator &it) const
		{
			TreeItem *item=(TreeItem*)AVLTreeAlgorithm::getPrevious(it);
			if (!item) throw OutOfBoundsEception();
			return Pair(item->key, item->value);
		}
		Pair getCurrent(Iterator &it) const
		{
			TreeItem *item=(TreeItem*)AVLTreeAlgorithm::getCurrent(it);
			if (!item) throw OutOfBoundsEception();
			return Pair(item->key, item->value);
		}
		Pair getRoot() const
		{
			TreeItem *item=(TreeItem*)AVLTreeAlgorithm::getRoot();
			if (!item) throw OutOfBoundsEception();
			return Pair(item->key, item->value);
		}



};


}	// EOF namespace ppl7


#endif /* PPL7ALGORITHMS_H_ */
