/*
 * Gnome Interface for NetHack
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
 *
*/

#ifndef GnomeHackMenuWindow_h
#define GnomeHackMenuWindow_h

#include <gnome.h>
#include "config.h"
#include "global.h"
#include "GnomeHack.h"

GtkWidget* ghack_init_menu_window( void );

struct _GHackMenuItem
{
  int		glyph; 
  const ANY_P *identifier;
  CHAR_P       accelerator;
  CHAR_P       group_accel;
  int          attr;
  const char*  str;
  BOOLEAN_P    presel;
};

typedef struct _GHackMenuItem GHackMenuItem;

int ghack_menu_window_select_menu (GtkWidget *menuWin, 
	MENU_ITEM_P **_selected, gint how);


#endif  /* GnomeHackMenuWindow_h */
