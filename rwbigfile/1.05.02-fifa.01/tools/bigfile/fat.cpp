// (c) Electronic Arts. All Rights Reserved.

#include <cassert>

#include "eastl/map.h"
#include "fat/fat.h"
#include "bigfileindexentry.h"

namespace FAT {

using namespace eastl;

// The index starts at 1 because the static root node is, implicitly, the zero element.
static unsigned Index = 1;
static Node Root;
static map<string, unsigned> StringTable;
static vector<string> StringList;
static list<Node *> Nodes;
static unsigned StringIndex = 0;


static inline bool StringTableContains(const string &str)
{
    return StringTable.find(str) != StringTable.end();
}


static unsigned FetchStringIndex(const string &str)
{
    map<string, unsigned>::iterator I = StringTable.find(str);

    if (I == StringTable.end())
    {
        // The string table is written in as a series of null terminated C-strings
        // and the index is really the byte index to the beginning of the string we are
        // looking for.
        unsigned index = StringIndex;

        StringList.push_back(str);
        StringTable.insert(make_pair(str, index));
        StringIndex += str.length() + 1;

        return index;
    }

    return I->second;
}


static void InternalCreate(const BigFileIndexEntry *BFE, const char *pathString, Node *currentNode)
{
    const char *firstTerminator = strchr(pathString, '/');

    if (!firstTerminator && *pathString)
        firstTerminator = pathString + strlen(pathString);

    ptrdiff_t length = firstTerminator - pathString;
    
    if (firstTerminator && strcmp(pathString, ".") != 0)
    {
        assert(length > 0);

        string subdirectoryName = string(pathString, length);
        list<Node *>::iterator I = currentNode->Constituents.begin(), E = currentNode->Constituents.end();

        if (StringTableContains(subdirectoryName))
        {
            unsigned index = FetchStringIndex(subdirectoryName);

            for (; I != E; ++I)
                if ((*I)->NameIndex == index)
                    break;
        }
        else
        {
            I = E;
        }

        Node *node;
        if (I == E)
        {
            Node *FN = new Node(FetchStringIndex(subdirectoryName), false);
            FN->ContainingNodeIndex = currentNode->NodeIndex;

            currentNode->Constituents.push_front(FN);
            node = FN;
        }
        else
            node = *I;

        InternalCreate(BFE, *firstTerminator ? firstTerminator + 1 : firstTerminator, node);
    }
    else
    {
        Node *FN = new Node(FetchStringIndex(string(BFE->name)), true);
        FN->FileIndex = BFE->index;
        FN->ContainingNodeIndex = currentNode->NodeIndex;

        currentNode->Constituents.push_front(FN);
    }
}

    
Node::Node(unsigned nameIndex, bool isFile)
    : NameIndex(nameIndex),
      IsFile(isFile)
{
    NodeIndex = Index++;
    Nodes.push_back(this);
}


void Node::Create(const BigFileIndexEntry *BFE)
{
    assert(BFE != NULL);
    InternalCreate(BFE, BFE->path, &Root);
}


unsigned StringTableSize()
{
    return StringIndex;
}


vector<string>::const_iterator StringListBegin()
{
    return StringList.begin();
}


vector<string>::const_iterator StringListEnd()
{
    return StringList.end();
}


list<Node *>::const_iterator NodeListBegin()
{
    return Nodes.begin();
}


list<Node *>::const_iterator NodeListEnd()
{
    return Nodes.end();
}


void AddRoot()
{
    Nodes.push_front(&Root);
}


void Release()
{
    // The first node is the root node which we don't want to delete, so pop it from the list.
    Nodes.pop_front();

    for (list<Node *>::const_iterator I = Nodes.begin(), E = Nodes.end(); 
            I != E; ++I)
        delete *I;
}

}

