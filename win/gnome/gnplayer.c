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

#include "gnplayer.h"
#include "gnmain.h"
#include <gnome.h>
#include <ctype.h>
#include "hack.h"

static gint role_number;                                                      
static GtkWidget* clist;

static void                                                                   
player_sel_key_hit (GtkWidget *widget, GdkEventKey *event, gpointer data)     
{                                                                             
      const char** roles = data;                                              
      int i;                                                                  
      for (i = 0; roles[i] != 0; ++i) {                                       
	      if (roles[i][0] == toupper(event->keyval)) {                    
		      role_number = i;                                        
		      gtk_clist_select_row( GTK_CLIST (clist), i, 0);         
	      }                                                               
      }                                                                       
}                                                                             

static void
player_sel_row_selected (GtkCList *clist, int row, int col, GdkEvent *event)
{
    role_number = row;
}

int
ghack_player_sel_dialog( const char** roles)
{
    int i;
    static GtkWidget* dialog;
    static GtkWidget* swin;
    static GtkWidget* frame1;

    dialog = gnome_dialog_new (_("Player selection"),
			    GNOME_STOCK_BUTTON_OK,
			    _("Random role"),
			    GNOME_STOCK_BUTTON_CANCEL,
			    NULL);
    gnome_dialog_close_hides (GNOME_DIALOG (dialog), FALSE);
    gtk_signal_connect (GTK_OBJECT (dialog), "key_press_event",               
		      GTK_SIGNAL_FUNC (player_sel_key_hit), roles );          

    frame1 = gtk_frame_new (_("Choose one of the following roles:"));
    gtk_object_set_data (GTK_OBJECT (dialog), "frame1", frame1);
    gtk_widget_show (frame1);
    gtk_container_border_width (GTK_CONTAINER (frame1), 3);

    swin = gtk_scrolled_window_new (NULL, NULL);
    clist = gtk_clist_new (2);
    gtk_clist_column_titles_hide (GTK_CLIST (clist));
    gtk_widget_set_usize (GTK_WIDGET (clist), 100, 180);
    gtk_container_add (GTK_CONTAINER (swin), clist);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swin),
	    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    gtk_signal_connect (GTK_OBJECT (clist), "select_row",
			GTK_SIGNAL_FUNC (player_sel_row_selected), NULL );

    gtk_container_add (GTK_CONTAINER (frame1), swin);
    gtk_box_pack_start_defaults (GTK_BOX (GNOME_DIALOG (dialog)->vbox), frame1);
 
    /* Add the roles into the list here... */
    for (i=0; roles[i]; i++) {
	    gchar accelBuf[BUFSZ];
	    const char *text[3]={accelBuf, roles[i],NULL};
	    sprintf( accelBuf, "%c ", tolower(roles[i][0]));
	    gtk_clist_insert (GTK_CLIST (clist), i, (char**)text);
    }
 
    gtk_clist_columns_autosize (GTK_CLIST (clist));
    gtk_widget_show_all (swin);

    /* Center the dialog over over parent */
    gnome_dialog_set_default( GNOME_DIALOG(dialog), 0);
    gtk_window_set_modal( GTK_WINDOW(dialog), TRUE);
    gnome_dialog_set_parent (GNOME_DIALOG (dialog), 
	    GTK_WINDOW (ghack_get_main_window ()) );

    /* Run the dialog -- returning whichever button was pressed */
    i = gnome_dialog_run (GNOME_DIALOG (dialog));
    gnome_dialog_close (GNOME_DIALOG (dialog));

    /* Quit on button 2 or error */
    if (i < 0  || i > 1) {
	return( -1);
    }
    /* Random is button 1*/
    if (i == 1 ) {
	return( -2);
    }
    return ( role_number);
}
