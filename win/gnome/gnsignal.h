/*
 * Gnome Interface for NetHack
 *
 * Copyright (C) 1998 by Anthony Taylor <tonyt@ptialaska.net>
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
 *
*/

#ifndef GnomeHackSignals_h
#define GnomeHackSignals_h

#include <gtk/gtk.h>
#include <gnome.h>
#include "gnomeprv.h"
#include "gnglyph.h"

/* The list of custom signals */

enum {
  GHSIG_CURS,
  GHSIG_PUTSTR,
  GHSIG_PRINT_GLYPH,
  GHSIG_CLEAR,
  GHSIG_DISPLAY,
  GHSIG_START_MENU,
  GHSIG_ADD_MENU,
  GHSIG_END_MENU,
  GHSIG_SELECT_MENU,
  GHSIG_CLIPAROUND,
  GHSIG_FADE_HIGHLIGHT,
  GHSIG_DELAY,
  GHSIG_LAST_SIG
};

guint ghack_signals[GHSIG_LAST_SIG];

extern void ghack_init_signals( void);


void ghack_handle_key_press(GtkWidget *widget, GdkEventKey *event, 
	gpointer data);
void ghack_handle_button_press(GtkWidget *widget, GdkEventButton *event, 
	gpointer data);

typedef struct {
        int x, y, mod;
} GHClick;

extern GList *g_keyBuffer;
extern GList *g_clickBuffer;
extern int g_numKeys;
extern int g_numClicks;

extern int g_askingQuestion;

void ghack_delay( GtkWidget *win, int numMillisecs, gpointer data);


#endif    /* GnomeHackSignals_h */

