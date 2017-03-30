/************************************************************************************\
 * FuncHooker - An Andrew Shurney Production                                        *
\************************************************************************************/

/*! \file		MemoryTree.cpp
 *  \author		Andrew Shurney
 *  \brief		A special kind of bsearch tree where the nodes are the data
 */

#include "privateInc/MemoryTree.h"
#include <cstdint>
#include <algorithm>

MemoryTree::MemoryTree(unsigned objSize) : objSize(objSize), root(nullptr)
{
}

MemoryTree::~MemoryTree()
{
}

MemoryTree::GenericObject *MemoryTree::GetRoot() const
{
	return root;
}

void MemoryTree::BinaryInsert(void *memStartPtr, void *memEndPtr)
{
	uint8_t *memStart = reinterpret_cast<uint8_t*>(memStartPtr);
	uint8_t *memEnd = reinterpret_cast<uint8_t*>(memEndPtr);

	if(memEnd < memStart+objSize)
		return;

	unsigned objectsLeft = static_cast<unsigned>(memEnd - memStart) / objSize;
	unsigned mid = objectsLeft / 2;
	uint8_t *midAddr = memStart + mid*objSize;

	Insert(midAddr);

	BinaryInsert(midAddr + objSize, memEnd);
	BinaryInsert(memStart, midAddr);
}

void MemoryTree::Insert(void *objPtr)
{
	GenericObject *obj = reinterpret_cast<GenericObject*>(objPtr);

	obj->left = obj->right = NULL;

	if(!root)
	{
		root = obj;
		return;
	}

	GenericObject *parent = NULL;
	GenericObject *curNode = root;
	while(curNode)
	{
		parent = curNode;

		if(obj < curNode)
			curNode = curNode->left;
		else
			curNode = curNode->right;
	}

	if(obj < parent)
		parent->left = obj;
	else
		parent->right = obj;
}

MemoryTree::GenericObject *MemoryTree::FindFirstInRange(void *start, void *end, GenericObject ***parentNodePtr)
{
	if(parentNodePtr)
		*parentNodePtr = &root;

	return FindFirstInRangeRec(root, start, end, parentNodePtr);
}

unsigned MemoryTree::CountInRange(GenericObject *node, void *start, void *end) const
{
	if(!node || node < start || node > end)
		return 0;

	return CountInRange(node->left, start, end) + CountInRange(node->right, start, end) + 1;
}

void MemoryTree::EraseRange(GenericObject *start, GenericObject *end, GenericObject **parentPtr)
{
	if(!start || !end)
		throw std::exception();

	if(!parentPtr)
		parentPtr = &root;

	if(!start->left && !end->right)
		*parentPtr = NULL;
	else if(!start->left && end->right)
	{
		*parentPtr = end->right;
	}
	else if(start->left && !end->right)
	{
		*parentPtr = start->left;
	}
	else
	{
		GenericObject *newRootParent;
		GenericObject *newRoot = FindPredessesor(start, &newRootParent);

		*parentPtr = newRoot;
		
		if(newRootParent->left == newRoot)
			newRootParent->left = NULL;
		else
			newRootParent->right = NULL;

		newRoot->left = start->left;
		newRoot->right = end->right;
	}
}

void MemoryTree::Erase(GenericObject *node, GenericObject **parentPtr)
{
	if(!node)
		throw std::exception();

	if(!parentPtr)
		parentPtr = &root;

	if(!node->left && !node->right)
		*parentPtr = NULL;
	else if(!node->left && node->right)
	{
		*parentPtr = node->right;
	}
	else if(node->left && !node->right)
	{
		*parentPtr = node->left;
	}
	else
	{
		GenericObject *newRootParent;
		GenericObject *newRoot = FindPredessesor(node, &newRootParent); 

		*parentPtr = newRoot;

		if(newRootParent->left == newRoot)
			newRootParent->left = NULL;
		else
			newRootParent->right = NULL;

		newRoot->left = node->left;
		newRoot->right = node->right;
	}
}

//
// Static
//

unsigned MemoryTree::MinObjectSize()
{
	return sizeof(GenericObject);
}

//
// Private
//

MemoryTree::GenericObject *MemoryTree::Find(GenericObject *node, void *addr) const
{
	if(!node)
		return NULL;

	if(node == addr)
		return reinterpret_cast<GenericObject*>(addr);
	else if(addr < node)
		return Find(node->left, addr);
	else
		return Find(node->right, addr);
}

MemoryTree::GenericObject *MemoryTree::FindPredessesor(GenericObject *tree, GenericObject **parentPtr) const
{
	*parentPtr = tree;

	GenericObject *prev = tree->left;
	while(prev->right)
	{
		*parentPtr = prev;
		prev = prev->right;
	}

	return prev;
}

MemoryTree::GenericObject *MemoryTree::FindFirstInRangeRec(GenericObject *node, void *start, void *end, GenericObject ***parentNodePtr)
{
	if(!node)
		return NULL;

	if(end < start)
		std::swap(end, start);

	if(node >= start || node <= end)
		return reinterpret_cast<GenericObject*>(node);
	else
	{
		if(parentNodePtr)
			*parentNodePtr = &node;

		 if(end < node)
			return FindFirstInRangeRec(node->left, start, end, parentNodePtr);
		else
			return FindFirstInRangeRec(node->right, start, end, parentNodePtr);
	}
}
