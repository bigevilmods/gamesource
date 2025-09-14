// (c) 1998-2004 Electronic Arts Inc.
//
/// @file                lib/locale.cpp
/// @brief               Locale API implementation

#include <stdlib.h>     // bsearch

#ifdef RWDEBUG
#include <assert.h>
#endif

#include "EABase/eabase.h"
#include "eacore/locale/locale.h"
#include "Endian/endian.h"
namespace EA
{
namespace Core
{

///////////////////////////////////////////////////////////////////////////////
// File Format Structures
///////////////////////////////////////////////////////////////////////////////

const uint32_t LOCALE_HEADERCHUNKID     = EA::Endian::make_id32('L', 'O', 'C', 'H');

struct LocaleHeader
{
    unsigned int   chunkid;        // 'LOCH' LOCALEFILE_HEADERCHUNKID
    unsigned int   chunksize;      // size of chunk in bytes
    unsigned int   flags;          // 0=no index chunk present,1=index chunk present
    unsigned short languagecount;  // number of language chunks in this file
    unsigned short languageid;     // current language selected in file
};

// offset LOCALE_HEADER_LANGUAGETABLE_OFFSET bytes from the
// start of the chunk to the language offset table
const int LOCALE_HEADER_LANGUAGETABLE_OFFSET = 16;

struct LocaleIndex
{
    unsigned int   chunkid;        // 'LOCI' LOCALEFILE_INDEXCHUNKID
    unsigned int   chunksize;      // size of chunk in bytes
    unsigned int   stringcount;    // number of string ids in this chunk (same value in all language chunks)
    unsigned int   pad;            // must be zero
};

// offset LOCALE_INDEX_STRINGID_OFFSET bytes from
// the start of the chunk to the string id table
const int LOCALE_INDEX_STRINGID_OFFSET = 16;

struct LocaleLanguage
{
    unsigned int   chunkid;        // 'LOCL' LOCALEFILE_LANGUAGECHUNKID
    unsigned int   chunksize;      // size of chunk in bytes including this header and all string data
    unsigned int   languageid;     // language strings are in for this bank
    unsigned int   stringcount;    // number of strings in this chunk
};

// Locale header chunk flags bitmasks
const uint32_t LOCALE_FLAGS_HASINDEX = 0x00000001;

///////////////////////////////////////////////////////////////////////////////
///
/// @brief Creates a Locale object from a locale file.
///
/// On big-endian systems, if the locale file is unicode,
/// the constructor will byte-swap all the strings.
/// That means that the localefile memory may be modified
/// by the constructor on big-endian platforms, which means
/// that if the database is stored in read-only memory, an
/// exception will be thrown when the byte swapping is done.
/// The solution to that is to store the database in 
/// writeable memory.
///
/// The selected language is set to the first one in the
/// database.
///
/// @sa Locale::GetState, Locale::SetLanguage, Locale::GetString,
/// Locale::WGetString

Locale::Locale(const void* localefile,  ///< pointer to locale data
               CharacterType chtype     ///< CT_ASCII or CT_UNICODE
               ) :
                mLocaleFile(static_cast<const LocaleHeader *>(localefile)),
                mLanguageId(0),
                mLanguageCount(0)
{
#ifdef RWDEBUG
    assert( mLocaleFile->chunkid == LOCALE_HEADERCHUNKID );
#endif

    mLanguageCount = static_cast<int>(EA::Endian::little_get(
        &mLocaleFile->languagecount, sizeof(mLocaleFile->languagecount)));

#ifdef RWDEBUG
    assert(mLanguageCount >= 1);
#endif

//#if (rwHOST_ENDIAN == rwLIENDIAN)
#if defined(EA_SYSTEM_LITTLE_ENDIAN)
    // prevent compiler warning
    chtype = chtype;

#else   // big-endian systems
	#if defined(EA_PLATFORM_WINDOWS)
	#error endian logic or defines wrong
	#endif
    if (chtype == CT_ASCII)
        return;

    const char *lf = reinterpret_cast<const char *>(mLocaleFile);

    // for each language, byte swap every string
    for (int lang = 0; lang < mLanguageCount; ++lang)
    {
        unsigned int offset = EA::Endian::little_get(lf + LOCALE_HEADER_LANGUAGETABLE_OFFSET + lang*4, 4);

        const LocaleLanguage *bank = reinterpret_cast<const LocaleLanguage*>(lf + offset);

        int stringcount = static_cast<int>(EA::Endian::little_get(&bank->stringcount, sizeof(bank->stringcount)));

        for (int str = 0; str < stringcount; ++str)
        {
            unsigned int offset = EA::Endian::little_get(
                reinterpret_cast<const unsigned int*>(bank) + 4 + str, 4);

            wchar_t *p = reinterpret_cast<wchar_t *>(reinterpret_cast<char*>(
                const_cast<LocaleLanguage *>(bank)) + offset);

            while (*p)
            {
                *p = static_cast<wchar_t>(EA::Endian::little_get(p, 2));
                p++;
            }
        }
    }

#endif
}

///////////////////////////////////////////////////////////////////////////////
///
/// @brief get a Locale state
///
/// Returns the state specified from the specified Locale (.loc) file.  
/// The state must be a valid LocaleState. The .loc file must be loaded 
/// into memory by the client before Locale::Getstate is called.
///
/// @return the value of the give locale state.
/// @sa Locale::Locale, Locale::SetLanguage, Locale::GetString, Locale::WGetString

int Locale::GetState(LocaleState state       ///< which locale state to get
                    ) const
{
    int value = 0;
    unsigned int offset;
    const LocaleLanguage *lang;
    const char *lf = reinterpret_cast<const char *>(mLocaleFile);

    switch (state)
    {
    case LANGUAGEID:
        value = mLanguageId;
        break;

    case LANGUAGECOUNT:
        value = mLanguageCount;
        break;

    case STRINGCOUNT:
        // get pointer to string bank
        offset = EA::Endian::little_get((lf + LOCALE_HEADER_LANGUAGETABLE_OFFSET +
            (mLanguageId*4)), 4);
        lang = reinterpret_cast<const LocaleLanguage *>(lf + offset);
        value = static_cast<int>(EA::Endian::little_get(&lang->stringcount,
            sizeof(lang->stringcount)));
        break;

    case HASINDEX:
        value = static_cast<int>(EA::Endian::little_get(&mLocaleFile->flags,
            sizeof(mLocaleFile->flags)) & LOCALE_FLAGS_HASINDEX);
        break;

    default:
#ifdef RWDEBUG
        // invalid state; throw assert
        assert(false);
#endif
        break;
    }

    return value;
}

///////////////////////////////////////////////////////////////////////////////
///
/// @brief set a Locale state
///
/// Sets the current Locale state based on the specified Locale (.loc) file. The
/// state must be a valid LocaleState and must be able to be set.  The .loc file
/// must be loaded into memory by the client before this function is called.
///    
/// Currently, the only settable state is LANGUAGEID, so if you try to set
/// any other state, it will be ignored.
///
/// @return none
/// @sa Locale::Locale, Locale::GetState, Locale::GetString, Locale::WGetString
void Locale::SetLanguage(int langId ///< new language
                         )
{
#ifdef RWDEBUG
    assert((langId >= 0) && (langId < mLanguageCount));
#endif

    mLanguageId = langId;
}

///////////////////////////////////////////////////////////////////////////////
///
/// @brief return selected string from the specified .loc file (ASCII version)
///
/// Returns the string identified by stringID from the specified
/// Locale (.loc) file.  Use the stringID specified in the header file created
/// by Locomoto.
///
/// This function should be used for strings which are in ASCII format.  If
/// your strings are in Unicode format, use the Locale::WGetString function.
///
/// If the .loc file was created with an index, stringID can be any
/// valid integer in the range 0 to 65535.  If no index was created
/// with the .loc file, stringID will be a zero based array index.
///
/// Note that the string is returned as a const. It must NOT be modified.
///
/// Note that switching to a new language will make this pointer invalid.
///
/// @return A pointer to the requested string.  If the cannot be found, NULL is returned.
/// @sa Locale::Locale, Locale::GetState, Locale::SetLanguage, Locale::WGetString

const char* Locale::GetString(int stringid  ///< ID of string to fetch
                             ) const
{
    const char *p = NULL;       // pointer to string, NULL if string cannot be found
    const LocaleLanguage *bank; // pointer to language bank that contains the strings
    unsigned int offset;
    const char *lf = reinterpret_cast<const char *>(mLocaleFile);

    if (GetState(HASINDEX) > 0)
    {
        // get real stringid by looking it up in the index
        stringid = GetStringIDByIndex(static_cast<unsigned short>(stringid));
    }

    // get pointer to string bank
    offset = EA::Endian::little_get(lf + LOCALE_HEADER_LANGUAGETABLE_OFFSET + (mLanguageId*4), 4);
    bank = reinterpret_cast<const LocaleLanguage *>(lf + offset);

    // get pointer to string
    if ((stringid >= 0) && (stringid < static_cast<int>(EA::Endian::little_get(&bank->stringcount, sizeof(bank->stringcount)))))
    {
        p = reinterpret_cast<const char *>(bank) + EA::Endian::little_get(reinterpret_cast<const char*>(bank) + 16 + (stringid*4), 4);
    }

    return p;
}

///////////////////////////////////////////////////////////////////////////////
///
/// @brief return selected string from the specified .loc file (UNICODE version)
///
/// Returns the string identified by stringID from the specified
/// Locale (.loc) file.  Use the stringID specified in the header file created
/// by Locomoto.
///
/// This function should be used for strings which are in Unicode format.  If
/// your strings are in ASCII format, use the Locale::GetString function.
///
/// If the .loc file was created with an index, stringID can be any
/// valid integer in the range 0 to 65535.  If no index was created
/// with the .loc file, stringID will be a zero based array index.
///
/// Note that the string is returned as a const. It must NOT be modified.
///
/// Note that switching to a new language will make this pointer invalid.
///
/// @return A pointer to the requested string.  If the cannot be found, NULL is returned.
/// @sa Locale::Locale, Locale::GetState, Locale::SetLanguage, Locale::SetString

const wchar_t* Locale::WGetString(int stringid              ///< ID of string to fetch
                                 ) const
{
    const wchar_t *p = NULL;       // pointer to string, NULL if string cannot be found
    const LocaleLanguage  *bank;    // pointer to language bank that contains the strings
    unsigned int offset;
    const char *lf = reinterpret_cast<const char *>(mLocaleFile);

    if (GetState(HASINDEX) > 0)
    {
        // get real stringid by looking it up in the index
        stringid = GetStringIDByIndex(static_cast<unsigned short>(stringid));
    }

    // get pointer to string bank
    offset = EA::Endian::little_get(lf + LOCALE_HEADER_LANGUAGETABLE_OFFSET + (mLanguageId*4), 4);
    bank = reinterpret_cast<const LocaleLanguage *>(lf + offset);

    // get pointer to string
    if ((stringid >= 0) && (stringid < static_cast<int>(EA::Endian::little_get(&bank->stringcount, sizeof(bank->stringcount)))))
    {
        p = reinterpret_cast<const wchar_t *>(reinterpret_cast<const char *>(bank) + EA::Endian::little_get(reinterpret_cast<const char*>(bank) + 16 + (stringid*4), 4));
    }

    return p;
}

// private functions

int Locale::GetStringIDByIndex(unsigned short key) const
{
    int stringid;
    const unsigned short *result;
    const unsigned char  *base;   // pointer to base of string id table
    const char *lf = reinterpret_cast<const char *>(mLocaleFile);

    const LocaleIndex* index = reinterpret_cast<const LocaleIndex *>(lf +
        EA::Endian::little_get( &mLocaleFile->chunksize, sizeof(mLocaleFile->chunksize)));
    base = (reinterpret_cast<const unsigned char*>(index)) + LOCALE_INDEX_STRINGID_OFFSET;
    result = reinterpret_cast<unsigned short *>(bsearch(
        reinterpret_cast<unsigned char *>(&key), base,
        EA::Endian::little_get(&index->stringcount, sizeof(index->stringcount)), 
        sizeof(index->stringcount), Compare));

    if (result != 0)
    {
        // index is the second unsigned short
        ++result;
        stringid = static_cast<int>(EA::Endian::little_get(result, sizeof(*result)));
    }
    else
    {
        stringid = -1;
    }

    return stringid;
}

int Locale::Compare(const void *arg1, const void *arg2)
{
    const unsigned short *s1 = reinterpret_cast<const unsigned short*>(arg1);
    const unsigned short *s2 = reinterpret_cast<const unsigned short*>(arg2);
    // note: ugly hack to enable comparison on big-endian systems; assumes that we know that
    //       the first parm is endian correct (the key) whereas the second parm needs to be
    //       endian corrected
    return (*s1) - (static_cast<unsigned short>(EA::Endian::little_get(s2, sizeof(*s2))));
}

}   // namespace Core
}   // namespace EA
