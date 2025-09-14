// (c) 2002-2004 Electronic Arts Inc.
//
/// @file                locale/locale.h
/// @brief               Locale library header file.


#ifndef EA_CORE_LOCALE_H
#define EA_CORE_LOCALE_H

namespace EA
{
    namespace Core
    {
        // forward delclaration of internal data types
        struct LocaleHeader;

        /// The Locale class is used to lookup strings by ID number
        /// in the compiled locale database.
        class Locale
        {
        public:
            /// @brief Locale states
            enum LocaleState
            {
                LANGUAGEID,     ///< Language ID
                LANGUAGECOUNT,  ///< How many languages are in this database
                STRINGCOUNT,    ///< How many strings per language
                HASINDEX        ///< Does the database have an index?
            };

            /// @brief Database string types.
            /// The type of database must be specified when the Locale
            /// object is instantiated.
            enum CharacterType
            {
                CT_ASCII,       ///< Locale database has ASCII strings (8 bits per character)
                CT_UNICODE      ///< Locale database has UNICODE strings (16 bits per character)
            };

            Locale(const void* localefile, CharacterType chType = CT_UNICODE);

            int GetState(LocaleState state) const;
            void SetLanguage(int langId);

            const char* GetString(int StringID) const;
            const wchar_t* WGetString(int StringID) const;

        private:
            int GetStringIDByIndex(unsigned short) const;
            static int Compare(const void *, const void *);

            const LocaleHeader* mLocaleFile;
            int mLanguageId;
            int mLanguageCount;
        };

    }   // namespace Core
}   // namespace EA
#endif

