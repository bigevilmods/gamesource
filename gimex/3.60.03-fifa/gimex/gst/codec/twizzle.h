// (c) 2003-2007 Electronic Arts Inc.

#ifndef _TWIZZLE_H
#define _TWIZZLE_H

namespace Gst
{

int BaseOffset(int x, int y, int width, int blockwidth, int blockheight);
int Detail1Offset(int x, int y, int width);
int Detail2Offset(int x, int y, int width);

void TwiddleBase(const unsigned char *source, unsigned char *dest, int width, int height, int blockwidth, int blockheight);
void TwizzleDetail1(const unsigned char *source, unsigned char *dest, int width, int height);
void TwizzleDetail2(const unsigned char *source, unsigned char *dest, int width, int height);

void UntwiddleBase(const unsigned char *source, unsigned char *dest, int width, int height, int blockwidth, int blockheight);
void UntwizzleDetail1(const unsigned char *source, unsigned char *dest, int width, int height);
void UntwizzleDetail2(const unsigned char *source, unsigned char *dest, int width, int height);

} // end of Gst namespace

#endif
