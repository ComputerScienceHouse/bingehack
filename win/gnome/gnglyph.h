/* Gnome Interface for NetHack
 *
 * Copyright (C) 1998 by Erik Andersen <andersee@debian.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

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
