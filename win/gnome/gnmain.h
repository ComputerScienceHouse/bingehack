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

#ifndef GnomeHackMainWindow_h
#define GnomeHackMainWindow_h

#include <gnome.h>
#include <gtk/gtk.h>
#include "gnmain.h"


void ghack_init_main_window( int argc, char** argv);
void ghack_main_window_add_map_window(GtkWidget* win);
void ghack_main_window_add_message_window(GtkWidget* win);
void ghack_main_window_add_status_window(GtkWidget* win);
void ghack_main_window_add_text_window(GtkWidget *);
void ghack_main_window_remove_window(GtkWidget *);
void ghack_main_window_update_inventory();
void ghack_save_game_cb(GtkWidget *widget, gpointer data);
GtkWidget* ghack_get_main_window();



#endif /* GnomeHackMainWindow_h */

