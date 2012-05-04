#include "curses.h"
#include "hack.h"
#include "wincurs.h"
#include "cursmesg.h"

/* Message window routines for curses interface */

/* Private declatations */

typedef struct nhpm
{
    char *str;  /* Message text */
    long turn;  /* Turn number for message */
    struct nhpm *prev_mesg;    /* Pointer to previous message */
    struct nhpm *next_mesg;    /* Pointer to next message */
} nhprev_mesg;

static void scroll_window(winid wid);

static void mesg_add_line(char *mline);

static nhprev_mesg *get_msg_line(boolean reverse, int mindex);

static int turn_lines = 1;
static int mx = 0;
static int my = 0;  /* message window text location */
static nhprev_mesg *first_mesg = NULL;
static nhprev_mesg *last_mesg = NULL;
static int max_messages;
static int num_messages = 0;


static void strip_characters( char *message, const char *illegal ) {
    size_t message_len = strlen(message), illegal_len = strlen(illegal);
    // First, find the illegal characters and map them to \0
    for( size_t i = 0; i < message_len; i++ ) {
        for( size_t j = 0; j < illegal_len; j++ ) {
            if( message[i] == illegal[j] ) {
                message[i] = '\0';
                break;
            }
        }
    }
    // Now, remove the illegal characters by shifting left
    size_t strptr = 0;
    for( size_t i = 0; i < message_len; i++ ) {
        size_t next_legal = i;
        for( size_t j = i; j < message_len; j++ ) {
            if( message[j] == '\0' ) {
                strptr++;
            } else {
                next_legal = j;
                break;
            }
        }
        if( i != next_legal ) message[i] = message[next_legal];
        strptr++;
    }
}


/* Write a string to the message window.  Attributes set by calling function. */

void curses_message_win_puts(const char *message, boolean recursed)
{
    int height, width, linespace;
    char *tmpstr;
    WINDOW *win = curses_get_nhwin(MESSAGE_WIN);
    boolean border = curses_window_has_border(MESSAGE_WIN);
    int message_length = strlen(message);
    int border_space = 0;
    static long suppress_turn = -1;

    // This could be done better (without the cast).
    strip_characters((char *) message, "\n\t");

    if (strncmp("Count:", message, 6) == 0)
    {
        curses_count_window(message);
        return;
    }
    
    if (suppress_turn == moves)
    {
        return;
    }
    
    curses_get_window_size(MESSAGE_WIN, &height, &width);
    if (border)
    {
        border_space = 1;
        if (mx < 1)
        {
            mx = 1;
        }
        if (my < 1)
        {
            my = 1;
        }
    }
    
    linespace = ((width + border_space) - 3) - mx;
    
    if (strcmp(message, "#") == 0)  /* Extended command or Count: */
    {
        if ((strcmp(toplines, "#") != 0) && (my >= (height - 1 +
         border_space)) && (height != 1)) /* Bottom of message window */
        {
            scroll_window(MESSAGE_WIN);
            mx = width;
            my--;
            strcpy(toplines, message);
        }
        
        return;
    }

    if (!recursed)
    {
        strcpy(toplines, message);
        mesg_add_line((char *) message);
    }
    
    if (linespace < message_length)
    {
        if (my >= (height - 1 + border_space)) /* bottom of message win */
        {
            if ((turn_lines > height) || (height == 1))
            {
                /* Pause until key is hit - Esc suppresses any further
                messages that turn */
                if (curses_more() == '\033')
                {
                    suppress_turn = moves;
                    return;
                }
            }
            else
            {
                scroll_window(MESSAGE_WIN);
                turn_lines++;
            }
        }
        else
        {
            if (mx != border_space)
            {
                my++;
                mx = border_space;
            }
        }
    }

    if (height > 1)
    {
        curses_toggle_color_attr(win, NONE, A_BOLD, ON);
    }
    
    if ((mx == border_space) && ((message_length + 2) > width))
    {
        tmpstr = curses_break_str(message, (width - 2), 1);
        mvwprintw(win, my, mx, "%s", tmpstr);
        mx += strlen(tmpstr);
        if (strlen(tmpstr) < (width - 2))
        {
            mx++;
        }
        free(tmpstr);
        if (height > 1)
        {
            curses_toggle_color_attr(win, NONE, A_BOLD, OFF);
        }
        wrefresh(win);
        curses_message_win_puts(curses_str_remainder(message, (width - 2), 1),
         TRUE);
    }
    else
    {
        mvwprintw(win, my, mx, "%s", message);
        curses_toggle_color_attr(win, NONE, A_BOLD, OFF);
        mx += message_length + 1;
    }
    wrefresh(win);
}


int curses_more()
{
    int height, width, ret;
    WINDOW *win = curses_get_nhwin(MESSAGE_WIN);
    
    curses_get_window_size(MESSAGE_WIN, &height, &width);
    curses_toggle_color_attr(win, MORECOLOR, NONE, ON);
    mvwprintw(win, my, mx, ">>");
    curses_toggle_color_attr(win, MORECOLOR, NONE, OFF);
    wrefresh(win);
    ret = wgetch(win);
    if (height == 1)
    {
        curses_clear_unhighlight_message_window();
    }
    else
    {
        mvwprintw(win, my, mx, "  ");
        scroll_window(MESSAGE_WIN);
        turn_lines = 1;
    }
    
    return ret;
}


/* Clear the message window if one line; otherwise unhighlight old messages */

void curses_clear_unhighlight_message_window()
{
    int mh, mw, count;
    boolean border = curses_window_has_border(MESSAGE_WIN);
    WINDOW *win = curses_get_nhwin(MESSAGE_WIN);

    turn_lines = 1;
    
    curses_get_window_size(MESSAGE_WIN, &mh, &mw); 
    
    mx = 0;
    
    if (border)
    {
        mx++;
    }
       
    if (mh == 1)
    {
        curses_clear_nhwin(MESSAGE_WIN);
    }
    else
    {
        mx += mw;    /* Force new line on new turn */
        
        if (border)
        {

            for (count = 0; count < mh; count++)
            {
                mvwchgat(win, count+1, 1, mw, COLOR_PAIR(8), A_NORMAL, NULL);
            }
        }
        else
        {
            for (count = 0; count < mh; count++)
            {
                mvwchgat(win, count, 0, mw, COLOR_PAIR(8), A_NORMAL, NULL);
            }
        }

        wrefresh(win);
    }
}


/* Reset message window cursor to starting position, and display most
recent messages. */

void curses_last_messages()
{
    boolean border = curses_window_has_border(MESSAGE_WIN);

    if (border)
    {
        mx = 1;
        my = 1;
    }
    else
    {
        mx = 0;
        my = 0;
    }
    
    pline("%s", toplines);
}


/* Initialize list for message history */

void curses_init_mesg_history()
{
    max_messages = iflags.msg_history;
    
    if (max_messages < 1)
    {
        max_messages = 1;
    }

    if (max_messages > MESG_HISTORY_MAX)
    {
        max_messages = MESG_HISTORY_MAX;
    }
}


/* Display previous message window messages in reverse chron order */

void curses_prev_mesg()
{
    int count;
    winid wid;
    long turn = 0;
    anything *identifier;
    nhprev_mesg *mesg;
    menu_item *selected = NULL;

    wid = curses_get_wid(NHW_MENU);
    curses_create_nhmenu(wid);
    identifier = malloc(sizeof(anything));
    identifier->a_void = NULL;
    
    for (count = 0; count < num_messages; count++)
    {
        mesg = get_msg_line(TRUE, count);
        if ((turn != mesg->turn) && (count != 0))
        {
            curses_add_menu(wid, NO_GLYPH, identifier, 0, 0, A_NORMAL,
             "---", FALSE);
        }
        curses_add_menu(wid, NO_GLYPH, identifier, 0, 0, A_NORMAL,
         mesg->str, FALSE);
        turn = mesg->turn;
    }
    
    curses_end_menu(wid, "");
    curses_select_menu(wid, PICK_NONE, &selected);
}


/* Shows Count: in a separate window, or at the bottom of the message
window, depending on the user's settings */

void curses_count_window(const char *count_text)
{
    int startx, starty, winx, winy;
    int messageh, messagew;
    static WINDOW *countwin = NULL;

    if ((count_text == NULL) && (countwin != NULL))
    {
        delwin(countwin);
        countwin = NULL;
        counting = FALSE;
        return;
    }
    
    counting = TRUE;

    if (iflags.wc_popup_dialog) /* Display count in popup window */
    {
        startx = 1;
        starty = 1;
        
        if (countwin == NULL)
        {
            countwin = curses_create_window(25, 1, UP);
        }
    
    }
    else /* Display count at bottom of message window */
    {
        curses_get_window_xy(MESSAGE_WIN, &winx, &winy);
        curses_get_window_size(MESSAGE_WIN, &messageh, &messagew);
        
        if (curses_window_has_border(MESSAGE_WIN))
        {
            winx++;
            winy++;
        }
        
        winy += messageh - 1;
        
        if (countwin == NULL)
        {
            pline("#");
#ifndef PDCURSES
            countwin = newwin(1, 25, winy, winx);
#endif  /* !PDCURSES */
        }
#ifdef PDCURSES
        else
        {
            curses_destroy_win(countwin);
        }
        
        countwin = newwin(1, 25, winy, winx);
#endif  /* PDCURSES */
        startx = 0;
        starty = 0;
    }
    
    mvwprintw(countwin, starty, startx, "%s", count_text);
    wrefresh(countwin);
}


/* Scroll lines upward in given window, or clear window if only one line. */

static void scroll_window(winid wid)
{
    int wh, ww, s_top, s_bottom;
    boolean border = curses_window_has_border(wid);
    WINDOW *win = curses_get_nhwin(wid);
    
    curses_get_window_size(wid, &wh, &ww);
    if (wh == 1)
    {
        curses_clear_nhwin(wid);
        return;
    }
    if (border)
    {
        s_top = 1;
        s_bottom = wh;
    }
    else
    {
        s_top = 0;
        s_bottom = wh - 1;
    }
    scrollok(win, TRUE);
    wsetscrreg(win, s_top, s_bottom);
    scroll(win);
    scrollok(win, FALSE);
    if (wid == MESSAGE_WIN)
    {
        if (border)
            mx = 1;
        else
            mx = 0;
    }
    if (border)
    {
        box(win, 0, 0);
    }
    wrefresh(win);
}


/* Add given line to message history */

static void mesg_add_line(char *mline)
{
    nhprev_mesg *tmp_mesg = NULL;
    nhprev_mesg *current_mesg = malloc(sizeof(nhprev_mesg));

    current_mesg->str = curses_copy_of(mline);
    current_mesg->turn = moves;
    current_mesg->next_mesg = NULL;

    if (num_messages == 0)
    {
        first_mesg = current_mesg;
    }
    
    if (last_mesg != NULL)
    {
        last_mesg->next_mesg = current_mesg;
    }
    current_mesg->prev_mesg = last_mesg;
    last_mesg = current_mesg;


    if (num_messages < max_messages)
    {
        num_messages++;
    }
    else
    {
        tmp_mesg = first_mesg->next_mesg;
        free(first_mesg);
        first_mesg = tmp_mesg;
    }
}


/* Returns specified line from message history, or NULL if out of bounds */

static nhprev_mesg *get_msg_line(boolean reverse, int mindex)
{
    int count;
    nhprev_mesg *current_mesg;

    if (reverse)
    {
        current_mesg = last_mesg;
        for (count = 0; count < mindex; count++)
        {
            if (current_mesg == NULL)
            {
                return NULL;
            }
            current_mesg = current_mesg->prev_mesg;
        }
        return current_mesg;
    }
    else
    {
        current_mesg = first_mesg;
        for (count = 0; count < mindex; count++)
        {
            if (current_mesg == NULL)
            {
                return NULL;
            }
            current_mesg = current_mesg->next_mesg;
        }
        return current_mesg;
    }
}

