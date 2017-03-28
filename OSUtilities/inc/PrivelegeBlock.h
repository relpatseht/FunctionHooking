/************************************************************************************\
 * OSUtilities - An Andrew Shurney Production                                       *
\************************************************************************************/

/*! \file		PrivelegeBlock.h
 *  \author		Andrew Shurney
 *  \brief		Scoped memory privelege changer
 */

/* All Content © 2011 DigiPen (USA) Corporation, all rights reserved.              */

#ifndef PRIVELEGE_BLOCK_H
#define PRIVELEGE_BLOCK_H

/* Assumes all pages in the region specified have the same protection */
class PrivelegeBlock
{
	private:
		void *addr;
		unsigned size;
		unsigned oldPrivelege;

        PrivelegeBlock(const PrivelegeBlock&);            // Do not implement
        PrivelegeBlock& operator=(const PrivelegeBlock&); // Do not implement

	public:
		enum
		{
			READ = 0x1,
			WRITE = 0x2,
			EXECUTE = 0x4,
			ALL = READ | WRITE | EXECUTE
		};

		PrivelegeBlock(void *addr, unsigned size, unsigned privelege);
		~PrivelegeBlock();
};

#endif
