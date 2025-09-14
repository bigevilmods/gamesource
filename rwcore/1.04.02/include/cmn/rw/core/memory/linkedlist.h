// (c) Electronic Arts. All Rights Reserved.

/// file
/// Header file for the general linked list implementation.

#ifndef RW_LINKEDLIST_H
#define RW_LINKEDLIST_H
#include <rw/core/base/ostypes.h>
#include <rw/core/debug/debug.h>

/*************************************************************************************************************
 Includes
 */

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

/**
A double linked list structure
 */
typedef struct LinkListTag LinkList;

/**
A link in a double linked list
 */
typedef struct LLLink LLLink;

/*************************************************************************************************************
 Typedef Function Pointers
 */

/*************************************************************************************************************
 Structs + Unions
 */

/**
Represents a link on a doubly-linked list.  It has a void 'data' pointer which
can point to user data.  Links are joined in a circular list, with the linked
list structure itself holding the 'terminator' element.

\member next Points to the next link in the chain.  May point to the current link
             if there are no elements but the terminator in the linked list.
\member prev Points to the previous link in the chain.  May point to the current link
             if there are no elements but the terminator in the linked list.
\member data Points to user data.  This is not initialized in the linked list module,
             so must be initialized by client if it is to be used.

\see LinkList
 */
struct LLLink
{
    struct LLLink *next;
    struct LLLink *prev;
    void            *data;
};

/**
Represents a circular doubly-linked list.

\member terminator This is the first link in the doubly-linked list that represents the
                   start (and end) of the circular doubly-linked list.

\see LLLink
 */
struct LinkListTag
{
    LLLink terminator; /* Terminator embedded in linked list */
};


/*************************************************************************************************************
 Global Variables
 */

/*************************************************************************************************************
 Internal Functions Prototypes
 */

/*************************************************************************************************************
 External Functions Prototypes
 */

extern void
LinkListInitialize(LinkList *list);

extern void
LLLinkInitialize(LLLink *link);

extern void
LinkListAddLLLink(LinkList *list, LLLink *link);

extern void
LinkListRemoveLLLink(LLLink *link);

/**
Returns the linked list element that would indicate the end of a linked list.
This element may be 0.  If not, no linked list calls should be made on this
element.  It should only be used for pointer comparison purposes.

\param list The linked list for which to obtain the terminator.

\return A pointer to the terminator element.
 */
inline const LLLink *
LinkListGetTerminator(const LinkList *list)
{
    rwFUNCTION();
    rwRETURN((const LLLink *)(&list->terminator));
}

/**
Returns the first linked list element in a linked list.

\param list The linked list for which to obtain the first element.

\return A pointer to the first linked list element.
 */
inline LLLink *
LinkListGetFirstLLLink(LinkList *list)
{
    rwFUNCTION();
    rwRETURN(list->terminator.next);
}

/**
Gets the previous element in a linked list, or returns the terminator
element.

\param link The link for which to obtain the previous element.

\return The previous element, or the terminator.


 */
inline LLLink *
LLLinkGetPrev(LLLink *link)
{
    rwFUNCTION();
    rwRETURN(link->prev);
}

/**
Gets the next element in a linked list, or returns the terminator
element.

\param link The link for which to obtain the next element.

\return The next element, or the terminator.
 */
inline LLLink *
LLLinkGetNext(LLLink *link)
{
    rwFUNCTION();
    rwRETURN(link->next);
}

/**
Obtains the general data pointer from a link.

\param link The link for which to obtain the data pointer.

\return The general data pointer.
 */
inline void *
LLLinkGetData(LLLink *link)
{
    rwFUNCTION();
    rwRETURN(link->data);
}

/**
Sets the general data pointer for a link.

\param link The link for which to set the data pointer
\param data The value to set the pointer to
 */
inline void
LLLinkSetData(LLLink *link, void *data)
{
    rwFUNCTION();

    link->data = data;

    rwRETURNVOID();
}

} // namespace rw

#endif /* RW_LINKEDLIST_H */
