/*	SCCS Id: @(#)gnglyph.h	3.3	2000/07/16	*/
/* Copyright (C) 1998 by Erik Andersen <andersee@debian.org> */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef GnomeHackGlyph_h
#define GnomeHackGlyph_h

#include "config.h"
#include "global.h"
#include <gdk_imlib.h>
#include <gdk/gdk.h>

extern short glyph2tile[];     /* From tile.c */

typedef struct {
  GdkImlibImage* im;
  int            count;
  int            width;
  int            height;
} GHackGlyphs;

extern int            ghack_init_glyphs( char* xpmFile);
extern void           ghack_dispose_glyphs( void);
extern int            ghack_glyph_count( void);
extern GdkImlibImage* ghack_image_from_glyph( int glyph, gboolean force);
extern int            ghack_glyph_height( void);
extern int            ghack_glyph_width( void);

#endif  /* GnomeHackGlyph_h */
