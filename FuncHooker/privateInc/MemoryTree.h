/************************************************************************************\
 * FuncHooker - An Andrew Shurney Production                                        *
\************************************************************************************/

/*! \file		MemoryTree.h
 *  \author		Andrew Shurney
 *  \brief		A special kind of bsearch tree where the nodes are the data
 */

/* All Content © 2011 DigiPen (USA) Corporation, all rights reserved.              */

#ifndef MEMORY_TREE_H
#define MEMORY_TREE_H


/*! \brief Describes a binary tree in which the nodes are the data */
class MemoryTree
{
	public:
			//! \brief Generic linked list for holding our blocks and pages
			struct GenericObject
			{
				GenericObject *left;
				GenericObject *right;
			};

	private:

		unsigned objSize;
        GenericObject *root; //!< A linked list of all unused blocks on the pages
		
		GenericObject *Find(GenericObject *node, void *addr) const;
		GenericObject *FindPredessesor(GenericObject *tree, GenericObject **parentPtr) const;
		
		GenericObject *FindFirstInRangeRec(GenericObject *node, void *addrStart, void *addrEnd, GenericObject ***parentNodePtr = nullptr);

	public:
		MemoryTree(unsigned objSize);
		~MemoryTree();
		
		GenericObject *GetRoot() const;

		void BinaryInsert(void *memStart, void *memEnd);
		void Insert(void *obj);
		
		GenericObject *FindFirstInRange(void *addrStart, void *addrEnd, GenericObject ***parentNodePtr = nullptr);
		unsigned CountInRange(GenericObject *node, void *start, void *end) const;

		void EraseRange(GenericObject *start, GenericObject *end, GenericObject **parentPtr);
		void Erase(GenericObject *node, GenericObject **parentPtr);

		static unsigned MinObjectSize();
		
};

#endif
