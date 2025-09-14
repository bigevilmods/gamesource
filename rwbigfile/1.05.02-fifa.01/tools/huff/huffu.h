// (c) Electronic Arts. All Rights Reserved.

#ifndef __HUFFU_H
#define __HUFFU_H 1

#if defined(_MSC_VER)
#pragma once
#endif

size_t 	SQunhuff(const unsigned char *source, unsigned char *dest, unsigned char *workbuf, int doit);
void	SQBlockMove(const unsigned char *s, unsigned char *d, size_t len);

#endif // __HUFFU_H
