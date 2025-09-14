// (c) 1999-2004 Electronic Arts Inc.

/* STL code can only be compiled at warning level 3 and even then some warnings need */
/* to be turned off */

#if defined(_MSC_VER)
#pragma once
#endif

#include <vector>

class DataTableIndex 
{
public:
    /* constructin\destruction */
    DataTableIndex(const void* pBuffer, int delimiter, int encoding, int rowComment);   
        /* pBuffer must be NULL terminated (if Unicode encoded, that means a 2 byte NULL) */
        /* delimiter must be one of k_DelimiterXXX */
        /* encoding must be one of k_EncodingXXX */
        /* rowComment must be a valid character or 0 for none */
    ~DataTableIndex(void) {}

    /* constants */
    enum {
        k_DelimiterComma = ',',
        k_DelimiterTab   = '\t',

        k_EncodingAscii   = 1,
        k_EncodingUnicode = 2,

        k_CharacterTypeNormal     = 0,
        k_CharacterTypeNewColumn  = 1,
        k_CharacterTypeNewRow     = 2,
        k_CharacterTypeEOF        = 3,
		k_CharacterTypeComment    = 4
    };

    /* attributes */
    int  getRowCount(void) const;
    int  getColumnCount(void) const;
    int  getCellOffset(int r, int c) const; /* return offset in bytes to specified row,column */
    int  getCellSize(int r, int c) const;   /* return size in bytes of data in specified row,column */
	int  getLineNumber(int row) const;		/* return line number for the given row */

    int  getDelimiter(void) const { return m_Delimiter; }
    int  getEncoding(void)  const { return m_Encoding; }

    int  getCharacter(int offset) const;    /* returns character code at the given offset */
    int  getCharacterType(int offset, bool lookForRowComments = false) const;    /* returns one of k_CharacterTypeXXX */
    int  getCharacterSize() const;          /* returns size of character at the given offset in bytes */

    const unsigned char*  getByteBuffer(void)    const { return static_cast<const unsigned char*>(m_pBuffer); }
    const char*           getAsciiBuffer(void)   const { return static_cast<const char*>(m_pBuffer); }
    const unsigned short* getUnicodeBuffer(void) const { return static_cast<const unsigned short*>(m_pBuffer); }

    /* operations */
    void print(void) const;
    void printSize(void) const;

private:
    /* implementation */
    typedef std::vector<int> IntVector;
    typedef std::vector<IntVector> IntTable;

    IntTable m_OffsetTable;     /* holds offset in bytes to every cell in the table */
    IntTable m_SizeTable;       /* holds size in in bytes to every cell in the table */
	IntVector m_LineTable;		/* holds line numbers to every row in the table */

    const void* m_pBuffer;
    int m_Delimiter;            /* one of k_DelimiterXXX */
    int m_Encoding;             /* one of k_EncodingXXX */
	int m_RowComment;			/* character that indicates that the row should be ignored */
	mutable int m_LineNumber;	/* current line number parse is working on */

    /* helper functions */
    void parse(void);
    void parseAscii(int offset = 0);
	bool isValidRow(IntVector& sizeRow) const;

    int  findCellSize(int offset) const;                    /* returns number of bytes that to cell's delimiter */
    int  findCellSize_CommaAscii(int offset) const;         /* handles Ascii CSV files */
    int  findCellSize_TabAsciiUnicode(int offset) const;    /* handles Ascii or Unicode tab delimited files */
};

