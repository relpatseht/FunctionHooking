/************************************************************************************\
 * OSUtilities - An Andrew Shurney Production                                       *
\************************************************************************************/

/*! \file		PrioirtyBlock.h
 *  \author		Andrew Shurney
 *  \brief		Scoped thread priority changer
 */

/* All Content © 2011 DigiPen (USA) Corporation, all rights reserved.              */

#ifndef PRIORITY_BLOCK_H
#define PRIORITY_BLOCK_H

class PriorityBlock
{
	private:
		int oldPriority;
		int newPriority;

	public:
		// newPriority should be between [-100, 100], with 100 being 
		// fastest and 0 being normal.
		PriorityBlock(int newPriority);
		~PriorityBlock();
};

#endif
