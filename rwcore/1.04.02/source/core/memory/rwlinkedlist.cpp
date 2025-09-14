// (c) Electronic Arts. All Rights Reserved.

/// \file
/// A general linked list implementation.

/*************************************************************************************************************
 Includes
 */
#include "rwsharedheaders.hpp"
#include "rw/core/memory/linkedlist.h"

namespace rw
{

/*************************************************************************************************************
 Defines
 */

/*************************************************************************************************************
 Enums
 */

/*************************************************************************************************************
 Typedef Enums
 */

/*************************************************************************************************************
 Typedef Structs + Unions
 */

/*************************************************************************************************************
 Typedef Function Pointers
 */

/*************************************************************************************************************
 Structs + Unions
 */

/*************************************************************************************************************
 Static Variables
 */

/*************************************************************************************************************
 Global Variables
 */

/*************************************************************************************************************
 Function Prototypes
 */

/*************************************************************************************************************
 Static Functions
 */

/*************************************************************************************************************
 Internal Functions
 */

/*************************************************************************************************************
 External Functions
 */

/**
Clears memory set aside for a linked list, so the other linked list functions
can be used on it.

\param list A pointer to the new memory to use as a linked list.
 */
void
LinkListInitialize(LinkList *list)
{
    rwFUNCTION();

    /* The linked list terminator element is embedded in the linked
     * list structure to save a second memory allocation */
    list->terminator.next = &list->terminator;
    list->terminator.prev = &list->terminator;

    rwRETURNVOID();
}

/**
Initializes memory to be an empty linked list element.  This function should
be called before adding a link to a linked list.

\param link Pointer to the memory to initialize as a link.
 */
void
LLLinkInitialize(LLLink *RWUNUSED_RELEASE(link))
{
    rwFUNCTION();

#ifdef EA_DEBUG

#if (8 == rwHOST_POINTERSIZE)
    link->next = (LLLink *)RWUINT64CONST(0xbadbeefbadbeef);
    link->prev = (LLLink *)RWUINT64CONST(0xbadbeefbadbeef);
    link->data = (void *)RWUINT64CONST(0xbadbeefbadbeef);
#elif (4 == rwHOST_POINTERSIZE)
    link->next = (LLLink *)0xbadbeef;
    link->prev = (LLLink *)0xbadbeef;
    link->data = (void *)0xbadbeef;
#else //
#error Unsupported pointer size; not 4 or 8 bytes.
#endif //

#endif /* def EA_DEBUG */

    rwRETURNVOID();
}

/**
Adds a link to the end of a linked list.  Each link should only be
added to a single linked list.

\param list The linked list to add the link to
\param link The link to add
 */
void
LinkListAddLLLink(LinkList *list, LLLink *link)
{
    rwFUNCTION();

    link->prev = list->terminator.prev;
    link->prev->next = link;
    list->terminator.prev = link;
    link->next = &list->terminator;

    rwRETURNVOID();
}

/**
Removes a link from a linked list.

\param link The link to remove.
 */
void
LinkListRemoveLLLink(LLLink *link)
{
    rwFUNCTION();

    /* Hook up either side of the double linked-list */
    link->prev->next = link->next;
    link->next->prev = link->prev;

#ifdef EA_DEBUG
    /* Clear the memory to a debug value */
    RwMemset((void *)link, 0xCD, sizeof(LLLink));
#endif /* def EA_DEBUG */

    rwRETURNVOID();
}

} // namespace rw
