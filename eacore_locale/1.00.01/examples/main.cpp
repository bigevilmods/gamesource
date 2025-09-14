// (c) 2002-2004 Electronic Arts Inc.
//
/// @ingroup             g_userguide_locale
/// @example             main.cpp
///
/// This example demonstates the locale API.

#include "rw/core/libcore.h"
#include "eacore/locale/locale.h"

// unicode and ascii versions of strtbl.h are the same.
#include "ascii/strtbl.h"

using namespace EA::Core;
using namespace rw::core;
using namespace rw::core::debug;

const unsigned char gLocFileAscii[] = 
{
    #include "ascii/strtbl.dat"
};

// note: not const because the linker may put this into read-only memory,
// which would then cause an exception in the Locale constructor for unicode
// files on big-endian systems, because the strings must be byte-swapped in
// that case
unsigned char gLocFileUnicode[] = 
{
    #include "unicode/strtbl.dat"
};

void DumpAscii();
void DumpUnicode();

rw::core::debug::Channel * channel = 0;

int main()
{
    //PRINT_init();
    rw::core::debug::Manager::CreateInstance(&rw::GetDefaultAllocator());
    rw::core::debug::ConsoleDevice *myConsoleDevice = new rw::core::debug::ConsoleDevice;
    channel = new rw::core::debug::Channel("output", myConsoleDevice);

    DumpAscii();
    DumpUnicode();

    // flush the console device to ensure all output is on the screen.
    myConsoleDevice->Flush();

    rw::core::debug::Manager::DestroyInstance();
    return 0;
}

void DumpAscii()
{
    // Create the localization file
    Locale myLocale(gLocFileAscii, Locale::CT_ASCII);

    // get some of its states to print out
    int locale_languageCount = myLocale.GetState(Locale::LANGUAGECOUNT);
    int locale_stringCount   = myLocale.GetState(Locale::STRINGCOUNT);

    rwASSERTM(locale_languageCount == LOCALE_LANGUAGE_COUNT,
              ("Error - incorrect number of languages in locomoto file"));

    rwASSERTM(locale_stringCount == LOCALE_STRING_COUNT,
              ("Error - incorrect number of strings in locomoto file"));

    int locale_hasIndex      = myLocale.GetState(Locale::HASINDEX);

    // load up our strings
    channel->Print( "Ascii data: Language count: %d, String count: %d, Has index: %d\n\n",
                           locale_languageCount, locale_stringCount, locale_hasIndex );

    for (int languageid = 0; languageid < locale_languageCount; ++languageid)
    {
        myLocale.SetLanguage(languageid);

        channel->Print("%s\n", myLocale.GetString(IDS_HELLO));
        channel->Print("%s\n", myLocale.GetString(IDS_HELLO3));
        channel->Print("%s\n", myLocale.GetString(IDS_HELLO_WORLD));
        channel->Print("%s\n", myLocale.GetString(IDS_SENTENCE));
        channel->Print("%s\n", myLocale.GetString(IDS_QUOTE_TEST));
        channel->Print("%s\n", myLocale.GetString(IDS_TRC));
        channel->Print("\n");
    }
}

void DumpUnicode()
{
    // Create the localization file
    Locale myLocale(gLocFileUnicode, Locale::CT_UNICODE);

    // get some of its states to print out
    int locale_languageCount = myLocale.GetState(Locale::LANGUAGECOUNT);
    int locale_stringCount   = myLocale.GetState(Locale::STRINGCOUNT);
    int locale_hasIndex      = myLocale.GetState(Locale::HASINDEX);

    // load up our strings
    //channel->Print( L"Unicode data: Language count: %d, String count: %d, Has index: %d\n\n",
    //                       locale_languageCount, locale_stringCount, locale_hasIndex );
    
    for (int languageid = 0; languageid < LOCALE_LANGUAGE_COUNT; languageid++)
    {
        myLocale.SetLanguage(languageid);

        //channel->Print(L"%s\n", myLocale.WGetString(IDS_HELLO));
        //channel->Print(L"%s\n", myLocale.WGetString(IDS_HELLO3));
        //channel->Print(L"%s\n", myLocale.WGetString(IDS_HELLO_WORLD));
        //channel->Print(L"%s\n", myLocale.WGetString(IDS_SENTENCE));
        //channel->Print(L"%s\n", myLocale.WGetString(IDS_QUOTE_TEST));
        //channel->Print(L"%s\n", myLocale.WGetString(IDS_TRC));
        //channel->Print(L"\n");
    }
}
