// (c) Electronic Arts. All Rights Reserved.

#pragma once

class SpitImpl
{
public:

    enum Command
    {
        CREATE,     // create a new big file
        EXTRACT,    // extract the files from the bigfile
        LIST,       // display a list of the file on the TTY.
        MERGE       // merge different bigfiles into one bigfile.
    };

    enum CompressionType
    {
        CompressionTypeNone,            // no compression used
        CompressionTypeRefpack,         // Refpack.  Buffered, like the original glump.  Cannot be decompressed on SPUs.
        CompressionTypeChunkedRefpack,  // Refpack.  Chunked into conveniently-sized blocks for parallel decompression.
        CompressionTypeChunkedZlib,     // Zlib.  Chunked into conveniently-sized blocks for parallel decompression.
        CompressionTypeChunkedLzx,      // LZX decompression is only supported on Xbox 360.

        CompressionTypeTotalOptions
    };

    enum IndexSortOrder
    {
        IndexSortHash,                  // Index is sorted by hash value
        IndexSortAlphabetical,          // Index is sorted by name
        IndexSortOffset,                // Index is sorted by file offset
        IndexSortNone					    // Not sorted
    };

    enum FileSortOrder
    {
        FileSortNone,                   // Leave the file list as specified on the command line
        FileSortAlphabetical,           // Sort the file list alphabetically
    };

    SpitImpl();
    ~SpitImpl();

    static void sUsage(const char *, const char *, const char *, int verboseLevel);
    static char *sOpt(const char *);

    static const char *sHistory[];
    static const char *ToolName;
    static const int ToolVersion = 10100;   // 1.01.00

    void SetMainCommand(Command cmd)
    { mMainCommand = cmd; }

    Command GetMainCommand() const
    { return mMainCommand; }

    CompressionType GetCompressionType() const
    { return mCompressionFormat; }

    int GetCompressionThreshold() const
    { return mCompressionThreshold; }

    int Alignment() const
    { return mAlignBoundary; }

    int GetHashSize() const
    { return mHashSize; }

    IndexSortOrder GetIndexSortOrder() const
    { return mIndexSortOrder; }

    bool FilesSortedAlphabetically() const
    { return mFilesSortedAlphabetically; }

    bool UseSourcePaths() const
    { return mUseSourcePaths; }

    bool GetFATInclusion() const
    { return mIncludeFAT; }

    bool GetLowercaseNames() const
    { return mLowercaseNames; }

    bool GetUppercaseNames() const
    { return mUppercaseNames; }

private:

    char *Opt(const char *);

    Command                 mMainCommand;
    CompressionType         mCompressionFormat;
    int                     mCompressionThreshold;
    int                     mAlignBoundary;
    int                     mHashSize;
    IndexSortOrder          mIndexSortOrder;
    bool                    mFilesSortedAlphabetically;
    bool                    mUseSourcePaths;
    bool                    mIncludeFAT;
    bool					       mLowercaseNames;
    bool					       mUppercaseNames;

};

extern SpitImpl *gpSpitInst;
