/*	SCCS Id: @(#)pcunix.c	3.3	94/11/07	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* This file collects some Unix dependencies; pager.c contains some more */

#include "hack.h"

#include	<sys/stat.h>
#if defined(WIN32) || defined(MSDOS)
#include	<errno.h>
#endif

#if defined(WIN32) || defined(MSDOS)
extern char orgdir[];
# ifdef WIN32
extern void NDECL(backsp);
# endif
extern void NDECL(clear_screen);
#endif

#ifdef OVLB

static struct stat buf;
# ifdef WANT_GETHDATE
static struct stat hbuf;
# endif

void
gethdate(name)
char *name;
{
# ifdef WANT_GETHDATE
/* old version - for people short of space */
/*
/* register char *np;
/*      if(stat(name, &hbuf))
/*	      error("Cannot get status of %s.",
/*		      (np = rindex(name, '/')) ? np+1 : name);
/*
/* version using PATH from: seismo!gregc@ucsf-cgl.ARPA (Greg Couch) */

/*
 * The problem with   #include  <sys/param.h> is that this include file
 * does not exist on all systems, and moreover, that it sometimes includes
 * <sys/types.h> again, so that the compiler sees these typedefs twice.
 */
#define	 MAXPATHLEN      1024

    register char *np, *path;
    char filename[MAXPATHLEN+1], *getenv();

    if (index(name, '/') != (char *)0 || (path = getenv("PATH")) == (char *)0)
	path = "";

    for (;;) {
	if ((np = index(path, ':')) == (char *)0)
	    np = path + strlen(path);       /* point to end str */
	if (np - path <= 1)		     /* %% */
	    Strcpy(filename, name);
	else {
	    (void) strncpy(filename, path, np - path);
	    filename[np - path] = '/';
	    Strcpy(filename + (np - path) + 1, name);
	}
	if (stat(filename, &hbuf) == 0)
	    return;
	if (*np == '\0')
	path = "";
	path = np + 1;
    }
    error("Cannot get status of %s.", (np = rindex(name, '/')) ? np+1 : name);
# endif /* WANT_GETHDATE */
}

#if 0
int
uptodate(fd)
int fd;
{
# ifdef WANT_GETHDATE
    if(fstat(fd, &buf)) {
	pline("Cannot get status of saved level? ");
	return(0);
    }
    if(buf.st_mtime < hbuf.st_mtime) {
	pline("Saved level is out of date. ");
	return(0);
    }
# else
#  if defined(MICRO) && !defined(NO_FSTAT)
    if(fstat(fd, &buf)) {
	if(moves > 1) pline("Cannot get status of saved level? ");
	else pline("Cannot get status of saved game");
	return(0);
    } 
    if(comp_times(buf.st_mtime)) { 
	if(moves > 1) pline("Saved level is out of date");
	else pline("Saved game is out of date. ");
	/* This problem occurs enough times we need to give the player
	 * some more information about what causes it, and how to fix.
	 */
#  ifdef MSDOS
	    pline("Make sure that your system's date and time are correct.");
	    pline("They must be more current than NetHack.EXE's date/time stamp.");
#  endif /* MSDOS */
	return(0);
    }
#  endif  /* MICRO */
# endif /* WANT_GETHDATE */
    return(1);
}
#endif

#ifdef PC_LOCKING
static int
eraseoldlocks()
{
	register int i;

	/* cannot use maxledgerno() here, because we need to find a lock name
	 * before starting everything (including the dungeon initialization
	 * that sets astral_level, needed for maxledgerno()) up
	 */
	for(i = 1; i <= MAXDUNGEON*MAXLEVEL + 1; i++) {
		/* try to remove all */
		set_levelfile_name(lock, i);
		(void) unlink(lock);
	}
	set_levelfile_name(lock, 0);
	if(unlink(lock)) return(0);			/* cannot remove it */
	return(1);					/* success! */
}

void
getlock()
{
	register int i = 0, fd, c, ci, ct;
	char tbuf[BUFSZ];
# if defined(MSDOS) && defined(NO_TERMS)
	int grmode;
# endif
	
	/* we ignore QUIT and INT at this point */
	if (!lock_file(HLOCK, 10)) {
		wait_synch();
		chdirx(orgdir, 0);
		error("Quitting.");
	}

	/* regularize(lock); */ /* already done in pcmain */
	Sprintf(tbuf,lock);
	set_levelfile_name(lock, 0);

	if((fd = open(lock,0)) == -1) {
		if(errno == ENOENT) goto gotlock;    /* no such file */
		chdirx(orgdir, 0);
		perror(lock);
		unlock_file(HLOCK); 
		error("Cannot open %s", lock);
	}

	(void) close(fd);

	if(iflags.window_inited) { 
	  pline("There is already a game in progress under your name.");
	  pline(tbuf,"You may be able to use \"recover %s\" to get it back.\n",lock);
	  c = yn("Do you want to destroy the old game?");
	} else {
# if defined(MSDOS) && defined(NO_TERMS)
		grmode = iflags.grmode;
		if (grmode) gr_finish();
# endif
		c = 'n';
		ct = 0;
		msmsg("\nThere is already a game in progress under your name.\n");
		msmsg("If this is unexpected, you may be able to use \n");
		msmsg("\"recover %s\" to get it back.",tbuf);
		msmsg("\nDo you want to destroy the old game? [yn] ");
		while ((ci=nhgetch()) != '\n') {
		    if (ct > 0) {
# if defined(WIN32CON)
			backsp();       /* \b is visible on NT */
# else
			msmsg("\b \b");
# endif
			ct = 0;
			c = 'n';
		    }
		    if (ci == 'y' || ci == 'n' || ci == 'Y' || ci == 'N') {
		    	ct = 1;
		        c = ci;
		        msmsg("%c",c);
		    }
		}
	}
	if(c == 'y' || c == 'Y')
		if(eraseoldlocks()) {
# if defined(WIN32CON)
			clear_screen();		/* display gets fouled up otherwise */
# endif
			goto gotlock;
		} else {
			unlock_file(HLOCK);
			chdirx(orgdir, 0);
			error("Couldn't destroy old game.");
		}
	else {
		unlock_file(HLOCK);
		chdirx(orgdir, 0);
		error("%s", "");
	}

gotlock:
	fd = creat(lock, FCMASK);
	unlock_file(HLOCK);
	if(fd == -1) {
		chdirx(orgdir, 0);
		error("cannot creat lock file.");
	} else {
		if(write(fd, (char *) &hackpid, sizeof(hackpid))
		    != sizeof(hackpid)){
			chdirx(orgdir, 0);
			error("cannot write lock");
		}
		if(close(fd) == -1) {
			chdirx(orgdir, 0);
			error("cannot close lock");
		}
	}
# if defined(MSDOS) && defined(NO_TERMS)
	if (grmode) gr_init();
# endif
}	
# endif /* PC_LOCKING */

# ifndef WIN32
void
regularize(s)
/*
 * normalize file name - we don't like .'s, /'s, spaces, and
 * lots of other things
 */
register char *s;
{
	register char *lp;

	for (lp = s; *lp; lp++)
		if (*lp <= ' ' || *lp == '"' || (*lp >= '*' && *lp <= ',') ||
		    *lp == '.' || *lp == '/' || (*lp >= ':' && *lp <= '?') ||
# ifdef OS2
		    *lp == '&' || *lp == '(' || *lp == ')' ||
# endif
		    *lp == '|' || *lp >= 127 || (*lp >= '[' && *lp <= ']'))
                        *lp = '_';
}
# endif /* WIN32 */
#endif /* OVLB */


#ifdef __EMX__
void seteuid(int i){;}
#endif
