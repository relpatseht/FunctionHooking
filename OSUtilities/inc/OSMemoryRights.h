/************************************************************************************\
 * OSUtilities - An Andrew Shurney Production                                       *
\************************************************************************************/

/*! \file		OSMemoryRights.h
 *  \author		Andrew Shurney
 *  \brief		Platform independed memory right translation
 */

/* All Content © 2011 DigiPen (USA) Corporation, all rights reserved.              */

#ifndef OS_MEMORY_RIGHTS_H
#define OS_MEMORY_RIGHTS_H

class OSMemoryRights
{
	public:
		enum
		{
			NO_ACCESS     = 0x0,
			READ          = 0x1,
			WRITE         = 0x2,
			EXECUTE       = 0x4,
			COPY_ON_WRITE = 0x8
		};

		/*! \brief Takes a bitfield of the Access enum and spits out OS specific flags. 
		
			\return OS specific access or ~0u if unsupported.
		*/
		static unsigned TranslateAccessToOS(unsigned access);

		/*! \brief Takes an OS specific access unsigned and translates it to a bitfield of the Access enum. */
		static unsigned TranslateAccessFromOS(unsigned osAccess);
};

#endif
