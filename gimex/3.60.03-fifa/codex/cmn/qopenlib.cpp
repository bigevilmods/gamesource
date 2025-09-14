// (c) 1995-2007 Electronic Arts Inc.

#include <string.h>
#include "qopen.h"

/*!
 * \file  qopenlib.cpp
 * \brief Lib version of the qopen interface
 */


/****************************************************************/
/*  Codex initialization                                        */
/****************************************************************/

/*!
******************************************************************************
\brief      Load Codex library.

    The CODEX_init function loads the CODEX library and must be called prior
    to using any CODEX function.
    If the application is loading CODEX from a DLL, "qopen.cpp" should be
    included as it contains the CODEX_init function for DLL loading.
    "gopen.cpp" is not needed if the application is statically linking the
    CODEX library.

    To unload the CODEX library, call CODEX_restore.


    RETURN VALUE

    true if successful.


\sa
    CODEX_restore

******************************************************************************/
bool GCALL CODEX_init(void)
{
    return true;
}

/*!
******************************************************************************
\brief      Unload CODEX library.

    The CODEX_restore function unloads the CODEX library after a call to
    CODEX_init.

\sa
   CODEX_init

******************************************************************************/
void GCALL CODEX_restore(void)
{
    return;
}

/****************************************************************/
/*  Set codex function pointers based on number                */
/****************************************************************/

static int currentcodex = 0;

/*!
******************************************************************************
\brief      Set the current CODEX module from a filename extension.

\return
    The enumerated value of the new CODEX module, 0 if failed.

\sa
    CODEX_set, CODEX_get, CODEX_gopen, QINFO

******************************************************************************/
int GCALL CODEX_find(const char *name)
{
    int besttype=0;
    int i;
    bool found=false;

    for (i=0; (i < CODEX_count()) && !found; i++)
    {
        CODEXABOUT *about;
        CODEX_set(i);
        about = CODEX_about();
        if (strcmp(about->shorttypestr, name) == 0)
        {
            besttype = i;
            found = true;
        }
        gfree(about);
    }

    CODEX_set(besttype);
    return(besttype);
}

/*!
******************************************************************************
\brief      Set the current CODEX module using enumerated value.

    The CODEX_set function sets the current CODEX module to that specified
    by an enumerated value.  This value should be obtained using the
    CODEX_get function.  It is also possible to loop through all CODEX
    modules from 0 (NULL module) to CODEX_count - 1.

    The CODEX_get function returns the enumerated value of the current
    CODEX module.  This value should not be assumed to remain constant between
    runtime sessions, particularly when using CODEX DLL.

\return
    CODEX_get - An integer value specifying the current CODEX module, 0 if failed.

\sa
    CODEX_find, CODEX_count

******************************************************************************/
void GCALL CODEX_set(int typenum)
{
    if (typenum >= CODEX_count() || typenum < 0)
        typenum = 0;

    currentcodex = typenum;
}

/*!
******************************************************************************
\brief      Get the current CODEX module using enumerated value.

    The CODEX_get function gets the current CODEX module number.
    This is useful if you need to switch modules temporarily or
    wish to loop through all the available modules.

\return
    CODEX_get - An integer value specifying the current CODEX module, 0 if failed.

\sa
    CODEX_find, CODEX_count

******************************************************************************/
int GCALL CODEX_get(void)
{
    return(currentcodex);
}

/*!
******************************************************************************
\brief      Get the number of CODEX modules.

\return
    The number of available CODEX modules, including the NULL module.

\sa
    CODEX_set, CODEX_get, CODEX_find


    EXAMPLE

    none available yet

******************************************************************************/
int GCALL CODEX_count(void)
{
    int num=0;

    while(qfunctions[num].CODEX_about != NULL)
    {
        num++;
    }

    return(num);
}
/*!
******************************************************************************
\brief      Get the best match codex module for a given set of compressed data

\return
    The module number that give the highest chance of decoding the data.

\sa
    CODEX_set, CODEX_get, CODEX_find

******************************************************************************/
int GCALL CODEX_best(char* pCompressedData)
{
    // given a pointer to compressed data, find the type of codex
    // module it is by calling is for each codex.  Return the
    // codex that gives highest percent.

    int BestMatchIndex = 0; // index of best match so far
    int BestMatch = 0;      // percentage of best match so far
    int i;                  // index used to check each module
    int chance;             // percentage of current module

    for (i = 0; i < CODEX_count(); i++)
    {
        CODEX_set(i);
        chance = CODEX_is(pCompressedData);

        if (chance > BestMatch)
        {
            BestMatch      = chance;
            BestMatchIndex = i;
        }
    }

    CODEX_set(BestMatchIndex);
    return BestMatchIndex;
}

/*!
******************************************************************************
\page       p_CODEXABOUT    CODEXABOUT
        \code
        #include "codex.h"
        #include "gimex.h"                Required for structure declarations

        typedef struct
        {
            int signature;               // signature of codex ie 'tga ' (optional)
            int size;                    // size of CODEXABOUT structure
            int version;                 // version number of CODEXABOUT structure (200)

            unsigned int decode    :1;   // supports decoding
            unsigned int encode    :1;   // supports encoding
            unsigned int size32    :1;   // support 32 bit size field
            unsigned int islossy   :1;   // module is lossy (ie dxt)
            unsigned int pad       :28;

            char versionstr[8];          // version number of codex module ie 1.00
            char shorttypestr[8];        // 3 or 4 character type string ie ref
            char longtypestr[16];        // full name of data format ie Refpack
        } CODEXABOUT;
        \endcode

\brief      About Codex Module Structure.

    The CODEXABOUT structure describes the characteristics of the Codex module compression format.  This
    structure is filled in by the function CODEX_about(), which may be called at any time, as it is not
    based on a particular data buffer.  The caller must free the structure.

\sa
    CODEX_about

******************************************************************************/

/*!
******************************************************************************
\brief      Gets about information on a given module

    The CODEX_about function retrieves information about a given Codex Module

\sa
    \ref p_CODEXABOUT

******************************************************************************/
CODEXABOUT* GCALL CODEX_about(void)
{
    if (qfunctions[currentcodex].CODEX_about)
        return(qfunctions[currentcodex].CODEX_about());
    return((CODEXABOUT*)NULL);
}

/*!
******************************************************************************
\brief      Identify if the specified data is a format parsable by the given Module

\return
    - A value of 0 indicates the data is not parsable by the CODEX module.
    - A value of 100 indicates the data definitely is parsable.
    - A value in between 0 and 100 represents the probability that the data is
      of a parsable format.

\sa
    CODEX_about

******************************************************************************/
bool GCALL CODEX_is(const void *compressed)
{
    if (qfunctions[currentcodex].CODEX_is)
        return(qfunctions[currentcodex].CODEX_is(compressed));
    return(0);
}

/*!
******************************************************************************
\brief      Returns the required destination buffer size for decompression

\return
    returns the size of the destination buffer in bytes for the decompressed
    data.

\sa
    CODEX_about

******************************************************************************/
int GCALL CODEX_size(const void *compressed)
{
    if (qfunctions[currentcodex].CODEX_size)
        return(qfunctions[currentcodex].CODEX_size(compressed));
    return(0);
}

/*!
******************************************************************************
\brief      decodes from a compressed data stream into a user supplied destination buffer

\return
    returns non zero on success.

\sa
    CODEX_encode

******************************************************************************/
int GCALL CODEX_decode(void *uncompressed, const void *compressed, int *compressedsizeptr)
{
    if (qfunctions[currentcodex].CODEX_decode)
        return(qfunctions[currentcodex].CODEX_decode(uncompressed, compressed, compressedsizeptr));
    return(0);
}

/*!
******************************************************************************
\brief      encodes a data stream into a user supplied destination buffer

\return
    returns non zero on success.

\sa
    CODEX_decode

******************************************************************************/
int GCALL CODEX_encode(void *compressed, const void *uncompressed, int uncompressedsize, int *opts)
{
    if (qfunctions[currentcodex].CODEX_encode)
        return(qfunctions[currentcodex].CODEX_encode(compressed, uncompressed, uncompressedsize, opts));
    return(0);
}

