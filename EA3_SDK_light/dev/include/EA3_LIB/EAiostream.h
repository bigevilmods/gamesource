// (c) 1998-2010 Electronic Arts Inc.

#ifndef _EAIOSTREAM_H
#define _EAIOSTREAM_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "XbArray.h"

namespace EAios {
	const unsigned int failbit = 1;
	const unsigned int eofbit = 2;
};
class EAostream
{
public:
    EAostream() : m_ok(true)	{}

    void append(const char *s) {
	while(*s) {
	  m_data.append(*s);
	  s++;
	}
    }

    const char *str() const		{ return(m_data.buffer); }
    const char *stringBuffer() const	{ return(m_data.buffer); }
    int getLength() const		{ return m_data.getSize(); }

    bool fail() const			{ return (!m_ok); }
    bool bad() const			{ return(!m_ok); }

    EAostream &operator<<(int i) {
	char buf[20];
	sprintf(buf,"%d",i);
	append(buf);
	return(*this);
    }

    EAostream &operator<<(unsigned int i) {
	char buf[20];
	sprintf(buf,"%x",i);
	append(buf);
	return(*this);
    }

    EAostream &operator<<(float f) {
	char buf[30];
	sprintf(buf,"%f",f);
	append(buf);
	return(*this);
    }

    EAostream &operator<<(double d) {
	char buf[30];
	sprintf(buf,"%f",d);
	append(buf);
	return(*this);
    };

    EAostream &operator<<(const char *s) {
	append(s);
	return(*this);
    }

    EAostream &operator<<(char c) {
	m_data.append(c);
	return(*this);
    }

    EAostream &operator<< (EAostream& (*func)(EAostream&)) {
	return (*func)(*this);
    }

private:
    bool		m_ok;
    XbArray<char>	m_data;
};

inline EAostream& endl( EAostream& os )
{
    os << "\n";
    return os;
}

class EAistream
{
public:
    EAistream() : m_pos(0), m_ok(true), m_eof(false)	{}

    void append(const char *s) {
	while(*s) {
	  m_data.append(*s);
	  s++;
	}
    }

    void append(const char *s, int length) {
	while(length > 0) {
	  m_data.append(*s);
	  s++;
	  length--;
	}
    }

    const char *str() const	{ return(m_data.buffer); }
    int getLength() const	{ return m_data.getSize(); }

    bool fail() const		{ return (!m_ok); }
    bool bad() const		{ return(!m_ok); }
	void clear(unsigned int bits) {
    if (bits&1) m_ok = true;
    if (bits&2) m_eof = false;
	}
	void skipWhitespace() {
    bool more = true;
    do {
      char c = peek();
      if (isspace((int)(unsigned char)c)) {
        get();
      } else {
        more = false;
      }
    }
    while(more);
  }
   
    char get() {
	char c = m_data[m_pos++];
	if( m_pos >= m_data.getSize() ) m_eof = true;
	return(c);
    }

    char peek() {
      char c = m_data[m_pos];
      return(c);
    }

    bool eof() {
      return(m_eof);
    }

    EAistream &operator>>(int &i) {
	// skip initial whitespaces
	while( isspace( (int)(unsigned char)m_data[m_pos] ) && m_pos < m_data.getSize() )
	    m_pos++;

	if( m_pos >= m_data.getSize() )
	{
	    m_eof = true;
	    return *this;
	}

	int stringIdx = 0;
	char string[1024];

	// parse minus or plus sign, if it exists
	if( isdigit( (int)(unsigned char)m_data[m_pos] ) ||
	    m_data[m_pos] == '-' ||
	    m_data[m_pos] == '+' )
	{
	    string[stringIdx++] = m_data[m_pos++];
	}

	// parse integer number
	while( !isspace( (int)(unsigned char)m_data[m_pos] ) &&
	       isdigit( (int)(unsigned char)m_data[m_pos] ) &&
	       m_pos < m_data.getSize() )
	{
	    string[stringIdx++] = m_data[m_pos++];
	}

	string[stringIdx++] = '\0';
	i = atoi( string );

	// if no integer can be read
	if( string[0] == '\0' ) m_ok = false;

	// reach end of file yet?
	if( m_pos >= m_data.getSize() ) m_eof = true;

	return *this;
    }

    EAistream &operator>>(float &f) {
	// skip initial whitespaces
	while( isspace( (int)(unsigned char)m_data[m_pos] ) && m_pos < m_data.getSize() )
	    m_pos++;

	if( m_pos >= m_data.getSize() )
	{
	    m_eof = true;
	    return *this;
	}

	int stringIdx = 0;
	bool periodFound = false;
	char string[1024];

	// parse minus, plus sign, or decimal point, if they exist
	if( isdigit( (int)(unsigned char)m_data[m_pos] ) ||
	    m_data[m_pos] == '-' ||
	    m_data[m_pos] == '+' )
	{
	    string[stringIdx++] = m_data[m_pos++];
	}
	else if( m_data[m_pos] == '.' )
	{
	    string[stringIdx++] = m_data[m_pos++];
	    periodFound = true;
	}

	// parse floating number
	while( !isspace( (int)(unsigned char)m_data[m_pos] ) &&
	       (isdigit( (int)(unsigned char)m_data[m_pos] ) ||
		m_data[m_pos] == '.') &&
	       m_pos < m_data.getSize() )
	{
	    if( m_data[m_pos] == '.' )
	    {
		if( periodFound )
		{
		    break;
		}
		else
		{
		    periodFound = true;
		}
	    }

	    string[stringIdx++] = m_data[m_pos++];
	}

	string[stringIdx++] = '\0';
	f = (float)atof( string );

	// if no integer can be read
	if( string[0] == '\0' ) m_ok = false;

	// reach end of file yet?
	if( m_pos >= m_data.getSize() ) m_eof = true;

	return *this;
    }

    EAistream &operator>>(double &d) {
	// skip initial whitespaces
	while( isspace( (int)(unsigned char)m_data[m_pos] ) && m_pos < m_data.getSize() )
	    m_pos++;

	if( m_pos >= m_data.getSize() )
	{
	    m_eof = true;
	    return *this;
	}

	int stringIdx = 0;
	bool periodFound = false;
	char string[1024];

	// parse minus, plus sign, or decimal point, if they exist
	if( isdigit( (int)(unsigned char)m_data[m_pos] ) ||
	    m_data[m_pos] == '-' ||
	    m_data[m_pos] == '+' )
	{
	    string[stringIdx++] = m_data[m_pos++];
	}
	else if( m_data[m_pos] == '.' )
	{
	    string[stringIdx++] = m_data[m_pos++];
	    periodFound = true;
	}

	// parse floating number
	while( !isspace( (int)(unsigned char)m_data[m_pos] ) &&
	       (isdigit( (int)(unsigned char)m_data[m_pos] ) ||
		m_data[m_pos] == '.') &&
	       m_pos < m_data.getSize() )
	{
	    if( m_data[m_pos] == '.' )
	    {
		if( periodFound )
		{
		    break;
		}
		else
		{
		    periodFound = true;
		}
	    }

	    string[stringIdx++] = m_data[m_pos++];
	}

	string[stringIdx++] = '\0';
	d = atof( string );

	// if no integer can be read
	if( string[0] == '\0' ) m_ok = false;

	// reach end of file yet?
	if( m_pos >= m_data.getSize() ) m_eof = true;

	return *this;
    }

    EAistream &operator>>(char *s) {
	// skip initial whitespaces
	while( isspace( (int)(unsigned char)m_data[m_pos] ) && m_pos < m_data.getSize() )
	    m_pos++;

	if( m_pos >= m_data.getSize() )
	{
	    m_eof = true;
	    return *this;
	}

	// parse string
	while( !isspace( (int)(unsigned char)m_data[m_pos] ) && m_pos < m_data.getSize() )
	{
	    *s++ = m_data[m_pos++];
	}

	*s++ = '\0';

	// reach end of file yet?
	if( m_pos >= m_data.getSize() ) m_eof = true;

	return *this;
    }

    EAistream &operator>>(char &c) {
	// skip initial whitespaces
	while( isspace( (int)(unsigned char)m_data[m_pos] ) && m_pos < m_data.getSize() )
	    m_pos++;

	if( m_pos >= m_data.getSize() )
	{
	    m_eof = true;
	    return *this;
	}

	c = m_data[m_pos++];
	if( m_pos >= m_data.getSize() ) m_eof = true;
	return *this;
    }

    EAistream &get(char* b, int len, char delim='\n')
    {
	int ix = 0;

	while( m_pos < m_data.getSize() &&
	       m_data[m_pos] != delim &&
	       ix < len - 1 )
	{
	    b[ix++] = m_data[m_pos++];
	}

	b[ix++] = '\0';
	if( m_pos >= m_data.getSize() ) m_eof = true;

	return *this;
    }

    bool getline(char* b, int len, char delim='\n')
    {
	int ix = 0;

	while( m_pos < m_data.getSize() &&
	       m_data[m_pos] != delim &&
	       ix < len - 1 )
	{
	    b[ix++] = m_data[m_pos++];
	}

	b[ix++] = '\0';
	if( m_pos >= m_data.getSize() ) m_eof = true;
	if( ix >= len ) return true;
	if( m_data[m_pos] == delim ) m_pos++;

  return !m_eof || ix > 1;
    }

private:
    XbArray<char>	m_data;
    int			m_pos;
    bool		m_ok;
    bool		m_eof;
};

#endif	// #ifndef _EAIOSTREAM_H
