/*H************************************************************************************************/
/*!

    \File    AptAllocator.h

    \Description

      Summary:

        Provides an allocator interface for EASTL.

    \Copyright
        (c) 2020 Electronic Arts Inc.

    \Version    1.0        06/02/20 (nmeyer) First version.
*/
/************************************************************************************************H*/


#ifndef _AptAllocator_h
#define _AptAllocator_h


/*** Include files ********************************************************************************/
#include "Module.h"
#include "AptDefine.h"

/*** Defines **************************************************************************************/

/*** Macros ***************************************************************************************/

/*** Type Definitions *****************************************************************************/

/*** Functions ************************************************************************************/


class FB_APT_API AptStdAllocator
{
	public:
		AptStdAllocator();
		AptStdAllocator(const char* name);

	public:
		void* allocate(size_t n, int flags = 0);
		void* allocate(size_t n, size_t alignment, size_t offset, int flags = 0);
		void  deallocate(void* p, std::size_t n);

		const char* get_name() const;
		void        set_name(const char* pName);

	private:
		const char* m_name;
};

#endif
