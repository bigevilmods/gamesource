// (c) Electronic Arts. All Rights Reserved.

#include "EABase/eabase.h"
#include "rw/core/filesys/constants.h"
#include "strutf16to8.h"

#import <Foundation/Foundation.h>

namespace rw { 
namespace core {
namespace filesys {

    struct InitializePreMain
    {
        char16_t mTempDirectory[FILESYS_MAX_PATH];
        char16_t mHomeDirectory[FILESYS_MAX_PATH];
        char16_t mAppName[FILESYS_MAX_PATH];

        InitializePreMain()
            : mTempDirectory()
        {
            NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
            uint32_t length = 0;


            NSString *td = NSTemporaryDirectory();

            length = static_cast<uint32_t>(td.length);
            if (length >= sizeof(mTempDirectory))
                length = sizeof(mTempDirectory) - 1;

            [td getCharacters:reinterpret_cast<unichar*>(mTempDirectory) range:NSMakeRange(0, length)];
            mTempDirectory[length] = 0;


            NSString *hd = NSHomeDirectory();

            length = static_cast<uint32_t>(hd.length);
            if (length >= sizeof(mHomeDirectory))
                length = sizeof(mHomeDirectory) - 1;

            [hd getCharacters:reinterpret_cast<unichar*>(mHomeDirectory) range:NSMakeRange(0, length)];
            mHomeDirectory[length] = 0;


            NSString *an = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleName"];

            length = static_cast<uint32_t>(an.length);

            if (length >= sizeof(mAppName))
                length = sizeof(mAppName) - 1;

            [an getCharacters:reinterpret_cast<unichar*>(mAppName) range:NSMakeRange(0, length)];
            mAppName[length] = 0;


            [pool release];
        }
    };

    static InitializePreMain sInitializePreMain;

    int appleGetTempDirectory(char16_t* pDirectory, uint32_t nMaxPermittedLength)
    {
        int i = 0;
        for (; i < int(nMaxPermittedLength - 1) && sInitializePreMain.mTempDirectory[i] != 0; ++i)
            pDirectory[i] = sInitializePreMain.mTempDirectory[i];
        pDirectory[i] = 0;
        return i;
    }

    int appleGetTempDirectory(char8_t* pDirectory, uint32_t nMaxPermittedLength)
    {
        return static_cast<int>(StrlcpyUTF16ToUTF8(pDirectory, nMaxPermittedLength, sInitializePreMain.mTempDirectory));
    }

    int appleGetHomeDirectory(char16_t* pDirectory, uint32_t nMaxPermittedLength)
    {
        int i = 0;
        for (; i < int(nMaxPermittedLength - 1) && sInitializePreMain.mHomeDirectory[i] != 0; ++i)
            pDirectory[i] = sInitializePreMain.mHomeDirectory[i];
        pDirectory[i] = 0;
        return i;
    }

    int appleGetHomeDirectory(char8_t* pDirectory, uint32_t nMaxPermittedLength)
    {
        return static_cast<int>(StrlcpyUTF16ToUTF8(pDirectory, nMaxPermittedLength, sInitializePreMain.mHomeDirectory));
    }

    int appleGetAppName(char16_t* pDirectory, uint32_t nMaxPermittedLength)
    {
        int i = 0;
        for (; i < int(nMaxPermittedLength - 1) && sInitializePreMain.mAppName[i] != 0; ++i)
            pDirectory[i] = sInitializePreMain.mAppName[i];
        pDirectory[i] = 0;
        return i;
    }

    int appleGetAppName(char8_t* pDirectory, uint32_t nMaxPermittedLength)
    {
        return static_cast<int>(StrlcpyUTF16ToUTF8(pDirectory, nMaxPermittedLength, sInitializePreMain.mAppName));
    }


} } }

