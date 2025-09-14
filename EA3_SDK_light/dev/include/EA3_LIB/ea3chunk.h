// (c) 1998-2010 Electronic Arts Inc.

#ifndef EA3_CHUNK_H
#define EA3_CHUNK_H
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "XbPArray.h"
#include "EA3.h"

#include <xceedzip/zlib.h>

#define swap_word(a) ( ((a) << 24) | \
  (((a) << 8) & UINT32_C(0x00ff0000)) | \
  (((a) >> 8) & UINT32_C(0x0000ff00)) | \
  ((U32)(a) >> 24) )

#define EA3_BIG_ENDIAN 1
#define EA3_LITTLE_ENDIAN 0
typedef unsigned int U32;
typedef unsigned short U16;
class EA3_EXPORT EA3Chunk : public EA3Mem {
  class EA3_EXPORT StringTable {
  public:
    StringTable();
    int append(const XbString &str);
    XbString &get(int ix);
    XbIdentifier getIdentifier(int ix);
    ~StringTable();
    XbPArray<XbString *> data;
    void *internal;
  };
  public:
    EA3Chunk();
    EA3Chunk(int initSize);
    EA3Chunk(char *data,int size);
    EA3Chunk(FILE *f,int bufSize,bool doRead);
    EA3Chunk(gzFile gzf,int bufSize,bool doRead);
    void init(int initSize);
    void init(char *data,int size);
    void init(FILE *f,int bufSize,bool doRead);
    void init(gzFile gzf,int bufSize,bool doRead);
    void fillBuffer();
    void closeFile();
    static XbIdentifier &toIdentifier(const char *str) {
      static XbIdentifier x;
      x.data = const_cast<char *>(str); 
      return x;
    }
    ~EA3Chunk() {
      if (doFree) EA3Free(pStart);
    }
    void grow(U32 size);
    void appendHeader(const char *data,int len) {
      char *tmp = pCur;
      pCur = headerPtr;
      append(data,len);
      headerPtr = pCur;
      pCur = tmp;
    }
    void appendHeader(const char *data) {
      char *tmp = pCur;
      pCur = headerPtr;
      append(data);
      headerPtr = pCur;
      pCur = tmp;
    }
    void appendHeader(float data) {
      char *tmp = pCur;
      pCur = headerPtr;
      append(data);
      headerPtr = pCur;
      pCur = tmp;
    }
    void writeHeader(FILE *file) {
      unsigned int headerSize = (unsigned int)(headerPtr-header);
      fwrite(header,headerSize,1,file);
    }
    void writeHeader(gzFile file) {
#if 0 //ndef PARAGON_STANDALONE
      unsigned int headerSize = (unsigned int)(headerPtr-header);
      gzwrite(file,header,headerSize);
#endif
    }
    
    void writeStringTable(FILE *file) {
      int sz = stringTable.data.getSize();
      bswap(&sz);
      fwrite(&sz,sizeof(&sz),1,file);
      for(int i=0;i<stringTable.data.getSize();i++) {
        const char *str = stringTable.data[i]->getString();
        fwrite(str,strlen(str)+1,1,file);
      }
    }
    void writeStringTable(gzFile file) {
#if 0 //ndef PARAGON_STANDALONE
      int sz = stringTable.data.getSize();
      bswap(&sz);
      gzwrite(file,&sz,sizeof(&sz));
      for(int i=0;i<stringTable.data.getSize();i++) {
        const char *str = stringTable.data[i]->getString();
        gzwrite(file,const_cast<char *>(str),(unsigned int)strlen(str)+1);
      }
#endif
    }
    void initStringTable() {
      int num = getInt();
      XbPArray<char> tmp;
      for(int i=0;i<num;i++) {
        tmp.setSize(0);
        char ch;
        do {
          ch = getUChar();
          tmp.append(ch);
        } while(ch);
        stringTable.data.append(new XbString(tmp.buffer));
      }
    }
    static void bswap(void *data);
    static void bswap16(void *data);
    static void testBswap();
    void bswapAppend(U32 data);
    void append(float data);
    void append(int data);
    void append(U32 data);
    void append(unsigned char data);
    void append(const char *data,int len);
    void append(const char *data);
    inline float getFloat() {
      if (amountLeft < 10) {
        fillBuffer();
      }
      amountLeft -= 4;
      float val;
      memcpy(&val,pCur,sizeof(val));
      bswap(&val);
      pCur+=sizeof(val);
      return(val);
    }
    inline int getInt() {
      if (amountLeft < 10) {
        fillBuffer();
      }
      amountLeft -= 4;
      int val;
      memcpy(&val,pCur,sizeof(val));
      bswap(&val);
      pCur+=sizeof(val);
      return(val);
    }
    inline unsigned char getUChar() {
      if (amountLeft < 10) {
        fillBuffer();
      }
      amountLeft -= 1;
      unsigned char val = *pCur;
      pCur+=sizeof(val);
      return(val);
    }
    inline void reset() {
      pCur = pStart;
      amountLeft = maxsize;
    }
    char *getCurrent() const {
      return pCur;
    }
    char *getStart() const {
      return pStart;
    }
    void setCurrent(char *cur) {
      if (cur < pCur && cur >= pStart) {
        pCur = cur;
        amountLeft = maxsize - (int)(pCur-pStart);
        return;
      } else {
        int toRead = (int)(cur-pCur);
        while(toRead) {
          getUChar();
          --toRead;
        }
      }
    }
    inline U32 getU32() {
      if (amountLeft < 10) {
        fillBuffer();
      }
      amountLeft -= 4;
      U32 val;
      memcpy(&val,pCur,sizeof(val));
      bswap(&val);
      pCur+=sizeof(val);
      return(val);
    }
    inline U16 getU16() {
      if (amountLeft < 10) {
        fillBuffer();
      }
      amountLeft -= 2;
      U16 val;
      memcpy(&val,pCur,sizeof(val));
      bswap16(&val);
      pCur+=sizeof(val);
      return(val);
    }
    void startBlock();
    void endBlock();
    //    void append(EA3Header &data);
    //    void setHeaderVal(U32 sz,void *ptr);
    //    void finishBlock(EA3Block *block);
    //    void finish();
    //    void startBlock(EA3Block *block);
    void write(FILE *file);
    void write(gzFile file);
    
    
    static int endian;
    static int user_endian;
    StringTable stringTable;
    
  private:
    char header[10000];
    XbPArray<char *> ptrStack;
    bool doRead;
    bool doFree;
    bool doneReading;
    char *headerPtr;
    char *pStart;
    char *pCur;
    char *lastSubChunk;
    int size;
    int maxsize;
    int amountLeft;
    FILE *filePtr;
    gzFile gzPtr;
};

#endif
