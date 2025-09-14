// (c) 1999-2004 Electronic Arts Inc.

#include "DataTableIndex.h"
#include <stdio.h>

#ifndef ASSERT
#define ASSERT  assert
#include <assert.h>
#endif

DataTableIndex::DataTableIndex(const void* pBuffer, int delimiter, int encoding, int rowComment) :
    m_pBuffer(pBuffer),
	m_LineNumber(0),
    m_Delimiter(delimiter),
    m_Encoding(encoding),
	m_RowComment(rowComment)
{
    // currently only specific encoding, delimiter formats are supported
    ASSERT((m_Delimiter == k_DelimiterComma && m_Encoding == k_EncodingAscii)
        || (m_Delimiter == k_DelimiterTab && m_Encoding == k_EncodingUnicode)
        || (m_Delimiter == k_DelimiterTab && m_Encoding == k_EncodingAscii));

    if (m_Encoding == k_EncodingUnicode)
    {
        parse();
    }
    else
    {
        parseAscii();
    }
}

int
DataTableIndex::findCellSize(int offset) const
{
    if (m_Delimiter == k_DelimiterComma && m_Encoding == k_EncodingAscii)
        return findCellSize_CommaAscii(offset);

    if (m_Delimiter == k_DelimiterTab && m_Encoding == k_EncodingAscii)
        return findCellSize_TabAsciiUnicode(offset);

    if (m_Delimiter == k_DelimiterTab && m_Encoding == k_EncodingUnicode)
        return findCellSize_TabAsciiUnicode(offset);

    ASSERT(0);
    return 0;
}

int
DataTableIndex::findCellSize_TabAsciiUnicode(int initialoffset) const
{
    // returns offset to next cell and size of current cell

    int offset = initialoffset;
    int chtype = getCharacterType(offset);
    while (chtype == k_CharacterTypeNormal)
    {
        offset += getCharacterSize();
        chtype  = getCharacterType(offset);
    }

    int size = offset-initialoffset;
    return size;
}

int
DataTableIndex::findCellSize_CommaAscii(int initialoffset) const
{
    // this function only works with 1 byte characters
    // returns offset to next cell and size of current cell

    int offset = initialoffset;
    int ch = getCharacter(offset);
    if (ch != '"')
    {
        // cell is not quoted (and will not have any comma's in it)
        int chtype = getCharacterType(offset);
        while (chtype == k_CharacterTypeNormal)
        {
            ++offset;
            chtype  = getCharacterType(offset);
        }
    }
    else // cell is quoted
    {
        // skip opening quote
        ++offset;

        // search for the closing quote (or end of line)
        int chtype = getCharacterType(offset);
        while ((chtype != k_CharacterTypeNewRow)
            && (chtype != k_CharacterTypeEOF))
        {
            ch = getCharacter(offset);
            if (ch == '"')
            {
                // skip this quote, we want to check the next quote
                ++offset;

                // if the next character is not a quote than this is the end of the string
                // else this is just a nested quote
                ch = getCharacter(offset);
                if (ch != '"')
                {
                    // this is not a nested quote so we must be at the end
                    break;
                }
            }

            ++offset;
            chtype = getCharacterType(offset);
        }
    }

    int size = offset-initialoffset;
    return size;
}

int
DataTableIndex::getCharacter(int offset) const
{
    const char* p = static_cast<const char*>(m_pBuffer) + offset;
    int ch = 0;
    
    if (m_Encoding == k_EncodingUnicode)
    {
        ch = *(reinterpret_cast<const wchar_t*>(p));
    }
    else
    {
        ch = *p;
    }

    return ch;
}

int
DataTableIndex::getCharacterType(int offset, bool lookForRowComments) const
{
    int ch = getCharacter(offset);

    if (ch == m_Delimiter)
        return k_CharacterTypeNewColumn;

    if ((ch == 0x0a) || (ch == 0x0d))
	{
		if (ch == 0x0a)
		{
			++m_LineNumber;
		}
        return k_CharacterTypeNewRow;
	}

    if (ch == 0)
        return k_CharacterTypeEOF;

	if (lookForRowComments)
	{
		if (ch == ';')
			return k_CharacterTypeComment;

		// Excel will quote a comment line if there are spaces in the comment when save a .csv
		if (m_Delimiter == k_DelimiterComma)
		{
			if (ch == '"' && getCharacter(offset+1) == ';')
				return k_CharacterTypeComment;
		}
	}

    return k_CharacterTypeNormal;
}

int  
DataTableIndex::getCharacterSize() const
{
    if (m_Encoding == k_EncodingUnicode)
    {
        return 2;
    }
    return 1;
}

void
DataTableIndex::parse(void)
{
    int offset = 0;

    // Unicode text files often have a 0xfffe character as the first 2 bytes
    // I think this indicates the text direction.  In anycase we should ignore it
    const unsigned char* p = static_cast<const unsigned char*>(m_pBuffer);
    if (p[0] == 0xff && p[1] == 0xfe)
    {
        offset += 2;
    }

    parseAscii(offset);
}

bool
DataTableIndex::isValidRow(IntVector& sizeRow) const
{
	bool ok = false;
	for (IntVector::iterator i = sizeRow.begin(); i != sizeRow.end(); ++i)
	{
		if (*i > 0)
		{
			ok = true;
			break;
		}
	}
	return ok;
}

void
DataTableIndex::parseAscii(int offset)
{
    m_OffsetTable.clear();
    m_SizeTable.clear();
	m_LineTable.clear();

    std::vector<int> offsetRow;
    std::vector<int> sizeRow;

    unsigned int columnCount = 0;
	bool startOfLine = true;
	m_LineNumber = 1;

    int chtype = getCharacterType(offset, true);
    while (chtype != k_CharacterTypeEOF)
    {
		// skip over commented out rows
		if (startOfLine && chtype == k_CharacterTypeComment)
		{
			while (chtype != k_CharacterTypeNewRow)
			{
				offset += getCharacterSize();
				chtype  = getCharacterType(offset);
			}
			while (chtype == k_CharacterTypeNewRow)
			{
				offset += getCharacterSize();
				chtype  = getCharacterType(offset, true);
			}
		}
		else
		{
			startOfLine = false;

			int lineNumber = m_LineNumber;

			// save cell information
			int size = findCellSize(offset);
			ASSERT(offset >= 0 && offset < 10000000);
			ASSERT(size >= 0 && size < 10000);
			offsetRow.push_back(offset);
			sizeRow.push_back(size);

			// advance to cell delimiter
			offset += size;
			chtype = getCharacterType(offset, true);

			// skip over delimiters
			if (chtype == k_CharacterTypeNewColumn)
			{
				offset += getCharacterSize();
				chtype  = getCharacterType(offset);
			}
			else if (chtype == k_CharacterTypeNewRow)
			{
				// skip over blank rows and handle any type of EOL character (pc,mac,unix)
				while (chtype == k_CharacterTypeNewRow)
				{
					offset += getCharacterSize();
					chtype  = getCharacterType(offset, true);
				}

				if (columnCount > 0)
				{
					// make sure
					while (offsetRow.size() < columnCount)
					{
						offsetRow.push_back(0);
						sizeRow.push_back(0);
					}
				}
				else
				{
					columnCount = offsetRow.size();
				}

				// only add rows that have at least ONE item in them (ie sizeRow has one non-zero value
				if (isValidRow(sizeRow))
				{
					m_OffsetTable.push_back(offsetRow);
					m_SizeTable.push_back(sizeRow);
					m_LineTable.push_back(lineNumber);
				}
				sizeRow.clear();
				offsetRow.clear();

				startOfLine = true;
			}
		}
    }
}

int  
DataTableIndex::getLineNumber(int row) const
{
	if (row >= 0 && (unsigned int)row < m_LineTable.size())
	{
		return m_LineTable[row];
	}
	else
	{
		return -1;
	}
}

int 
DataTableIndex::getRowCount(void) const
{
    return m_OffsetTable.size();
}

int 
DataTableIndex::getColumnCount(void) const
{
    return m_OffsetTable[0].size();
}

int 
DataTableIndex::getCellOffset(int r, int c) const
{
#ifdef _DEBUG
	if((unsigned int)r >= m_OffsetTable.size() ||
        (unsigned int)c >= m_OffsetTable[r].size())
	{
		this->printSize();
	}
#endif
    ASSERT((unsigned int)r < m_OffsetTable.size());
    ASSERT((unsigned int)c < m_OffsetTable[r].size());

    return m_OffsetTable[r][c];
}

int 
DataTableIndex::getCellSize(int r, int c) const
{
#ifdef _DEBUG
	if((unsigned int)r >= m_SizeTable.size() ||
        (unsigned int)c >= m_SizeTable[r].size())
	{
		this->printSize();
	}
#endif
    ASSERT((unsigned int)r < m_SizeTable.size());
    ASSERT((unsigned int)c < m_SizeTable[r].size());

    return m_SizeTable[r][c];
}

void
DataTableIndex::print(void) const
{
    const int k_MaxCellSize = 4094;

    const char* pBuffer = static_cast<const char*>(m_pBuffer);

    int rowCount    = getRowCount();
    int columnCount = getColumnCount();

    printf("\n");
    for (int r = 0; r < rowCount; ++r)
    {
        for (int c = 0; c < columnCount; ++c)
        {
            int offset = getCellOffset(r,c);
            int size   = getCellSize(r,c);
            ASSERT(offset >= 0 && offset < 10000000);
            ASSERT(size >= 0 && size < 10000);

            if (size >= k_MaxCellSize)
                size = k_MaxCellSize;

            char cstr[k_MaxCellSize+1];
            if (getEncoding() == k_EncodingAscii)
            {
                memcpy(cstr, getByteBuffer()+offset, size);
                cstr[size] = 0;
            }
            else
            {
                unsigned short str[k_MaxCellSize+1];
                memcpy(str, pBuffer+offset, size);
                str[size] = 0;

                // convert Unicode string into Ascii string
				int i;
                for (i = 0; i < size/2; ++i)
                {
                    unsigned short ch = str[i];
                    if (ch < 256)
                        cstr[i] = static_cast<char>(ch);
                    else
                        cstr[i] = '#';
                }
                cstr[i] = 0;
            }
    
            printf("%s\n", cstr);
        }
        printf("\n");
    }
}

void
DataTableIndex::printSize(void) const
{
	// rcg, apr 5/01: Report how big we are.
    int rowCount    = getRowCount();
    int columnCount = getColumnCount();

	::printf("\nDataTableIndex size: %d rows x %d columns\n", rowCount, columnCount);
	::printf("\tOffset table size: %d\n", m_OffsetTable.size());
	::printf("\tSize table size: %d\n", m_SizeTable.size());
}
