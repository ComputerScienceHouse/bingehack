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

#include "gnaskstr.h"
#include "gnmain.h"
#include <gnome.h>


static
void ghack_ask_string_callback(gchar * string, gpointer data)
{
  char **user_text = (char **) data;

  g_assert(user_text != NULL);

  *user_text = string; /* note - value must be g_free'd */
}


int ghack_ask_string_dialog(const char *szMessageStr, 
	const char *szDefaultStr, const char *szTitleStr, 
	char *buffer)
{
    int i;
    GtkWidget* dialog;
    gchar   *user_text = NULL;

    dialog = gnome_request_dialog(FALSE, szMessageStr,
    				  szDefaultStr, 0,
				  ghack_ask_string_callback,
				  &user_text, NULL);
    g_assert(dialog != NULL);

    gtk_window_set_title(GTK_WINDOW(dialog), szTitleStr);

    gnome_dialog_set_default( GNOME_DIALOG(dialog), 0);
    gtk_window_set_modal( GTK_WINDOW(dialog), TRUE);
    gnome_dialog_set_parent (GNOME_DIALOG (dialog), 
	    GTK_WINDOW (ghack_get_main_window ()) );

    i = gnome_dialog_run_and_close (GNOME_DIALOG (dialog));
    
    /* Quit */
    if ( i != 0 || user_text == NULL ) {
	if (user_text)
	  g_free(user_text);
	return -1;
    }

    if ( *user_text == 0 ) {
      g_free(user_text);
      return -1;
    }

    g_assert(strlen(user_text) > 0);
    strcpy (buffer, user_text);
    g_free(user_text);
    return 0;
}

