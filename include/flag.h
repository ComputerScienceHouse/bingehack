/*	SCCS Id: @(#)flag.h	3.3	1999/11/26	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* If you change the flag structure make sure you increment EDITLEVEL in   */
/* patchlevel.h if needed.  Changing the instance_flags structure does	   */
/* not require incrementing EDITLEVEL.					   */

#ifndef FLAG_H
#define FLAG_H

/*
 * Persistent flags that are saved and restored with the game.
 *
 */

struct flag {
#ifdef AMIFLUSH
	boolean  altmeta;	/* use ALT keys as META */
	boolean  amiflush;	/* kill typeahead */
#endif
#ifdef	MFLOPPY
	boolean  asksavedisk;
#endif
	boolean  autoquiver;	/* Automatically fill quiver */
	boolean  beginner;
#ifdef MAIL
	boolean  biff;		/* enable checking for mail */
#endif
	boolean  botl;		/* partially redo status line */
	boolean  botlx;		/* print an entirely new bottom line */
	boolean  confirm;	/* confirm before hitting tame monsters */
	boolean  debug;		/* in debugging mode */
#define wizard	 flags.debug
	boolean  end_own;	/* list all own scores */
	boolean  explore;	/* in exploration mode */
#ifdef OPT_DISPMAP
	boolean  fast_map;	/* use optimized, less flexible map display */
#endif
#define discover flags.explore
	boolean  female;
	boolean  forcefight;
	boolean  friday13;	/* it's Friday the 13th */
	boolean  help;		/* look in data file for info about stuff */
	boolean  ignintr;	/* ignore interrupts */
#ifdef INSURANCE
	boolean  ins_chkpt;	/* checkpoint as appropriate */
#endif
	boolean  invlet_constant; /* let objects keep their inventory symbol */
	boolean  legacy;	/* print game entry "story" */
	boolean  lit_corridor;	/* show a dark corr as lit if it is in sight */
	boolean  made_amulet;
	boolean  mon_moving;	/* monsters' turn to move */
	boolean  move;
	boolean  mv;
	boolean  nap;		/* `timed_delay' option for display effects */
	boolean  nopick;	/* do not pickup objects (as when running) */
	boolean  null;		/* OK to send nulls to the terminal */
#ifdef MAC
	boolean  page_wait;	/* put up a --More-- after a page of messages */
#endif
	boolean  perm_invent;	/* keep full inventories up until dismissed */
	boolean  pickup;	/* whether you pickup or move and look */

	boolean  pushweapon;	/* When wielding, push old weapon into second slot */
	boolean  rest_on_space; /* space means rest */
	boolean  safe_dog;	/* give complete protection to the dog */
#ifdef EXP_ON_BOTL
	boolean  showexp;	/* show experience points */
#endif
#ifdef SCORE_ON_BOTL
	boolean  showscore;	/* show score */
#endif
	boolean  silent;	/* whether the bell rings or not */
	boolean  sortpack;	/* sorted inventory */
	boolean  soundok;	/* ok to tell about sounds heard */
	boolean  standout;	/* use standout for --More-- */
	boolean  time;		/* display elapsed 'time' */
	boolean  tombstone;	/* print tombstone */
	boolean  toptenwin;	/* ending list in window instead of stdout */
	boolean  verbose;	/* max battle info */
	boolean  prayconfirm;	/* confirm before praying */
	int	 end_top, end_around;	/* describe desired score list */
	unsigned ident;		/* social security number for each monster */
	unsigned moonphase;
	unsigned long suppress_alert;
#define NEW_MOON	0
#define FULL_MOON	4
	unsigned no_of_wizards; /* 0, 1 or 2 (wizard and his shadow) */
	unsigned run;		/* 0: h (etc), 1: H (etc), 2: fh (etc) */
				/* 3: FH, 4: ff+, 5: ff-, 6: FF+, 7: FF- */
	int	 djinni_count, ghost_count;	/* potion effect tuning */
	int	 pickup_burden;		/* maximum burden before prompt */
	char	 inv_order[MAXOCLASSES];
	char	 pickup_types[MAXOCLASSES];
	char	 end_disclose[5];	/* disclose various info upon exit */
	char	 menu_style;	/* User interface style setting */
#ifdef AMII_GRAPHICS
	int numcols;
	unsigned short amii_dripens[ 20 ]; /* DrawInfo Pens currently there are 13 in v39 */
	AMII_COLOR_TYPE amii_curmap[ AMII_MAXCOLORS ]; /* colormap */
#endif

	/* KMH, role patch -- Variables used during startup.
	 *
	 * If the user wishes to select a role, race, gender, and/or alignment
	 * during startup, the choices should be recorded here.  This
	 * might be specified through command-line options, environmental
	 * variables, a popup dialog box, menus, etc.
	 *
	 * These values are each an index into an array.  They are not
	 * characters or letters, because that limits us to 26 roles.
	 * They are not booleans, because someday someone may need a neuter
	 * gender.  Negative values are used to indicate that the user
	 * hasn't yet specified that particular value.	If you determine
	 * that the user wants a random choice, then you should set an
	 * appropriate random value; if you just left the negative value,
	 * the user would be asked again!
	 *
	 * These variables are stored here because the u structure is
	 * cleared during character initialization, and because the
	 * flags structure is restored for saved games.  Thus, we can
	 * use the same parameters to build the role entry for both
	 * new and restored games.
	 *
	 * These variables should not be referred to after the character
	 * is initialized or restored (specifically, after role_init()
	 * is called).
	 */
	int	 initrole;	/* starting role      (index into roles[])   */
	int	 initrace;	/* starting race      (index into races[])   */
	int	 initgend;	/* starting gender    (index into genders[]) */
	int	 initalign;	/* starting alignment (index into aligns[])  */
	int	 pantheon;	/* deity selection for priest character */
};

/*
 * Flags that are set each time the game is started.
 * These are not saved with the game.
 *
 */

struct instance_flags {
	boolean  cbreak;	/* in cbreak mode, rogue format */
	boolean  DECgraphics;	/* use DEC VT-xxx extended character set */
	boolean  echo;		/* 1 to echo characters */
#ifdef TTY_GRAPHICS
	boolean  extmenu;	/* extended commands use menu interface */
#endif
	boolean  IBMgraphics;	/* use IBM extended character set */
	unsigned msg_history;	/* hint: # of top lines to save */
	boolean  num_pad;	/* use numbers for movement commands */
	boolean  news;		/* print news */
	boolean  window_inited; /* true if init_nhwindows() completed */

#ifdef WIZARD
	boolean  sanity_check;	/* run sanity checks */
#endif
#ifdef TEXTCOLOR
	boolean  hilite_pet;	/* hilight pets on monochome displays */
	boolean  use_color;	/* use color graphics */
#endif
#ifdef MAC_GRAPHICS_ENV
	boolean  large_font;	/* draw in larger fonts (say, 12pt instead
				   of 9pt) */
	boolean  MACgraphics;	/* use Macintosh extended character set, as
				   as defined in the special font HackFont */
	unsigned  use_stone;		/* use the stone ppats */
#endif
#ifdef MAC
	boolean  popup_dialog;	/* put queries in pop up dialogs instead of
				   in the message window */
#endif
#ifdef MICRO
	boolean  BIOS;		/* use IBM or ST BIOS calls when appropriate */
	boolean  rawio;		/* whether can use rawio (IOCTL call) */
#endif
#ifdef MSDOS
	boolean hasvga;		/* has a vga adapter */
	boolean usevga;		/* use the vga adapter */
	boolean has8514;
	boolean use8514;
	boolean hasvesa;
	boolean usevesa;
	boolean grmode;		/* currently in graphics mode */
#endif
#if defined(MSDOS) || defined(WIN32)
	boolean hassound;	/* has a sound card */
	boolean usesound;	/* use the sound card */
	boolean usepcspeaker;	/* use the pc speaker */
	boolean preload_tiles;	/* preload the tiles into RAM */
	boolean tile_view;
	boolean over_view;
	boolean traditional_view;
#endif
#ifdef LAN_FEATURES
	boolean lan_mail;	/* mail is initialized */
	boolean lan_mail_fetched; /* mail is awaiting display */
#endif
};

extern NEARDATA struct flag flags;
extern NEARDATA struct instance_flags iflags;

#endif /* FLAG_H */
