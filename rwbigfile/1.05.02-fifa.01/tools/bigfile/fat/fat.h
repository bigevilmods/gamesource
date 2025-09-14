// (c) Electronic Arts. All Rights Reserved.

#ifndef BIGFILE_FAT_FAT_H
#define BIGFILE_FAT_FAT_H

#include "eastl/vector.h"
#include "eastl/string.h"
#include "eastl/list.h"

struct BigFileIndexEntry;

namespace FAT {

struct Node
{
    Node()
    {}

    Node(unsigned nameIndex, bool isFile);

    Node(const Node &FN)
        : NameIndex(FN.NameIndex),
          IsFile(FN.IsFile),
          FileIndex(FN.FileIndex),
          Constituents(FN.Constituents)
    {}

    unsigned NameIndex;
    bool IsFile;
    unsigned FileIndex;
    unsigned ContainingNodeIndex;
    unsigned NodeIndex;
    eastl::list<Node *> Constituents;

    static void Create(const BigFileIndexEntry *BFE);
};

unsigned StringTableSize();
eastl::vector<eastl::string>::const_iterator StringListBegin();
eastl::vector<eastl::string>::const_iterator StringListEnd();
eastl::list<Node *>::const_iterator NodeListBegin();
eastl::list<Node *>::const_iterator NodeListEnd();
void AddRoot();
void Release();

}

#endif

