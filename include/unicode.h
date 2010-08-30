/* SCCS Id: @(#)unicode.h	3.4	2007/03/17	*/
/* Copyright (c) Ray Chason, Matthew Snyder, 2003-2007 */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef UNICODE_H
#define UNICODE_H

#include <wchar.h>

int putwidechar(int c);

wchar_t uni_equiv(int ch);

char sym_glyph(char ch);

#endif /* UNICODE_H */
