/*	SCCS Id: @(#)windows.c	3.3	96/05/19	*/
/* Copyright (c) D. Cohrs, 1993. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#ifdef TTY_GRAPHICS
#include "wintty.h"
#endif
#ifdef X11_GRAPHICS
/* cannot just blindly include winX.h without including all of X11 stuff */
/* and must get the order of include files right.  Don't bother */
extern struct window_procs X11_procs;
extern void NDECL(win_X11_init);
#endif
#ifdef QT_GRAPHICS
extern struct window_procs Qt_procs;
#endif
#ifdef MAC
extern struct window_procs mac_procs;
#endif
#ifdef __begui__
extern struct window_procs be_procs;
extern void NDECL(be_win_init);
#endif
#ifdef AMIGA_INTUITION
extern struct window_procs amii_procs;
extern struct window_procs amiv_procs;
extern void NDECL(ami_wininit_data);
#endif
#ifdef WIN32_GRAPHICS
extern struct window_procs win32_procs;
#endif

STATIC_DCL void FDECL(def_raw_print, (const char *s));

NEARDATA struct window_procs windowprocs;

static
struct win_choices {
    struct window_procs *procs;
    void NDECL((*ini_routine));		/* optional (can be 0) */
} winchoices[] = {
#ifdef TTY_GRAPHICS
    { &tty_procs, win_tty_init },
#endif
#ifdef X11_GRAPHICS
    { &X11_procs, win_X11_init },
#endif
#ifdef QT_GRAPHICS
    { &Qt_procs, 0 },
#endif
#ifdef MAC
    { &mac_procs, 0 },
#endif
#ifdef __begui__
    { &be_procs, be_win_init },
#endif
#ifdef AMIGA_INTUITION
    { &amii_procs, ami_wininit_data },		/* Old font version of the game */
    { &amiv_procs, ami_wininit_data },		/* Tile version of the game */
#endif
#ifdef WIN32_GRAPHICS
    { &win32_procs, 0 },
#endif
    { 0, 0 }		/* must be last */
};

STATIC_OVL
void
def_raw_print(s)
const char *s;
{
    puts(s);
}

void
choose_windows(s)
const char *s;
{
    register int i;

    for(i=0; winchoices[i].procs; i++)
	if (!strcmpi(s, winchoices[i].procs->name)) {
	    windowprocs = *winchoices[i].procs;
	    if (winchoices[i].ini_routine) (*winchoices[i].ini_routine)();
	    return;
	}

    if (!windowprocs.win_raw_print)
	windowprocs.win_raw_print = def_raw_print;

    raw_printf("Window type %s not recognized.  Choices are:", s);
    for(i=0; winchoices[i].procs; i++)
	raw_printf("        %s", winchoices[i].procs->name);

    if (windowprocs.win_raw_print == def_raw_print)
	terminate(EXIT_SUCCESS);
    wait_synch();
}

/*
 * tty_message_menu() provides a means to get feedback from the
 * --More-- prompt; other interfaces generally don't need that.
 */
/*ARGSUSED*/
char
genl_message_menu(let, how, mesg)
char let;
int how;
const char *mesg;
{
    pline("%s", mesg);
    return 0;
}

/*windows.c*/
