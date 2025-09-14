/*H************************************************************************************************/
/*!

    \File
        _AptLoad.h

    \Description
        Simple description of main purpose of file.

    \Notes
        None.

    \Copyright
        (c) 2004 Electronic Arts Inc.

    \Version    1.0     2004-03-07  (ONA)   First Version
    \Version    1.1     2004-07-22  (rrv:1) enhancement for AS2 support
    \Version    1.1     11/01/04    (DC)    Changed to use allocation Macro to remove #if defines
                                            everywhere. There was not change to the logic.
    \Version    1.2     11/27/06    (ASK)   Changed links to coreforge for external references to Apt Wiki page
    \Version    1.3     02/12/08    (RB)    Renamed ThingyPtr as LinkerDataPtr; ThingyList as LinkerDataList;
                                            mThingys as mLinkerDataList; findThingy as findLinkerData.

    \Version    1.9     06/12/08    (JP)    The AptFile now knows about the MovieClipLoader to notify it when the load is complete
    \Version    1.10    05/05/09    (RB)    Removed const from GetState and GetPrevState return type. Prevents compiler warning.
*/
/************************************************************************************************H*/

#ifndef ___APTLOAD_H__
#define ___APTLOAD_H__

#include "CommonSense/SingleList/SingleList.h"
#include "Apt.h"
#include "_AptVector.h"
#include "AptCharacter/AptCharacter.h"
#include "_AptThread.h"

#include APT_INC_THREAD_FUTEX_H
#include APT_INC_THREAD_ATOMIC_H

struct AptSingleListPolicy
{
    struct Allocator
    {
        static void *Alloc( size_t size )
        {
            return APT_MALLOC_BLOCK(size);
        }

        static void Free( void* ptr, size_t size )
        {
            APT_FREE_BLOCK(ptr, size);
        }
    };
};

typedef VectorFactory< AptFilePtr >::type FilePtrVector;
typedef CommonSense::SingleList::SingleList< AptFile *, AptSingleListPolicy> RawFileList;

enum AptFileType
{
    AptFileType_Animation,
    AptFileType_Image,
};

struct AptLoader
{
private:
    RawFileList mFiles; // TODO need to enforce that there's only one file with a given name in this list
    APT_THREAD_NAMESPACE::Futex mFilesLock;

    AptLoader(const AptLoader&);
    AptLoader& operator=(const AptLoader&);
    void notify(AptFilePtr f);

public:
    AptLoader() {};
    ~AptLoader();

    void Invalidate(AptFile *pFile);

    AptFilePtr findFile(const AptNativeString &sFilename);

    AptFilePtr IsLoaded(const AptNativeString &sFilename);
    AptFilePtr Load(const AptNativeString &sFilename, AptFileType fileType);

    bool AllImportsAvailable(AptFilePtr f);

    // move all files in the list as far as possible through their states
    void Update();

    void CompleteAnimationLoad(AptFilePtr f, void *pData, void *pConstTable, void *pUser);
    void CompleteImageLoad(AptFilePtr f, AptAssetTexture texture, int width, int height, void* userData);

    void GetFileVector(AptFilePtr *aFilePtrs, int nMaxSize);

    void CancelPreloadedAnimation(const AptNativeString &sFilename);
    APT_NEW_DELETE_OPERATORS
    // DC - Added Metric defines
    APT_DECLARE_OBJECT_METRICS
};

struct AptSharedPtrRefCount
{
    APT_THREAD_NAMESPACE::AtomicInt32 mRefCount;
    AptSharedPtrRefCount() : mRefCount(0) {}

    APT_NEW_DELETE_OPERATORS
    // DC - Added Metric defines
    APT_DECLARE_OBJECT_METRICS
};

// (LEO) The following code was updated to resolve bug253 Flow broken on backward.  The Zombie state was added to keep track
//  of files that are being held onto after they were unloaded by Apt.  You can refer to this link to read about the bug:
//  http://globaltechdocs.ea.com/MediaWiki/index.php?title=Apt:External_function_references
struct AptFile : public AptSharedPtrRefCount
{
    APT_NEW_DELETE_OPERATORS
    // DC - Added Metric defines
    APT_DECLARE_OBJECT_METRICS

    AptFile(const AptNativeString &name) :
#if defined(EA_DEBUG) || defined(DO_COVERAGE) || defined(APT_ENABLE_ZOMBIE_OUTPUT)
        nFileSize(0),     // $bug879 - DC - this was not being initialized
#endif
        mName(name),
        mState(Queued),
        mPrevState(Queued),
        mMovieClipLoader(NULL)
        {}

    virtual ~AptFile();

    enum State
    {
        Invalid,
        Queued,
        WaitingForData,
        WaitingForImports,
        Resolved,
        Zombie,              // $BUG253 LEO    19/47
        Unloaded
    };

    virtual AptFileType GetFileType() const = 0;
    const AptNativeString &GetName() const { return mName; }
    State GetState() const { return mState; }
    State GetPrevState() const { return mPrevState; }

    void NotifyMovieClipLoader(int32_t notification);
    void SetMovieClipLoader(AptMovieClipLoader *pLoader);

    void setState(State s) {
        mPrevState = mState;
        mState = s;
    }      // (LEO) this was moved to public so we can set the ZOMBIE flag to the file     $BUG253 LEO    20/47

    friend struct AptLoader;

#if defined(EA_DEBUG) || defined(DO_COVERAGE) || defined(APT_ENABLE_ZOMBIE_OUTPUT)                              // $BUG253 LEO    21/47
    void SetFileSize(int nSize)
    {
        nFileSize = nSize;
    }
    int GetFileSize()
    {
        return nFileSize;
    }
#endif

protected:
    AptMovieClipLoader *GetMovieClipLoader() const;

#if defined(EA_DEBUG) || defined(DO_COVERAGE) || defined(APT_ENABLE_ZOMBIE_OUTPUT)          // $BUG253 LEO 22/47
    int nFileSize;
#endif
    AptNativeString mName;
    State mState;
    State mPrevState;
    AptMovieClipLoader *mMovieClipLoader;     // The loader that asked for this file. Removed after CompleteLoad
};

/// Apt File that wraps a SWF
struct AptAnimationFile : public AptFile
{
    APT_NEW_DELETE_OPERATORS
    APT_DECLARE_OBJECT_METRICS

    AptAnimationFile(const AptNativeString &name)
    : AptFile(name)
    , mAptData(NULL)
    , mCharacter(NULL)
    , mUserData(NULL)
    {}

    virtual ~AptAnimationFile();

    AptCharacter *FindExport(const char *szName);

    bool isFileImported(AptFilePtr pFile);

    // Cast an AptFilePtr to an AptAnimationFile
    static AptAnimationFile* Cast(AptFilePtr ptr);

    virtual AptFileType GetFileType() const { return AptFileType_Animation; }
    AptCharacter *GetMainCharacter() const { return mCharacter; }
    void *GetUserData() const { return mUserData; }
    void *GetAptData() const { return mAptData; }
    char* GetTimeStamp() { return mTimeStamp; }
    int   GetTimeStampSize() { return sizeof(mTimeStamp);}

    friend struct AptLoader;

private:
    void setDataPointers(void *aptData, AptCharacter *mainCharacter, void *userData)
    {
        mAptData = aptData;
        mCharacter = mainCharacter;
        mUserData = userData;
    }

    void *mAptData;
    AptCharacter *mCharacter;
    void *mUserData;
    char mTimeStamp[16];
};

/// Apt file representing an Image
struct AptImageFile : public AptFile
{
    APT_NEW_DELETE_OPERATORS
    APT_DECLARE_OBJECT_METRICS

    AptImageFile(const AptNativeString &name)
    : AptFile(name)
    , mTexture(NULL)
    , mCharacter(NULL)
    , mWidth(0)
    , mHeight(0)
    , mUserData(NULL)
    {}

    virtual ~AptImageFile();

    // Cast an AptFilePtr to an AptImageFile
    static AptImageFile* Cast(AptFilePtr ptr);
    virtual AptFileType GetFileType() const { return AptFileType_Image; }
    AptCharacter* GetCharacter() const { return mCharacter; }
    void *GetUserData() const { return mUserData; }
    int GetWidth() const { return mWidth; }
    int GetHeight() const { return mHeight; }

    void SetData(AptAssetTexture texture, int width, int height, void* userData);

private:
    AptAssetTexture mTexture;
    AptCharacter* mCharacter;
    int mWidth;
    int mHeight;
    void *mUserData;
};

struct AptLinkerThingy : AptSharedPtrRefCount
{
    APT_NEW_DELETE_OPERATORS
    // DC - Added Metric defines
    APT_DECLARE_OBJECT_METRICS

    AptFilePtr mFile;
    AptCIH *pTarget;
    bool mAttachedToMovie;

    AptFilePtr GetFile() const { return mFile; }
    AptCIH *GetTarget() const { return pTarget; }

    bool IsAttachedToMovie() const { return mAttachedToMovie; }
    void SetAttachedToMovie(bool attached) { mAttachedToMovie = attached; }

    AptLinkerThingy(AptFilePtr file, AptCIH *target) : mFile(file), pTarget(target), mAttachedToMovie(false) {}
};

struct AptLinker
{
    APT_NEW_DELETE_OPERATORS
    // DC - Added Metric defines
    APT_DECLARE_OBJECT_METRICS

    typedef AptSharedPtr< AptLinkerThingy > LinkerDataPtr;
    typedef CommonSense::SingleList::SingleList< LinkerDataPtr, AptSingleListPolicy > LinkerDataList;
    LinkerDataList mLinkerDataList;

    FilePtrVector mLoadedFilesWaitingForLink;

private:
    LinkerDataList::Iterator findLinkerData(AptCIH *pCIH);

public:

    void PrintLoadedFiles(bool bPrintImports = false, bool bPrintExports = false);
    AptCIH *ConvertToZombie(AptCIH *pCIH);

    void SwapOut(AptFilePtr f1, AptFilePtr f2);
    LinkerDataList::Iterator findLinkerData(AptFilePtr pFile);
    bool isFileImported(AptFilePtr pFile);

    AptLinker() {}

    void Update();
    void Notify(AptFilePtr f);
    void Load(const AptNativeString &sFilename, const AptNativeString& sTarget, AptMovieClipLoader *pLoader = NULL);
    void CancelLoad(AptCIH *pCIH);

};

struct AptFileSavedInputState
{
    enum State
    {
        Invalid,
        ExpectedBySavedInputButUnavailable,
        LoadedButNotAtCheckpoint,
        ReadyToBeLinked,
        Linked,
    };

    State GetState() const { return mState; }
    void SetState(State state) { mState = state; }
    const AptNativeString &GetName() const { return mName; }


    AptFileSavedInputState(const AptNativeString &name, State state = Invalid) : mName(name), mState(state) {}
    AptFileSavedInputState() : mState(Invalid) {}

private:
    AptNativeString mName;
    State mState;
};

typedef VectorFactory< AptFileSavedInputState >::type FileSavedInputStateVector;
struct AptSavedInputCheckpoints
{
    APT_NEW_DELETE_OPERATORS
    // DC - Added Metric defines
    APT_DECLARE_OBJECT_METRICS
private:
    FileSavedInputStateVector mPending;

    void updateState(const AptNativeString &name, AptFileSavedInputState::State lookFor, AptFileSavedInputState::State setTo, AptFileSavedInputState::State ifNotFound);
    bool allStatesAre1(AptFileSavedInputState::State state0);
    bool allStatesAre2(AptFileSavedInputState::State state0, AptFileSavedInputState::State state1);
    bool allStatesAre3(AptFileSavedInputState::State state0, AptFileSavedInputState::State state1, AptFileSavedInputState::State state2);
public:
    void Checkpoint(const AptNativeString &s);
    void FileLoaded(const AptNativeString &s);
    void AllLinked();
    bool CanLinkPendingFiles();
    bool CanContinueSavedInputs();

};
extern AptSavedInputCheckpoints *gpSavedInputCheckpoints;

#endif  //  ___APTLOAD_H__

/*H*************************************************************************************************
    EOF
*************************************************************************************************H*/

