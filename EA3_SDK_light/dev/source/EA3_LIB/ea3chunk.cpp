// (c) 1998-2010 Electronic Arts Inc.

#include <eabase/eabase.h>
#include "ea3chunk.h"
#include "ea3mem.h"
#include <eastl/hash_map.h>
using namespace eastl;

/* MB: Moved to EA3.h
template <class TYPE>
struct myhash : public unary_function<TYPE,size_t>  {
  size_t operator()(TYPE b) const {
    return((size_t)b);
  }
};
typedef hash_map< const char *,unsigned int,myhash<const char *>,equal_to<const char *>, StlAllocator > StringHash;
*/

EA3Chunk::StringTable::StringTable() {
  internal = new StringHash;
}

int EA3Chunk::StringTable::append(const XbString &str) {
  StringHash &strHash = *(StringHash*)internal;
  StringHash::iterator si = strHash.find(str.getString());
  if (si==strHash.end()) {
    int ix = data.getSize();
    data.append(new XbString(str));
    strHash[str.getString()]=ix;
  } else {
    return (*si).second;
  }
  XbString *tmp = new XbString(str);
  data.append(tmp);
  return(data.getSize()-1);
}
XbString &EA3Chunk::StringTable::get(int ix) {
  return(*data[ix]);
}
XbIdentifier EA3Chunk::StringTable::getIdentifier(int ix) {
		return(EA3Chunk::toIdentifier(data[ix]->getString()));
}
EA3Chunk::StringTable::~StringTable() {
  StringHash *strHash = (StringHash*)internal;
  delete strHash;
  for(int i=0;i<data.getSize();i++) {
    delete data[i];
  }
}
EA3Chunk::EA3Chunk() {
  pCur = pStart = NULL;
  doFree = false;
  size = 0;
  maxsize = 0;
  lastSubChunk = NULL;
  headerPtr = header;
  filePtr = NULL;
  gzPtr = NULL;
  doneReading = false;
  amountLeft = maxsize - (int)(pCur-pStart);
}
EA3Chunk::EA3Chunk(int initSize) {
  init(initSize);
}
void EA3Chunk::init(int initSize) {
  doneReading = false;
  pCur = pStart = (char *) EA3Malloc(initSize);
  doFree = true;
  size = 0;
  maxsize = initSize;
  lastSubChunk = NULL;
  headerPtr = header;
  filePtr = NULL;
  gzPtr = NULL;
  amountLeft = maxsize - (int)(pCur-pStart);
}
EA3Chunk::EA3Chunk(char *data,int initSize) {
  init(data,initSize);
}
void EA3Chunk::init(char *data,int initSize) {
  doneReading = false;
  pCur = pStart = data;
  doFree = false;
  size = 0;
  maxsize = initSize;
  lastSubChunk = NULL;
  headerPtr = header;
  filePtr = NULL;
  gzPtr = NULL;
  amountLeft = maxsize - (int)(pCur-pStart);
}
EA3Chunk::EA3Chunk(FILE *f,int initSize,bool do_read) {
  init(f,initSize,do_read);
}
void EA3Chunk::init(FILE *f,int initSize,bool do_read) {
  doneReading = false;
  doRead = do_read;
  gzPtr = NULL;
  filePtr = f;
  pCur = pStart = (char *)EA3Malloc(initSize);
  doFree = true;
  if (doRead) maxsize=(int)fread(pStart,1,initSize,filePtr);
  else maxsize = initSize;
  size = 0;
  lastSubChunk = NULL;
  headerPtr = header;
  amountLeft = maxsize - (int)(pCur-pStart);
}
EA3Chunk::EA3Chunk(gzFile gzf,int initSize,bool do_read) {
  init(gzf,initSize,do_read);
}
void EA3Chunk::init(gzFile gzf,int initSize,bool do_read) {
#if 1 //ndef PARAGON_STANDALONE
  doneReading = false;
  doRead = do_read;
  filePtr = NULL;
  gzPtr = gzf;
  pCur = pStart = (char *)EA3Malloc(initSize);
  doFree = true;
  if (doRead) maxsize = gzread(gzPtr,pStart,initSize);
  else maxsize = initSize;
  size = 0;
  lastSubChunk = NULL;
  headerPtr = header;
  amountLeft = maxsize - (int)(pCur-pStart);
#endif
}
void EA3Chunk::grow(U32 amount) {
  int nsize = size+amount;
  if (nsize < maxsize) {
    size = nsize;
    return;
  }
  if (ptrStack.getSize()==0) {
    if (filePtr || gzPtr) {
      if (filePtr) {
        fwrite(pStart,size,1,filePtr);
      } else {
#if 0 //ndef PARAGON_STANDALONE
        gzwrite(gzPtr,pStart,size);
#endif
      }
      if ((int)amount > maxsize) {
        maxsize = amount*2;
        pStart = (char *)EA3Realloc(pStart,maxsize);
      }
      size = amount;
      pCur = pStart;
      return;
    }
  }
  size = nsize;
  char *npStart = pStart;
  //printf("GROW.\n");
  while (size >= maxsize) {
    maxsize *= 2;
    npStart = (char *)EA3Realloc(npStart,maxsize);
  }
  intptr_t delta = (intptr_t)npStart-(intptr_t)pStart;
  for(int i=0;i<ptrStack.getSize();i++) {
    ptrStack[i] = ptrStack[i] + delta;
  }
  pStart = pStart + delta;
  pCur = pCur + delta;
}
void EA3Chunk::bswap16(void *data) {
  if (user_endian == endian) return;
  U16 res;
  memcpy(&res,data,sizeof(res));
  res = static_cast<U16>(((res << 8)&0xff00)|(res>>8));
  *(U16 *)data = res;
}
void EA3Chunk::bswap(void *data) {
  if (user_endian == endian) return;
  U32 res;
  memcpy(&res, data, sizeof(U32));
  res = swap_word(res);
  *(U32 *)data = res;
}
void EA3Chunk::bswapAppend(U32 data) {
  bswap(&data);
  memcpy(pCur,(void *)(&data),sizeof(U32));
  pCur += sizeof(U32);
}
void EA3Chunk::append(float data) {
  //printf("APPEND float %f\n",data);
  grow(sizeof(data));
  U32 data32;
  memcpy(&data32, &data, sizeof(U32));
  bswapAppend(data32);
}
void EA3Chunk::append(int data) {
  //printf("APPEND int %d\n",data);
  grow(sizeof(data));
  U32 data32;
  memcpy(&data32, &data, sizeof(U32));
  bswapAppend(data32);
}
void EA3Chunk::append(unsigned char data) {
  //printf("APPEND int %d\n",data);
  grow(sizeof(data));
  *pCur = data;
  pCur += sizeof(data);
}
void EA3Chunk::append(U32 data) {
  append((int)data);
}
void EA3Chunk::append(const char *data,int len) {
  //printf("APPEND char[%d]=%s\n",len,data);
  grow(len);
  memcpy(pCur,data,len);
  pCur += len;
  //assert(len%4==0);
}
void EA3Chunk::append(const char *data) {
  //printf("APPEND char*\n");
  int len = (int)strlen(data)+1;
  //int l = (len)+(4-((len)%4));
  //assert(l%4==0);
  grow(len);
  memcpy(pCur,data,len);
  pCur += len;
}

void EA3Chunk::fillBuffer() {
  if (!filePtr && !gzPtr) return;
  if (doneReading) return;
  if (amountLeft < 10) {
    int i;
    for(i=0;i<amountLeft;i++) {
      pStart[i] = pCur[i];
    }
    pCur = pStart;
    char *tmp = pStart + amountLeft;
    int toRead = maxsize-amountLeft;
    if (filePtr) {
      maxsize = (int)fread(tmp,1,toRead,filePtr);
      if (maxsize<toRead) doneReading = true;
    } else {
#if 1 //ndef PARAGON_STANDALONE
      maxsize = gzread(gzPtr,tmp,toRead);
#endif
      if (maxsize<toRead) doneReading = true;
    }
    maxsize = maxsize+amountLeft;
    amountLeft = maxsize;
  }
}

void EA3Chunk::startBlock() {
// Push the current pointer on the stack
  ptrStack.append(pCur);
  //printf("start %x\n",pCur);
// space for chunk size.  To be filled in by endBlock
  append(0xabcddcba);  // chances are if this value is in the file, something has gone wrong!
/*
  if (lastSubChunk) {
    unsigned int sz = (unsigned int)pCur-(unsigned int)lastSubChunk-sizeof(EA3Header);
    char *tmp = lastSubChunk+((unsigned long)&h.subChunkOffset-(unsigned long)&h);
    setHeaderVal(sz,tmp);
    //printf("last sub size = %d\n",sz);
  }
  block->userData = pCur;
  lastSubChunk = pCur;
  append(h);
*/
}
void EA3Chunk::endBlock() {
// current pointer
  char *lpCur = pCur;
// pop address to chunk size of stack
  pCur = ptrStack[ptrStack.getSize()-1];
  ptrStack.setSize(ptrStack.getSize()-1);
  uintptr_t sz = (uintptr_t)lpCur-(uintptr_t)pCur;
  //printf("end %x size = %x\n",pCur,sz);
  char *lstart = pStart;
  append((U32)sz);
  char *nstart = pStart;
  int delta = (int)(nstart-lstart);
  pCur = lpCur+delta;
}

void EA3Chunk::closeFile() {
  if (filePtr || gzPtr) {
    if (filePtr) {
      if (!doRead) fwrite(pStart,size,1,filePtr);
      fclose(filePtr);
    } else {
#if 0 //ndef PARAGON_STANDALONE
      if (!doRead) gzwrite(gzPtr,pStart,size);
      gzclose(gzPtr);
#endif
    }
  }
}
void EA3Chunk::write(FILE *file) {
  //finish();
  //printf("writing %d bytes\n",size);
  fwrite(pStart,1,size,file);
}
void EA3Chunk::write(gzFile file) {
  //finish();
  //printf("writing %d bytes\n",size);
#if 0 //ndef PARAGON_STANDALONE
  gzwrite(file,pStart,size);
#endif
}
#if (defined(EA_PLATFORM_WINDOWS) || defined(LINUX))
int EA3Chunk::endian = EA3_LITTLE_ENDIAN;
int EA3Chunk::user_endian = EA3_LITTLE_ENDIAN;
#else
int EA3Chunk::endian = EA3_LITTLE_ENDIAN;
int EA3Chunk::user_endian = EA3_BIG_ENDIAN;
#endif

