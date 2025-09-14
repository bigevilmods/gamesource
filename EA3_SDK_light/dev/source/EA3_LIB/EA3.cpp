// (c) 1998-2010 Electronic Arts Inc.

#include <eabase/eabase.h>
#include "EA3.h"
#include "XbArray.h"
#include "EA3Selection.h"
#include <eastl/set.h>
#include <eastl/hash_set.h>
#include <eastl/hash_map.h>
#include <eastl/map.h>

#include <xceedzip/zlib.h>

#include <ctype.h>
#include <stdlib.h>
//XX #include "fstream.h"
#include <sys/stat.h>
#include "EA3grammar.h"
#include <time.h>
#include "ea3mem.h"
#include "ea3chunk.h"
#ifndef EA_PLATFORM_MICROSOFT
#include <sys/time.h>
#include <unistd.h>
#else
#include <io.h>
#endif
#include <new>

#ifdef tolower
#undef tolower
#endif
#ifdef toupper
#undef toupper
#endif

EA_DISABLE_VC_WARNING(4996)

inline int isdigit_l( int c )
{
  return (c >='0' && c <='9');
}

 

inline int isalpha_l( int c )
{
  return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
}

 
inline int isalnum_l( int c )
{
  return isalpha_l( c ) || isdigit_l( c );
}


using namespace eastl;

#ifdef _MSC_VER
#pragma warning(disable : 4100)
#endif // unreferenced formal parameter

EA3_EXPORT int ea3asciiVerbose = 0;
float binVersion;
int ea3verbose = 0;
int setContainerCount = 0;
static int binaryLen = 0;
static char *binaryStart = NULL;
static int writeBlockCount;
static int numWritenBlocks;
static bool doingSceneDelete = false;
static bool deletingGrammar = false;
static bool thisIsTheScene = false;
static EA3Scene *curScene = NULL;

struct EA3BaseEqual {
  bool operator()(const EA3Base *b1,const EA3Base *b2) const {
    return(b1==b2);
  }
}; 
struct EA3BaseHash : public unary_function<const EA3Base *,size_t>  {
  size_t operator()(const EA3Base *b) const {
    return((size_t)b);
  }
};
/* MB: Moved to EA3.h
template <class TYPE>
struct myhash : public unary_function<TYPE,size_t>  {
  size_t operator()(TYPE b) const {
    return((size_t)b);
  }
};
*/
struct ltstr
{
  bool operator()(const char* s1, const char* s2) const
  {
    return strcmp(s1, s2) < 0;
  }
};

EA3IteratorTable masterTable;



typedef hash_set<const EA3Base *,EA3BaseHash,EA3BaseEqual, StlAllocator > EA3BaseSet;
typedef hash_map<const EA3Base *,int,EA3BaseHash,EA3BaseEqual, StlAllocator > EA3BaseIntMap;
//typedef hash_map<const char *,unsigned int,myhash<const char *>,equal_to<const char *>, StlAllocator > StringHash;
typedef map<const char *,unsigned int,ltstr, StlAllocator > StringMap;
static EA3BaseIntMap refIDmap(1000000);
static EA3BaseIntMap uniqueIDmap;
static int uniqueIDcounter = 0;

hash_map<const EA3Base *,const char *,EA3BaseHash,EA3BaseEqual, StlAllocator > extraFieldTypesMap;
hash_map<const EA3Base *,void *,EA3BaseHash,EA3BaseEqual, StlAllocator > userDataMap;
hash_map<const EA3Base *,EA3Base *,EA3BaseHash,EA3BaseEqual, StlAllocator > cloneMap;
static EA3Verbose verbose;

static int Ftell32(FILE *file)
{
    long result = ftell(file);
    assert(result <= INT32_MAX);
    return static_cast<int>(result);
}

void getEA3Version(int &major,int &minor,int &interm) {
  major = EA3_MAJOR_VERSION;
  minor = EA3_MINOR_VERSION;
  interm = EA3_INTERM_VERSION;
}
const char *getEA3Label() {
  return EA3_LABEL;
}

inline int getWriteId(const EA3Base *b) {
  //return (int)b;
  EA3BaseIntMap::iterator it = uniqueIDmap.find(b);
  if (it != uniqueIDmap.end()) {
    return (*it).second;
  } 
  uniqueIDcounter++;
  uniqueIDmap[b] = uniqueIDcounter;
  return uniqueIDcounter;
}
static void resetUniqueIDs() {
  uniqueIDmap.clear();
  //uniqueIDmap.resize(100000);
  uniqueIDcounter = 0;
}

#define skipWhiteSpace(ch) while((*ch==' ') || (*ch == '\t') || (*ch == '\r') || (*ch == '\n')) ch++;

#define nextToken(str) {\
  while (*str && *str!=' ' && *str!='\t' && *str!='\n' && *str!='\r' && *str !=']') str++;\
  skipWhiteSpace(str);\
}


#define string2int(str,i) {i=(int)atoi(str); nextToken(str);}
#define string2float(str,f) {f=(float)atof(str); nextToken(str);}
#define string2XbIdentifier(str,s) str=s.read(str);
#define string2XbQString(str,s) str=s.read(str);
#define string2XbColour(str,r) str=r.read(str);

inline int refCount(EA3Base *b) {
  int rc = b->ref()-1;
  b->unrefNoDelete();
  return(rc);
}


void countWriteBlocks(const EA3Block *b) {
  writeBlockCount++;
  for(int i=b->getNumMembers()-1;i>=0;i--) {
    EA3Block *bl = b->getByIndex(i);
    if (bl) countWriteBlocks(bl);
  }
}

template <int SIZE> 
struct addressHash {
  addressHash() {
    bitSize = 1;
    unsigned int s = SIZE*65536;
    while(s) {
      bitSize++;
      s = s >> 1;
    }
  }
  size_t operator()(void *address) const {
    size_t x = (size_t)address;
    x = x >> bitSize;
    return(x);
  }
  unsigned int bitSize;
};

template <int SIZE>
class CustomHeap : public EA3Mem {
public:
  typedef hash_map< size_t,eastl::pair<int,int>,hash<size_t>,equal_to<size_t>, StlAllocator > IndexMap;
  CustomHeap() {
    heaps.setSize(1);
    lastHeap = lastHeap2 = heaps[0] = new Heap;
    indexMap[ad(&lastHeap->data[0])].first = 1;  // 1 more than index, 0 means unused
  }
  typedef void* Item[(SIZE/4)+(SIZE%4?1:0)];
  class Heap : public EA3Mem {
  public: 
    Heap() {
      index = 0;
      lastFree = -1;
    }
    ~Heap() {
    }
    void *getItem() {
      if (lastFree>-1) {
        Item *ptr = &data[lastFree];
        memcpy(&lastFree,ptr,sizeof(lastFree));
        return(ptr);
      }
      if (index < 65535) {
        void *ptr = &(data[index]);
        index++;
        return(ptr);
      }
      return(NULL);
    }
    bool freeItem(void *ptr) {
      if (&(data[0]) <= ptr && ptr <= &(data[65535])) {
        intptr_t ix = ((char *)ptr-(char*)data) / sizeof(Item);
        memcpy(ptr,&lastFree,sizeof(lastFree));
        lastFree = (int)ix;
        return(true);
      }
      return(false);
    }
    bool containsItem(void *ptr) {
      if (&(data[0]) <= ptr && ptr <= &(data[65535])) {
        return(true);
      }
      return(false);
    }
    Item data[65536];
    int lastFree;
    //XbPArray<void *> freeList;
    int index;
    int num;
  };
  Heap *getHeap(int ix) {
    if (ix >= heaps.getSize()) {
      heaps.setSize(ix+1);
      heaps[ix] = new Heap;
      unsigned int hashval = (unsigned int)ad(&(heaps[ix]->data[0]));
      const IndexMap::iterator &it = indexMap.find(hashval);
      if (it==indexMap.end()) {
        indexMap[hashval].first = ix+1;
      } else {
        (*it).second.second = ix+1;
      }
    }
    return heaps[ix];
  }
  void *malloc() {
    void *it = lastHeap->getItem();
    if (it) return(it);
    int i=0;
    do {
      lastHeap=getHeap(i);
      it = lastHeap->getItem();
      i++;
    } while(!it);
    return(it);
  }
  bool contains(void *ptr) {
    if (lastHeap2->containsItem(ptr)) return(true);
    int i;
    int num = heaps.getSize();
    for(i=0;i<num;i++) {
      if (heaps[i]->containsItem(ptr)) {
        lastHeap2 = heaps[i];
        return(true);
      }
    }
    return(false);
  };
  void dumpIndexMap() {
    IndexMap::iterator i;
    for(i=indexMap.begin();i!=indexMap.end();++i) {
      printf("index %d to %d\n",(*i).first,(*i).second);
    }
  }
  bool free(void *ptr) {
    if (lastHeap2->freeItem(ptr)) return(true);
    //dumpIndexMap();
    IndexMap::iterator it = indexMap.find(ad(ptr));
    if (it!=indexMap.end()) {
      eastl::pair<int,int> &ind = (*it).second;
      int i = ind.first-1;
      if (i!=-1) {
        lastHeap2 = heaps[i];
        if (lastHeap2->freeItem(ptr)) {
          return(true);
        }
      }
      i = ind.second-1;
      if (i!=-1) {
        lastHeap2 = heaps[i];
        if (lastHeap2->freeItem(ptr)) {
          return(true);
        }
      }
    }
    int num = heaps.getSize();
    int i = 0;
    for(i=0;i<num;i++) {
      if (heaps[i]->freeItem(ptr)) {
        lastHeap2 = heaps[i];
        return(true);
      }
    }
    return(false);
  };
  ~CustomHeap() {
    int i;
    int num = heaps.getSize();
    for(i=0;i<num;i++) {
      delete heaps[i];
    }
  }
  addressHash<SIZE> ad;
  Heap *lastHeap,*lastHeap2;
  XbPArray<Heap *> heaps;
  IndexMap indexMap;
};

CustomHeap<sizeof(EA3Field)> basesHeap;

CustomHeap<2*4> cheap2;
CustomHeap<3*4> cheap3;
CustomHeap<4*4> cheap4;
CustomHeap<5*4> cheap5;
CustomHeap<6*4> cheap6;
CustomHeap<16*4> cheap16;

class EA3GrammarTable;
typedef hash_map <const EA3Base *,EA3GrammarTable *,EA3BaseHash,EA3BaseEqual, StlAllocator> GrammarTableMap;
GrammarTableMap gtMap;

class EA3GrammarTable : public EA3Mem {
public:
  EA3GrammarTable(const EA3Block *block,bool recurse = true) {
    mScene = block->getScene();
    fillTable(block,recurse);
  }
  void fillTable(const EA3Block *block,bool recurse) {
    int i;
    int num=block->getNumMembers();
    for(i=0;i<num;i++) {
      EA3Base *base = block->getByIndex(i);
      if (base->isField()) {
        grammarFieldMap[base->getName().getString()] = base;
        continue;
      }
      if (base->isReference()) {
        if (base->getName()[0]!=0) {
          grammarRefMap[base->getName().getString()] = base;
        } else {
          grammarRefMap[base->getType().getString()] = base;
        }
        continue;
      }
      EA3Block *bl = base->toBlock();
      if (!bl) continue;
      if(bl->isArray()) {
        if (bl->getName()[0] == 0 || bl->getName()=="children") {
          grammarArrayMap[bl->getType().getString()] = bl;
        } else {
          grammarArrayMap[bl->getName().getString()] = bl;
        }
      } else {
        XbIdentifier inh = bl->getInherits();
        if (inh[0] != '_') {
          int num2 = block->getNumMembers();
          EA3Base *inbl = NULL;
          for(int j=0;j<num2;j++) {
            EA3Base *b = block->getByIndex(j);
            if (b->isArray()) continue;
            if (b->isBlock() && b->getType()==inh) {
              inbl = b->toBlock();
              break;
            }
          }
          if (inbl) {
            grammarBlockMap[bl->getType().getString()] = bl;
            EA3GrammarTable *gt = new EA3GrammarTable(bl->toBlock(),false);
            gtMap[bl] = gt;
            gt->fillTable(inbl->toBlock(),false);
            continue;
          }
        } else {
          grammarBlockMap[bl->getType().getString()] = bl;
        }
      }
      if (recurse) {
        EA3GrammarTable *gt = new EA3GrammarTable(bl,false);
        gtMap[bl] = gt;
      }
    }
  }
  EA3Base *getGrammar(const EA3Block *par,const XbIdentifier &tp,char what) {
    EA3Base *b;
    const char *type = tp.getString();
    if (what == 'B') {
      b = grammarBlockMap[type];
      if (b) return(b);
    }
    if (what == 'A') {
      b = grammarArrayMap[type];
      if (b) return(b);
    }
    if (what == 'F') {
      b = grammarFieldMap[type];
      if (b) return(b);
    }
    if (what == 'R') {
      b = grammarRefMap[type];
      if (b) return(b);
    }
    if (par) {
      EA3GrammarTable *gt = gtMap[par->getGrammar()];
      if (gt) {
        b = gt->getGrammar(NULL,tp,what);
        if (b) {
          return(b);
        } else {
        }
      }
    }
    return NULL;
  }
  typedef hash_map< const char *,EA3Base *,myhash<const char *>,equal_to<const char *>, StlAllocator > GrammarMap;
  GrammarMap grammarBlockMap,grammarArrayMap,grammarFieldMap,grammarRefMap;
  EA3Scene *mScene;
  private:
    int refCount;
};

class GTMapCleanup {
public:
  ~GTMapCleanup() {
    GrammarTableMap::iterator gti;
    for(gti = gtMap.begin();gti!=gtMap.end();++gti) {
      delete (*gti).second;
    }
  }
} _gtmapcleanup;

void *EA3Base::operator new(size_t s) {
  if (s==sizeof(EA3Field)) {
    return(basesHeap.malloc());
  }
  return EA3Malloc(s);
}
void EA3Base::operator delete(void *v,size_t s) {
  if (s==sizeof(EA3Field) && basesHeap.free(v)) {
    return;
  }
  EA3Free(v);
}

/*
class gzWriter : public EA3Mem {
public:
  gzWriter();
  gzWriter(int fd);
  gzWriter(gzFile gzf);
  const char *str();
  bool fail() return (!ok);
  bool bad() return(!ok);
  void open(const char *filename);
  void close();
  gzWriter &operator<<(int i);
  gzWriter &operator<<(unsigned int i);
  gzWriter &operator<<(float i);
  gzWriter &operator<<(const char *str);
  gzWriter &operator<<(char c);
private:
  bool ok;
  gzFile gz;
  int fd;
  XbPArray<char> data;
};
*/


//int resolveClonePtrCount = 0;

/*
Notes on EA3 binary file format:
File format (Intel Little Endian) (CHUNK_HEADER_SIZE is 20 bytes)

BEA3                4 character file format id
BINARY_VERSION            32 bit float
VERSION_STRING      0 terminated string of chars
STRING_TABLE_SIZE   32 bit int
<string table>      STRING_TABLE_SIZE number of 0 terminated strings of chars
<chunks>

Chunk Header:

CHUNK_TYPE                    4 characters (32 bit)
CHUNKSIZE           32 bit int
  Incrementing the pointer after reading CHUNKSIZE by CHUNKSIZE 
  will take you to a next CHUNK at the same hierarchical level, 
  not a child chunk.
ID                  32 bit int
TYPE_IX             32 bit int (type is this entry in the string table)
NAME_IX             32 bit int (name is this entry in the string table)
<chunk specific data>

For BLK\n, ARR\n and GRM\n chunks:
NUM_CHILD_CHUNKS    32 bit int
<child chunks>

For FLD\n
To calculate number of 32 bit values per field: num = (chunkSize-5*4)/4
DATA_TYPE_IX        32 bit int (index into string table)
<data>              num * 32 bit values 
(see readBinaryField on how to interpret data)

For MFL\n (multiple fields with same type and name)
DATA_TYPE_IX        32 bit int (index into string table)
NUM_DATA            32 bit int (number of 32 bit entries/field)
To calculate number of fields: numFields = (chunkSize-6*4)/(numD*4);
<data>              numFields * NUM_DATA 32 bit values

For REF\n (arrays of references)
IS_ALL_ARRAY        8 bit unsigned char (0 or 1)
If reference list is all to members of the same array (IS_ALL_ARRAY==1) then:
(This is the most common case)
To calculate number of references:  num = (chunkSize-(4*5+1))/4
  ID                32 bit id of array
  <indices>         num*32 bit ints for each index into array
Else
To calculate number of references:  num = (chunkSize-(4*4+1))/8
  ID                32 bit int id 
  INDEX             32 bit int index 
    (if -1 then just use id, else use index'th child of id)

*/

static EA3Selection ea3selection;
static void Internal_ea3SetSelectCB(const EA3Base *b,bool state) {
  if (state) ea3selection.select(b); else ea3selection.unselect(b);
}
static bool Internal_ea3IsSelectedCB(const EA3Base *b) {
  return ea3selection.isSelected(b);
}
static void Internal_ea3GetSelectedCB(XbPArray<EA3Base *> &sel) {
  ea3selection.getSelected(sel);
}
EA3_EXPORT EA3SetSelectCB *ea3setSelectCB = Internal_ea3SetSelectCB;
EA3_EXPORT EA3IsSelectedCB *ea3isSelectedCB = Internal_ea3IsSelectedCB;
EA3_EXPORT EA3GetSelectedCB *ea3getSelectedCB = Internal_ea3GetSelectedCB;

EA3_EXPORT void EA3SetSelect(const EA3Base *b,bool state) {
  if (ea3setSelectCB) {
    if (b) {
      (*ea3setSelectCB)(b,state);
    }
  }
}
EA3_EXPORT bool EA3IsSelected(const EA3Base *b) {
  if (ea3isSelectedCB) {
    return (*ea3isSelectedCB)(b);
  }
  return(false);
}
EA3_EXPORT void EA3GetSelected(XbPArray<EA3Base *> &sel) {
  if (ea3getSelectedCB) {
    (*ea3getSelectedCB)(sel);
  }
}

#define XbHashString_SIZE 4096
class XbHashString : public EA3Mem {
  public:
    XbHashString();
    ~XbHashString();

    char *get(const char *str);

    void printAll() {
      int i;
      for(i=0;i<XbHashString_SIZE;i++) {
        for(int j=0;j<list[i].getSize();j++) {
          printf("list %d %s\n",i,list[i][j]);
        }
      }
    };
    int getMemSize() {
      int s = 0;
      int i;
      for(i=0;i<XbHashString_SIZE;i++) {
        for(int j=0;j<list[i].getSize();j++) {
          s+=(int)strlen(list[i][j])+1;
        }
      }
      return(s);
    }
    int getSize() {
      int s = 0;
      int i;
      for(i=0;i<XbHashString_SIZE;i++) {
        s+=list[i].getSize();
      }
      return(s);
    }
    void cleanUp();
  private:
    void init();
    XbPArray<char *> *list;
    int numAdded;
    int bytesLeft;
    char *mem;
    XbPArray<char *> *memList;
};

static XbHashString XB_hashString; 
EA3_EXPORT int getStringHashMemSize() {
  return XB_hashString.getMemSize();
}
EA3_EXPORT int getStringHashSize() {
  return XB_hashString.getSize();
}
static char *XbHS_last = NULL;
static char *XbHS_empty = NULL;
static char *XbHS_underScore = NULL;

class XbRefName : public EA3Mem {
// Internal use only.  Used to resolve references at load time
  public:
    XbRefName() { 
      id = -1;
      childIndex = -1;
      type = XbHS_underScore;
      name = XbHS_empty;
    }
    XbRefName(const XbRefName &other) {
      *this = other;
    }

    inline void *operator new(size_t s);
    inline void operator delete(void *v);

    char *read(char *is);
    //virtual gzWriter& write( gzWriter& os ) const;
    // don't try this at home kids.  I use the toIdentifier call becauce I KNOW the original const char* are in the XbHashString
    XbString getName() {return(EA3Chunk::toIdentifier(name));}
    XbIdentifier getType() {return(EA3Chunk::toIdentifier(type));}
    int getId() const {return(id);}
    void setName(const XbString &n) {name = n.getString();}
    void setType(const XbIdentifier &t) {type = t.getString();}
    void setId(int i) {id = i;}
    XbRefName & operator =(const XbRefName &other) {
      type = other.type;
      name = other.name;
      id = other.id;
      childIndex = other.childIndex;
      return(*this);
    }
    int childIndex;
    int id;
    const char*type;
    const char*name;
};
    
CustomHeap<sizeof(XbRefName)> XbRefNameHeap;

void *XbRefName::operator new(size_t s) {
  return(XbRefNameHeap.malloc());
}
void XbRefName::operator delete(void *v) {
  if(v) XbRefNameHeap.free(v);
}

EA3_EXPORT int getNumBaseInfos();
EA3_EXPORT int getNumFieldInfos();

CustomHeap<sizeof(EA3BaseInfo)> baseInfoHeap;

void *EA3BaseInfo::operator new(size_t s) {
  return(baseInfoHeap.malloc());
}
void EA3BaseInfo::operator delete(void *v) {
  baseInfoHeap.free(v);
}

struct ltEA3BaseInfo {
  bool operator()(const EA3BaseInfo *b1,const EA3BaseInfo *b2) const {
    if (b1->container < b2->container) return(1);
    if (b1->container > b2->container) return(0);
    if (b1->grammar < b2->grammar) return(1);
    if (b1->grammar > b2->grammar) return(0);
    if (b1->type < b2->type) return(1);
    if (b1->type > b2->type) return(0);
    if (b1->name < b2->name) return(1);
    return(0);
  }
};
struct EA3BaseInfoEqual {
  bool operator()(const EA3BaseInfo *b1,const EA3BaseInfo *b2) const {
    return(b1->container == b2->container
       && b1->grammar == b2->grammar
       && b1->type == b2->type
       && b1->name == b2->name);
  }
};

struct EA3BaseInfoHash : public unary_function<const EA3BaseInfo *,size_t> {
  size_t operator()(const EA3BaseInfo *b1) const {
    size_t key = (size_t)b1->container ^ (size_t)b1->grammar ^ (size_t)b1->type ^ (size_t)b1->name;
    return(key);
  }
};
class EA3BaseInfoList : public EA3Mem {
// Hashes EA3BaseInfo's
  public:
    EA3BaseInfoList();
    ~EA3BaseInfoList();
    EA3BaseInfo *get(EA3Block *c,EA3Base *g,const char *t,const char *n);
    void setGrammar(EA3Base *b, EA3Base *g);
    void setContainer(EA3Base *b, EA3Block *c);
    void setName(EA3Base *b,const XbString &n);
    void getStrings(const EA3Scene *s,StringMap &types,StringMap &names) {
      Hash::iterator i;
      // zero is reserved to indicate string is not in table
      int s_count = 0;
      // We want lower index values for types so that we can use less bytes
      // for the object when saving a binary file.
      for(i=list.begin();i!=list.end();++i) {
        s_count++;
        EA3BaseInfo *bi = *i;
        const EA3Block *s2 = bi->getScene();
        if (s2!=s && s2!=NULL) continue;
        if (types.find(bi->type)==types.end()) {
          types[(*i)->type]=1;
        }
        if (bi->name[0] == 0) continue;
        if (names.find(bi->name)==names.end()) {
          names[(*i)->name]=1;
        }
      }
    }
    void cleanUp();
  private:
    typedef hash_set<EA3BaseInfo*,EA3BaseInfoHash,EA3BaseInfoEqual, StlAllocator> Hash;
    Hash list;
    //XbPArray<EA3BaseInfo *> *list[EA3BASE_INFO_LIST_SIZE];
    int numAdded;
    EA3BaseInfo *last;
  friend class EA3BaseInfo;
  friend int getNumBaseInfos();
  friend int getNumFieldInfos();
};


static EA3BaseInfoList gInfoList;
EA3_EXPORT int getNumBaseInfos() {
  return gInfoList.numAdded;
}
EA3_EXPORT int getNumFieldInfos() {
  int con_count = 0;
  EA3BaseInfoList::Hash::iterator i;
  for(i=gInfoList.list.begin();i!=gInfoList.list.end();++i) {
    EA3BaseInfo *bi = *i;
    if (bi->container) con_count++;
  }
  return con_count;
}

#define EA3_ERROR1(a) if(EA3Scene::errorCallback) {\
  sprintf(EA3Scene::errorData,a);\
  (*EA3Scene::errorCallback)(EA3Scene::errorData);\
}
#define EA3_ERROR2(a,b) if(EA3Scene::errorCallback) {\
  sprintf(EA3Scene::errorData,a,b);\
  (*EA3Scene::errorCallback)(EA3Scene::errorData);\
}
#define EA3_ERROR3(a,b,c) if(EA3Scene::errorCallback) {\
  sprintf(EA3Scene::errorData,a,b,c);\
  (*EA3Scene::errorCallback)(EA3Scene::errorData);\
}
#define EA3_ERROR4(a,b,c,d) if(EA3Scene::errorCallback) {\
  sprintf(EA3Scene::errorData,a,b,c,d);\
  (*EA3Scene::errorCallback)(EA3Scene::errorData);\
}
#define EA3_ERROR5(a,b,c,d,e) if(EA3Scene::errorCallback) {\
  sprintf(EA3Scene::errorData,a,b,c,d,e);\
  (*EA3Scene::errorCallback)(EA3Scene::errorData);\
}
#define EA3_ERROR6(a,b,c,d,e,f) if(EA3Scene::errorCallback) {\
  sprintf(EA3Scene::errorData,a,b,c,d,e,f);\
  (*EA3Scene::errorCallback)(EA3Scene::errorData);\
}
#define ERROR_LINE(a) EA3Scene::readStatus=a;EA3_ERROR2(" at line: %d",EA3Scene::lineNum);

#define XbNumber(arr) ((int) (sizeof(arr) / sizeof(arr[0])))

#define _TIME_(label,func) {\
  struct timeval tp;\
  gettimeofday(&tp);\
  printf("TIME_" #label " " # func " %ld %ld\n",tp.tv_sec,tp.tv_usec);\
}
#define TIME(label,func) 

static void defaultError(char *msg) {
  fprintf(stderr,"ERR: %s\n",msg);
}

#define EA3BaseLRU_SIZE 0xffff
class EA3BaseLRU : public EA3Mem {
// A Least Recently Used list of EA3Base's
  public:
    inline EA3BaseLRU() {
        //refHash.resize(1000000);
    }
    inline const EA3Base *find(int id) {
      const EA3Base *b = refHash[id];
      /*
      hash_map<int, const EA3Base *>::iterator i = refHash.find(id);
      if (i!=refHash.end()) {
        b = b;
      } else {
        b = NULL;
      }
      */
      return b;
    }
    inline void add(const EA3Base *b,int id) {
      if (id != -1) refHash[id] = b;
    }
    inline void clear() {
      refHash.clear();
    }
  //private:
    hash_map< int, const EA3Base *,hash<int>,equal_to<int>, StlAllocator > refHash;
};



int counter;

EA3_EXPORT int numInfos = 0;
EA3_EXPORT int numBases = 0;

XbPArray<char> XbString::linebuf;
EA3Pointer::EA3Null EA3Pointer::ea3null;
int EA3Scene::lineNum = 0;
char EA3Scene::errorData[MAX_LINE_LEN];
EA3Status EA3Scene::readStatus = EA3OK;
int EA3Scene::numReferences = 0;
int EA3Scene::numResReferences = 0;
EA3Scene::ErrorCallback *EA3Scene::errorCallback = defaultError;
EA3Scene::ProgressCallback *EA3Scene::progressCallback = NULL;
EA3BaseLRU EA3Reference::LRU;
EA3Base::EA3BaseDeleteCB *EA3Base::deleteCB;

EA3Field *EA3Block::createField(const XbIdentifier &t,const XbString &nm,bool isGram) {
  if (!isArray() && !isGram) {
    if (EA3Base *b = getByType(t)) {
      if (b->isField()) return(b->toField());
    }
  }
  EA3Grammar *g = (EA3Grammar *)getGrammar();
  return(g->createFieldInstance(this,t,nm,isGram));
}

XbIdentifier convertDataType(XbIdentifier dataType) {
  static XbIdentifier table[] = {
    "float3","fff",
    "float","f",
    "float2","ff",
    "float4","ffff",
    "float9","fffffffff",
    "float16","ffffffffffffffff",
    "int","i",
    "XbColour","c",
    "XbIdentifier","d",
    "XbQString","s"
  };
  for(int i=0;i<10;i++) {
    if (table[i*2]==dataType) {
      return(table[i*2+1]);
    }
  }
  return dataType;
}

inline int getAllocated(int s) {
  if (s == 0) return(1);
  int size = s-1;
  int n = 0;
  n = 1;
  if (size >= 65536) {
    size = size >> 16;
    n = n << 16;
  }
  if(size >= 256) {
    n = n << 8;
    size = size >> 8;
  }
  if (size >= 16) {
    n = n << 4;
    size = size >> 4;
  }
  if (size >= 4) {
    n = n << 2;
    size = size >> 2;
  }
  if (size >= 2) {
    n = n << 1;
    size = size >> 1;
  }
  if (size >= 1) {
    n = n << 1;
  }
  assert(n>=s);
  return(n);
}

inline intptr_t getMemberSize(const struct EA3Base::Flags &flags,EA3Base **const members) {
  if (flags.num < 127) return (int)flags.num;
  return (intptr_t)members[0];
}
int EA3Block::getNumMembers() const {
  return (int)getMemberSize(flags,members);
}
void *heapMalloc(int size) {
  if (size==2) {
    return cheap2.malloc();
  }
  if (size==3) {
    return cheap3.malloc();
  }
  if (size==4) {
    return cheap4.malloc();
  }
  if (size==5) {
    return cheap5.malloc();
  }
  if (size==6) {
    return cheap6.malloc();
  }
  return NULL;
}
bool heapFree(void *ptr,int size) {
  if (size == 2) {
    return(cheap2.free(ptr));
  }
  if (size == 3) {
    return(cheap3.free(ptr));
  }
  if (size == 4) {
    return(cheap4.free(ptr));
  }
  if (size == 5) {
    return(cheap5.free(ptr));
  }
  if (size == 6) {
    return(cheap6.free(ptr));
  }
  EA3Free(ptr);
  return false;
}

void *EA3HeapAlloc(int size) {
	if (size == 16) return cheap16.malloc();
	return heapMalloc(size);
}
bool EA3HeapFree(void *ptr,int size) {
	if (size == 16) return cheap16.free(ptr);
	return heapFree(ptr,size);
}

inline void setMemberSize(struct EA3Base::Flags &flags,EA3Base **&members,int newSize) {
  int i;
  int oldSize = (int)getMemberSize(flags,members);

  if (oldSize == newSize) return;
  if (oldSize == 0) {
	EA3Base** oldStart = (EA3Base**)&members;

    if (newSize == 0) return;
    if (newSize == 1) {
		flags.num = 1;

		// the allocation doesnt change but end should now move over one
		masterTable.updateIterators( oldStart, oldStart, oldStart, oldStart + 1);
		return;
	}
    if (newSize < 127) {
      flags.num = newSize;
      members = (EA3Base **)heapMalloc(newSize);
	  if (members){
		  masterTable.updateIterators( oldStart, oldStart, (EA3Base**)members, (EA3Base**)(members + newSize) );
		  return;
	  }
      members = (EA3Base **)EA3Malloc(sizeof(EA3Base*)*getAllocated(newSize));
	  masterTable.updateIterators( oldStart, oldStart, (EA3Base**)members, (EA3Base**)(members + newSize) );
      return;
    }
    flags.num = 127;
    members = (EA3Base **)EA3Malloc(sizeof(EA3Base*)*getAllocated(newSize+1));
    members[0] = (EA3Base *)(static_cast<intptr_t>(newSize));
	masterTable.updateIterators( oldStart, oldStart, (EA3Base**)(members + 1), (EA3Base**)(members + newSize + 1) );
    return;
  }
  if (oldSize == 1) {
	  EA3Base** oldStart = (EA3Base**)&members;
	  EA3Base** oldEnd = (EA3Base**)(&members + 1);
    if (newSize == 0) {
		flags.num = 0;
		masterTable.updateIterators( oldStart, oldEnd, (EA3Base**)&members, (EA3Base**)&members );
		members = 0;
		return;
	}
    if (newSize == 1) {
		flags.num = 1;
		return;
	}
    EA3Base *last = (EA3Base *)members;
    if (newSize < 127) {
      flags.num = newSize; 
      members = (EA3Base **)heapMalloc(newSize);
      if (!members) 
		  members = (EA3Base **)EA3Malloc(sizeof(EA3Base*)*getAllocated(newSize));
      members[0] = last;

	  masterTable.updateIterators( oldStart, oldEnd, (EA3Base**)members, (EA3Base**)(members + newSize) );

      return;
    }
    members = (EA3Base **)EA3Malloc(sizeof(EA3Base*)*getAllocated(newSize+1));
    members[0] = (EA3Base *)(static_cast<intptr_t>(newSize));
    members[1] = last;

	masterTable.updateIterators( oldStart, oldEnd, (EA3Base**)(members + 1), (EA3Base**)(members + newSize + 1) );
    return;
  }
  if (oldSize < 127) {
	EA3Base** oldStart = (EA3Base**)members;
	EA3Base** oldEnd = (EA3Base**)(members + oldSize);

    if (newSize == 0) {
      flags.num = 0;
      heapFree(members,oldSize);
	  masterTable.updateIterators( oldStart, oldEnd, (EA3Base**)&members, (EA3Base**)&members );
      members = 0;
      return;
    }
    if (newSize == 1) {
      EA3Base *last = members[0];
      flags.num = newSize;
      heapFree(members,oldSize);
      members = (EA3Base **)last;
	  masterTable.updateIterators( oldStart, oldEnd, (EA3Base**)(&members), (EA3Base**)(&members + 1) );
      return;
    }
    if (newSize < 127) {
      flags.num = newSize;
      EA3Base **newMembers = (EA3Base **)heapMalloc(newSize);
      if (newMembers) {
        memcpy(newMembers,members,newSize*sizeof(EA3Base*));
        heapFree(members,oldSize);
        members = newMembers;
		masterTable.updateIterators( oldStart, oldEnd, (EA3Base**)members, (EA3Base**)(members + newSize) );
        return;
      }
      if (oldSize < 7) {
        newMembers = (EA3Base **)EA3Malloc(sizeof(EA3Base*)*getAllocated(newSize));
        memcpy(newMembers,members,newSize*sizeof(EA3Base*));
        heapFree(members,oldSize);
        members = newMembers;
		masterTable.updateIterators( oldStart, oldEnd, (EA3Base**)members, (EA3Base**)(members + newSize) );
        return;
      }
      int nasize = getAllocated(newSize);
      if (getAllocated(oldSize)!=nasize) {
        members = (EA3Base **)EA3Realloc(members,sizeof(EA3Base*)*nasize);
      }
	  masterTable.updateIterators( oldStart, oldEnd, (EA3Base**)members, (EA3Base**)(members + newSize) );
      return;
    }
    flags.num = 127;
    if (oldSize < 7) {
      EA3Base ** newMembers = (EA3Base **)EA3Malloc(sizeof(EA3Base*)*getAllocated(newSize+1));
      memcpy(newMembers,&members[1],newSize*sizeof(EA3Base*));
      heapFree(members,oldSize);
      members=newMembers;
    } else {
      members = (EA3Base **)EA3Realloc(members,sizeof(EA3Base*)*getAllocated(newSize+1));
      // move things over so we can store the size at index 0
      for (i = newSize; i > 0; i-- ) members[i] = members[i-1];
    }
    members[0] = (EA3Base *)(static_cast<intptr_t>(newSize));
	masterTable.updateIterators( oldStart, oldEnd, (EA3Base**)(members + 1), (EA3Base**)(members + newSize + 1) );
    return;
  }
  // oldSize >= 127

  EA3Base** oldStart = (EA3Base**)(members + 1);
  EA3Base** oldEnd = (EA3Base**)(members + oldSize + 1);
  if (newSize == 0) {
    flags.num = 0;
    EA3Free(members);
	masterTable.updateIterators( oldStart, oldEnd, (EA3Base**)members, (EA3Base**)members );
    members = 0;
    return;
  }
  if (newSize == 1) {
    EA3Base *last = members[1];
    flags.num = newSize;
    EA3Free(members);
    members = (EA3Base **)last;
	masterTable.updateIterators( oldStart, oldEnd, (EA3Base**)members, (EA3Base**)(members + 1));
    return;
  }
  if (newSize < 127) {
    EA3Base *lastMem = members[newSize];
    flags.num = newSize;
    EA3Base **newMembers = (EA3Base **)heapMalloc(newSize);
    if (newMembers) {
      memcpy(newMembers,members,newSize*sizeof(EA3Base*));
      heapFree(members,oldSize);
      members = newMembers;
    } else {
      members = (EA3Base **)EA3Realloc(members,sizeof(EA3Base*)*getAllocated(newSize));
    }
    // move things over since we no longer need the size at index 0
    for(i=0;i<newSize-1;i++) {
      //printf("i %d\n",i);
      members[i] = members[i+1];
    }
    members[newSize-1] = lastMem;
    flags.num = newSize;
	masterTable.updateIterators( oldStart, oldEnd, (EA3Base**)members, (EA3Base**)(members + newSize) );
    return;
  }
  // oldSize >= 127 and newSize >= 127
  int nasize = getAllocated(newSize+1);
  if (getAllocated(oldSize+1)!=nasize) {
    members=(EA3Base **)EA3Realloc(members,sizeof(EA3Base*)*nasize);
  }
  members[0]=(EA3Base *)(static_cast<intptr_t>(newSize));
  masterTable.updateIterators( oldStart, oldEnd, (EA3Base**)(members + 1), (EA3Base**)(members + newSize + 1) );

}


inline void addMember(struct EA3Base::Flags &flags,EA3Base **&members,EA3Base *item) {
  int oldSize = (int)getMemberSize(flags,members);
  setMemberSize(flags,members,oldSize+1);
  if (oldSize == 0) {
    members = (EA3Base **)item;
    return;
  }
  if (oldSize < 126) {
    members[oldSize] = item;
  } else {
    members[oldSize+1] = item;
  }
}
inline void insertMember(struct EA3Base::Flags &flags,EA3Base **&members,EA3Base *item,int ix) {
  int oldSize = (int)getMemberSize(flags,members);
  int newSize = oldSize+1;
  setMemberSize(flags,members,newSize);
  if (newSize == 1) {
    members = (EA3Base **)item;
    return;
  }
  int offset = 0;
  if (flags.num == 127) offset = 1;
  for (int i = newSize-1; i > ix; i-- ) members[i+offset] = members[i-1+offset];
  members[ix+offset] = item;
}
inline void setMember(struct EA3Base::Flags &flags,EA3Base **&members,EA3Base *item,int ix) {
  if (flags.num == 0) return;
  if (flags.num == 1 && ix == 0) {
    members = (EA3Base **)item;
    return;
  }
  if (flags.num < 127 && ix < (int)flags.num) {
    members[ix] = item;
    return;
  } 
  intptr_t size = (intptr_t)members[0];
  if (ix < size) {
    members[ix+1] = item;
  } else {
    printf("here\n");
  }
}
void verify(EA3Base *b=curScene) {
  b->isArray();
  b->isField();
  b->isBlock();
  if (b->isReference()) {
    EA3Reference *r = b->toReference();
	EA3ConstIterator it = r->beginConst();
	EA3Base** end = it.end();
    for(;it!=end;++it) {
      it->isField();
    }
  }
  EA3Block *bl = b->toBlock();
  if (!bl) return;
  EA3ConstIterator it = bl->beginConst();
  EA3Base** end = it.end();
  for(;it!=end;++it) {
    EA3Base *ch=*it;
    verify(ch);
  }
}
  
inline void removeMember(struct EA3Base::Flags &flags,EA3Base **&members,int ix) {
  static int _count=0;
  _count++;
  int oldSize = (int)getMemberSize(flags,members);
  int offset = 0;
  if (flags.num == 127) offset = 1;
  //verify();
  int newSize=oldSize-1;
  if (oldSize == 0) return;
  if (oldSize == 1) {
    setMemberSize(flags,members,newSize);
    return;
  }
  
  EA3Base *lastMem = members[oldSize-1+offset];
  setMemberSize(flags,members,newSize);
  
  if (flags.num != 127) offset = 0;
  //printf("ix = %d newSize = %d\n",ix,newSize);
  for(int i = ix+1;i<newSize;i++) {
    //printf("%d from %d\n",i-1,i);
    members[i-1+offset] = members[i+offset];
  }
  if (ix != newSize) {
    if (newSize == 1) {
      members = (EA3Base**)lastMem;
    } else {
      members[newSize-1+offset] = lastMem;
    }
  }
  //verify();
}

EA3Field *EA3Block::realCreateField(const EA3Base *g,const XbIdentifier &bType, const XbIdentifier &t,const XbString &nm,int ix) {
  EA3Grammar *gr = const_cast<EA3Grammar *>(static_cast<const EA3Grammar *>(g));
  EA3Field *field = NULL;
  const char *bt = bType.getString();
  int num = (int)strlen(bt);
  field = new EA3Field(bType,num,this,gr,getNumMembers(),t,nm);
  if (field) {
    if (ix == -1) {
      addMember(flags,members,field);
    } else {
      ::setMember(flags,members,field,ix);
    }
    flags.needsReindex=1;
    field->ref();
  }
  return(field);
}


XbHashString::XbHashString()
{
} 

void XbHashString::init()
{
  //printf("initializing hash table\n");
  list = new XbPArray<char *>[XbHashString_SIZE];
  int i;
  for(i=0;i<XbHashString_SIZE;i++) {
    list[i].setSize(4);
    list[i].setSize(0);
  }
  memList = new XbPArray<char *>;
  numAdded = 0;
  bytesLeft = 0;
  mem = NULL;
}

void XbHashString::cleanUp()
{
  if (!list) return;
  int i;
  for(i=0;i<XbHashString_SIZE;i++) {
    for(int j=0;j<list[i].getSize();j++) {
      //EA3Free(list[i][j]); //TODO
    }
  }
  for(i=0;i<memList->getSize();i++) {
    EA3Free(memList[0][i]);
  }
  delete memList;
  EA3Free(XbHS_empty);
  EA3Free(XbHS_underScore);
  delete [] list;
  list = NULL;
  XbHS_last = NULL;
}

void EA3_cleanUp()
{
  XB_hashString.cleanUp();
  gInfoList.cleanUp();
}

XbHashString::~XbHashString()
{
  cleanUp();
}

inline int strEqual(const char *s1, const char *s2)
{
	if(s1 == s2) return 1;		// If the addresses are equal, the strings are too...
	while(*s1 && *s1 == *s2)	// No "loop" variable necessary
		s1++, s2++;
	return *s1 == *s2;
}

inline char *XbHashString::get(const char *str)
{
  if (XbHS_last == NULL)
  {
    init();
    //printf("First hash\n");
    XbHS_empty = XbString::_strdup("");
    list[0].append(XbHS_empty);
    XbHS_underScore = XbString::_strdup("_");
    list[(int)'_'].append(XbHS_underScore);
    XbHS_last=XbHS_empty;
  }
  if(str[0] == 0) return(XbHS_empty);
  if(str[0] == '_' && str[1] == 0) return(XbHS_underScore);

  int i;
  unsigned int hashVal = 0;
  const char *tmp = str;

  if(strEqual(XbHS_last, str))
  {
    return(XbHS_last);
  }

  // New way - Faster to compute length outside the loop - No need to
  // intialize it to zero, and we do the +1 right here while it's still "warm"
  while(*tmp)
  {
	hashVal += (hashVal<<3) + *tmp++;
  }
  int len = (int)(tmp - str + 1);

  char *val;
  //int misses = 0;
  hashVal = hashVal % (XbHashString_SIZE);
  XbPArray<char *> &sublist = list[hashVal];
  for(i=0;i<sublist.getSize();i++)
  {
    val = sublist[i];
    if(strEqual(val,str))
	{
      XbHS_last = val;
      return(val);
    }
    //misses++;
    //printf("misses = %d\n",misses);
  }

  if(bytesLeft < len)
  {
      const int MaxAllocSize = (128 * 1024);
      mem = (char *) EA3Malloc(len < MaxAllocSize ? MaxAllocSize : len);
      memList->append(mem);
      bytesLeft = MaxAllocSize-len;
      val = mem;
      mem += len;
  }
  else
  {
      val = mem;
      mem += len;
      bytesLeft -= len;
  }

  //val = XbString::_strdup(str);
  memcpy(val, str, len);
  sublist.append(val);
  XbHS_last = val;
  return(val);
}
// XbString //

XbString::XbString() {
  data = 0;
  setString("");
}

XbString::XbString(const char *str) {
  data = 0;
  setString(str);
}

XbString::XbString(const XbString &other) {
  data = other.data;
}

XbString::XbString(const EA3FieldDataString &other) {
  data = const_cast<char *>((const char*)other);
  //setString((const char*)other);
}

//XbString::~XbString() {
//}

XbString XbString::tolower() const {
  XbString n = *this;
  int len = (int)strlen(n.data)+1;
  char *tl = (char *)EA3Malloc(len);
  for(int i=0;i<len;i++) {
    tl[i] = (char)::tolower(n.data[i]);
  }
  n.setString(tl);
  EA3Free(tl);
  return(n);
}

int XbString::length() const {
  return((int)strlen(data));
}

char *XbString::_strdup(const char *str) {
  int len = (int)strlen(str)+1;
  //printf("len = %d\n",len);
  char *s = (char *)EA3Malloc(len);
  strcpy(s,str);
  return(s);
}

void XbString::setString(const char *str) {
  data = XB_hashString.get(str);
}

void XbString::setString(const XbString &str) {
  data = str.data;
}
/*
istream& XbString::read( istream& is ) {
  static char buf[ 999 ];
  static char buf2[ 999 ];
  buf2[0] = 0;
  for ( ;; ) {
    is.width( sizeof( buf ) );
    if ( ! ( is >> buf ) ) {
      break;
    }
    strcat( buf2,buf );
    // must check for eof before peek() or else peek() will
    // leave stream in fail state
    if ( is.eof() ) {
      // end of file means end of string
      break;
    }
    int p = is.peek();
    if ( p == EOF || isspace( p ) ) {
      // end of file or white space means end of string
      break;
    }
  }
  setString(buf2);
  return is;
}
*/
XbString XbString::getSubString(int startChar, int endChar) const {
  char *dupl = XbString::_strdup(data);
  char *newDup = &dupl[startChar];
  if (endChar != -1) newDup[endChar+1] = 0;
  XbString res;
  res.setString(newDup);
  EA3Free(dupl);
  return(res);
}

void XbString::deleteSubString(int startChar, int endChar) {
  char *dupl = XbString::_strdup(data);
  int j=0;
  for(int i=0;i<(int)strlen(data);i++) {
    if(i<startChar || (endChar != -1 && (i > endChar))) {
      dupl[j]=data[i];
      dupl[j+1]=0; 
      j++;
    }
  }
  setString(dupl);
  EA3Free(dupl);
}

void XbString::setSubString( const XbString& s, int begin, int end ) {
  XbString beginStr;
  if ( begin > 0 ) {
    beginStr = getSubString( 0, begin - 1 );
  }
  XbString endStr;
  if ( end >= 0 ) {
    endStr = getSubString( end + 1 );
  }
  *this = beginStr + s + endStr;
}

int XbString::index( char c ) const {
  const char *s = getString();
  const char *i = strchr( s, c );
  return i ? (int)(i - s) : -1;
}

int XbString::rindex( char c ) const {
  const char *s = getString();
  const char *i = strrchr( s, c );
  return i ? (int)(i - s) : -1;
}

XbString XbString::prefix( char sep ) const {
  int i = index( sep );
  return i < 0 || i == 0 ? XbString( "" ) : getSubString( 0, i - 1 );
}

XbString XbString::suffix( char sep ) const {
  int i = rindex( sep );
  return i < 0 || i == getLength() - 1 ?  XbString( "" ) : getSubString( i + 1 );
}

XbString XbString::removePrefix( char sep ) {
  XbString r;
  int i = index( sep );
  if ( i >= 0 ) {
    r = getSubString( 0, i );
    deleteSubString( 0, i );
  }
  return r;
}

XbString XbString::removeSuffix( char sep ) {
  XbString r;
  int i = rindex( sep );
  if ( i >= 0 ) {
    r = getSubString( i );
    deleteSubString( i );
  }
  return r;
}

void XbString::changePrefix( char sep, const XbString& s ) {
  removePrefix( sep );
  operator=( s + *this );
}

void XbString::changeSuffix( char sep, const XbString& s ) {
  removeSuffix( sep );
  operator+=( s );
}

XbString XbString::operator =(const char *str) {
  setString(str);
  return(*this);
}

XbString XbString::operator =(const XbString &str) {
  data = str.data;
  return(*this);
}
XbString XbString::operator =(const EA3FieldDataString &str) {
  setString((const char*)str);
  return(*this);
}


int operator ==(const XbString &str, const char *s) {
  if (!s) return(0);
  if(str.data == s) return (1);	// Quick check to see if the pointers themselves are equal
  if (str.data == XB_hashString.get(s)) return(1);
  return(0);
}

int operator <(const XbString &str1, const XbString &str2) {
  return(strcmp(str1.data,str2.data)<0);
}

int operator ==(const char *s, const XbString &str) {
  return(str == s); 
}
int operator ==(const XbString &str1, const XbString &str2) {
  return(str1.data == str2.data); 
}

XbString operator+(XbString str1, const XbString &str2) {
  str1 += str2;
  return(str1);
}

XbQString operator+(XbQString str1, const XbQString &str2) {
  str1 += str2;
  return(str1);
}

XbString & XbString::operator +=(const XbString &str) {
  return(operator+=(str.data));
}

XbString & XbString::operator +=(const char *str) {
  char *s = (char*)EA3Malloc(strlen(data)+strlen(str)+1);
  s[0] = 0;
  strcat(s,data);
  strcat(s,str);
  setString(s);
  EA3Free(s);
  //data = (char *)EA3Realloc(data,strlen(data)+strlen(str)+1);
  //strcat(data,str);
  return(*this);
}

int operator !=(const XbString &str, const char *s) {
  return(!(str == s));
}
int operator !=(const char *s, const XbString &str) {
  return(!(str == s));
}
int operator !=(const XbString &str1, const XbString &str2) {
  return(str1.data != str2.data);
}

bool regexpMatch(const XbString &str,char *pattern) {
  static char *lastStr = NULL;
  static regexp *re = NULL;
  XbString p(pattern);
  if (lastStr != p.getString()) {
    EA3Free(re);
    lastStr = const_cast<char *>(p.getString());
    re = regComp(lastStr);
  }
  return regExec(re,const_cast<char *>(str.getString()),const_cast<char *>(str.getString()))?true:false;
}

gzWriter& XbString::write(gzWriter& os) const {
  os << (const char *)data;
  return(os);
}

char *XbString::read( char *is ) {
  setString(is);
  return(is);
}

// XbIdentifier

XbIdentifier::XbIdentifier() {
  setString("_");
}

XbIdentifier::XbIdentifier(const char *str) {
  setString(str);
}

XbIdentifier::XbIdentifier(const XbString &str) {
  setString(str);
}
XbIdentifier::XbIdentifier(const EA3FieldDataString &str) {
  setString((const char*)str);
}

void XbIdentifier::setString(const XbIdentifier &str) {
  XbString::setString(str);
}

void XbIdentifier::setString(const XbString &str) {
  setString(str.getString());
}

void XbIdentifier::setString(const char *str)
{
	static char tmpdata[1000];
	const char *tmp = str;
	if (*tmp == 0)
	{
		XbString::setString("_");
	}
	else
	{
		int i=0;
		char c = *tmp;
		if (!isalpha_l(c) && c!='_' && c!='.')
		{
			XbString stmp = "_" + XbString(str);
			setString(stmp);
		}
		else
		{
			while(c)
			{
				if (isalnum_l(c) || c=='_' || c == '.')
				{
					tmpdata[i] = c;
				}
				else
				{
					tmpdata[i] = '_';
				}

				tmp++;
				c = *tmp;
				i++;
			}
			tmpdata[i] = 0;
			XbString::setString(tmpdata);
		}
	}
}

XbIdentifier XbIdentifier::operator =(const char *str) {
  setString(str);
  return(*this);
}
XbIdentifier XbIdentifier::operator =(const XbIdentifier &str) {
  XbString::setString(str);
  return(*this);
}
XbIdentifier XbIdentifier::operator =(const EA3FieldDataString &str) {
  XbString::operator =(str);
  return(*this);
}
/*
istream& XbIdentifier::read( istream& is ) {
  //assert("not implemented"=="");
  return(is);
}
*/
char *XbIdentifier::read( char *is ) {
  int i = 0;
  linebuf.setSize(1000);
  linebuf[0] = 0;
  char *c = is;
  if (!isalpha_l(*c) && *c != '_') {
    ERROR_LINE(EA3SYNTAX_ERROR);
    EA3_ERROR2("identifier must start with a letter or _ not %c",*c);
    return(NULL);
  }
  linebuf.setSize(i+1);
  linebuf[i] = *c;
  i++;
  c++;
  while (*c) {
    if (isalnum_l(*c) || *c == '_' || *c=='.') {
      linebuf.setSize(i+1);
      linebuf[i] = *c;
      i++;
      c++;
    } else {
      skipWhiteSpace(c);
      linebuf.setSize(i+1);
      linebuf[i] = 0;
      //printf("XbIdentifier::read %s\n",linebuf);
      XbString::setString(linebuf.buffer);
      return(c);
    }
  }
  linebuf.setSize(i+1);
  linebuf[i] = 0;
  XbString::setString(linebuf.buffer);
  return(c);
}

// XbQString //

XbQString::XbQString() : XbString() {
}

XbQString::XbQString(const char *str) {
  setString(str);
}

XbQString::XbQString(const XbString &str) {
  setString(str);
}
XbQString::XbQString(const EA3FieldDataString &str) {
  setString((const char*)str);
}

gzWriter& XbQString::write(gzWriter& os) const {
  int len = length();
  static char tmp[1000];
  char *foo = tmp;
  if (len > 490) {
    foo = (char *)EA3Malloc(len*2+4);
  }
  int i,j = 0;
  foo[j++] = '\"';
  for(i=0;i<len;i++) {
    if(getString()[i] == '\"') {
      foo[j++] = '\\';
      foo[j++] = '\"';
    } else if(getString()[i] == '\n') {
      foo[j++] = '\\';
      foo[j++] = 'n';
    } else if(getString()[i] == '\r') {
      foo[j++] = '\\';
      foo[j++] = 'r';
    } else if(getString()[i] == '\\') {
      foo[j++] = '\\';
      foo[j++] = '\\';
    } else {
      foo[j++] = getString()[i];
    }
  }
  foo[j++] = '\"';
  foo[j++] = 0;
  assert(j<len*2+4);
  os << (const char *)foo;
  if (foo != tmp) EA3Free(foo);
  return(os);
}
/*
istream& XbQString::read( istream& is ) {
  //assert("not implemented"=="");
  return(is);
}
*/
char *XbQString::read( char *is ) {
  linebuf.setSize(1000);
  linebuf[0] = 0;
  int i = 0;
  int escape = 0;
  char *ch = is;
  skipWhiteSpace(ch);
  if (*ch==0) return(ch);
  if (*ch != '"') {
    ERROR_LINE(EA3SYNTAX_ERROR);
    EA3_ERROR2("expected an opening \" but got:(%c)",*ch);
    return(ch);
  } else {
    ch++;
    while(*ch) {
      if (*ch == '\"' && !escape) break;
      if (*ch == '\\' && !escape) {
        escape = 1;
        ch++;
        continue;
      }
      if (escape) {
        if(*ch == 'n') *ch = '\n';
        if(*ch == 'r') *ch = '\r';
      }
      escape = 0;
      linebuf.setSize(i+1);
      linebuf[i++] = *ch;
      ch++;
    }
    if (*ch != '\"') {
      ERROR_LINE(EA3SYNTAX_ERROR);
      EA3_ERROR1("expected a closing \"");
      return(ch);
    }
    ch++;
    linebuf.setSize(i+1);
    linebuf[i++] = 0;
    XbString::setString(linebuf.buffer);
    //cerr << "string = " << (XbString)*this << endl;
  }
  return(ch);
}

// XbColour

XbColour::XbColour() {
  col = 0;
}

XbColour::XbColour(const unsigned char r,const unsigned char g,const unsigned char b,const unsigned char a) {
  col = a | ((int)b << 8) | ((int)g << 16) | ((int)r << 24);
}

XbColour::XbColour(const unsigned int c) {
  col = c;
}

char *XbColour::read( char *is ) {
  col = (unsigned int)strtoul(is,0,16);
  nextToken(is);
  return(is);
}

gzWriter& XbColour::write( gzWriter& os ) const {
  os << col;
  return(os);
}

/*
gzWriter& XbRefName::write( gzWriter& os ) const {
  if (name == "") {
    if (id == -1) {
      os << XbIdentifier(type);
    } else {
      os << "'" << type << " " << id << "'";
    }
  } else {
    if (id == -1) {
      os << "'" << type << " " << name << "'";
    } else {
      os << "'" << type << " " << name << " " << id << "'";
    }
  }
  return(os);
}
*/

char *XbRefName::read(char *is) {
  id = -1;
  childIndex = -1;
  name = XbHS_empty;
  type = XbHS_underScore;
  char *ch = is;
  skipWhiteSpace(ch);
  XbIdentifier _type;
  ch = _type.read(ch);
  if (!ch) return(NULL);
  type = const_cast<char *>(_type.getString());
  skipWhiteSpace(ch);
  if (*ch=='(') {
    ch++;
  } else {
    return(ch);
  }
  skipWhiteSpace(ch);
  if (*ch == '"') {
    XbQString qname;
    ch = qname.read(ch);
    setName(qname);
  }
  if (!ch) return(NULL);
  skipWhiteSpace(ch);
  if (*ch == ')') {
    ch++;
    return(ch);
  }
  if (*ch == ':') {
    ch++;
    id = atoi(ch);
    while(*ch && *ch != ')') ch++;
// errno
/*
    if (!(ch)) {
      EA3Scene::readError(cerr << "ERROR: parsing reference");
      return(is);
    }
*/
  }
  if (*ch == ')') {
    ch++;
    return(ch);
  } else {
    ERROR_LINE(EA3SYNTAX_ERROR);
    EA3_ERROR1("parsing ref name. expected a )");
  }
  return(ch);
}

// EA3BaseInfo //

EA3BaseInfo::EA3BaseInfo(EA3Block *con,EA3Base *g, const char* t,const char *n) : container(con), grammar(g), type(t), name(n) {
 refCount = 0;

}

void EA3BaseInfo::ref() {
  refCount++;
}
void EA3BaseInfo::unref() {
  if (refCount <= 0) {
    abort();
    //cerr << "ref count <= 0 already? refCount = " << refCount << endl;
  }
  refCount--;
  if(refCount <= 0) {
    gInfoList.last = NULL;
    gInfoList.numAdded--;
    EA3BaseInfoList::Hash::iterator i = gInfoList.list.find(this);
    if (i == gInfoList.list.end()) {
      abort();
    }
    gInfoList.list.erase(i);
    delete this;
  }
}

// EA3BaseInfoList //
EA3BaseInfoList::EA3BaseInfoList() {
  last = NULL;
  numAdded = 0;
  //list.resize(1000000);
} 

void EA3BaseInfoList::cleanUp() {
  int cnt;

  //
  //  Can't delete the EA3BaseInfo pointer while iterating through the
  //  hash table because it will screw up the iterator.
  //
  //  The hash table iterator requires the hash value to exists in order
  //  to find the next item.
  //
  XbPArray<EA3BaseInfo*> binfos;
  for(EA3BaseInfoList::Hash::iterator i = list.begin();i!=list.end();++i) {
    EA3BaseInfo *info = (*i);
    binfos.append(info);
  }

  for(cnt=0; cnt<binfos.getSize(); cnt++) {
    delete binfos[cnt];
  }

  binfos.setSize(0);
  list.clear();
}
EA3BaseInfoList::~EA3BaseInfoList() {
  cleanUp();
}

EA3BaseInfo *EA3BaseInfoList::get(EA3Block *c,EA3Base *g,const char *t,const char *n) {
  EA3BaseInfo *info;
  if (last) {
    if (last->container == c && last->grammar == g && last->name == n && last->type == t) {
      //printf("in EA3BaseInfoList %s %s\n",t,n);
      last->ref();
      return(last);
    }
  }
  EA3BaseInfo tmp(c,g,t,n);
  typedef EA3BaseInfoList::Hash::iterator SetIt;
  SetIt i;
  i = list.find(&tmp);
  
  if (i==list.end()) {
    info = new EA3BaseInfo(c,g,t,n);
    numAdded++;
    numInfos++;
    info->ref();
    list.insert(info);
    last = info;
  } else {
    info = (*i);
    info->ref();
    last = info;
  }
  
  /*
  int misses = 0;
  unsigned int uc = (unsigned int)c;
  unsigned int ug = (unsigned int)g;
  unsigned int hashVal = (uc/2+ug/2) & 0xffff;
  XbPArray<EA3BaseInfo *> &list2 = *(list[hashVal]);
  for(i=0;i<list2.getSize();i++) {
    info = list2[i];
    if (info == NULL) {
      numAdded++;
      //printf("hash misses %d size %d\n",misses,n);
      list2[i] = last = info = new EA3BaseInfo(c,g,t,n,hashVal,i);
      info->ref();
      return(info);
    }
    if (info->container == c && info->grammar == g && info->name == n && info->type == t) {
      //printf("hash misses %d\n",misses);
      last = info;
      info->ref();
      //printf("in EA3BaseInfoList %s %s\n",t.getString(),n.getString());
      return(info);
    }
    misses++;
  }
  numAdded++;
  //printf("hash misses %d size %d\n",misses,numAdded);
  info = new EA3BaseInfo(c,g,t,n,hashVal,list2.getSize());
  info->ref();
  list2.append(info);
  //printf("not in EA3BaseInfoList(%d)  %s %s\n",list2.getSize(),t.getString(),n.getString());
  last = info;
  */
  return(info);
}

void EA3BaseInfoList::setGrammar(EA3Base *b, EA3Base *g) {
  EA3BaseInfo *_last = b->info;
  b->info = get(b->getContainer(),g,b->info->type,b->info->name);
  _last->unref();
}
void EA3BaseInfoList::setName(EA3Base *b, const XbString &nm) {
  EA3BaseInfo *_last = b->info;
  b->info = get(b->getContainer(),b->info->grammar,b->info->type,nm.getString());
  _last->unref();
}
void EA3BaseInfoList::setContainer(EA3Base *b, EA3Block *con) {
  setContainerCount++;
  EA3BaseInfo *_last = b->info;
  if (!con) {
    b->info = get(NULL,b->getGrammar(),b->info->type,b->info->name);
    _last->unref();
    return;
  }
  EA3Base *oldGram = b->getGrammar();
  EA3Block *g = (EA3Block *)con->getGrammar();
  EA3Base *base=g->getByName(b->getType());
  if (base) {
    b->info = get(con,base,b->info->type,b->info->name);
  } else {
    b->info = get(con,oldGram,b->info->type,b->info->name);
  }
  _last->unref();
}

// EA3Base //

int EA3Base::id = 0;
intptr_t EA3Base::firstBase = 0;

EA3Base::EA3Base(EA3Block *con, EA3Grammar *g,int ind,const XbIdentifier &t, const XbString &n) {
  if (firstBase==0) firstBase = (intptr_t)this;
  numBases++;
  flags.refCount = 0;
  info = gInfoList.get(con,g,t.getString(),n.getString());
  //index = ind;
  id++;
  //myId = id;
  //internalPtr = NULL;
  //userData = NULL;
  flags.touched = 1;
  flags.binaryDataField = 0;
  flags.internalFieldDataAlloc = 0;
  flags.num = 0;
}

EA3Base::~EA3Base() {
  if (info) {
    if (!deletingGrammar && !thisIsTheScene) {
      EA3Scene *s=getScene();
      EA3BaseSet &bs = *(EA3BaseSet *)s->uncontained;
      bs.erase(this);
    }
    if (deletingGrammar) {
      GrammarTableMap::iterator gti = gtMap.find(this);
      if (gti != gtMap.end()) {
        delete (*gti).second;
        gtMap.erase(gti);
      }
    }
    info->unref();
    if (deleteCB) {
      (*deleteCB)(this);
    }
    numBases--;
  }
  EA3SetSelect(this,0);
  setUserData(0);
  if (thisIsTheScene) thisIsTheScene = false;
}

void EA3Base::touch() {
  flags.touched = true;
}
void EA3Base::untouch() {
  flags.touched = false;
}
bool EA3Base::isTouched() {
  return(flags.touched);
}

void EA3Base::setUserData(void *ptr) {
  if (ptr == 0) {
    userDataMap.erase(this);
  } else {
    userDataMap[this] = ptr;
  }
  //userData = ptr;
}

void *EA3Base::getUserData() const {
  hash_map<const EA3Base *,void *,EA3BaseHash,EA3BaseEqual, StlAllocator >::iterator it = userDataMap.find(this);
  if (it==userDataMap.end()) return(NULL);
  return (*it).second;
}

intptr_t EA3Base::getUniqueId() const {
  return (intptr_t)this;
}

void EA3Base::clearRefId() {
  setRefId(0);
}

EA3Base *EA3Base::getGrammar() const {
  return(info->grammar);
}

void EA3Base::setRefId(int _id) {
  refIDmap[this] = _id;
}

int EA3Base::getRefId() const {
  return(refIDmap[this]);
  //return (int)internalPtr;
}

void EA3Base::setClone(EA3Base *c) const {
  static bool first = 1;
  if (first) {
      //cloneMap.resize(1000000);
  }
  cloneMap[this] = c;
  //internalPtr = c;
}

inline EA3Base *EA3Base::getClone() const {
  return(cloneMap[this]);
}




int EA3Base::isGrammar() const {
  if (!isBlock()) return(0);
  return(getContainer()->isGrammar());
}
EA3Grammar *EA3Base::toGrammar() const {
  if (isGrammar()) return const_cast<EA3Grammar *>(static_cast<const EA3Grammar *>(this));
  return(NULL);
}

void EA3Base::setDeleteCB(EA3Base::EA3BaseDeleteCB *cb) {
  deleteCB = cb;
}

EA3Base::EA3BaseDeleteCB *EA3Base::getDeleteCB() {
  return(deleteCB);
}

void EA3Base::setName(const XbString &n) {
  touch();
  gInfoList.setName(this,n);
}

XbString EA3Base::getName() const {
  static XbString s;
  if (!info) return("");
  s.data = const_cast<char *>(info->name);
  return(s);
}

EA3Status EA3Base::resolveReferences() {
  static int _counter = 0;
  if (EA3Reference *r = toReference()) {
    //if (!r->resolveRefNames()) return(0);
    r->resolveRefNames();
    _counter++;
    if (EA3Scene::progressCallback && (_counter > 100000 || EA3Scene::numResReferences==EA3Scene::numReferences)) {
      _counter = 0;
      (*EA3Scene::progressCallback)(2,EA3Scene::numResReferences,EA3Scene::numReferences);
    }
  }
  if (EA3Scene::numResReferences==EA3Scene::numReferences) return(EA3OK);
  if (EA3Block *g = toBlock()) {
    EA3ConstIterator it = g->beginConst();
	EA3Base** end = it.end();
	for(;it!=end;++it){
      EA3Base *b = *it;
      //if (!b->resolveReferences()) return(0);
      b->resolveReferences();
    }
  }
  return(EA3OK);
}

void EA3Base::clearReferences() {
  refIDmap.clear();
  /*
  int i;
  clearRefId();
  if (EA3Reference *r = toReference()) {
    r->clearRefNames();
  }
  if (EA3Block *g = toBlock()) {
    for(i=0;i<g->getNumMembers();i++) {
      EA3Base *b = g->getByIndex(i);
      b->clearReferences();
    }
  }
  return;
  */
}

EA3Base *EA3Base::findByRefName(XbRefName *refName,const EA3Base *exclude) const {
  return NULL;
  /*
  counter++;
  //cerr << "RN:" << refName.getType() << endl;
  int i;
  //if (refName.getType() == "GLOBAL") return(getScene());
  if (getRefId() == refName->id) {
    EA3Reference::LRU.add(this);
    return(const_cast<EA3Base *>(this));
  }
  //printf("LRU did not find\n");
  if (EA3Block *g = toBlock()) {
    if (g->isArray()) {
// if not an array of blocks, then array's member's type == refName type
      EA3Base *b = g->getByIndex(0);
      EA3Block *tmp = b? b->toBlock() : NULL;
      if (!tmp && b && b->getType() != refName->getType()) {
        //cerr << "skipping array " << g->getType() << endl;
        return(0);
      }
    }
    for(i=0;i<g->getNumMembers();i++) {
      EA3Base *b = g->getByIndex(i);
      if (b==exclude) continue;
      if (EA3Reference *findR = b->toReference()) {
        continue;
      }
      if(EA3Base *find = b->findByRefName(refName,0)) {
        return(find);
      }
    }
  }
  return(NULL);
  */
}

//
// When an EA3Reference is copied, there will be two references to the same
// EA3Base pointer. If the object that they are referring to is being copied
// as well, the new reference will still be pointing to the old object. This
// method changes the references to point to the new objects.
//
void EA3Base::resolveCopyReferences() {
  int i;
  int num;
  //resolveClonePtrCount++;
  if (EA3Reference *r = toReference()) {
    num = r->getNumReferences();
	EA3Iterator it = r->begin();
	//EA3ConstIterator it = r->beginConst();
    for(i=0;i<num;i++,++it) {
      EA3Base *b = *it;

      // the new, copied object is stored as a clone pointer
      EA3Base *c = b->getClone();

      if (c) {
        //b->ref();
        r->setReference(i,c);
      } else {
        //assert(0);
      }
    } 
  }
  if (EA3Block *g = toBlock()) {
    //printf("%x %s %s %d\n",g,g->getType().getString(),g->getName().getString(),num);
	EA3ConstIterator it = g->beginConst();
	EA3Base** end = it.end();
    for(;it!=end;++it) {
      EA3Base *b = *it;
      if (b->isField()) continue;
      b->resolveCopyReferences();
    }
  }
}

void EA3Base::nullifyClonePointers() const {
  cloneMap.clear();
  /*
  internalPtr = NULL;
  int i;
  if (EA3Block *g = toBlock()) {
    for(i=0;i<g->getNumMembers();i++) {
      EA3Base *b = g->getByIndex(i);
      b->nullifyClonePointers();
    }
  }
  */
}

EA3BaseInfo *EA3Base::getInfo() const {
  return(info);
}


EA3Scene *EA3Base::getScene() const {
  EA3Scene *sc = info->getScene();
  if (sc) return(sc);
  return ((EA3Scene*)const_cast<EA3Base *>(this));
}

int EA3Base::typeIs(const char *dtype) const {
  if (dtype == NULL) return(1);
  if (strcmp(getClassType(),dtype)==0) return(1);
  return(0);
}

//int EA3Base::hasRefName() const {
  //return(1);
/*
  const EA3Array *a = dynamic_cast<const EA3Array *>(this);
  int rc = refCount;
  if (rc == 1 && a) {
    int i;
    for(i=0;i<a->getNumMembers();i++) {
      //if (a->getByIndex(i)->refCount > rc) {rc = 2; break;}
    }
  }
  if ((rc == 1) || hasUniqueTypeAndName() || (container != NULL && container->isArray())) {
    if (getName() == "") {
      return(0);
    }
  }
  return(1);
*/
//}

gzWriter& EA3Base::writeRefName(gzWriter& os) const {
  static XbString noname("");
  int needsID = 1;
  if (isField() && getContainer()->isArray()) needsID = 0;
  if (flags.refCount < 2) needsID = 0;
  if (isArray()) needsID = 1;
  if (getName() == noname) {
    if (needsID) {
      return(os << "(:" << getWriteId(this) << ")");
    } else {
      return(os);
    }
  } else {
    if (needsID) {
      //const XbString &name = getName();
      return(os << "(" << XbQString(getName()) << ":" << getWriteId(this) << ")");
    } else {
      return(os << "(" << XbQString(getName()) << ")");
    }
  }
}

XbIdentifier EA3Base::getType() const {
  static XbIdentifier i;
  if (!info) return("_");
  i.data = const_cast<char *>(info->type);
  return(i);
}

char *getIndent(const EA3Base *b) {
  static char indent[100];
  EA3Block *bl = b->getContainer();
  int i=0;
  while(bl) {
    indent[i]='\t';
     bl = bl->getContainer();
     i++;
  }
  indent[i] = 0;
  return(indent);
}

gzWriter& EA3Base::writeIndent(gzWriter& os) const {
  static EA3Block *lastContainer = NULL;
  static char *lastIndent;
  if (getContainer()==lastContainer) {
    os << lastIndent;
    return(os);
  }
  lastIndent = getIndent(this);
  lastContainer = getContainer();
  os << lastIndent;
  return(os);
}

EA3Block *EA3Field::getContainer() const {
  return(info->container);
}
EA3Block *EA3Reference::getContainer() const {
  return(container);
}
EA3Block *EA3Block::getContainer() const {
  return(container);
}

EA3Pointer EA3Block::getByIndex(int ix) const {
  //if (ix >= getNumMembers()) return(NULL);
  if (flags.num == 0) return NULL;
  if (flags.num == 1) {
    if (ix == 0) {
      return (EA3Base *)members;
    } 
    return NULL;
  }
  if (flags.num < 127) {
    if (ix < (int)flags.num) {
      return (EA3Base *)members[ix];
    } else {
      return NULL;
    }
  }
  intptr_t size = (intptr_t)members[0];
  if (ix < size) {
    return (EA3Base *)members[ix+1];
  }
  return NULL;
}
void EA3Block::sortMembers( int (*keyCompare)( EA3Base **, EA3Base ** ) ) {
  if (flags.num <= 1) return;
  int offset = 0;
  intptr_t size = flags.num;
  if (flags.num == 127) {
    offset = 1;
    size = (intptr_t)members[0];
  }
  qsort(
    (void *)&members[offset],
    size,
    sizeof( EA3Base* ),
    (int (*)(const void *,const void *))keyCompare );
  reindex();
}

int EA3Field::setContainer(EA3Block *con) {
  touch();
  EA3Block *prevCon = getContainer();
  if (con == prevCon) return(1);
  if (con == NULL) {
    EA3Scene *s=getScene();
    EA3BaseSet &bs = *(EA3BaseSet *)s->uncontained;
    bs.insert(this);
    gInfoList.setContainer(this,NULL);
    return(1);
  }
  if (prevCon == NULL) {
    EA3Scene *s=getScene();
    EA3BaseSet &bs = *(EA3BaseSet *)s->uncontained;
    bs.erase(this);
  }
  gInfoList.setContainer(this,con);
  return(1);
}
EA3Pointer EA3Reference::getReference(int ix) const {
  if (flags.num == 0) return NULL;
  if (flags.num == 1) {
    if (ix == 0) {
      return (EA3Base *)members;
    } 
    return NULL;
  }
  if (flags.num < 127) {
    if (ix < (int)flags.num) {
      return (EA3Base *)members[ix];
    } else {
      return NULL;
    }
  }
  intptr_t size = (intptr_t)members[0];
  if (ix < size) {
    return (EA3Base *)members[ix+1];
  }
  return NULL;
}

EA3Iterator EA3Reference::begin() const 
{
	if (flags.num == 0) return EA3Iterator((EA3Base **)&members, (EA3Base **)(&members) );
	if (flags.num == 1) return EA3Iterator( (EA3Base **)&members, (EA3Base **)(&members + 1) );
	if (flags.num < 127) return EA3Iterator( (EA3Base **)members, (EA3Base **)(members + (int)flags.num) );

	// more than 128 members
	intptr_t size = (intptr_t)members[0];
	return EA3Iterator( (EA3Base**)(members + 1), (EA3Base**) (members + size + 1));
}

EA3ConstIterator EA3Reference::beginConst() const 
{
	if (flags.num == 0) return EA3ConstIterator((EA3Base **)&members, (EA3Base **)(&members) );
	if (flags.num == 1) return EA3ConstIterator( (EA3Base **)&members, (EA3Base **)(&members + 1) );
	if (flags.num < 127) return EA3ConstIterator( (EA3Base **)members, (EA3Base **)(members + (int)flags.num) );

	// more than 128 members
	intptr_t size = (intptr_t)members[0];
	return EA3ConstIterator( (EA3Base**)(members + 1), (EA3Base**) (members + size + 1));
}

int EA3Reference::getNumReferences() const {
  return (int)getMemberSize(flags,members);
}
int EA3Reference::setContainer(EA3Block *con) {
  // Caller is responsible for removing member from container
  touch();
  EA3Block *prevCon = getContainer();
  if (con == prevCon) return(1);
  if (con == NULL) {
    EA3Scene *s=getScene();
    EA3BaseSet &bs = *(EA3BaseSet *)s->uncontained;
    bs.insert(this);
    container = NULL;
    return(1);
  }
  if (prevCon == NULL) {
    EA3Scene *s=getScene();
    EA3BaseSet &bs = *(EA3BaseSet *)s->uncontained;
    bs.erase(this);
  }
  ref();
  //getContainer()->removeMember(this);
  unrefNoDelete();
  container = con;
  return(1);
}
int EA3Block::setContainer(EA3Block *con) {
  // Caller is responsible for removing member from container
  touch();
  EA3Block *prevCon = getContainer();
  if (con == prevCon) return(1);
  if (con == NULL) {
    EA3Scene *s=getScene();
    EA3BaseSet &bs = *(EA3BaseSet *)s->uncontained;
    bs.insert(this);
    container = NULL;
    return(1);
  }
  if (prevCon == NULL) {
    EA3Scene *s=getScene();
    EA3BaseSet &bs = *(EA3BaseSet *)s->uncontained;
    bs.erase(this);
  }
  ref();
  //getContainer()->removeMember(this);
  unrefNoDelete();
  container = con;
  return(1);
}

void EA3Base::setIndex(int i) {
  //touch();
  //index = i;
}

int getIndex(EA3Block *con,const EA3Base *b) {
  static hash_map<const EA3Base *,int,EA3BaseHash,EA3BaseEqual, StlAllocator > indexMap[6];
  static EA3Block *list[6] = {0,0,0,0,0,0};
  static int ix = 0;
  int i,j;
  int num = con->getNumMembers();
  EA3Field *fld = b->toField();
  if (fld) {
    if (!con->flags.needsReindex) {
      return fld->index;
    }
  }
  if (num<20) {
    if (num == 1) return(0);
    if (fld && con->flags.needsReindex) {
      for(i=0;i<num;i++) {
        EA3Field *f = con->members[i]->toField();
        if (f) f->index = i;
      }
      con->flags.needsReindex = false;
      return(fld->index);
    }
    for(i=0;i<num;i++) {
      if (con->members[i]==b) {
        return(i);
      }
    }
  }
  for(i=0;i<6;i++) {
    if(list[ix]==con) {
      if (con->flags.needsReindex) {
        break;
      }
      return(indexMap[ix][b]);
    }
    ix = (ix+1)%6;
  }
  if (list[ix]!=con) ix = (ix+1)%6;
  bool onlyFields = true;
  for(j=0;j<num;j++) {
    EA3Base *base=con->getByIndex(j);
    if (base->isField()) {
      base->toField()->index = j;
      continue;
    }
    if (onlyFields) {
      list[ix] = con;
      indexMap[ix].clear();
      //indexMap[ix].resize(num*2);
      onlyFields = false;
    }
    indexMap[ix][base] = j;
  }
  con->flags.needsReindex = 0;
  if (fld) return fld->index;
  return indexMap[ix][b];
}
int EA3Base::getIndex() const {
  EA3Block *con = getContainer();
  if (!con) return(-1);
  return (::getIndex(con,this));
  //if (getContainer()) getContainer()->real_reindex();
  //return(index);
}

int EA3Base::ref() {
  flags.refCount++;
  return(flags.refCount);
}

int EA3Base::unref() {
  if (doingSceneDelete) {
    flags.refCount = 1;
  }
  //printf("unref %s %s %d\n",getName().getString(),getType().getString(),refCount);
  if (flags.refCount <= 0) {
    //cerr << "ref count <= 0 already? refCount = " << refCount << "name " << getName() << " type " << getType() << endl;
  }
  flags.refCount--;
  if (flags.refCount <= 0) {
    //cerr << "deleting " << type << " " << name << " " << id << endl;
    delete this;
    return(1);
  }
  return(0);
}

int EA3Base::unrefNoDelete() {
  flags.refCount--;
  return(flags.refCount);
}

// EA3Field //

static CustomHeap<sizeof(EA3FieldData)*3> customFieldDataMem;

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4355)
#endif // 'this' : used in base member initializer list

EA3Field::EA3Field(const XbIdentifier &dt,int num,EA3Block *con, EA3Grammar *g,int ind, const XbIdentifier &t, const XbString &nm) : EA3Base(con,g?g:(EA3Grammar *)this,ind,t,nm) {
  //num = (unsigned char)nb;
  const char *dataType = dt.getString();
  if (num <= 3) {
    flags.internalFieldDataAlloc = true;
    data = (EA3FieldData*)customFieldDataMem.malloc();
  } else {
    data = (EA3FieldData*)EA3Malloc(sizeof(EA3FieldData)*num);
  }
  for(int i=0;i<num;i++) {
    data[i].i = 0;
  }
  if (dataType[0]=='b') flags.binaryDataField=1;
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif // 'this' : used in base member initializer list

EA3Field::~EA3Field() {
  if (flags.fieldHasExtraData) {
    extraFieldTypesMap.erase(this);
  }
  if (flags.binaryDataField) {
    EA3Free(data[2].bindata);
  }
  if (flags.internalFieldDataAlloc) {
    customFieldDataMem.free(data);
  } else {
    EA3Free(data);
  }
  
}
inline char *readExtraTypes(EA3Field *f, char *is) {
  skipWhiteSpace(is);
  if (!(*is)) return(is);
  if (*is == '#') return(is);
  int i = 0;
  char tmp[1000];
  while(*is) {
    if (*is == ' ') break;
    if (*is == 'f' || *is == 'i' || *is == 'd' || *is == 's' || *is == 'n' || *is == 'c') {
      tmp[i] = *is;
      i++;
    } else {
      return(is);
    }
    is++;
  }
  skipWhiteSpace(is);
  tmp[i] = 0;
  const char *etypes = tmp;
  if (f->setExtraTypes(etypes)) {
    unsigned int num = f->getNum();
    unsigned int exnum = (unsigned int)strlen(etypes);
    EA3FieldData *data = f->data;
    XbIdentifier id;
    XbQString str;
    XbColour rgba;
    for(unsigned int j=0;j<exnum;j++) {
      switch(etypes[j]) {
        case 'f': { string2float(is,data[num+j].f); break; }
        case 'i': { string2int(is,data[num+j].i); break; }
        case 'n':
        case 'd': { string2XbIdentifier(is,id); data[num+j].str = const_cast<char*>(id.getString()); break;} 
        case 's': { string2XbQString(is,str); data[num+j].str = const_cast<char*>(str.getString()); break;} 
        case 'c': { string2XbColour(is,rgba); data[num+j].ui = rgba.getColor(); break;}
      }
      skipWhiteSpace(is);
    }
  }
  return(is);
}
char *EA3Field::read(char *is) {
  int i;
  skipWhiteSpace(is);
  int num=getNum();
  XbIdentifier _id;
  XbQString str;
  XbColour rgba;
  const char *dt = getDataType();
  for(i=0;i<num;i++) {
    switch(dt[i]) {
      case 'f': { string2float(is,data[i].f); break; }
      case 'i': { string2int(is,data[i].i); break; }
      case 'n':
      case 'd': { string2XbIdentifier(is,_id); data[i].str = const_cast<char*>(_id.getString()); break;} 
      case 's': { string2XbQString(is,str); data[i].str = const_cast<char*>(str.getString()); break;} 
      case 'c': { 
        string2XbColour(is,rgba); data[i].ui = rgba.getColor(); break;
      }
      case 'b': { 
        string2int(is,data[i].i); // num
        string2int(is,data[i+1].i); // endianness
        data[i+2].bindata = (char *)EA3Malloc(data[i].i);
        for(int j=0;j<data[i].i;j++) {
          char c[3];
          int k = 0;
          for(k = 0;k<2;k++) {
            c[k] = *is;
            if (isxdigit(c[k])) {
            } else {
              is = getScene()->getLine();
              c[k] = *is;
              if (isxdigit(c[k])) {
              } else {
                ERROR_LINE(EA3SYNTAX_ERROR);
                break;
              }
            }
            is++;
          }
          if (k != 2) break;
          c[2] = 0;
          unsigned int C;
          sscanf(c,"%x",&C);
          data[i+2].bindata[j] = (unsigned char)C;
        }
        i = num;
        break;
      }
      default: { 
        ERROR_LINE(EA3SYNTAX_ERROR);
        EA3_ERROR2("unknown field data type %c\n",dt[i]);
        return(NULL);
      }
    }
  }
  is = readExtraTypes(this,is);
  return(is);
}

void EA3Field::set(const char *str) {
  EA3Field::read(const_cast<char *>(str));
}

char *EA3Field::get(int ix) const {
  gzWriter os;
  write1(os,ix);
  os << (char)0;
  char *buf = XbString::_strdup(os.str());
  return(buf);
}

EA3Base *EA3Field::copy(const EA3Block *, EA3Block *newcontainer) const {
  EA3Field *c = newcontainer->createField(getType(),getName());

  if(!c) return c;
  if (EA3IsSelected(this)) {
    EA3SetSelect(c,1);
  }
  if (refCount()>1) setClone(c);
  //c->data = new EA3FieldData[num];
  unsigned int num=getNum();
  unsigned int exnum = getNumExtraTypes();
  if (exnum) c->setExtraTypes(getExtraTypes());
  if (getDataType(0)=='b') {
    c->data[0].ui = data[0].ui;
    c->data[1].ui = data[1].ui;
    c->data[2].bindata = (char*)EA3Malloc(data[0].ui);
    memcpy(c->data[2].bindata,data[2].bindata,data[0].ui);
  } else {
    memcpy(c->data,data,sizeof(EA3FieldData)*(num+exnum));
  }
  return(c);
}
unsigned int EA3Field::getNum() const {
  const char *dt = getDataType();
  if (dt[0] == 'b') return(3);
  return((unsigned int)strlen(dt));
}
const char *EA3Field::getDataType() const {
  const char *ext = strrchr(getType().getString(),'.');
  if (ext) {
    ext++;
    return(ext);
  }
  const char *gdt = getGrammar()->getType().getString();
  return gdt;
}

char EA3Field::getDataType(int i) const {
  const char *dataType = getDataType();
  return(dataType[i]);
}

const char *EA3Field::getExtraTypes() const {
  if (!flags.fieldHasExtraData) return(NULL);
  const char *types = extraFieldTypesMap[this];
  if (!types)
    return(NULL);
  return(types);
}
bool EA3Field::setExtraTypes(const char *types) {
  int i;
  unsigned int num = getNum();
  if (getExtraTypes()!=NULL) {
    EA3FieldData *ndata = (EA3FieldData*)EA3Malloc(num*sizeof(EA3FieldData));
    memcpy(ndata,data,num*sizeof(EA3FieldData));
    if (flags.internalFieldDataAlloc) {
      customFieldDataMem.free(data);
      flags.internalFieldDataAlloc = false;
    } else {
      EA3Free(data);
    }
    data = ndata;
  }
  int len = (int)strlen(types);
  if (len == 0) {
    flags.fieldHasExtraData = false;
    extraFieldTypesMap.erase(this);
    return(true);
  }
  if (getDataType(0)=='b') return(false);
  for(i=len-1;i>=0;--i) {
    if (types[i] == 'f') continue;
    if (types[i] == 'i') continue;
    if (types[i] == 'c') continue;
    if (types[i] == 's') continue;
    if (types[i] == 'n') continue;
    if (types[i] == 'd') continue;
    return(false);
  }
  XbString s(types);
  EA3FieldData *ndata = (EA3FieldData*)EA3Malloc((num+len)*sizeof(EA3FieldData));
  memcpy(ndata,data,num*sizeof(EA3FieldData));
  if (flags.internalFieldDataAlloc) {
    customFieldDataMem.free(data);
    flags.internalFieldDataAlloc = false;
  } else {
    EA3Free(data);
  }
  data = ndata;
  for(i=len-1;i>=0;--i) {
    data[num+i].i = 0;
  }
  flags.fieldHasExtraData = true;
  extraFieldTypesMap[this] = s.getString();
  return(true);
}

bool EA3Field::appendExtraTypes(const char *types) {
  int i;
  unsigned int num = getNum() + getNumExtraTypes();
  int len = (int)strlen(types);

  if (!((len) || (getNumExtraTypes()))) {
    flags.fieldHasExtraData = false;
    extraFieldTypesMap.erase(this);
    return(true);
  }

  if (getDataType(0)=='b') return(false);

  for(i=len-1;i>=0;--i) {
    if (types[i] == 'f') continue;
    if (types[i] == 'i') continue;
    if (types[i] == 'u') continue;
    if (types[i] == 's') continue;
    if (types[i] == 'n') continue;
    if (types[i] == 'd') continue;
    len=0;
  }

	if (len) {
		EA3FieldData *ndata = (EA3FieldData*)EA3Malloc((num+len)*sizeof(EA3FieldData));

		for(i=len-1;i>=0;--i) {
			ndata[num+i].i = 0;
		}

		memcpy(ndata,data,num*sizeof(EA3FieldData));
		if (flags.internalFieldDataAlloc) {
			customFieldDataMem.free(data);
			flags.internalFieldDataAlloc = false;
		} else {
			EA3Free(data);
		}

		data = ndata;
    flags.internalFieldDataAlloc = false;
		flags.fieldHasExtraData = true;

		XbString s( (getExtraTypes()?getExtraTypes():"") );
		s += types;
		extraFieldTypesMap[this] = s.getString();

		return true;
	}

  return(false);
}


unsigned int EA3Field::getNumExtraTypes() const {
  if (!flags.fieldHasExtraData) return(0);
  const char *types = extraFieldTypesMap[this];
  if (!types) return(0);
  return((unsigned int)strlen(types));
}

gzWriter &EA3Field::write1(gzWriter& os,int i) const {
  const char *dt = getDataType();
  switch(dt[i]) {
  case 'f': { os << data[i].f; break;}
  case 'i': { os << data[i].i; break; }
  case 'n':
  case 'd': { os << XbIdentifier(data[i].str); break;}
  case 's': { os << XbQString(data[i].str); break;}
  case 'c': { os << data[i].ui; break;}
  case 'b': { 
    os << data[i].i << " " << data[i+1].i << " # Binary data follows"; 
    for (int j=0;j<data[i].i;j++) {
      unsigned char c = data[i+2].str[j];
      char buf[3];
      sprintf(buf,"%.2x",c);
      if (j % 32 == 0) os.append("\n");
      buf[2] = 0;
      os.append(buf);
    }
    os << "\n# End of binary data.";
    break;
            }
  default: { 
    ERROR_LINE(EA3SYNTAX_ERROR);
    EA3_ERROR2("unknown field data type %c\n",dt[i]);
    //assert("unknown field data"=="");
           }
  }
  return(os);
}

gzWriter &EA3Field::write(gzWriter& os) const {
  int i;
  writeIndent(os);
  os << getType();
  //if (hasRefName()) {
    writeRefName(os);
  //}
  int num=getNum();
  if (num && getDataType(0)=='b') {
    os << " "; write1(os,0);
  } else {
    for(i=0;i<num;i++) {
      os << " ";
      write1(os,i);
    }
  }
  const char *et = getExtraTypes();
  if (et) {
    os << " " << et;
    int exnum = (int)strlen(et);
    for(i=0;i<exnum;i++) {
      os << " ";
      switch(et[i]) {
        case 'f': { os << data[num+i].f; break;}
        case 'i': { os << data[num+i].i; break; }
        case 'n':
        case 'd': { os << XbIdentifier(data[num+i].str); break;}
        case 's': { os << XbQString(data[num+i].str); break;}
        case 'c': { os << data[num+i].ui; break;}
      }
    }
  }
  if (ea3asciiVerbose > 1) {
    int rc = refCount();
    if (getContainer()->isArray()) {
      if (rc > 1) {
        os << "\t# (" << refCount()-1 << ")\t" << getIndex();
      } else {
        os << "\t# " << getIndex();
      }
    } else {
      if (rc > 1) {
        os << "\t# (" << refCount()-1 << ")";
      }
    }
  }
  os << '\n';
  return(os);
}

EA3Status EA3Field::write1Binary(EA3Chunk *chunk,int i) const {
  const char *dt = getDataType();
  switch(dt[i]) {
  case 'f': 
  case 'i':
  case 'c': chunk->append(data[i].i); break;
  case 'n':
  case 'd': 
  case 's': chunk->append(chunk->stringTable.append(data[i].str)); break;
  case 'b': {
    chunk->append(data[0].i);
    chunk->append(data[1].i);
    for(int j=0;j<data[0].i;j++) {
      chunk->append((unsigned char)data[2].bindata[j]);
    }
    break;
            }
  default: { 
    ERROR_LINE(EA3SYNTAX_ERROR);
    EA3_ERROR2("unknown field data type %c\n",dt[i]);
    return(EA3UNKNOWN_TYPE);
           }
  }
  return(EA3OK);
}

EA3Status EA3Field::writeBinary(EA3Chunk *chunk) const {
// Caller resposible for header!!
  unsigned int num = getNum();
  if (getDataType(0)=='b') {
    write1Binary(chunk,0);
  } else {
    for(unsigned int i=0;i<num;i++) {
      write1Binary(chunk,i);
    }
  }

  return(EA3OK);
}
 
EA3Status EA3Field::readBinary(EA3Chunk *chunk, int) {
  unsigned int num=getNum();
  const char *dt = getDataType();
  for(unsigned int i=0;i<num;i++) {
    switch(dt[i]) {
    case 'f': 
    case 'i':
    case 'c': data[i].i = chunk->getInt(); break;
    case 'n':
    case 'd': 
    case 's': data[i].str = (chunk->stringTable.get(chunk->getInt()).getString()); break;
    case 'b': {
      data[0].i = chunk->getInt();
      data[1].i = chunk->getInt();
      data[2].bindata = (char*)EA3Malloc(data[0].i);
      for(int j=0;j<data[0].i;j++) {
        data[2].bindata[j] = chunk->getUChar();
      }
      i = num;
      break;
              }
    default: {
      ERROR_LINE(EA3SYNTAX_ERROR);
      EA3_ERROR2("unknown field data type %c\n",dt[i]);
      return(EA3UNKNOWN_TYPE);
             }
    }
  }
  return(EA3OK);
}


// EA3Reference //

EA3Reference::EA3Reference(EA3Block *con, EA3Grammar *g,int ind, const XbIdentifier &t, const XbString &nm) : EA3Base(NULL,g,ind,t,nm), container(con) {
}
EA3Reference::~EA3Reference() {
  if (doingSceneDelete) {
    setMemberSize(flags,members,0);
    return;
  }

  {
	EA3ConstIterator it = beginConst();
	EA3Base** end = it.end();
	for(;it!=end;++it) {
		(*it)->unref();
	}
  }

  setMemberSize(flags,members,0);
}

EA3Base *EA3Reference::copy(const EA3Block *, EA3Block *newcontainer) const {
  EA3Reference *c = newcontainer->createReference(getType(),getName());
  if (refCount()>1) setClone(c);
  if (EA3IsSelected(this)) {
    EA3SetSelect(c,1);
  }
  int size = getNumReferences();
  setMemberSize(c->flags,c->members,size);
  
  EA3ConstIterator it = beginConst();
  EA3Base** end = it.end();
  for( int d = 0; it!=end; d++, ++it )
  {
    EA3Base *base = *it;
    setMember(c->flags,c->members,base,d);
    base->ref();
  }
  return(c);
}

EA3Base *EA3Reference::operator[](int ix) {
  return(getReference(ix));
}

int EA3Reference::find(const EA3Base *b, XbPArray<int> &outlist) const {
  int _count = 0;
  //int order = container->getOrder();
  EA3ConstIterator it = beginConst();
  EA3Base** end = it.end();
  for(int i = 0;it!=end;++it, ++i) {
    if (*it == b) {
      outlist.append(i);
      _count++;
    } else {
    }
  }
/*
  if (last) {
    for(int i=data.getSize();i<order;i++) {
      list.append(i);
      _count++;
    }
  }
*/
  return(_count);
}

int EA3Reference::replaceReference(EA3Base *o, EA3Base *n) {
  touch();
  XbPArray<int> list;
  find(o,list);
  int i;
  int _count = 0;
  for(i=0;i<list.getSize();i++) {
    _count += setReference(list[i],n);
  }
  return(_count);
}
void EA3Reference::removeAllReferences() {
  {
	EA3ConstIterator it = beginConst();
	EA3Base** end = it.end();
	for(;it!=end;++it) {
		it->unref();
	}
  }

  setMemberSize(flags,members,0);
}

int EA3Reference::setReference(int ix, EA3Base *n) {
  touch();
  int size = getNumReferences();
  if (ix < 0) return(0);
  if (ix >= size) {
    return(addReference(n));
  }
  getReference(ix)->unref();
  n->ref();
  setMember(flags,members,n,ix);
  return(1);
}

int EA3Reference::insertReference(int ix,EA3Base *b) {
  touch();
  if (ix < 0) ix = 0;
  int num = getNumReferences();
  if (ix >= num) ix = num;
  ::insertMember(flags,members,b,ix);
  b->ref();
  return(ix);
}

int EA3Reference::removeReference(EA3Base *n) {
  touch();
  int i;
  int _count = 0;
  int size = getNumReferences();
  for(i=size-1;i>=0;i--) {
    if (getReference(i) == n) {
      removeMember(flags,members,i);
      n->unref();
      _count++;
    }
  }
  return(_count);
}

int EA3Reference::removeReference(int ix) {
  touch();
  if (ix < 0) return(0);
  EA3Base *b = getReference(ix);
  if (!b) return(0);
  removeMember(flags,members,ix);
  b->unref();
  //verify();
  return(1);
}

int EA3Reference::addReference(EA3Base *a,int check) {
  if (check) {
	EA3ConstIterator it = beginConst();
	EA3Base** end = it.end();
	for(;it!=end;++it) {
      if(*it == a) return(1);
    }
  }
  touch();
  if (a == NULL) return(0);
  a->ref();
  addMember(flags,members,a);
  return(1);
}

void EA3Reference::addRefName(XbRefName *r) {
  //refnames += r;
  EA3Scene::numReferences++;
  addMember(flags,members,(EA3Base*)r);
}

void EA3Reference::clearRefNames() {
/*
  for(int i = 0; i < refnames.getSize(); i++) {
    delete refnames[i];
  }
  refnames.setSize(0);
*/
}

//int here1=0;
//int here2=0;
//int here3=0;
EA3Status EA3Reference::resolveRefNames() {
  EA3Base *b = NULL;
  EA3Status status = EA3OK;
  int i = 0;
  bool hadMissingRefs = false;
  for(EA3Iterator it = begin(); it != it.end(); ++it, ++i) {
    counter = 0;
    XbRefName *rf = (XbRefName*)((EA3Base*)*it);
    if (!XbRefNameHeap.contains(rf)) {
      continue; //already resolved
    }
    if (b && b->getRefId() == rf->id) {
      //here1++;
    } else {
      //here2++;
      b = const_cast<EA3Base*>(EA3Reference::LRU.find(rf->id));
      if (!b) {
        //here3++;
        EA3Block *con = getContainer();
        EA3Base *exclude = NULL;
        b = NULL;
        while(con && !b) {
          b = con->findByRefName((XbRefName*)rf,exclude);
          exclude = con;
          con = con->getContainer();
        }
      }
    }
    //cerr << "counter = " << counter << endl;
    if (!b) {
      ERROR_LINE(EA3MISSING_REFERENCE);
      EA3_ERROR5("can not find reference for type: %s name: %s id: %d ix: %d",rf->getType().getString(),rf->getName().getString(),rf->getId(),rf->childIndex);
      status = EA3MISSING_REFERENCE;
      setMember(flags,members,NULL,i);
      hadMissingRefs = true;
      continue;
    }
    if (rf->childIndex > -1) {
      if (EA3Block *g = b->toBlock()) {
        EA3Base *b2 = g->getByIndex(rf->childIndex);
        if (!b2) {
          ERROR_LINE(EA3MISSING_REFERENCE);
          EA3_ERROR6("index out of range (%d) for type: %s name: %s id: %d ix: %d",g->getNumMembers(),g->getType().getString(),g->getName().getString(),rf->getId(),rf->childIndex);
          status = EA3MISSING_REFERENCE;
          setMember(flags,members,NULL,i);
          hadMissingRefs = true;
          continue;
        }
        //addReference(b2);
        delete rf;
        b2->ref();
        setMember(flags,members,b2,i);
        EA3Scene::numResReferences++;
        continue;
      } else {
        //cerr << "Referencing a non-block type: (";
        const char* ct = b->getClassType();
        ERROR_LINE(EA3MISSING_REFERENCE);
        
        EA3_ERROR6("Referencing a non-block C-type %s type: %s name: %s id: %d ix: %d",
          ct,rf->getType().getString(),rf->getName().getString(),rf->getId(),rf->childIndex);
        EA3_ERROR3("Referencing type: %s name: %s",
          b->getType().getString(),b->getName().getString());
        status = EA3MISSING_REFERENCE;
        setMember(flags,members,NULL,i);
        hadMissingRefs = true;
        continue;
      }
    } else {
      delete rf;
      b->ref();
      setMember(flags,members,b,i);
      EA3Scene::numResReferences++;
    }
  }
  if (hadMissingRefs) {
    int j = 0;
	EA3ConstIterator it = beginConst();
	EA3Base** end = it.end();
    for(;it!=end;++it) {
      while(it != end && !(*it)) ++it;
      if (it != end) {
        EA3Base *base = *it;
        setMember(flags,members,base,j);
        j++;
      }
    }
    setMemberSize(flags,members,j);
  }
  clearRefNames();
  return(status);
}

gzWriter& EA3Reference::write(gzWriter& os) const {
  EA3Base *b = NULL;
  int size = getNumReferences();
  if (size > 0) b = (EA3Base*)getReference(0);
  if (!b) return(writeBlockRef(os));
  int _i = 1;
  while (!b->getContainer() && _i < size ) {
    b = (EA3Base*)getReference(_i);
    _i++;
  };
  if (b->getContainer()==NULL) {
    return(writeBlockRef(os));
  }
  if (b && b->getContainer()->isArray()) {
    return(writeNormalRef(os));
  } else {
    return(writeBlockRef(os));
  }
}

EA3Status EA3Reference::readBinary(EA3Chunk *chunk, int chunkSize) {
  int allArray = chunk->getUChar();
  int num = 0;
  if (allArray) {
    num = (chunkSize-(4*5+1))/4;  // header + 4 bytes per entry
  } else {
    num = (chunkSize-(4*4+1))/8;  // header + 8 bytes per entry
  }
  //printf("size = %d %d\n",chunkSize,num);
  if (allArray) {
    int _id = chunk->getInt();
    //printf("array id = %d\n",_id);
    const EA3Array *a = (const EA3Array *)EA3Reference::LRU.find(_id);
    int maxIx = a ? a->getNumMembers() : 0;
    XbRefName r;
    for(int i=0;i<num;i++) {
      r.id = _id;
      r.childIndex = chunk->getInt();
      if (a && r.childIndex < maxIx) {
        addReference(a->getByIndex(r.childIndex));
      } else {
        addRefName(new XbRefName(r));
      }
    }
  } else {
    XbRefName r;
    for(int i=0;i<num;i++) {
      r.id = chunk->getInt();
      r.childIndex = chunk->getInt();
      EA3Base *b = NULL;
      if (r.childIndex == -1) {
        b = const_cast<EA3Base *>(EA3Reference::LRU.find(r.id));
      } else {
        const EA3Array *a = (const EA3Array*)EA3Reference::LRU.find(r.id);
        if (a && a->getNumMembers() > r.childIndex) {
          b = a->getByIndex(r.childIndex);
        }
      }
      if (b) {
        addReference(b);
      } else {
        addRefName(new XbRefName(r));
      }
    }
  }
  return(EA3OK);
}

EA3Status EA3Reference::writeBinary(EA3Chunk *chunk) const {
  chunk->append("REF\n",4);
  chunk->startBlock();
  chunk->append(getWriteId(this));
  chunk->append(chunk->stringTable.append(getType().getString()));
  chunk->append(chunk->stringTable.append(getName().getString()));
  int allArray = 1;
  EA3Base *contnr = 0;
  EA3ConstIterator it = beginConst();
  EA3Base** end = it.end();
  for(;it!=end;++it) {
    EA3Base *b = *it;
    EA3Block *c = b->getContainer();
    if (!c) continue;
    if (contnr == NULL) contnr = c;
    if (contnr != c || !c->isArray()) {
      allArray = 0;
      break;
    }
  }
  chunk->append((unsigned char)allArray);

  it = beginConst();
  end = it.end();
  if (allArray) {
    if (contnr) {
      chunk->append(getWriteId(contnr));
    } else {
      chunk->append((int)0);
    }

    for(;it!=end;++it) {
      EA3Base *b = *it;
      chunk->append(b->getIndex());
    }
  } else {
    for(;it!=end;++it) {
      EA3Base *b = *it;
      EA3Block *c = b->getContainer();
      if (c->isArray()) {
        chunk->append(getWriteId(c));
        chunk->append(b->getIndex());
      } else {
        chunk->append(getWriteId(b));
        chunk->append(-1);
      }
    }
  }
  chunk->endBlock();
  return(EA3OK);
}

gzWriter& EA3Reference::writeBlockRef(gzWriter& os) const {
  EA3Base *b = NULL;
  int size = getNumReferences();
  if (size > 0) b = (EA3Base*)getReference(0);
  writeIndent(os);
  XbIdentifier bType = "__";
  XbIdentifier lastType = "__";
  if (b) {
    bType = b->getType();
    lastType = b->getType();
  }
  if (bType != getType() || getName()!="") {
    os << getType();
    //if (hasRefName()) {
      writeRefName(os);
    //}
  }
  if (b) os << " " << bType;
  os << "[";
  if (b) {
    EA3ConstIterator it = beginConst();
	EA3Base** end = it.end();
    for(;it!=end;++it) {
      b = *it;
      if (!b->getContainer()) continue;
      bType = b->getType();
      if (bType != lastType) {
        os << "] " << bType << "[";
        lastType = bType;
      }
      os << XbQString(b->getName());
      //if (!(b->hasUniqueTypeAndName())) {
        os << ":" << getWriteId(b);
      //}
      os << " ";
    }
  }
  os << "]" << '\n';
  return(os);
}

gzWriter& EA3Reference::writeNormalRef(gzWriter& os) const {
  int i;
  int size = getNumReferences();
  EA3Base *b = (EA3Base*)getReference(0);
  int _i = 1;
  while (!b->getContainer() && _i < size ) {
    b = (EA3Base *)getReference(_i);
    _i++;
  };
  if (!b->getContainer()) return(os);
  writeIndent(os);
  XbIdentifier bType = "__";
  XbIdentifier lastType = "__";
  if (b) {
    bType = b->getContainer()->getType();
    lastType = bType;
  }
  if (bType != getType() || getName()!="") {
    os << getType();
    writeRefName(os);
    os << " ";
  }
  EA3Block *con = b->getContainer();
  EA3Block *lastCon = con;
  //EA3Block *myCon = getContainer();
  os << con->getType();
  //if (con->hasRefName()) {
    con->writeRefName(os);
  //}
  os << "[";
  int _count = 0;
  int numWriten = 0;

  EA3ConstIterator it = beginConst();
  EA3Base** end = it.end();
  for(i = 0;it!=end;++it,++i) {
    b = *it;
    if (b->flags.refCount < 2) continue;
    con = b->getContainer();
    if (con == NULL) continue;
    numWriten++;
    _count++;
    if (con != lastCon || _count == 5) {
      os << "] ";
      if (_count == 5) {
        _count = 0;
        os << '\n';
        writeIndent(os);
        os << "+";
      }
      os << con->getType();
      //if (con->hasRefName()) {
        con->writeRefName(os);
      //}
      os << "[";
      lastCon = con;
    }
    if (i == size-1) {
      os << b->getIndex();
    } else {
      os << b->getIndex() << " ";
    }
  }
  os << "]" << '\n';
  if (ea3asciiVerbose > 1 && numWriten > 4) {
    writeIndent(os);
    os << "#" << numWriten << '\n';
  } 
  return(os);
}

char *EA3Reference::read( char *is ) {
  return(readRef(is,0));
}
char *EA3Reference::readAppend( char *is ) {
  return(readRef(is,1));
}
static XbRefName *curRefName = NULL;
void EA3Reference::setCurRefName(XbRefName *rn) {
  delete curRefName;
  curRefName = rn;
}
char *EA3Reference::readRef( char *is, int) {
  touch();
  XbRefName rn;
  int noRefName;
  XbIdentifier curType;
  char *ch = is;
  //printf("start %s\n",ch);
  if (curRefName) {
    curType = curRefName->getType();
    rn.setType(curType);
    rn.setId(curRefName->getId());
  } else {
    curType = getType();
    rn.setType(curType);
    rn.setId(getRefId());
  }
  noRefName = 1;
  int skip = 1;
  skipWhiteSpace(ch);
  char *tmp;
  if(*ch != '[' && *ch != 0) {
	tmp=rn.read(ch);
    if (!tmp) {
      EA3_ERROR1("while reading reference ");
      return(NULL);
    }
    ch = tmp;
    curType = rn.getType();
  }
  if(*ch == '[') ch++;
  while(*ch) {
    skipWhiteSpace(ch);
    if (*ch == '\n') return(ch);
    if (*ch == '\r') return(ch);
    if (*ch == 0) return(ch);
// CHECK for other new line chars!
    if (!noRefName) {
	  tmp = rn.read(ch);
      if (!tmp) {
        EA3_ERROR1("while reading reference ");
        return(NULL);
      }
      ch = tmp;
      curType = rn.getType();
    }
    noRefName = 0;
    skipWhiteSpace(ch);
    if (!skip && *ch != '[') {
      ERROR_LINE(EA3SYNTAX_ERROR);
      EA3_ERROR1("expected a [");
      return(NULL);
    } else {
      if (!skip) ch++;
    }
    skip = 0;
    int numIndex = 0;
    int ix = 0;
    while(*ch) {
      int _id = -1;
      skipWhiteSpace(ch);
      if (*ch == 0) break;
      if (*ch == ']') {
        ch++;
        break;
      }
      if (*ch == '\"' || *ch == ':') {
        XbQString str;
        if (*ch == '\"') ch = str.read(ch);
        if (*ch == ':') {
          ch++;
          string2int(ch,_id);
          //while(*ch && *ch != ']' && *ch!=' ') ch++;
        }
// errno
        if (!(is)) {
          ERROR_LINE(EA3SYNTAX_ERROR);
          EA3_ERROR1("while reading index");
        }
        rn.setName(str);
        rn.setType(curType);
        rn.setId(_id);
        EA3Base *bref = const_cast<EA3Base *>(EA3Reference::LRU.find(_id));
        if (bref) {
          addReference(bref);
        } else {
          addRefName(new XbRefName(rn));
        }
        numIndex++;
      } else {
        string2int(ch,ix);
        //while(*ch && *ch != ']' && *ch!=' ') ch++;
// errno
        if (!(is)) {
          ERROR_LINE(EA3SYNTAX_ERROR);
          EA3_ERROR1("while reading integer index");
          return(is);
        }
        rn.childIndex = ix;
        EA3Block *bref = const_cast<EA3Block *>(static_cast<const EA3Block*>(EA3Reference::LRU.find(rn.id)));
        if (bref && bref->getNumMembers()>ix) {
          addReference(bref->getByIndex(ix));
        } else {
          addRefName(new XbRefName(rn));
        }
        numIndex++;
      }
      //ch++;
    } 
    if (numIndex == 0) {
      //EA3Scene::readError(cerr << "nothing in []? ");
    }
  }
  return(ch);
}

// EA3Block //

EA3Block::EA3Block(EA3Block *con, EA3Grammar *g,int ind, const XbIdentifier &t, const XbString &n) : EA3Base(NULL,g,ind,t,n),container(con) {
  touch();
  flags.needsReindex = 1;
  members = NULL;
}

EA3Base *EA3Block::copy(const EA3Block *root, EA3Block *newcontainer) const {
  EA3Block *ng;
	static XbIdentifier GLOBAL("GLOBAL");
  if (getType()!=GLOBAL) { // Not the scene ptr.
    if (isArray()) {
      ng = newcontainer->createArray(getType(),getName());
    } else {
      ng = newcontainer->createBlock(getType(),getName());
    }
    if (refCount()>1) setClone(ng);
    if (EA3IsSelected(this)) {
      EA3SetSelect(ng,1);
    }
  } else {
    ng = newcontainer;
  }
  int i;
  int numM = getNumMembers();
  for(i=0;i<numM;i++) {
    //EA3Base *b = 
		getByIndex(i)->copy(root,ng);
    //if (!b->setContainer(ng)) return(0);
  //infoList.setContainer(b,ng);
    //b->ref();
    //ng->members += b;
    ng->reindex();
  }
  if (root == this) {
    if (ng == this) {
      // we are copying scene data to same scene.
      for(i=numM;i<ng->getNumMembers();i++) {
        EA3Block *b = getByIndex(i);
        if (b) b->resolveCopyReferences();
      }
    } else {
      ng->resolveCopyReferences();
    }
    nullifyClonePointers();
    //printf("resolveClonePtrCount - %d\n",resolveClonePtrCount);
  }
  return(ng);
}

void copyFieldValues(const EA3Field *src, EA3Field *dst) {
  if (!src || !dst) return;
  const char *srcTypes = src->getDataType();
  const char *dstTypes = dst->getDataType();
  const char *extraTypes = src->getExtraTypes();
  if (extraTypes)
    dst->setExtraTypes(extraTypes);
  int i=0;
  while (*srcTypes && *dstTypes) {
    if (*srcTypes == *dstTypes) {
      dst->data[i].i = src->data[i].i;
    }
    i++;
    srcTypes++;
    dstTypes++;
  }
  i = dst->getNum();
  int j=src->getNum();
  while (extraTypes) {
    dst->data[i].i = src->data[j].i;
    i++;
    j++;
    extraTypes++;
  }
}

void EA3Block::real_reindex() {
  /*
  if (flags.needsReindex) {
    for (int i=0;i<members.getSize();i++) {
      members[i]->setIndex(i);
    }
  }
  flags.needsReindex = 0;
  */
}

hash_map<const EA3Base *,char *,EA3BaseHash,EA3BaseEqual, StlAllocator> inheritsMap;

void EA3Block::setInherits(const XbIdentifier &in) {
  inheritsMap[this] = const_cast<char *>(in.getString());
}
XbIdentifier EA3Block::getInherits() const {
  static XbIdentifier i;
  char *str = inheritsMap[this];
  if (str) {
    i.data = str;
  } else {
    i.data = XbHS_underScore;
  }
  return(i);
}

int EA3Block::setMember(int ix, EA3Base *b) {
  touch();
  //if (!canAddType(b)) return(0);
  if (ix < 0 || ix >= getNumMembers()) return(0);
  EA3Base *cur = getByIndex(ix);
  if (cur == b) return(1);
	EA3Block *con = b->getContainer();
	b->ref();
  if (con) { con->removeMember(b);}
  if (!b->setContainer(this)) return(0);
  if (cur->flags.refCount>1) {
    cur->setContainer(NULL);
  }
  cur->unref();
  ::setMember(flags,members,b,ix);
  //members[ix]->setIndex(ix);
  reindex();
  return(1);
}

int EA3Block::setNum(int num) {
  int s = getNumMembers();
  if (s <= num) return(s);
  for(int i=s-1;i>=num-1;i--) {
    removeMember(i);
  }
  return(num);
}

int EA3Block::insertMember(int ix,EA3Base *b) {
  touch();
  //if (!canAddType(b)) return(0);
  EA3Block *con = b->getContainer();
  if (con) {b->ref(); con->removeMember(b);}
  if (!b->setContainer(this)) return(0);
  if (ix < 0) ix = 0;
  if (ix >= getNumMembers()) ix = getNumMembers();
  ::insertMember(flags,members,b,ix);
  reindex();
  return(ix);
}

int EA3Block::appendMember(EA3Base *b) {
  touch();
  //if (!canAddType(b)) return(0);
  EA3Block *con = b->getContainer();
  if (con) {
    int ix = b->getIndex();
    if (ix < con->getNumMembers()) {
      if (ix != con->getNumMembers()-1) con->reindex();
      //EA3Base *b = con->members[ix];
      ::removeMember(con->flags,con->members,ix);
      //con->members.remove(ix);
    }
  } else {
    b->ref();
  }
  if (!b->setContainer(this)) return(0);
  addMember(flags,members,b);
  //b->index = members.getSize()-1;
  flags.needsReindex=1;
  return(1);
}

int EA3Block::memberIndex(EA3Base *b) {
  if (b->getContainer()!=this) return(-1);
  return(b->getIndex());
}

int EA3Block::removeMember(int ix) {
  touch();
  int size = getNumMembers();
  if (ix < 0 || ix >= size) {
    return(0);
  }
  EA3Base *b = getByIndex(ix);
  if (!b) {
    return(0);
  }
  ::removeMember(flags,members,ix);
  if (b->flags.refCount>1) {
    b->setContainer(NULL);
  }
  b->unref();
  if (ix != size-1) reindex();
  return(1);
}

int EA3Block::swapMembers( int m1, int m2 )
{
    if( m1 == m2 ) return 1;
    int size = getNumMembers();
    if( m1 < 0 || m1 >= size ) return 0;
    if( m2 < 0 || m2 >= size ) return 0;

    touch();
    EA3Base *b1 = getByIndex(m1);
    EA3Base *b2 = getByIndex(m2);
    ::setMember(flags,members,b1,m2);
    ::setMember(flags,members,b2,m1);
    flags.needsReindex=1;
    return 1;
}

static bool didSomeUnrefs = false;
static void doUnRefs(EA3Block *bl) {
  if (refCount(bl)>1) {
    //printf("block %s %s had refCount > 1\n",bl->getType().getString(),bl->getName().getString());
    return;
  }
  EA3ConstIterator it = bl->beginConst();
  EA3Base** end = it.end();
  for(;it!=end;++it){
    EA3Reference *ref = *it;
    if (ref && ref->getNumReferences() && refCount(ref) == 1) {
      ref->removeAllReferences();
      didSomeUnrefs = true;
    } 
    EA3Block *b = *it;
    if (b) doUnRefs(b);
  }
}


int EA3Block::removeAllMembers() {
  // Try to avoid b->setContainer(NULL) since b may get deleted anyway
  // The doUnRefs function gets called (but we only do it at the beginning of the recursion)
  // to minimize the refCount of nodes.  Unless there are external references, deleting a scene
  // should never call b->setContainer(NULL)
  
  touch();
  bool unset = false;
  static bool didUnRefs = false;
  if (!doingSceneDelete) {
    if (!didUnRefs) {
      didUnRefs = true;
      unset = true;
      do {
        didSomeUnrefs = false;
        //printf("doing unrefs\n");
        doUnRefs(this);
      }
      while(didSomeUnrefs);
    }
  }
 
  EA3ConstIterator it = beginConst();
  EA3Base** end = it.end();
  for(;it!=end;++it){
    EA3Base *b = *it;
    int rc = b->flags.refCount;
    b->unref();
    if (!doingSceneDelete) {
      if (rc > 1) {
        //printf("NULL: %s %s\n",b->getType().getString(),b->getName().getString());
        b->setContainer(NULL);
      }
    }
  }
  setMemberSize(flags,members,0);
  reindex();
  if (unset) {
    didUnRefs = false;
  }
  return(1);
}

//! if (keep) baseList is a list of members to keep, else it is a list of members to remove
int EA3Block::removeMembers(const XbPArray<EA3Base *> &baseList,bool keep) {
  int size = getNumMembers();
  hash_set<EA3Base*,EA3BaseHash,EA3BaseEqual, StlAllocator> tagged;
  int i;
  if (keep) {
    for(i=0;i<size;i++) {
      tagged.insert(getByIndex(i));
    }
  }
  for(i=0;i<baseList.getSize();i++) {
    EA3Base *b=baseList[i];
    if (!b || b->getContainer()!=this) continue;
    if (keep) {
      tagged.erase(b);
    } else {
      tagged.insert(b);
    }
  }
  int tsize = (int)tagged.size();
  if (tsize == size) {
    removeAllMembers();
    return(size);
  }
  if (tsize == 0) return(0);
  for(i=0;i<size;i++) {
    EA3Base *b2 = getByIndex(i); 
    if (tagged.find(b2)!=tagged.end()) {
      int rc = b2->flags.refCount;
      b2->unref();
      if (rc > 1) {
        b2->setContainer(NULL);
      }
      ::setMember(flags,members,NULL,i);
    }
  }
  int j = 0;
  for(i=0;i<size;i++) {
    while(j<size && !getByIndex(j)) j++;
    if (j==size) break;
    EA3Base *b = getByIndex(j);
    ::setMember(flags,members,b,i);
    j++;
  }
  setMemberSize(flags,members,i);
  flags.needsReindex=1;
  return(tsize);
}

int EA3Block::removeMember(EA3Base *b) {  
  touch();
  return(removeMember(b->getIndex()));
}

gzWriter& EA3Block::write(gzWriter& os) const {
  static int progressCount = 1001;
  if (EA3Scene::progressCallback) {
    numWritenBlocks++;
    progressCount++;
    if (progressCount > 10000) {
      (*EA3Scene::progressCallback)(3,numWritenBlocks,writeBlockCount);
      progressCount = 0;
    }
  }
  if (!isArray()) {
    int needsBrace = 0;
    //if (hasRefName()) {
      needsBrace = 1;
    //}
    writeIndent(os);
    os << getType();
    if (needsBrace) {
      writeRefName(os);
    }
    //int wroteOne = hasRefName();
    const char *inherits = getInherits().getString();
    if (inherits[0]!='_') {
      os << " : " << inherits;
    }
    os << " {";
    if (ea3asciiVerbose > 1) {
      int rc = refCount();
      if (getContainer()->isArray()) {
        if (rc > 1) {
          os << "\t# (" << refCount()-1 << ")\t" << getIndex();
        } else {
          os << "\t# " << getIndex();
        }
      } else {
        if (rc > 1) {
          os << "\t# (" << refCount()-1 << ")";
        }
      }
    }
    os << '\n';
  } else {
    writeIndent(os);
    os << getType();
    //if (hasRefName()) {
      writeRefName(os);
    //}
    os << "[] " << "{ # n=" << getNumMembers() << '\n';
  }
  EA3ConstIterator it = beginConst();
  EA3Base** end = it.end();
  for(;it!=end;++it){
	it->write(os);
  }
  writeIndent(os);
  os << "}" << '\n';
  return(os);
}

EA3Status EA3Block::writeBinary(EA3Chunk *chunk) const {
  static int progressCount = 10001;
  if (EA3Scene::progressCallback) {
    numWritenBlocks++;
    progressCount++;
    if (progressCount > 10000) {
      (*EA3Scene::progressCallback)(3,numWritenBlocks,writeBlockCount);
      progressCount = 0;
    }
  }
  int isA = 0;
  if (isGrammar() && (getContainer() == NULL || getContainer()->getContainer()==NULL)) {
    chunk->append("GRM\n",4);
  } else if (isArray()) {
    isA = 1;
    chunk->append("ARR\n",4);
  } else {
    chunk->append("BLK\n",4);
  }
  chunk->startBlock();
  if (isA || flags.refCount > 1) {
    chunk->append(getWriteId(this));
  } else {
    chunk->append(-1);
  }
  chunk->append(chunk->stringTable.append(getType().getString()));
  chunk->append(chunk->stringTable.append(getName().getString()));
  int num=getNumMembers();
  int extra = 0;
  EA3ConstIterator it = beginConst();
  EA3Base** end = it.end();
  for(;it!=end;++it){
    EA3Base *b = *it;
    if (EA3Field *f = b->toField()) {
      if (f->getExtraTypes()) extra++;
    }
  }
  chunk->append(num+extra);
  EA3Field *lastF = NULL;

  it = beginConst();
  end = it.end();
  for(;it!=end;++it){
    EA3Base *b = *it;
    if (EA3Field *f = b->toField()) {
      if (isA && f->getDataType(0)!='b' && !f->getExtraTypes()) {
        if(lastF == NULL || lastF->getType() != f->getType() || lastF->getName() != f->getName()) {
          if(lastF) chunk->endBlock();
          lastF = f;
          chunk->append("MFL\n",4);
          chunk->startBlock();
          chunk->append(getWriteId(this));
          chunk->append(chunk->stringTable.append(f->getType().getString()));
          chunk->append(chunk->stringTable.append(f->getName().getString()));
          chunk->append(chunk->stringTable.append(f->getDataType()));
          chunk->append((unsigned int)f->getNum());
          f->writeBinary(chunk);
        } else {
          f->writeBinary(chunk);
        }
      } else {
        if(lastF) {chunk->endBlock(); lastF = NULL;}
        chunk->append("SFL\n",4);
        chunk->startBlock();
        if (f->flags.refCount > 1) {
          chunk->append(getWriteId(f));
        } else {
          chunk->append(-1);
        }
        chunk->append(chunk->stringTable.append(f->getType().getString()));
        chunk->append(chunk->stringTable.append(f->getName().getString()));
        chunk->append(chunk->stringTable.append(f->getDataType()));
        f->writeBinary(chunk);
        chunk->endBlock();
        const char *extypes = f->getExtraTypes();
        if (extypes) {
          chunk->append("SFL\n",4);
          chunk->startBlock();
          chunk->append(-1);
          XbString ext("_");
          ext += extypes;
          chunk->append(chunk->stringTable.append(ext));
          chunk->append(chunk->stringTable.append(f->getName().getString()));
          chunk->append(chunk->stringTable.append(f->getDataType()));
          int len = (int)strlen(extypes);
          int fnum = f->getNum();
          for(int xi=0;xi<len;xi++) {
            if (extypes[xi]=='s' || extypes[xi]=='d' || extypes[xi] == 'n') {
              chunk->append(chunk->stringTable.append(f->data[fnum+xi].str));
            } else {
              chunk->append(f->data[fnum+xi].i);
            }
          }
          chunk->endBlock();
        }
      }
    } else { // Not a field
      if(lastF) { // Make sure we close the last one...
        chunk->endBlock();
        lastF = NULL;
      }
      it->writeBinary(chunk);
    }
  }
  if(lastF) {
    chunk->endBlock();
    lastF = NULL;
  }
  chunk->endBlock();
  return(EA3OK);
}

EA3Status EA3Base::readBinary(EA3Chunk *, int) {
  return(EA3OK);
}

EA3Status EA3Block::readBinary(EA3Chunk *chunk, int isGram) {
// Header
  if (EA3Scene::progressCallback) {
    static int lastNumDone = 0;
    if (EA3Scene::progressCallback) {
      EA3Scene *mScene = getScene();
      if (mScene && mScene->filePtr != 0) {
        int numDone = Ftell32(mScene->filePtr);
        if (numDone != lastNumDone) {
          (*EA3Scene::progressCallback)(1,numDone,mScene->fileSize);
        }
        lastNumDone = numDone;
      }
    }
  }
  int num = chunk->getInt();
  XbRefName refName;
  EA3Status status = EA3OK;
  EA3Field *lastFld = NULL;
  for(int i=0;i<num;i++) {
    int doArray = 0;
    int doRef = 0;
    int doBlock = 0;
    int do1Field = 0;
    int doMField = 0;
    char buf[4];
    buf[0] = chunk->getUChar();
    buf[1] = chunk->getUChar();
    buf[2] = chunk->getUChar();
    buf[3] = chunk->getUChar();
    if (memcmp(buf,"ARR\n",4)==0) {
      doArray = 1;
    } else if (memcmp(buf,"GRM\n",4)==0) {
      isGram = 1;
      doBlock = 1;
    } else if (memcmp(buf,"BLK\n",4)==0) {
      doBlock = 1;
    } else if (memcmp(buf,"SFL\n",4)==0) {
      do1Field = 1;
    } else if (memcmp(buf,"MFL\n",4)==0) {
      doMField = 1;
    } else if (memcmp(buf,"REF\n",4)==0) {
      doRef = 1;
    } else {
      EA3_ERROR1("unknown block encountered\n");
      return(EA3UNKNOWN_TYPE);
    }
    int chunkSize = chunk->getInt();
    char *skip = chunk->getCurrent()+chunkSize-4;
    int ID = chunk->getInt();
    int typeIx = chunk->getInt();
    int nameIx = chunk->getInt();
    XbString n(chunk->stringTable.get(nameIx));
    XbIdentifier t(chunk->stringTable.getIdentifier(typeIx));
    const char *_t = t.getString();
    refName.setType(t);
    refName.setName(n);
    refName.id = ID;
    EA3Grammar *g = ((EA3Grammar*)getGrammar());
    if (!g) g = (EA3Grammar*)this;
    if (do1Field) {
      if (_t[0] == '_') { // hack to support extra data and be backwards compatible
        if (lastFld) {
          const char *et = &_t[1];
          if (lastFld->setExtraTypes(et)) {
            unsigned int exnum = (unsigned int)strlen(et);
            int fnum = lastFld->getNum();
            chunk->stringTable.get(chunk->getInt()).getString();
            for(unsigned int ei=0;ei<exnum;ei++) {
              switch(et[ei]) {
                case 'f': 
                case 'i':
                case 'c': lastFld->data[fnum+ei].i = chunk->getInt(); break;
                case 'n':
                case 'd': 
                case 's': lastFld->data[fnum+ei].str = (chunk->stringTable.get(chunk->getInt()).getString()); break;
                default: chunk->getInt();
              } 
            }
          }
        }
        chunk->setCurrent(skip); 
        continue;
      }
      EA3Field *fld = g->createFieldInstance(this,t,n,isGram);
      if (!fld) {
        status = EA3UNKNOWN_TYPE;
        EA3_ERROR2("skipping unknown field (%s)\n",t.getString());
        chunk->setCurrent(skip); 
        continue;
      }
      if (binVersion > 1.0) {
        fld->setRefId(refName.id);
      }
      //EA3Reference::LRU.add(fld,refName.id);
      //char *dataType = (char *)
	  chunk->stringTable.get(chunk->getInt()).getString();
      fld->readBinary(chunk,chunkSize);
      lastFld = fld;
      continue;
    } 
    if (doMField) {
      //char *dataType = (char *)
	  chunk->stringTable.get(chunk->getInt()).getString();
      //printf("dataType = %s\n",dataType);
      int _num = chunk->getInt();
      int headerSize = 6*4;
      int numFields = (chunkSize-headerSize)/(_num*4);
      //printf("numFields = %d\n",numFields);
      for(int i2=0;i2<numFields;i2++) {
        EA3Field *fld = g->createFieldInstance(this,t,n,isGram);
        if (!fld) {
          status = EA3UNKNOWN_TYPE;
          EA3_ERROR2("skipping unknown fields (%s)\n",t.getString());
          chunk->setCurrent(skip); 
          break;
        }
        fld->readBinary(chunk,_num);
      }
      i += numFields-1;
      continue;
    }
    if (doRef) {
      EA3Reference *rf = g->createReferenceInstance(this,t,n,isGram);
      if (!rf) {
        status = EA3UNKNOWN_TYPE;
        EA3_ERROR2("skipping unknown reference (%s)\n",t.getString());
        chunk->setCurrent(skip); 
        continue;
      }
      rf->setRefId(refName.id);
      rf->readBinary(chunk,chunkSize);
      continue;
    }
    if (doBlock) {
      EA3Block *bl = NULL;
      bl = g->createBlockInstance(this,t,n,isGram);
      if (!bl) {
        status = EA3UNKNOWN_TYPE;
        EA3_ERROR2("skipping unknown block (%s)\n",t.getString());
        chunk->setCurrent(skip); 
        continue;
      }
      //bl->setRefId(refName.id);
      EA3Reference::LRU.add(bl,refName.id);
      //if (!bl) return(-2);
      bl->readBinary(chunk,isGram);
      continue;
    }
    if (doArray) {
      EA3Array *bl = g->createArrayInstance(this,t,n,isGram);
      if (!bl) {
        status = EA3UNKNOWN_TYPE;
        EA3_ERROR2("skipping unknown array (%s)\n",t.getString());
        chunk->setCurrent(skip); 
        continue;
      }
      //bl->setRefId(refName.id);
      EA3Reference::LRU.add(bl,refName.id);
      //if (!bl) return(-2);
      bl->readBinary(chunk,isGram);
      continue;
    }
  }
  return(status);
}

#define EA3_GZREADER_BUF_SIZE 1000000
class gzReader : public EA3Mem {
  public:
    gzReader(gzFile g) : gz(g), bufRemain(0), bufOffset(0) {
    }
    gzReader(const char *str) : strPtr(str), gz(NULL), bufRemain((int)strlen(str)+1),bufOffset(0) {
    }
    inline int getChar() {
      if(gz) {
        fillCache();
        if (bufRemain <= 0) return(-1000);
        --bufRemain;
        return(gzbuf[bufOffset++]);
      } else {
        if (bufRemain <= 0) return(-1000);
        char c = *strPtr;
        if (!c) return(-1000);
        strPtr++;
        return(c);
      }
    }
    inline int peek() {
      if(gz) {
        fillCache();
        if (bufRemain <= 0) return(-1);
        return(gzbuf[bufOffset]);
      } else {
        if (bufRemain <= 0) return(-1);
        char c = *strPtr;
        if (!c) return(-1);
        return(c);
      }
    }
    inline char* getLine() {
      line.setSize(1000);
      line[0] = 0;
      int c;
      int ix=0;
      int loop = 1;
      while(loop) {
        line.setSize(ix+2);
        line[ix+1] = 0;
        c = getChar();
        if (c=='\n') {
          discardNLChars();
          return(line.buffer);
        } else if(c=='\r') {
          discardNLChars();
          return(line.buffer);
        } else if(c==-1000) {
          if (ix==0) return(0);
          return(line.buffer);
        }
        line[ix] = (char)c;
        ix++;
      }
      return(0);
    }
    inline const char *getBuf() {
      fillCache();
      return(gzbuf);
    }
    int readAll(char *&data, int &len) {
      int allocd = EA3_GZREADER_BUF_SIZE*10;
      len = 0;
      data = (char*)EA3Malloc(allocd);
      fillCache();
      char *ptr = data;
      do {
        if (len+bufRemain > allocd) {
          allocd *= 2;
          data=(char *)EA3Realloc(data,allocd);
          ptr = data+len;
        }
        if (bufRemain) {
          memcpy(ptr,gzbuf,bufRemain);
          len+=bufRemain;
          ptr+=bufRemain;
        }
        bufRemain = 0;
        fillCache();
      } while(bufRemain!=0);
      return(0);
    }
  private:
    inline void discardNLChars() {
	  int loop = 1;
      while(loop) {
        int c = peek();
        if(c=='\n') { c=getChar(); continue; }
        if(c=='\r') { c=getChar(); continue; }
        break;
      }
    }
    inline void fillCache() {
      if (bufRemain == 0 && gz) {
#if 1 //ndef PARAGON_STANDALONE
        bufRemain = gzread(gz,gzbuf,EA3_GZREADER_BUF_SIZE);
#endif
        //printf("gzread %d\n",bufRemain);
        bufOffset = 0;
      }
    }
    const char *strPtr;
    static char gzbuf[EA3_GZREADER_BUF_SIZE+1];
    XbPArray<char> line;
    const gzFile gz;
    int bufRemain;
    int bufOffset;
    gzReader &operator=(gzReader &) {return *this;}
};
char gzReader::gzbuf[EA3_GZREADER_BUF_SIZE+1];


EA3Status EA3Block::parse(const char *str) {
  EA3Scene *s = getScene();
  int num = s->getNumMembers();
  char *data = XbString::_strdup(str);
  s->gzr = new gzReader(data);
  s->readAll();
  delete s->gzr;
  s->gzr = NULL;
  EA3Free(data);
  int numAfter=s->getNumMembers();
  if (numAfter > num && s!=this) {
    for(int i=num;i<numAfter;i++) {
      this->appendMember(s->getByIndex(num));
    }
  }
  return(EA3Scene::readStatus);
}

char *EA3Block::read( char *is ) {
  touch();
  return(readMembers(is,isGrammar()));
}

EA3Block *EA3Block::createBlock(const XbIdentifier &t,const XbString &nm,bool isGram) {
  if (!isArray() && !isGram) {
    if (EA3Base *b = getByType(t)) {   
      if (b->isBlock() && b->getContainer()->getContainer()) return(b->toBlock());
    }
  }
  EA3Grammar *g = (EA3Grammar *)getGrammar();
  EA3Block *b = g->createBlockInstance(this,t,nm,isGram);
  return(b);
}
EA3Block *EA3Block::realCreateBlock(const EA3Base *g,const XbIdentifier &t,const XbString &nm,int ix) {
  if (!g) return(NULL);
  //if (!canAddBlock()) return(0);
  EA3Block *b = new EA3Block(this,const_cast<EA3Grammar *>(static_cast<const EA3Grammar*>(g)),getNumMembers(),t,nm);
  b->ref();
  if (ix == -1) {
    addMember(flags,members,b);
  } else {
    ::setMember(flags,members,b,ix);
  }
  flags.needsReindex=1;
  //appendMember(b);
  return(b);
}

EA3Array *EA3Block::createArray(const XbIdentifier &t,const XbString &nm,bool isGram) {
  if (!isArray() && !isGram) {
    if (EA3Base *b = getByType(t)) {   
      if (b->isArray() && b->getContainer()->getContainer()) return(b->toArray());
    }
  }
  EA3Grammar *g = (EA3Grammar *)getGrammar();
  return(g->createArrayInstance(this,t,nm,isGram));
}
EA3Array *EA3Block::realCreateArray(const EA3Base *g,const XbIdentifier &t,const XbString &nm,int ix) {
  //if (!canAddArray()) return(0);
  EA3Array *a = new EA3Array(this,const_cast<EA3Grammar *>(static_cast<const EA3Grammar*>(g)),getNumMembers(),t,nm);
  if (!g) return(NULL);
  if (ix == -1) {
      addMember(flags,members,a);
    } else {
      ::setMember(flags,members,a,ix);
    }
  flags.needsReindex=1;
  a->ref();
  //appendMember(a);
  return(a);
}

EA3Reference *EA3Block::createReference(const XbIdentifier &t,const XbString &nm,bool isGram) {
  if (!isArray() && !isGram) {
    if (EA3Base *b = getByType(t)) {   
      if (b->isReference()) return(b->toReference());
    }
  }
  EA3Grammar *g = (EA3Grammar *)getGrammar();
  return(g->createReferenceInstance(this,t,nm,isGram));
}
EA3Reference *EA3Block::realCreateReference(const EA3Base *g,const XbIdentifier &t,const XbString &nm,int ix) {
  if (!g) return(NULL);
  //if (!canAddReference()) return(0);
  EA3Reference *r = new EA3Reference(this,const_cast<EA3Grammar *>(static_cast<const EA3Grammar*>(g)),getNumMembers(),t,nm);
  if (ix == -1) {
    addMember(flags,members,r);
  } else {
    ::setMember(flags,members,r,ix);
  }
  flags.needsReindex=1;
  r->ref();
  //appendMember(r);
  return(r);
}

EA3Pointer EA3Block::get(const XbIdentifier &ftype,const XbString &fname, char *dtype) const {
  EA3ConstIterator it = beginConst();
  EA3Base** end = it.end();
  for(;it!=end;++it){
    EA3Base *b = *it;
    if (b->getName() == fname && b->getType() == ftype && b->typeIs(dtype)) return(b);
  }
  return(NULL);
}

EA3Pointer EA3Block::getByType(const XbIdentifier &ftype, char *dtype) const {
  const char *tmp = ftype.getString();
  EA3ConstIterator it = beginConst();
  EA3Base** end = it.end();

  if (dtype) {
	for(;it!=end;++it){
      EA3Base *b = *it;
      if (b->getType().getString() == tmp && b->typeIs(dtype)) {
        return(b);
      }
    }
  } else {
	for(;it!=end;++it){
      EA3Base *b = *it;
      if (b->getType().getString() == tmp) {
        return(b);
      }
    }
  }
  return(NULL);
}

EA3Pointer EA3Block::getByName(const XbString &fname, char *dtype) const {
  const char *tmp = fname.getString();

  EA3ConstIterator it = beginConst();
  EA3Base** end = it.end();
  if (dtype) {
    for(;it!=end;++it) {
      EA3Base *b = *it;
      if (b->getName().getString() == tmp && b->typeIs(dtype)) {
        return(b);
      }
    }
  } else {
    for(;it!=end;++it) {
      EA3Base *b = *it;
      if (b->getName().getString() == tmp) {
        return(b);
      }
    }
  }
  return(NULL);
}

extern int trackMalloc;
char *EA3Block::readMembers(char *is, int isGram) {
  touch();
  EA3Reference *lastRef = NULL;
  int bracketCount = 0;
  int skipData = 0;
  char *origLine;
  XbRefName refName;
  int loop = 1;
  EA3Scene *sc = getScene();
  while(loop) {
    char *line = sc->getLine();
    //printf("line = %s\n",line);
    if (!line) break;
    skipWhiteSpace(line);
    if (*line == '#') continue; // a comment
    origLine = line;
    int lineIsARef = 0;
    int lineIsARefCont = 0;
    if (*line=='+') {
      line++;
      lineIsARef = 1;
      lineIsARefCont = 1;
    }
    int lineIsArray = 0;
    int lineIsBlock = 0;
    char lastCh = 0;
    if (*line == 0) {
      continue;
      //return(line);
    }
    if (*line == '}') {
      line++;
      if(skipData) {
        ERROR_LINE(EA3UNKNOWN_TYPE);
        EA3_ERROR2("skipping line: %s",origLine);
        bracketCount--;
        if (bracketCount==0) {
          skipData = 0;
        }
        continue;
      }
      EA3Scene::lineCheck(line);
      return(line);
    }
    line = refName.read(line);
    if (!line) {
      return(line);
    }
    const XbIdentifier &rtype = refName.getType();
    const XbString &rname = refName.getName();
    //cerr << "ref name: " << refName << endl;
    skipWhiteSpace(line);
    char *ch = line;
    char *endCh = ch;
    XbIdentifier inherits;
    if (*ch == ':') {
      ch++;
      XbRefName rn;
      ch = rn.read(ch);
      inherits = rn.getType();
    }
    bool referenceHasOwnName=false;
    if (!isdigit_l(*ch) && *ch != '-') { // quick check to see if it is a numberic field
      while(*ch) {
        if (*ch == 0) break;
        if (*ch == ' ') {
          skipWhiteSpace(ch);
          if (*ch == '{') {
            ch++;
            lineIsArray++;
            lineIsBlock++;
          }
          break;
        }
        if (*ch == '[') { 
          lineIsARef++; 
          lineIsArray++; 
          if (*(ch+1) != ']') {
            break;
          } else if (!isGram) {
            referenceHasOwnName = true;
          }
        }
        if (*ch == ']') { 
          lineIsARef++; 
          if (lastCh == '[') lineIsArray++;
          else {
            // line is a ref 
            break;
          }
        }
        if (*ch == '{') {
          lineIsArray++;
          lineIsBlock++;
        }
        if (*ch == '(') { // a reference like: type(:id)[...]
          lineIsARef++;
          referenceHasOwnName = true;
          break;
        }
        if (*ch == '"') { // A field with a string
          break;
        }
      
        // Note the difference between:
        // field XbIdentifier
        //    AND
        // reference XbIdentifier[...]
        // example:
        // type local
        // refname blockname[...]
        // ALSO
        //  reference refname(:id)[...]
        
        lastCh = *ch;
        ch++;
      }
    }
    if (endCh) {
      ch = endCh;
      //printf("endCh = %s\n",ch);
    }
    //printf("r %d(%d) a %d b %d ch = %s\n",lineIsARef,lineIsARefCont,lineIsArray,lineIsBlock,ch);
    skipWhiteSpace(ch);
    if (*ch == '}') {
      ch++;
      if(skipData) {
        ERROR_LINE(EA3UNKNOWN_TYPE);
        EA3_ERROR2("skipping line: %s",origLine);
        bracketCount--;
        if (bracketCount==0) {
          skipData = 0;
        }
        continue;
      }
      EA3Scene::lineCheck(ch);
      return(ch);
    }
    if (skipData) {
      ERROR_LINE(EA3UNKNOWN_TYPE);
      EA3_ERROR2("skipping line: %s",origLine);
      if(lineIsArray==3 || lineIsBlock) {
        bracketCount++;
      }
      continue;
    }
    if (lineIsArray == 3) {
      EA3Array *a = NULL;
      if (getGrammar()) {
        a = ((EA3Grammar*)getGrammar())->createArrayInstance(this,rtype,rname,isGram);
      } else {
        a = ((EA3Grammar*)this)->createArrayInstance(this,rtype,rname,isGram);
      }
      if (!a) {
        ERROR_LINE(EA3UNKNOWN_TYPE);
        EA3_ERROR2("grammar can not create array: %s ",rtype.getString());
        bracketCount = 1;
        skipData = 1;
        continue;
      }
      //a->setRefId(refName.id);
      EA3Reference::LRU.add(a,refName.id);
      if (!(a->readMembers(ch,isGram))) return(NULL);
      continue;
    } else if (lineIsBlock) {
      static XbIdentifier ID_Grammar("Grammar");
      if (rtype == ID_Grammar) {
        EA3Scene *scene = getScene();
        scene->newGrammar();
        EA3Grammar *g = (EA3Grammar *)scene->getGrammar();
        //g->setRefId(refName.id);
        g->readMembers(ch,1);
        scene->rebuildGrammarTable();
        EA3Scene::numReferences=0;
      } else {
        EA3Block *bl = ((EA3Grammar *)getGrammar())->createBlockInstance(this,rtype,rname,isGram);
        if (!bl) {
          ERROR_LINE(EA3UNKNOWN_TYPE);
          EA3_ERROR2("grammar can not create block: %s ",rtype.getString());
          bracketCount = 1;
          skipData = 1;
          continue;
        }
        //bl->setRefId(refName.id);
        EA3Reference::LRU.add(bl,refName.id);
        if (inherits[0]!='_') {
          bl->setInherits(inherits);
        }
        if (!(bl->readMembers(ch,isGram))) return(NULL);
      }
      continue;
    } else {
      if (lineIsARef) {
        if (lineIsARefCont) {
          //line2.get();
          if (!lastRef) {
            ERROR_LINE(EA3SYNTAX_ERROR);
            EA3_ERROR1("while appending to reference.  No previous refernce?!");
            return(NULL);
          }
          lastRef->setCurRefName(new XbRefName(refName));
          if (!lastRef->readAppend(ch)) {
            ERROR_LINE(EA3SYNTAX_ERROR);
            EA3_ERROR1("while appending to reference.");
            return(NULL);
          }
        } else {
          EA3Grammar *g = (EA3Grammar *)getGrammar();
          EA3Reference *rf = g->createReferenceInstance(this,rtype,(referenceHasOwnName||isGram)? rname : XbString(""),isGram);
          if (!rf) {
            ERROR_LINE(EA3UNKNOWN_TYPE);
            EA3_ERROR1("cound not create reference.");
            continue;
          }
          rf->setRefId(refName.id);
          rf->setCurRefName(NULL);
          lastRef = rf;
          if (!(rf->read(ch))) {
            ERROR_LINE(EA3UNKNOWN_TYPE);
            EA3_ERROR1("while reading reference.");
            continue;
          }
        }
      } else {
// Key word or a field
        static XbString ug("UseGrammar");
        if (rtype == ug) {
          skipWhiteSpace(ch);
          int ix = atoi(ch);
          //fprintf(stderr,"Using grammar %d\n",ix);
          getScene()->useGrammar(ix);
          while (*ch != 0) ch++;
          EA3Scene::numReferences=0;
        } else {
          skipWhiteSpace(ch);
          EA3Field *bf = createField(rtype,rname,isGram?true:false);
          if (bf) {
            //bf->setRefId(refName.id);
            EA3Reference::LRU.add(bf,refName.id);
            ch = bf->read(line);
          } else {
            ERROR_LINE(EA3UNKNOWN_TYPE);
            EA3_ERROR2("grammar can not create field: %s ",rtype.getString());
            continue;
          }
        }
      }
      //if (EA3Scene::lineCheck(line2)) return(is);
    }
    //if (EA3Scene::lineCheck(line)) return(is);
  }
  return(is);
}

EA3Pointer EA3Block::find(const XbIdentifier &ftype, const XbString &fname, char *dtype) const {
  if (getName() == fname && getType() == ftype && typeIs(dtype)) return(const_cast<EA3Block *>(this));
  EA3ConstIterator it = beginConst();
  EA3Base** end = it.end();
  for(;it!=end;++it) {
    EA3Base *b = *it;
    if (b->getName() == fname && b->getType() == ftype && b->typeIs(dtype)) return(b);
  }
  it = beginConst();
  for(;it!=end;++it) {
    if (EA3Block *g = it->toBlock()) {
      EA3Base *f = g->find(ftype,fname,dtype);
      if(f) return(f);
    }
  }
  return(NULL);
}
EA3Pointer EA3Block::findByName(const XbString &fname,char *dtype) const {
  if (getName() == fname && typeIs(dtype)) return(const_cast<EA3Block *>(this));
  EA3ConstIterator it = beginConst();
  EA3Base** end = it.end();
  for(;it!=end;++it) {
    EA3Base *b = *it;
    if (b->getName() == fname && b->typeIs(dtype)) return(b);
  }
  it = beginConst();
  for(;it!=end;++it) {
    if (EA3Block *g = it->toBlock()) {
      EA3Base *f = g->findByName(fname,dtype);
      if(f) return(f);
    }
  }
  return(NULL);
}

EA3Pointer EA3Block::findByType(const XbIdentifier &ftype,char *dtype) const {
  if (getType() == ftype && typeIs(dtype)) return(const_cast<EA3Block *>(this));

  EA3ConstIterator it = beginConst();
  EA3Base** end = it.end();
  for(;it!=end;++it) { 
    EA3Base *b = *it;
    if (b->getType() == ftype && b->typeIs(dtype)) return(b);
  }
  it = beginConst();
  for(;it!=end;++it) { 
    if (EA3Block *g = it->toBlock()) {
      EA3Base *f = g->findByType(ftype,dtype);
      if(f) return(f);
    }
  }
  return(NULL);
}

EA3Iterator EA3Block::begin() const 
{
	if (flags.num == 0) return EA3Iterator( (EA3Base **)&members, (EA3Base **)(&members) );
	if (flags.num == 1) return EA3Iterator( (EA3Base **)&members, (EA3Base **)(&members + 1) );
	if (flags.num < 127) return EA3Iterator( (EA3Base **)members, (EA3Base **)(members + (int)flags.num) );

	// more than 128 members
	intptr_t size = (intptr_t)members[0];
	return EA3Iterator( (EA3Base**)(members + 1), (EA3Base**) (members + size + 1));
}

EA3ConstIterator EA3Block::beginConst() const 
{
	if (flags.num == 0) return EA3ConstIterator( (EA3Base **)&members, (EA3Base **)(&members) );
	if (flags.num == 1) return EA3ConstIterator( (EA3Base **)&members, (EA3Base **)(&members + 1) );
	if (flags.num < 127) return EA3ConstIterator( (EA3Base **)members, (EA3Base **)(members + (int)flags.num) );

	// more than 128 members
	intptr_t size = (intptr_t)members[0];
	return EA3ConstIterator( (EA3Base**)(members + 1), (EA3Base**) (members + size + 1));
}

EA3Block::~EA3Block() {
  inheritsMap.erase(this);
  removeAllMembers();
}

// EA3Array //

EA3Array::EA3Array(EA3Block* con, EA3Grammar *g, int ind, const XbIdentifier &t, const XbString &nm) : EA3Block(con,g,ind,t,nm) {
}

EA3Array::~EA3Array() {
}

// EA3Grammar //

EA3Grammar::EA3Grammar(EA3Block *con, int ind, const XbIdentifier &t, const XbString &nm) : EA3Block(con,(EA3Grammar *)con,ind,t,nm) {
  gt = NULL;
}
EA3Grammar::~EA3Grammar() {
}


EA3Grammar *EA3Scene::findGrammar(const XbIdentifier &tp, const XbIdentifier &childType, bool is_array) const {
// no longer needed
  return NULL;
}

EA3Field *EA3Grammar::createFieldInstance(EA3Block *par,const XbIdentifier &tpin, const XbString &nm, int isGram,int ix) const {
  XbIdentifier tp=tpin;
  if (isGram) {
    tp = convertDataType(tpin);
    EA3Field *field = par->realCreateField(NULL,tp,tp,nm,ix);
    return(field);
  }
// creating a field in an array (field type must == grammar (of array) type)
  if (par->isArray() && tp.getString() == getName().getString()) {
    return(par->realCreateField(this,getType(),tp,nm,ix));
  }
  EA3GrammarTable *gtab=NULL;
  if (!isArray()) {
    if (!gt) gt = gtMap[this];
    gtab = gt;
  } else {
    gtab = gtMap[this];
  }
  //EA3GrammarTable *gt = gtMap[this];
  const EA3Base *sf = gtab? gtab->getGrammar(this,tpin,'F') : NULL;
  if (!sf) {
    // Create a user field if it has a '.' in name
    // Example: foo.ff -> creates a field of two floats
    const char *ext=strrchr(tp.getString(),'.');
    if (ext) {
      ext++;
      return(par->realCreateField(this,ext,tp,nm,ix));
    }
    EA3_ERROR3("could not create field type (%s) name (%s)",tp.getString(),nm.getString());
    EA3_ERROR3("  parent type (%s) isArray=%d",par->getType().getString(),par->isArray());
    return(NULL);
    //assert(gram && gram != this);
  }
  if (!sf) return(NULL);
  //assert(sf);
/*
  EA3Field *bsf = dynamic_cast<EA3_FIELD(XbString,1) *>(sf);
  if (!bsf) return(0);
  XbString str = "";
  for(int i = 0; i< bsf->getNum();i++) {
    str += bsf->get(i);
    str += " ";
  }
  const char *defaultValue = str.getString();
*/
  XbIdentifier baseType = sf->getType();
  EA3Field *field = par->realCreateField(sf,baseType,tp,nm,ix);
  //EA3Free(defaultValue);
  return(field);
}

EA3Block *EA3Grammar::createBlockInstance(EA3Block *par,const XbIdentifier &tp, const XbString &nm, int isGram,int ix) const {
  if (isGram) {
    EA3Grammar *gram = new EA3Grammar(par,par->getNumMembers(),tp,nm);
    addMember(par->flags,par->members,gram);
    gram->ref();
    return(gram);
  }
  EA3Base *g = info->getScene()->grammarTable->getGrammar(par,tp,'B');
  if (g) {
    return(par->realCreateBlock(g,tp,nm,ix));
  } 
  const char *ext=strrchr(tp.getString(),'.');
  if (ext) {
    return(par->realCreateBlock(this,tp,nm,ix));
  }
  EA3_ERROR3("could not create block type (%s) name (%s)",tp.getString(),nm.getString());
  EA3_ERROR3("  parent type (%s) isArray=%d",par->getType().getString(),par->isArray());
  return(NULL);
}

EA3Reference *EA3Grammar::createReferenceInstance(EA3Block *par,const XbIdentifier &tp, const XbString &nm, int isGram,int ix) const {
  TIME(IN,createReferenceInstance);
  if (isGram) {
    EA3Reference *rf = par->realCreateReference(this,tp,nm,ix);
    TIME(OUT,createReferenceInstance);
    return(rf);
  }
// creating a ref in an array (ref type must == grammar (of array) type)
  if (par->isArray() && tp == getName()) {
    TIME(OUT,createReferenceInstance);
    return(par->realCreateReference(this,tp,nm,ix));
  }
  EA3GrammarTable *gtab=NULL;
  if (!isArray()) {
    if (!gt) gt = gtMap[this];
    gtab = gt;
  } else {
    gtab = gtMap[this];
  }
  const EA3Base *b = gtab? gtab->getGrammar(this,tp,'R') : NULL;
  if (!b) {
    const char *ext=strrchr(tp.getString(),'.');
    if (ext) {
      return(par->realCreateReference(this,tp,nm,ix));
    }
    EA3_ERROR3("could not create reference type (%s) name (%s)",tp.getString(),nm.getString());
    EA3_ERROR3("  parent type (%s) isArray=%d",par->getType().getString(),par->isArray());
    return(NULL);
  }
  EA3Reference *nref = par->realCreateReference(b,tp,nm,ix);
  return(nref);
}

EA3Array *EA3Grammar::createArrayInstance(EA3Block *par,const XbIdentifier &tp, const XbString &nm, int isGram,int ix) const {
  if (isGram) {
    EA3Array *array = par->realCreateArray(this,convertDataType(tp),nm,ix);
    return(array);
  }
  EA3Base *g = info->getScene()->grammarTable->getGrammar(par,tp,'A');
  if (g) {
    return(par->realCreateArray(g,tp,nm,ix));
  } 
  const char *ext=strrchr(tp.getString(),'.');
  if (ext) {
    return(par->realCreateArray(this,tp,nm,ix));
  }
  return(NULL);
}
// EA3Scene //

EA3Scene::EA3Scene() : EA3Block(NULL,NULL,-1,"GLOBAL","") {
  curScene = this;
  grammarTable = NULL;
  gzr = NULL;
  fileSize = -1;
  filePtr = NULL;
  touch();
  version = "1.0";
  //_is = NULL;
  //sceneIs = NULL;
  grammarIndex = -1;
  useGrammar(EA3_grammar_str.num-1);
  uncontained = new EA3BaseSet;
}
void EA3Scene::grammarModified() {
  grammarIndex = -1;
}
void EA3Scene::rebuildGrammarTable() {
  delete grammarTable;
  grammarTable = new EA3GrammarTable(getGrammar()->toBlock());
}

int EA3Scene::useGrammar(int ix) {
  if (grammarIndex == ix) return(1);
  if (getNumMembers()!=0) {
    EA3_ERROR1("You can not replace the grammar if the scene has data in it!");
    return(0);
    //assert(getNumMembers()==0);
  }
  deletingGrammar=true;
  if(getGrammar()) getGrammar()->unref();
  deletingGrammar=false;
  EA3Grammar *g = new EA3Grammar(this,0,"Grammar","");
  g->ref();
  gInfoList.setGrammar(this,g);
  assert(ix<EA3_grammar_str.num);
  gzReader *lastGZR = gzr;
  gzr = new gzReader(EA3_grammar_str.data[ix]);
  //sceneIs = &gis;
  //fprintf(stderr,"reading grammar...\n");
  g->readMembers(NULL,1);
  delete gzr;
  g->clearReferences();
  EA3Reference::LRU.clear();
  //fprintf(stderr,"done reading grammar...\n");
  grammarIndex = ix;
  //sceneIs = lastSceneIs;
  gzr = lastGZR;
  rebuildGrammarTable();
//  EA3Free(buf);
  return(1);
}

void EA3Scene::getUncontained(XbPArray<EA3Base *> &unconList) const {
  EA3BaseSet &bs = *(EA3BaseSet*)uncontained;
  for(EA3BaseSet::iterator bsi = bs.begin();bsi!=bs.end();++bsi) {
    unconList.append(const_cast<EA3Base *>(*bsi));
  }
}

EA3Scene::~EA3Scene() {
  doingSceneDelete = true;
  setContainerCount = 0;
  
  removeAllMembers();
  int i;
  XbPArray<EA3Base *> garbage;
  getUncontained(garbage);
  for(i=0;i<garbage.getSize();i++) {
    garbage[i]->unref();
  }
  deletingGrammar=true;
  getGrammar()->unref();
  deletingGrammar=false;
  EA3BaseSet *uc = (EA3BaseSet*)uncontained;
  delete uc;
  doingSceneDelete = false;
  thisIsTheScene = true;
  delete grammarTable;
}

EA3Grammar *EA3Scene::newGrammar() {
  touch();
  if (getNumMembers()!=0) {
    EA3_ERROR1("You can not replace the grammar if the scene has data in it!");
    return(0);
    //assert(getNumMembers()==0);
  }
  grammarIndex = -1;
  EA3Base *g = getGrammar();
  deletingGrammar=true;
  g->unref();
  deletingGrammar=false;
  g = new EA3Grammar(this,0,"Grammar","");
  g->ref();
  gInfoList.setGrammar(this,g);
  return((EA3Grammar *)getGrammar());
}


int EA3Scene::lineCheck(char *) {
  return(0);
/*
  while(line.peek() == ' ') line.get();
  while(line.peek() == ';') return(0);
  while(line.peek() == 0) line.get();
  if(line.peek() == -1) return(0);
  if (line.eof()) return(0);
  if (!(line.peek() == '\r' || line.peek() == '\n')) {
    EA3Scene::readError(cerr << "extra characters after end of line. dec=" << line.peek() << " ascii = " << (char)line.peek());
    return(1);
  }
  return(0);
*/
}

char *EA3Scene::getLine() {
  static int lastNumDone = 0;
  lineNum++;
  if (progressCallback && fileSize > 0 && filePtr != 0) {
    int numDone = Ftell32(filePtr);
    if (numDone != lastNumDone) {
      (*progressCallback)(1,numDone,fileSize);
    }
    lastNumDone = numDone;
  }
  if (gzr) {
    return(gzr->getLine());
  }
  return(NULL);
}


gzWriter& EA3Scene::write(gzWriter& os) const {
  int mj,mn,in;
  getEA3Version(mj,mn,in);
  os << "#EA3 " << "1.0" << " (" << mj << "." << mn << "." << in << ") " << getEA3Label() << '\n';
  if (grammarIndex == -1) {
    os << *(getGrammar());
  } else {
    os << "UseGrammar " << grammarIndex << " [" << EA3_grammar_str.date[grammarIndex] << "]" << '\n';
  }
  EA3ConstIterator it = beginConst();
  EA3Base** end = it.end();
  for (;it!=end;++it) {
    os << *( (EA3Base*)*it);
  }
  return(os);
}

EA3Status EA3Scene::readFile(const char *filename) {
  return readInternal(filename);
}

EA3Status EA3Scene::readFile(char *buffer, int size) {
    return readInternal(buffer, size);
}


struct FlushData {
  FlushData() {
    gz = NULL;
    filePtr = NULL;
  };
  gzFile gz;
  FILE *filePtr;
};

void flushCB(XbPArray<char> &data,void *ud) {
  FlushData *fd = (FlushData *)ud;
  if (fd->gz) {
#if 0 // ndef PARAGON_STANDALONE
    gzwrite(fd->gz,const_cast<char *>(data.buffer),data.getSize());
#endif
  } else if (fd->filePtr) {
    fwrite(data.buffer,data.getSize(),1,fd->filePtr);
  }
}

EA3Status EA3Scene::writeFile(const char *filename,int comp) {
  resetUniqueIDs();
  FlushData fd;
  if (comp) {
#if 0 //ndef PARAGON_STANDALONE
    gzFile gz = gzopen(filename,"wb6");
    if (!gz) return(EA3CANT_OPEN);
    //EA3StreamBuf *easb = new EA3StreamBuf(0,gz);
    gzWriter out;
    out.flushCB = flushCB;
    out.CBdata = &fd;
    fd.gz = gz;
    write(out);
    //delete easb;
    gzwrite(gz,const_cast<char *>(out.str()),out.getLength());
    gzclose(gz);
    if (out.bad() || out.fail()) 
#endif
	    return(EA3ERROR_WRITING);
  } else {
#ifdef EA_PLATFORM_WINDOWS
    filePtr = fopen(filename,"wt");
#else
    filePtr = fopen(filename,"wb");
#endif
    if (!filePtr) return(EA3CANT_OPEN);
    //int fd = fileno(filePtr);
    gzWriter out;
    out.flushCB = flushCB;
    out.CBdata = &fd;
    fd.filePtr = filePtr;
    if (progressCallback) {
      writeBlockCount = 0;
      numWritenBlocks = 0;
      countWriteBlocks(this);
    }
    if (out.bad() || out.fail()) return(EA3ERROR_WRITING);
    write(out);
    if (out.bad() || out.fail()) return(EA3ERROR_WRITING);
    fwrite(out.str(),out.getLength(),1,filePtr);
    fclose(filePtr);
  }
  numWritenBlocks++;
  if (EA3Scene::progressCallback) {
    (*EA3Scene::progressCallback)(3,numWritenBlocks,writeBlockCount);
  }
  return(EA3OK);
}

int gz_magic[2] = {0x1f, 0x8b};


/*
EA3Status EA3Scene::writeFD(int fd,int comp) {
  if (comp) {
    gzFile gz = gzdopen(fd,"wb");
    if (!gz) return(EA3CANT_OPEN);
    //EA3StreamBuf *easb = new EA3StreamBuf(0,gz);
    gzWriter out;
    write(out);
    if (out.bad() || out.fail()) return(EA3ERROR_WRITING);
    //delete easb;
    gzclose(gz);
  } else {
    gzWriter out;
    if (out.bad() || out.fail()) return(EA3ERROR_WRITING);
    write(out);
    if (out.bad() || out.fail()) return(EA3ERROR_WRITING);
  }
  return(EA3OK);
}
*/
/*
istream& EA3Scene::readStream( istream& is ) {
  gzr = NULL;
  //sceneIs = &is;
  if (readAll() != EA3OK) {
    is.clear(ios::badbit|is.rdstate());
  }
  return(is);
}
*/
EA3Status EA3Scene::readAll() {
  touch();
  readStatus = EA3OK;
  lineNum = 0;
  char *line;
  line = getLine();
  char *c = line;
  if (strlen(c) < 5) return(EA3BAD_HEADER);
  if (*c!='#') return(EA3BAD_HEADER);
  ++c; if (*c!='E') return(EA3BAD_HEADER);
  ++c; if (*c!='A') return(EA3BAD_HEADER);
  ++c; if (*c!='3') return(EA3BAD_HEADER);
  ++c; if (*c!=' ') return(EA3BAD_HEADER);
  numReferences = 0;
  numResReferences = 0;
  EA3Reference::LRU.clear();
  if (version.read(c)) {
    //fprintf(stderr,"version %s\n",version.getString());
    readMembers(NULL,0);
  }
  //fprintf(stderr,"Resolve references...\n");
  // printf("LRU size = %d\n",EA3Reference::LRU.refHash.size());
  EA3Status res=resolveReferences();
  if (res!=EA3OK) readStatus = res;
  //fprintf(stderr,"Done resolve references.\n");
  clearReferences();
  return(readStatus);
}

void EA3Scene::appendHistory(const XbString &toolName,const XbString &comment) {
  const char *tmp;
  EA3Array *histA = (EA3Array *)getByType("history");
  if (!histA) {
    histA = createArray("history","");
  }
  EA3Block *hist = histA->createBlock("history","");
  if (!hist) return;
// EA3 Version
  char buf[1000];
  int mj,mn,in;
  getEA3Version(mj,mn,in);
  sprintf(buf,"%d.%d.%d (%s)",mj,mn,in,getEA3Label());
  EA3Field *fld = hist->createField("ea3version");
  if (fld) fld->data[0].str = buf;
// EA3 Grammar Version
  tmp = strrchr(ea3grammarVersion,'#');
  if( tmp )
  {
    tmp++;
    int gramVersion;
    sscanf(tmp,"%d",&gramVersion);
    if (grammarIndex != -1) {
      sprintf(buf,"%d.%d (%s)",grammarIndex,gramVersion,EA3_grammar_str.date[grammarIndex]);
      fld = hist->createField("ea3grammarVersion");
      if (fld) fld->data[0].str = buf;
    }
  }
// TOOL
  EA3Field *tool = hist->createField("tool","");
  if (tool) tool->data[0].str = (toolName.getString());
// DATE
  EA3Field *date = hist->createField("date","");
  time_t aclock;
  time(&aclock);
  if (date) date->data[0].str = (XbString(asctime(localtime(&aclock))).getString());
// USER
  EA3Field *user = hist->createField("user","");
#ifdef EA_PLATFORM_WINDOWS
  tmp = getenv("USER");
  if (!tmp) tmp = getenv("USERNAME");
  if (!tmp) tmp = "unknown";
  if (user) user->data[0].str = (XbString(tmp).getString());
#endif
// COMMENT
  EA3Field *com = hist->createField("comment","");
  if (com) com->data[0].str = comment;
}

EA3Selection::EA3Selection() {
  internals = new EA3BaseSet;
}
EA3Selection::~EA3Selection() {
  delete (EA3BaseSet*)internals;
}
void EA3Selection::select(const EA3Base *object) {
  if (!object) {
    return;
  }
  EA3BaseSet &selection = *(EA3BaseSet*)internals;
  selection.insert(object);
}
void EA3Selection::unselect(const EA3Base *object) {
  if (!object) {
    return;
  }
  EA3BaseSet &selection = *(EA3BaseSet*)internals;
  EA3BaseSet::iterator hi = selection.find(object);
  if(hi!=selection.end()) {
    selection.erase(hi);
  }
}
bool EA3Selection::isSelected(const EA3Base *object) {
  EA3BaseSet &selection = *(EA3BaseSet*)internals;
  EA3BaseSet::iterator hi = selection.find(object);
  return(hi!=selection.end());
}
void EA3Selection::getSelected(XbPArray<EA3Base *> &sel) {
  EA3BaseSet &selection = *(EA3BaseSet*)internals;
  for(EA3BaseSet::iterator hi=selection.begin();hi!=selection.end();++hi) {
    EA3Base *b = const_cast<EA3Base *>(*hi);
    sel.append(b);
  }
}
void EA3Selection::getSelected(XbPArray<EA3Base *> &sel,int &maxNum) {
  int numMax = maxNum;
  maxNum = 0;
  EA3BaseSet &selection = *(EA3BaseSet*)internals;
  for(EA3BaseSet::iterator hi=selection.begin();hi!=selection.end();++hi) {
    EA3Base *b = const_cast<EA3Base *>(*hi);
    sel.append(b);
    maxNum++;
    if (maxNum==numMax) return;
  }
}
unsigned int EA3Selection::numSelected() const {
  EA3BaseSet &selection = *(EA3BaseSet*)internals;
  return static_cast<unsigned int>(selection.size());
}
void EA3Selection::clear() {
  EA3BaseSet &selection = *(EA3BaseSet*)internals;
  selection.clear();
}

class EA3BinaryReadWrite {
public:
  struct Header {
    U32 tix,nix;
    U8 header;
    U8 isGrammar;
    Header() {
      tix=nix=0;
      header=0;
      isGrammar=0;
    }
    Header(const Header &other) {
      *this = other;
    }
    Header &operator=(const Header &other) {
      tix=other.tix;
      nix=other.nix;
      header=other.header;
      isGrammar=other.isGrammar;
      return *this;
    }
    bool operator==(const Header &other) const {
      return (tix==other.tix && nix==other.nix && header==other.header && isGrammar==other.isGrammar);
    }
    bool operator!=(const Header &other) const {
      return !(*this==other);
    }
  };
  EA3BinaryReadWrite() {
  }
  inline XbString readString(EA3Chunk &c) {
    XbPArray<char> tmp;
    char ch;
    do {
      ch = (char)c.getUChar();
      tmp.append(ch);
    } while(ch);
    XbString str(tmp.buffer);
    return str;
  }
  EA3Status write(EA3Chunk &chunk,const EA3Scene *s) {
    mScene = s;
    if (EA3Scene::progressCallback) {
      numWritenBlocks = 0;
      writeBlockCount = 0;
      countWriteBlocks(s);
    }
    fieldCount = 0;
    refCount = 0;
    blockCount = 0;
    fieldHCount = 0;
    refHCount = 0;
    blockHCount = 0;
    //printf("strings from info list...\n");
    StringMap types,names;
    gInfoList.getStrings(s,types,names);
    //printf("strings from scene\n");
    //sceneGetStrings(s);
    //printf("writing\n");
    const char *bea3 = "BEA3";
    chunk.append(bea3,4);
    chunk.append(2.0f);
    int numStrings = static_cast<int>(types.size()+names.size());
    
    int i;
    StringMap::iterator si;
    XbPArray<const char *> stmp;
    stmp.setSize(numStrings+1);
    strings[XbString("").getString()] = 1;
    stmp[0] = XbString("").getString();
    for(i=0,si=types.begin();!(si==types.end());++si,++i) {
      stmp[i+1] = (*si).first;
      strings[(*si).first] = i+2;
    }
    for(si=names.begin();!(si==names.end());++si) {
      if (strings.find((*si).first) != strings.end()) continue; // could have added it above
      stmp[i+1] = (*si).first;
      strings[(*si).first] = i+2;
      i++;
    }
    numStrings = i+1;
    chunk.append(numStrings);
    for(i=0;i<numStrings;i++) {
      chunk.append(stmp[i]);
    }
    chunk.append(s->getGrammarIndex());
    chunk.append(s->getNumMembers());
    writeMembers(chunk,s,false);
    //printf("size = %d\n",chunk.size);
    chunk.closeFile();
    if (EA3Scene::progressCallback) {
      (*EA3Scene::progressCallback)(3,writeBlockCount,writeBlockCount);
    }
    return(EA3OK);
  }
  EA3Status read(EA3Chunk &c,EA3Scene *s) {
    mScene = s;
    strings.clear();
    stringTable.setSize(0);
    if (c.getUChar() != (U8)'B') return(EA3BAD_HEADER);
    if (c.getUChar() != (U8)'E') return(EA3BAD_HEADER);
    if (c.getUChar() != (U8)'A') return(EA3BAD_HEADER);
    if (c.getUChar() != (U8)'3') return(EA3BAD_HEADER);
    if (c.getFloat() != 2.0) return(EA3BAD_HEADER);
    int numStrings = c.getU32();
    stringTable.append("BAD_INDEX");
    //strings.resize(numStrings*4);

	while(numStrings) {
      XbString st = readString(c);
      stringTable.append(st);
      strings[st.getString()]=0;
      numStrings--;
    }

    int grammarIndex = c.getInt();
    s->useGrammar(grammarIndex);

    U32 numMembers = c.getU32();
    readMembers(c,s,numMembers,false);

    for(int i=0;i<refsToResolve.getSize();i++) {
      refsToResolve[i]->resolveReferences();
    }

    //s->resolveReferences();
    s->clearReferences();

    return(EA3OK);
  }
  void writeMembers(EA3Chunk &c,const EA3Block *bl,bool isGrammar) {
    static int progressCount = 10001;
    if (EA3Scene::progressCallback) {
      numWritenBlocks++;
      progressCount++;
      if (progressCount > 10000) {
        (*EA3Scene::progressCallback)(3,numWritenBlocks,writeBlockCount);
        progressCount = 0;
      }
    }
    typedef enum {UNKNOWN,BLOCK,FIELD,REFERENCE} EA3Types;
    EA3Types lastEA3Type = UNKNOWN;
    bool doHeader = true;
    XbString lastName;
    XbString lastType;
    int i;
    int lastNumMembers = -1;
    int num = bl->getNumMembers();
    Header fieldHeader;
    Header nextHeader;
    // make sure that i+1 element is a field too so that we can use nextHeader
    // We like to cache the getHeader results, so that we don't always call it
    // twice, once for i and another for i+1.
    bool nextHeaderIsValid=false; 
    for(i=0;i<num;i++) {
      doHeader = true;
      EA3Base *b = bl->getByIndex(i);
      bool isLast = false;
      if (nextHeaderIsValid) {
        // the i+1'th element was a field last time, so we can use its header
        fieldHeader = nextHeader;
      } else {
        fieldHeader.isGrammar = isGrammar;
        getHeader(b,fieldHeader);
      }
      nextHeaderIsValid = false;
      if (i == num-1) {
        isLast = true;
      } else {
        EA3Base *b2 = bl->getByIndex(i+1);
        if (
          (b2->getName() != b->getName()) ||
          (b2->getType() != b->getType()) ||
          (b2->isField() && !b->isField()) ||
          (b2->isReference() && !b->isReference()) ||
          (b2->isBlock() && !b->isBlock()) ) {
          isLast = true;
        }
        
        if (b->isField() && b2->isField()) {
          nextHeader.isGrammar = isGrammar;
          getHeader(b2->toField(),nextHeader);
          nextHeaderIsValid = true;
          if (nextHeader != fieldHeader) {
            isLast = true;
          }
        }
      }
      if (b->isBlock()) {
        EA3Block *chbl = b->toBlock();
        if (lastType == b->getType() && lastName == b->getName() && lastEA3Type == BLOCK) doHeader = false;
        lastEA3Type = BLOCK;
        writeBlock(c,chbl,isLast,isGrammar,doHeader,lastNumMembers);
        lastNumMembers = chbl->getNumMembers();
      } else if (b->isField()) {
        if (lastType == b->getType() && lastName == b->getName() && lastEA3Type == FIELD) doHeader = false;
        writeField(c,b->toField(),isLast,doHeader,fieldHeader);
        if (isLast) {
          lastEA3Type = UNKNOWN;
        } else {
          lastEA3Type = FIELD;
        }
      } else if (b->isReference()) {
        //if (lastType == b->getType() && lastName == b->getName() && lastEA3Type == FIELD) doHeader = false;
        // Note: the above line has a bug: FIELD should have been REFERENCE
        // To fix the bug, we always do the header on reading and writing References.
        // This should be very rare anyway, since references in an array should
        // not have the same name and type.
        doHeader = true;
        writeReference(c,b->toReference(),isLast,doHeader);
        lastEA3Type = REFERENCE;
      } else {
        lastEA3Type = UNKNOWN;
      }
      lastType = b->getType();
      lastName = b->getName();
    }
  }
  inline void getFieldHeader(EA3Field *f,U8 &header,U32 &maxIx) {
    const char *dataTypes = f->getDataType();
    const char *et = f->getExtraTypes();
    {
      int ix = 0;
      StringHash::iterator sti,sti2;
      sti2 = sti = strings.find(dataTypes);
      if (et) sti2 = strings.find(et);
      if (sti==strings.end() || (et && sti2 == strings.end())) {
        header |= 8;
      } else {
        U32 dix = (*sti).second;
        if (dix > maxIx) maxIx = dix;
        if (et) {
          dix = (*sti2).second;
        }
        if (dix > maxIx) {
          maxIx = dix;
        }
        while(*dataTypes) {
          char dt = *dataTypes;
          if (dt=='s' || dt=='n'||dt=='d') {
            const char *str = f->data[ix].bindata;
            dix = 0;
            sti = strings.find(str);
            if (sti!=strings.end()) {
              dix = (*sti).second;
            }
            if (dix == 0) {
              header |= 8; // field strings not stored as STID's
            } else {
              maxIx = 0xffffff+1;
              //break;
            }
          }
          dataTypes++;
          ix++;
          if (*dataTypes == 0 && et) {
            dataTypes = et;
            et = NULL;
          }
        }
      }
    }
  }
  void getHeader(EA3Base *b,Header &h) {
    h.header=0;
    const char *type = b->getType().getString();
    const char *name = b->getName().getString();
    h.tix = strings[type];
    h.nix = 0;
    if (name[0]) h.nix = strings[name];
    U32 maxIx = h.tix;
    if (name[0] && h.nix > maxIx) maxIx=h.nix;
    if (b->isField()) {
      h.header = 0;
      getFieldHeader(b->toField(),h.header,maxIx);
    } else if(b->isReference()) {
      h.header = 1;
    } else if(h.isGrammar) {
      h.header = 2;
    } else if (b->isArray()) {
      h.header = 3;
    } else {
      h.header = 4;
    }
    if (maxIx > 0xffffff) {
      h.header |= (16|32);
      //printf("maxIx > 0xffffff\n");
    } else if (maxIx > 0xffff) {
      //printf("maxIx > 0xffff\n");
      h.header |= 32;
    } else if (maxIx > 0xff) {
      //printf("here %s (%s)\n",type,name);
      h.header |= 16;
    }
    if (name[0]==0) {
    } else if (name == type) {
      h.header |= 64;
    } else {
      h.header |= 128;
    }
  }
  inline U8 writeHeader(EA3Chunk &c,EA3Base *b,Header &h) {
    c.append(h.header);
    U8 numBytes = static_cast<U8>(((h.header & 48)>>4)+1);
    writeU32(c,h.tix,numBytes);
    if (h.header & 128) {
      writeU32(c,h.nix,numBytes);
    }
    return(h.header);
  }
  inline U8 writeHeader(EA3Chunk &c,EA3Base *b,bool isGrammar=false) {
    Header header;
    header.isGrammar = isGrammar;
    getHeader(b,header);
    return writeHeader(c,b,header);
  }
  inline U32 readU32(EA3Chunk &c,U8 numBytes) {
    if (numBytes == 1) {
      return c.getUChar();
    }
    if (numBytes == 2) {
      U32 val = c.getUChar();
      val |= (c.getUChar()<<8);
      return val;
    }
    if (numBytes == 3) {
      U32 val = c.getUChar();
      val |= (c.getUChar()<<8);
      val |= (c.getUChar() << 16);
      return val;
    }
    U32 val = c.getUChar();
    val |= (c.getUChar()<<8);
    val |= (c.getUChar() << 16);
    val |= (c.getUChar() << 24);
    return val;
  }
  inline void writeU32(EA3Chunk &c,U32 val,U8 numBytes) {
    if (numBytes == 1) {
      c.append((U8)(val&0xff));
      return;
    }
    if (numBytes == 2) {
      c.append((U8)(val&0xff));
      val = val >> 8;
      c.append((U8)(val&0xff));
      return;
    }
    if (numBytes == 3) {
      c.append((U8)(val&0xff));
      val = val >> 8;
      c.append((U8)(val&0xff));
      val = val >> 8;
      c.append((U8)(val&0xff));
      return;
    }
    c.append((U8)(val&0xff));
    val = val >> 8;
    c.append((U8)(val&0xff));
    val = val >> 8;
    c.append((U8)(val&0xff));
    val = val >> 8;
    c.append((U8)(val&0xff));
  }
  inline void read_progress() {
    if (!EA3Scene::progressCallback) return;
    static int lastNumDone = 0;
    static int pCount = 0;
    pCount ++;
    if (pCount > 10000) {
      pCount = 0;
    } else {
      return;
    }
    if (mScene->filePtr != 0) {
      int numDone = Ftell32(mScene->filePtr);
      if (numDone != lastNumDone) {
        (*EA3Scene::progressCallback)(1,numDone,mScene->fileSize);
      }
      lastNumDone = numDone;
    }
  }
  void readMembers(EA3Chunk &c,EA3Block *con,U32 numMembers,bool isGrammar) {
    //printf("reading members for %s %s\n",con->getType().getString(),con->getName().getString());
    bool doHeader = 1;
    int lastNumMembers = -1;
    XbIdentifier type;
    XbString name;
    bool isField = false;
    bool isGrammarBlock = false;
    bool isReference = false;
    bool isArray = false;
    bool isBlock = false;
    U8 header = 0;
    EA3Grammar *g = con ? (EA3Grammar *)con->getGrammar() : NULL;
    int ix=0;
    if (con && con->getContainer()) { // we do not want to do this to the scene, because we need to append members.
      setMemberSize(con->flags,con->members,numMembers);
      // we need to zero out members in case we read a reference that indexes into an un read block.
      if (numMembers == 1) {
        con->members=NULL;
      } else if (numMembers < 127) {
        memset(con->members,0,numMembers*sizeof(EA3Base*));
      } else {
        memset(&con->members[1],0,numMembers*sizeof(EA3Base*));
      }
    } else {
      ix=-1;
    }
    
    int orig = numMembers;
    while(numMembers) {  
      if (doHeader) {
        isField = false;
        isGrammar = false;
        isReference = false;
        isArray = false;
        isBlock = false;
        header = c.getUChar();
        U8 dType = static_cast<U8>(header & 7);
        if (dType == 0) {
          isField = true;
        } else if (dType == 1) {
          isReference = true;
        } else if (dType == 2) {
          isGrammarBlock = true;
        } else if (dType == 3) {
          isArray = true;
        } else if (dType == 4) {
          isBlock = true;
        } else {
          EA3_ERROR1("Unknown EA3Base type.  Is the binary data corrupt?  If in revision control, verify that the source file is stored as binary, not ASCII.  (Can not recover, exiting)\n");
          exit(-1);
        }
        U8 ixNumBytes = static_cast<U8>(((header & 48)>>4)+1);
        U32 tix = readU32(c,ixNumBytes);
        type = EA3Chunk::toIdentifier(stringTable[tix].getString());
        
        //type = stringTable[tix];
        if (header & 128) {
          U32 nix = readU32(c,ixNumBytes);
          name = stringTable[nix];
        } else if (header & 64) {
          name = type;
        } else {
          name = "";
        }
      }
      if (isField) {
        EA3Field *f = g? g->createFieldInstance(con,type,name,0,ix) : NULL;
        if (f && ix!=-1) ix++;
        doHeader = readField(c,f,header,doHeader);
        read_progress();
        numMembers--;
        continue;
      } else if (isReference) {
        EA3Reference *ref = g ? g->createReferenceInstance(con,type,name,0,ix) : NULL;
        if (ref && ix!=-1) ix++;
        doHeader = readReference(c,ref,header);
        doHeader = true;
      } else if (isBlock || isArray || isGrammar) {
        EA3Block *bl = NULL;
        if (isBlock || isGrammarBlock) {
          bl = g ? g->createBlockInstance(con,type,name,0,ix) : NULL;
          if (bl && ix!=-1) ix++;
        } else if (isArray) {
          bl = g ? g->createArrayInstance(con,type,name,0,ix) : NULL;
          if (bl && ix!=-1) ix++;
        }
        U8 flags = c.getUChar();
        if (flags & 2) {
          U32 uid = c.getU32();
          EA3Reference::LRU.add(bl,uid);
        }
        if (flags & 4) {
        } else {
          U8 numBytes = static_cast<U8>(((flags&24)>>3)+1);
          lastNumMembers = readU32(c,numBytes);
        }
        if (con) con->reindex(); 
        readMembers(c,bl,lastNumMembers,isGrammar);
        doHeader = flags & 1;
      }
      read_progress();
      numMembers--;
    }
    if (ix!=-1 && ix!=orig) {
      printf("%d members did not read\n",orig-ix);
    }
    if (con && ix!=-1) {
      setMemberSize(con->flags,con->members,ix);
      con->reindex();
    }
  }
  void writeBlock(EA3Chunk &c,EA3Block *bl,bool lastBlock,bool isGrammar,bool doHeader,int lastNumMembers) {
    blockCount++;
    if (doHeader) {
      blockHCount++;      
      writeHeader(c,bl,isGrammar);  
    }
    U8 flags=0;
    if (lastBlock) {
      flags |= 1; // this is the last block
    }
    if (bl->isArray() || bl->refCount() > 1) {
      flags |= 2; // hasUniqueID
    }
    int numMembers  = bl->getNumMembers();
    if (lastNumMembers == numMembers) {
      flags |= 4; // same num members as last block;
    }
    if (numMembers != lastNumMembers) {
      if (numMembers <= 0xff) {
      } else if (numMembers <= 0xffff) {
        flags |= 8;
      } else if (numMembers <= 0xffffff) {
        flags |= 16;
      } else {
        flags |= 24;
      }
    }
    c.append(flags);
    if (flags & 2) {
      c.append(getWriteId(bl)); // unique ID
    }
    if (numMembers != lastNumMembers) {
      U32 nm = numMembers;
      do {
        c.append((U8)(nm&0xff));
        nm = nm >> 8;
      } while(nm);
    }
    lastNumMembers = numMembers;
    writeMembers(c,bl,isGrammar);
  }
  void addString(const char *tmp) {
    XbString str(tmp);
    if (strings.find(str.getString())==strings.end()) {
      U32 ix = static_cast<U32>(strings.size()+1);
      strings[str.getString()] = ix;
    }
  }
  U8 writeField(EA3Chunk &c,EA3Field *f,bool &last,bool doHeader,Header &head) {
    fieldCount++;
    const char *dataTypes = f->getDataType();
    bool inLineStrings = (head.header & 8)?true:false;
    if (doHeader) {
      fieldHCount++;
      writeHeader(c,f,head);
      U8 ixNumBytes = static_cast<U8>(((head.header & 48)>>4)+1);
      inLineStrings = (head.header & 8)?true:false;
      if (inLineStrings) {
        c.append(dataTypes);
        addString(dataTypes);
      } else {
        writeU32(c,strings[XbString(dataTypes).getString()],ixNumBytes);
      }
    }
    U8 flags=0;
    const char *et = f->getExtraTypes();
    if (et) {
      flags |= 1;
    }
    
    if (last || inLineStrings) {
      flags |= 2;
      last = true; // indicate that we need to do the next header
    }
    if (f->refCount() > 1 && !f->getContainer()->isArray()) {
      flags |= 4;
    }
    c.append(flags);
    if (flags & 4) {
      c.append(getWriteId(f));
    }
    if (et) {
      U8 ixNumBytes = static_cast<U8>(((head.header & 48)>>4)+1);
      
      if (inLineStrings) {
        c.append(et);
        addString(et);
      } else {
        writeU32(c,strings[et],ixNumBytes);
      }
    }
    unsigned int i;
    if (dataTypes[0]=='b') {
      writeFieldValue(c,f,0,"b",0,head.header);
    } else {
      unsigned int num = f->getNum();
      for(i=0;i<num;i++) {
        writeFieldValue(c,f,i,dataTypes,0,head.header);
      }
      unsigned int numExtra = et ? (unsigned int)strlen(et) : 0;
      for(i=0;i<numExtra;i++) {
        writeFieldValue(c,f,i+num,et,num,head.header);
      }
    }
    return head.header;
  }
  
  bool readField(EA3Chunk &c,EA3Field *f,U8 header,bool doHeader) {
    U8 ixNumBytes = static_cast<U8>(((header & 48)>>4)+1);
    bool inLineStrings = (header & 8)?true:false;
    static const char *dt = NULL;
    if (doHeader) {
      if (inLineStrings) {
        XbString str = readString(c);
        if (strings.find(str.getString())==strings.end()) {
          strings[str.getString()] = 0;
          stringTable.append(str);
        }
        dt = str.getString();
      } else {
        dt = stringTable[readU32(c,ixNumBytes)].getString();
      }
    }
    U8 flags = c.getUChar();
    const char *et = NULL;
    if (flags & 4) {
      U32 uid = c.getU32();
      EA3Reference::LRU.add(f,uid);
    }
    if (flags & 1) {
      if (inLineStrings) {
        XbString str = readString(c);
        if (strings.find(str.getString())==strings.end()) {
          strings[str.getString()] = 0;
          stringTable.append(str);
        } 
        et = str.getString();
      } else {
        et = stringTable[readU32(c,ixNumBytes)].getString();
      }
      if (f) f->setExtraTypes(et);
    } 
    
    int i;
    const char *dtch = dt;
    if (*dtch == 'b') {
      int i0 = c.getInt();
      int i1 = c.getInt();
      if (f) {
        f->data[0].i = i0;
        f->data[1].i = i1;
        f->data[2].bindata = (char *)EA3Malloc(f->data[0].i);
      }
      for(i=0;i<i0;i++) {
        char ch = c.getUChar();
        if (f) f->data[2].bindata[i] = ch;
      }
      return(flags&2?true:false);
    }
    // most common case...
    if (!et && dtch[0] == 'f' && dtch[1] == 'f' && dtch[2] == 'f' && dtch[3] == 0) {
      U32 v1=c.getU32();
      U32 v2=c.getU32();
      U32 v3=c.getU32();
      if (f) {
        f->data[0].i = v1; 
        f->data[1].i = v2;
        f->data[2].i = v3; 
      }
      return(flags&2?true:false);
    }
    i = 0;
    while(*dtch) {
      switch(*dtch) {
      case 'f': 
      case 'i':
      case 'c': {
        U32 val = c.getU32();
        if (f) f->data[i].i = val; 
        break;
                }
      case 'n':
      case 'd': 
      case 's': {
        if (inLineStrings) {
          XbString str=readString(c);
          if (strings.find(str.getString())==strings.end()) {
            strings[str.getString()] = 0;
            stringTable.append(str);
          }
          if (f) f->data[i].str = str;
        } else {
          U32 ix = readU32(c,ixNumBytes);
          if (f) f->data[i].str = stringTable[ix]; 
        }
        break;
                }
      default: { 
        EA3_ERROR2("unknown field data type %c (can not recover, exiting)\n",*dtch);
        exit(-1);
               }
      }
      ++dtch;
      if (*dtch==0 && et) {
        dtch = et;
        et = NULL;
      }
      ++i;
    }
    return(flags&2?true:false);
  }
  EA3Status writeFieldValue(EA3Chunk &c,EA3Field *f,int i,const char *dt,int offset,U8 header) {
    bool inLineStrings = (header & 8)?true:false;
    U8 ixNumBytes = static_cast<U8>(((header & 48)>>4)+1);
    //const char *dt = f->getDataType();
    switch(dt[i-offset]) {
    case 'f': 
    case 'i':
    case 'c': c.append(f->data[i].i); break;
    case 'n':
    case 'd': 
    case 's': {
      if (!inLineStrings) {
        writeU32(c,strings[f->data[i].bindata],ixNumBytes);
      } else {
        const char *tmp = f->data[i].bindata;
        c.append(tmp);
        addString(tmp);
      }
      break;
              }
    case 'b': {
      c.append(f->data[0].i);
      c.append(f->data[1].i);
      for(int j=0;j<f->data[0].i;j++) {
        c.append((unsigned char)f->data[2].bindata[j]);
      }
      break;
              }
    default: { 
      ERROR_LINE(EA3SYNTAX_ERROR);
      EA3_ERROR2("unknown field data type %c\n",dt[i]);
      return(EA3UNKNOWN_TYPE);
             }
    }
    return(EA3OK);
  }
  void writeReference(EA3Chunk &c,EA3Reference *r,bool last,bool doHeader) {
    refCount++;
    if (doHeader) {
      refHCount++;
      writeHeader(c,r);
    }
    if (r->getNumReferences()==0) {
      c.append(U32(0));
      return;
    }
    //int i;
    EA3Block *lastCon = NULL;
    //int num = r->getNumReferences();
    bool wroteOne = false;
	EA3ConstIterator it = r->beginConst();
	EA3Base** end = it.end();
    for(;it!=end;) {
      EA3Base *b=*it;
	  ++it;
      //EA3Base *nextB = i==num-1 ? NULL : (EA3Base *)r->getReference(i+1);
	  EA3Base *nextB = it==end ? NULL : (EA3Base *)*it;
      EA3Block *con = b->getContainer();
      EA3Block *nextCon = nextB? nextB->getContainer() : NULL;
      if (!con) {
        continue;
      }
      wroteOne = true;
      if (!con->isArray()) {
        c.append(getWriteId(b));
      } else {
        if (lastCon != con) {
          lastCon = con;
          c.append(getWriteId(con));
        }
      }
      U32 ix = 0;
      U8 flags = 0;
      if (con->isArray()) {
        ix = b->getIndex();
      } else {
        flags |= 1; // reference is block itself
      }
      if (nextCon != con) {
        flags |= 2;
      }
      if (it==end) {
        flags |= 4;
      }
      if (last) {
        flags = static_cast<U8>(flags | 8);
      }
      if (!(flags&1)) {
        if (ix <= 0xff) {
        } else if (ix <= 0xffff) {
          flags |= 16;
        } else if (ix <= 0xffffff) {
          flags |= 32;
        } else {
          flags |= 48;
        }
      } 
      c.append(flags);
      if (!(flags&1)) {
        U8 numBytes = static_cast<U8>(((flags&48)>>4)+1);
        writeU32(c,ix,numBytes);
      }
    }
    if (!wroteOne) {
      c.append(U32(0));
    }
  }
  bool readReference(EA3Chunk &c,EA3Reference *ref,U8 header) {
    U8 flags = 10;
    XbRefName rn;
    EA3Base *b = NULL;
    bool didAdd = false;
    refList.setSize(0);
    do {
      if(flags & 3) {
        rn.id = c.getInt();
        b = NULL;
      }
      rn.childIndex = -1;
      if (rn.id == 0) {
        break;
      }
      flags = c.getUChar();
      
      if (flags & 1) {
        b = NULL;
      } else {
        U8 numBytes = static_cast<U8>(((flags&48)>>4)+1);
        rn.childIndex = readU32(c,numBytes);
      }
      
      if (!b) b = const_cast<EA3Base*>(EA3Reference::LRU.find(rn.id));
      if (b) {
        if (rn.childIndex!=-1) {
          EA3Block *bl=b->toBlock();
          EA3Base *ch = bl->getByIndex(rn.childIndex);
          if (ch) {
            refList.append(ch);
            if (ref) ch->ref();
          } else {
            refList.append((EA3Base*)(new XbRefName(rn)));
            EA3Scene::numReferences++;
            didAdd = true;
          }
        } else {
          refList.append(b);
          if (ref) b->ref();
        }
      } else {
        if (ref) {
          refList.append((EA3Base*)(new XbRefName(rn)));
          EA3Scene::numReferences++;
          didAdd = true;
        }
      }
    } while(rn.id && !(flags & 4));
    if (didAdd && ref) refsToResolve.append(ref);
    if (ref) {
      int num = refList.getSize();
      setMemberSize(ref->flags,ref->members,num);
      if (num == 1) {
        ref->members = (EA3Base **)refList[0];
      } else if (num < 127) {
        memcpy(ref->members,refList.buffer,num*sizeof(EA3Base*));
      } else {
        memcpy(&ref->members[1],refList.buffer,num*sizeof(EA3Base*));
      }
    }
    return(flags&8?true:false);
  }
  
  void stats() {
    printf("field count %d (%d headers)\n",fieldCount,fieldHCount);
    printf("ref   count %d (%d headers)\n",refCount,refHCount);
    printf("block count %d (%d headers)\n",blockCount,blockHCount);
  }
  StringHash strings;
  const EA3Scene *mScene;
  XbArray<XbString> stringTable;
  XbPArray<EA3Reference *> refsToResolve;
  XbPArray<EA3Base *> refList;
  int fieldCount;
  int refCount;
  int blockCount;
  int fieldHCount;
  int refHCount;
  int blockHCount;
}; // Class


EA3Status EA3Scene::writeBinaryFile(const char *filename,int comp) const {
  resetUniqueIDs();
  //int fd = fileno(f);
  bool DO_OLD_BINARY = false;
  if (!DO_OLD_BINARY) {
    if (comp) {
#if 0 //ndef PARAGON_STANDALONE
      gzFile gzf = gzopen(filename,"wb6");
      if (!gzf) return(EA3CANT_OPEN);
      EA3Chunk c(gzf,1000000,false);
      EA3BinaryReadWrite brw;
      EA3Status st = brw.write(c,this);
      return(st);
#else
			return (EA3CANT_OPEN);
#endif
    } else {
      FILE *fp = fopen(filename,"wb");
      if (!fp) return(EA3CANT_OPEN);
      EA3Chunk c(fp,1000000,false);
      EA3BinaryReadWrite brw;
      EA3Status st = brw.write(c,this);
      return(st);
    }
  } else {
    int initSize = 1000000; // 1 meg
    EA3Chunk chunk(initSize);
    const char *buf = "BEA3";
    chunk.appendHeader(buf,4);
    // Binary file version
    chunk.appendHeader(1.1f);
    chunk.appendHeader(version.getString());
    if (progressCallback) {
      writeBlockCount = 0;
      numWritenBlocks = 0;
      countWriteBlocks(this);
    }
    if (grammarIndex != -1) {
      chunk.append("USE\n",4);
      chunk.startBlock();
      chunk.append(grammarIndex);
      chunk.endBlock();
    } else {
      EA3Grammar *g = (EA3Grammar*)getGrammar();
      g->writeBinary(&chunk);
    }
    if (comp) {
#if 0 //ndef PARAGON_STANDALONE
      gzFile f = gzopen(filename,"wb6");
      writeBinary(&chunk);
      chunk.writeHeader(f);
      chunk.writeStringTable(f);
      chunk.write(f);
      gzclose(f);
#endif
    } else {
      FILE *f = fopen(filename,"wb");
      writeBinary(&chunk);
      chunk.writeHeader(f);
      chunk.writeStringTable(f);
      chunk.write(f);
      fclose(f);
    }
    if (progressCallback) {
      (*progressCallback)(3,writeBlockCount,writeBlockCount);
    }
  }

  return(EA3OK);
}


EA3Status EA3Scene::readInternal(const char *filename) {
  filePtr = fopen(filename,"rb");
  if (!filePtr) return(EA3CANT_OPEN);
  int fd = fileno(filePtr);
  if (fd < 0) return(EA3CANT_OPEN);
  numReferences = 0;
  numResReferences = 0;
  EA3Reference::LRU.clear();
  fileSize = -1;
  struct stat st;
  fstat(fd,&st);
  fileSize = (int)st.st_size;
  int isBinary = 0;
  EA3Status res = EA3OK;
  gzFile gz = NULL;
  wasBinary = false;
  wasCompressed = false;
  char buf[4];
  if (::read(fd,buf,4) != 4) {
    fclose(filePtr);
    return(EA3BAD_HEADER);
  }
  lseek(fd,0,SEEK_SET);
  if ((int)(unsigned char)buf[0] == gz_magic[0] && (int)(unsigned char)buf[1] == gz_magic[1]) {
    wasCompressed = true;
  }
  if (buf[0] == 'B' && 
    buf[1] == 'E' && 
    buf[2] == 'A' && 
    buf[3] == '3') {
    isBinary = 1;
  } else if(wasCompressed) {
#if 1 //ndef PARAGON_STANDALONE
    fclose(filePtr);
	filePtr = NULL;
    gz = gzopen(filename,"rb");
    if (!gz) {
      return(EA3CANT_OPEN);
    }
    gzread(gz,buf,4);
    if (buf[0] == 'B' &&
      buf[1] == 'E' &&
      buf[2] == 'A' &&
      buf[3] == '3') {
      isBinary = 1;
    }
    gzseek(gz,0,SEEK_SET);
#else
  return(EA3CANT_OPEN);
#endif
  }
  if (isBinary) {
    wasBinary = true;
    EA3Chunk chunk;
    if (wasCompressed) {
      
      chunk.init(gz,1000000,true);
    } else {
      chunk.init(filePtr,1000000,true);
    }
    binaryLen = fileSize;
    binaryStart = chunk.getCurrent();
    chunk.getInt(); // 4 bytes for header
    binVersion = chunk.getFloat();
    if (binVersion == 2.0) {
      EA3BinaryReadWrite brw;
      chunk.reset();
      EA3Status ea3st = brw.read(chunk,this);
	  if (filePtr) fclose(filePtr);
	  filePtr = NULL;
#if 1 //ndef PARAGON_STANDALONE
      if (gz) gzclose(gz);
#endif
      return(ea3st);
    } else {
      char *ver = chunk.getCurrent();
      unsigned char ch = 1;
      while(ch) ch = chunk.getUChar();
      //printf("EA3 Binary file.  Version (%f) %s\n",binVer,ver);
      version = ver;
      chunk.initStringTable();
      EA3Scene::numReferences=0;
      if (memcmp(chunk.getCurrent(),"USE\n",4)==0) {
        chunk.getInt(); // USE\n 
        chunk.getInt(); //chunk size
        useGrammar(chunk.getInt());
      } else if (memcmp(chunk.getCurrent(),"GRM\n",4)==0) {
        newGrammar();
        EA3Block *g = (EA3Block*)getGrammar();
        chunk.getInt(); // GRM\n
        //int chunkSize = chunk.getInt();
        //int id = chunk.getInt();
        //int typeIx = chunk.getInt();
        //int nameIx = chunk.getInt();
        chunk.getInt();
        chunk.getInt();
        chunk.getInt();
        chunk.getInt();
        res = g->readBinary(&chunk,1);
      }
      if (res == EA3OK) {
        chunk.getInt();
        //int chunkSize = chunk.getInt();
        //int id = chunk.getInt();
        //int typeIx = chunk.getInt();
        //int nameIx = chunk.getInt();
        chunk.getInt();
        chunk.getInt();
        chunk.getInt();
        chunk.getInt();
        res = readBinary(&chunk,0); 
        if (EA3Scene::progressCallback) {
          (*EA3Scene::progressCallback)(1,binaryLen,binaryLen);
        }
        //fprintf(stderr,"Resolve references...\n");
        // printf("LRU size = %d\n",EA3Reference::LRU.refHash.size());
        EA3Status res2=resolveReferences();
        if (res2!=EA3OK) {
          res = res2;
        }
        clearReferences();
        //fprintf(stderr,"Done resolve references.\n");
      }
    }
  } else {
    //sceneIs = NULL;
#if 1 //ndef PARAGON_STANDALONE
    if (!gz) {
	  if (filePtr) fclose(filePtr);
      filePtr = NULL;
      gz = gzopen(filename,"rb");
    }
    gzr = new gzReader(gz);

    res = readAll();
#else
		res = EA3CANT_OPEN;
#endif
  }
  //if (in.bad() || (in.fail() && !in.eof())) return(-2);
  //delete easb;
  delete gzr;
  gzr = NULL;
#if 1 //ndef PARAGON_STANDALONE
  if (gz) { 
    gzclose(gz);
  }
#endif
  if (filePtr) fclose(filePtr);
  filePtr = NULL;
  return(res);
}


EA3Status EA3Scene::readInternal(char *buffer, int size)
{
    numReferences = 0;
    numResReferences = 0;
    EA3Reference::LRU.clear();
    fileSize = size;
    int isBinary = 0;
    EA3Status res = EA3OK;
    gzFile gz = NULL;
    wasBinary = false;
    wasCompressed = false;
    char buf[4];
    if (size < 4) {
        return(EA3BAD_HEADER);
    }
    for (int i = 0; i < 4; i++) buf[i] = buffer[i];
    if ((int)(unsigned char)buf[0] == gz_magic[0] && (int)(unsigned char)buf[1] == gz_magic[1]) {
        wasCompressed = true;
    }
    if (buf[0] == 'B' && 
        buf[1] == 'E' && 
        buf[2] == 'A' && 
        buf[3] == '3') {
            isBinary = 1;
    } else if(wasCompressed) {
#if 1 //ndef PARAGON_STANDALONE
        /* TODO: allow compressed reading
        gz = gzopen(filename,"rb");
        if (!gz) {
            printf("readInternal: gzopen failed");
            return(EA3CANT_OPEN);
        }
        gzread(gz,buf,4);
        if (buf[0] == 'B' &&
            buf[1] == 'E' &&
            buf[2] == 'A' &&
            buf[3] == '3') {
                isBinary = 1;
        }
        gzseek(gz,0,SEEK_SET);
        */
        return(EA3CANT_OPEN);
#else
        return(EA3CANT_OPEN);
#endif
    }
    if (isBinary) {
        wasBinary = true;
        EA3Chunk chunk;
        if (wasCompressed) {
            chunk.init(gz,1000000,true);
        } else {
            chunk.init(buffer,1000000);
        }
        binaryLen = fileSize;
        binaryStart = chunk.getCurrent();
        chunk.getInt(); // 4 bytes for header
        binVersion = chunk.getFloat();
        if (binVersion == 2.0) {
            EA3BinaryReadWrite brw;
            chunk.reset();
            EA3Status ea3st = brw.read(chunk,this);
#if 1 //ndef PARAGON_STANDALONE
            //if (gz) gzclose(gz);
#endif
            return(ea3st);
        } else {
            char *ver = chunk.getCurrent();
            unsigned char ch = 1;
            while(ch) ch = chunk.getUChar();
            //printf("EA3 Binary file.  Version (%f) %s\n",binVer,ver);
            version = ver;
            chunk.initStringTable();
            EA3Scene::numReferences=0;
            if (memcmp(chunk.getCurrent(),"USE\n",4)==0) {
                chunk.getInt(); // USE\n 
                chunk.getInt(); //chunk size
                useGrammar(chunk.getInt());
            } else if (memcmp(chunk.getCurrent(),"GRM\n",4)==0) {
                newGrammar();
                EA3Block *g = (EA3Block*)getGrammar();
                chunk.getInt(); // GRM\n
                //int chunkSize = chunk.getInt();
                //int id = chunk.getInt();
                //int typeIx = chunk.getInt();
                //int nameIx = chunk.getInt();
                chunk.getInt();
                chunk.getInt();
                chunk.getInt();
                chunk.getInt();
                res = g->readBinary(&chunk,1);
            }
            if (res == EA3OK) {
                chunk.getInt();
                //int chunkSize = chunk.getInt();
                //int id = chunk.getInt();
                //int typeIx = chunk.getInt();
                //int nameIx = chunk.getInt();
                chunk.getInt();
                chunk.getInt();
                chunk.getInt();
                chunk.getInt();
                res = readBinary(&chunk,0); 
                if (EA3Scene::progressCallback) {
                    (*EA3Scene::progressCallback)(1,binaryLen,binaryLen);
                }
                //fprintf(stderr,"Resolve references...\n");
                // printf("LRU size = %d\n",EA3Reference::LRU.refHash.size());
                EA3Status res2=resolveReferences();
                if (res2!=EA3OK) {
                    res = res2;
                }
                clearReferences();
                //fprintf(stderr,"Done resolve references.\n");
            }
        }
    } else {
        //sceneIs = NULL;
#if 1 //ndef PARAGON_STANDALONE
        if (!gz) {
            //gz = gzopen(filename,"rb");
        }
        gzr = new gzReader(buffer);

        res = readAll();
#else
        res = EA3CANT_OPEN;
#endif
    }
    //if (in.bad() || (in.fail() && !in.eof())) return(-2);
    //delete easb;
    delete gzr;
    gzr = NULL;
#if 1 //ndef PARAGON_STANDALONE
    if (gz) { 
        //gzclose(gz);
    }
#endif
    return(res);
}

