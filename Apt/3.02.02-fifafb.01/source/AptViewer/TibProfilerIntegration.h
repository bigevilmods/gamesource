
#ifndef APT_TIBPROFILER_INTEGRATION_H
#define APT_TIBPROFILER_INTEGRATION_H 1

#include <prof.h>
#include <CommonProfInstance.h>


int gnFrameNumber = 0;

class AptIProfInstance : public CommonProfInstance
{
public:
    AptIProfInstance(int nFrames, uint32_t uFlags) :CommonProfInstance (nFrames, uFlags)
    {

    }

    /*F************************************************************************************************/
    /*!
    \Function    GetFileName

    \Description
    Gets the name to write the log file out as.

    There a 3 possible filenames that will be used to write out the log file.  First is
    the name passed into ProfStartProfile.  If NULL is passed in, this function will be called
    to get the filename.  If this function returns false, the library will generate a filename
    automatically.

    \Input      *strFileName      - the buffer to write the filename to
    \Input      uBufferSize       - the size of the buffer

    \Output     bool8_t           - true if this function returns a filename.
    false to use the default filename

    \Notes
    None.

    \Version    1.0        03/19/04 (JSH) First Version

    */
    /************************************************************************************************F*/

    virtual bool8_t     GetFileName(char8_t *strFilename, uint32_t uBufferSize)
    {
        strFilename[0] = '\0';

#if defined(EA_PLATFORM_PLAYSTATION2)
        strcat(strFilename, "host:");
#elif defined (EA_PLATFORM_XBOX)
        strcat(strFilename, "E:\\AptViewerEAGLREAL\\");
#elif defined (EA_PLATFORM_GAMECUBE)
        // No need to name file anything special.
#else
#error "Need to add file naming convention for new consoles!";
#endif

        strcat(strFilename, "Frame");
        char szBuf[16];
        sprintf(szBuf, "%06d", gnFrameNumber++);
        strcat(strFilename, szBuf);
        strcat(strFilename, ".log");
        return true;
    };

protected:
private:
};

static AptIProfInstance *gProfInst;

inline void AptProfileInitialize(void * logBuffer, int uLogBufferSizeInBytes)
{
    ProfInitParmsT    params;
    params.pProfLogBase = (ProfRecordT *)logBuffer;
    params.numRecords = (uLogBufferSizeInBytes/sizeof(ProfRecordT))-0x100;
    params.Counter0Type = 0;
    params.Counter1Type = 0;
    ProfInit(&params);
}

inline void AptProfileShutdown()
{
    ProfShutdown();
}

inline void AptProfileProcess()
{
    ProfProcess();
}

inline void AptProfileStart(uint32_t uFrames, uint32_t uFlags)
{
    gProfInst = new AptIProfInstance(uFrames, 0|uFlags);
    ProfStartProfile(gProfInst);
}

inline void AptProfileStop()
{
    ProfStopProfile();
    delete gProfInst;
}




#endif // #ifndef APT_TIBPROFILER_INTEGRATION_H