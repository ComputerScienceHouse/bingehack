/*	SCCS Id: @(#)patchlevel.h	3.3	1999/11/27	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* NetHack 3.3.0 */
#define VERSION_MAJOR	3
#define VERSION_MINOR	3
/*
 * PATCHLEVEL is updated for each release.
 */
#define PATCHLEVEL	0
/*
 * Incrementing EDITLEVEL can be used to force invalidation of old bones
 * and save files.
 */
#define EDITLEVEL	19

#define COPYRIGHT_BANNER_A \
"NetHack, Copyright 1985-1999"

#define COPYRIGHT_BANNER_B \
"         By Stichting Mathematisch Centrum and M. Stephenson."

#define COPYRIGHT_BANNER_C \
"         See license for details."

#if 0
/*
 * If two successive patchlevels have compatible data files (fat chance),
 * defining this with the value of the older one will allow its bones and
 * save files to work with the newer one.  The format is
 *	0xMMmmPPeeL
 * 0x = literal prefix "0x", MM = major version, mm = minor version,
 * PP = patch level, ee = edit level, L = literal suffix "L",
 * with all four numbers specified as two hexadecimal digits.
 */
#define VERSION_COMPATIBILITY 0x03030000L
#endif

/*****************************************************************************/
/* Version 3.3.0 */

/*
 *  NetHack 3.3.0, December 10, 1999
 *
 *  Implement the differentiation of character class or role from the
 *  character race.
 *  Removal of the Elf class, in preference to the Elf as a race.
 *  Introduction of Dwarves, Elves, Gnomes and Orcs as distinct races in
 *  addition to the Human "norm".
 *  Addition of the Monk and Ranger classes.
 *  Integrate some of the features of several branch versions of the game,
 *  notably NetHack--, NHplus, SLASH, and Slash'em.
 *  Adopt "the wizard patch" spellcasting system.
 *  Support for the Qt widget set.
 *  Y2K fix: use 4 digit year values for the dates in the score file
 *  updated COPYRIGHT_BANNER_A to reflect year of release.
 *  Dozens of other bug fixes, and minor improvements.
 */
/* Version 3.3 */

/*****************************************************************************/
/* Version 3.2.3 */

/*  Patch 3, December 10, 1999
 *  Released simultaneously with 3.3.0 for the benefit of
 *  ports and platforms that were unable to get working
 *  versions of 3.3.0 ready prior to the year 2000. It
 *  consisted of just a few bug fixes and offered no new
 *  functionality changes over 3.2.2.
 *
 *  Y2K fix: use 4 digit year values for the dates in the score file
 *  updated COPYRIGHT_BANNER_A to reflect year of release
 *  Fatal Mac bug removed
 *  DOS Makefile problem removed
 *  several bugs that could potentially trigger crashes removed
 */

/*  Patch 2, December 10, 1996
 *  fix the `recover' utility
 *  fix priest/minion name overflow which could cause Astral Plane crashes
 *  avoid crash when hit by own thrown boomerang
 *    "     "	 "   worn blindfold pushed off by applying cursed towel
 *  handle returning live Wizard correctly in deep dungeon levels
 *  don't occasionally display unseen areas of new levels during level change
 *  other minor display fixes
 *  fix several minor reason for death inconsistencies and shop bugs
 *  high dexterity doesn't guarantee that thrown eggs & potions will hit
 *
 *  Selected platform- or configuration-specific changes:
 *  Mac: update `mrecover'
 *  MSDOS: don't switch into tiles mode when resuming play on rogue level
 *  tty: support object class characters for 'I' command in menu mode
 *  Unix: work around several <curses.h> compilation problems
 *  X11: as tty above, plus implement tty-style count handling in menus;
 *	better window placement support for old window managers
 */

/*  Patch 1, May 28, 1996
 *  eliminate `#qualifications'; fix weapon proficiency handling for missiles
 *  keep Medusa from continuing to move after she's been killed by reflection
 *	of her own gaze (fixes relmon panic)
 *  make monsters a little smarter; assorted eating and chatting changes
 *  fix object amnesia for spellbooks; fix Nazgul's sleep gas attack
 *  fix bullwhip usage for case of having recently been in a trap
 *  egg hatching fixes, oil potion fixes, magic marker fixes
 *  support object class chars as selection accelerators for some menus
 *  stricter parsing of run-time options at startup time
 *  interactive setting of options via menu (courtesy Per Liboriussen)
 *
 *  Selected platform- or configuration-specific changes:
 *  Amiga: fix panic for tiles display in Gnomish mines
 *  BeOS: preliminary support for new BeBox platform; initially tty only
 *  DLB: avoid excessive fseek calls (major performance hit for MSDOS)
 *  HPUX: workaround for gcc-2.6.3 bug adversely affecting monster generation
 *  Mac: avoid MW 68K struct copy optimization bug which caused crashes;
 *	fix dragging of scrollbar; boost partitions to 2MB minimum
 *  MSDOS: wasn't safe to enter endgame for MFLOPPY configuration;
 *	fix re-entry into game after "!" (shell escape) + chdir + EXIT;
 *	F3/F4/F5 display interface swapping improvements;
 *	add support for preloading all tiles in protected mode environment
 *  TERMINFO: colors were wrong for some systems, such as Linux
 *  X11: display help files properly
 */

/*
 *  NetHack 3.2.0, April 11, 1996
 *  enhancements to the windowing systems including "tiles" or icons to
 *	visually represent monsters and objects (courtesy Warwick Allison)
 *  window based menu system introduced for inventory and selection
 *  moving light sources besides the player
 *  improved #untrap (courtesy Helge Hafting)
 *  spellcasting logic changes to balance spellcasting towards magic-using
 *	classes (courtesy Stephen White)
 *  many, many bug fixes and abuse eliminations
 */

/* Version 3.2 */

/*****************************************************************************/
/* Version 3.1.x */

/*
 *  Patch 3, July 12, 1993
 *  further revise Mac windowing and extend to Think C (courtesy
 *	Barton House)
 *  fix confusing black/gray/white display on some MSDOS hardware
 *  remove fatal bugs dealing with horns of plenty and VMS bones levels,
 *	as well as more minor ones
 */

/*
 *  Patch 2, June 1, 1993
 *  add tty windowing to Mac and Amiga ports and revise native windowing
 *  allow direct screen I/O for MS-DOS versions instead of going through
 *	termcap routines (courtesy Michael Allison and Kevin Smolkowski)
 *  changes for NEC PC-9800 and various termcap.zip fixes by Yamamoto Keizo
 *  SYSV 386 music driver ported to 386BSD (courtesy Andrew Chernov) and
 *	SCO UNIX (courtesy Andreas Arens)
 *  enhanced pickup and disclosure options
 *  removed fatal bugs dealing with cursed bags of holding, renaming
 *	shopkeepers, objects falling through trapdoors on deep levels,
 *	and kicking embedded objects loose, and many more minor ones
 */

/*
 *  Patch 1, February 25, 1993
 *  add Windows NT console port (courtesy Michael Allison)
 *  polishing of Amiga, Mac, and X11 windowing
 *  fixing many small bugs, including the infamous 3.0 nurse relmon bug
 */

/*
 *  NetHack 3.1.0, January 25, 1993
 *  many, many changes and bugfixes -- some of the highlights include:
 *  display rewrite using line-of-sight vision
 *  general window interface, with the ability to use multiple interfaces
 *	in the same executable
 *  intelligent monsters
 *  enhanced dungeon mythology
 *  branching dungeons with more special levels, quest dungeons, and
 *	multi-level endgame
 *  more artifacts and more uses for artifacts
 *  generalization to multiple shops with damage repair
 *  X11 interface
 *  ability to recover crashed games
 *  full rewrite of Macintosh port
 *  Amiga splitter
 *  directory rearrangement (dat, doc, sys, win, util)
 */

/* Version 3.1 */

/*****************************************************************************/
/* Version 3.0 */

/*
 *  Patch 10, February 5, 1991
 *  extend overlay manager to multiple files for easier binary distribution
 *  allow for more system and compiler variance
 *  remove more small insects
 */

/*
 *  Patch 9, June 26, 1990
 *  clear up some confusing documentation
 *  smooth some more rough edges in various ports
 *  and fix a couple more bugs
 */

/*
 *  Patch 8, June 3, 1990
 *  further debug and refine Macintosh port
 *  refine the overlay manager, rearrange the OVLx breakdown for better
 *	efficiency, rename the overlay macros, and split off the overlay
 *	instructions to Install.ovl
 *  introduce NEARDATA for better Amiga efficiency
 *  support for more VMS versions (courtesy Joshua Delahunty and Pat Rankin)
 *  more const fixes
 *  better support for common graphics (DEC VT and IBM)
 *  and a number of simple fixes and consistency extensions
 */

/*
 *  Patch 7, February 19, 1990
 *  refine overlay support to handle portions of .c files through OVLx
 *	(courtesy above plus Kevin Smolkowski)
 *  update and extend Amiga port and documentation (courtesy Richard Addison,
 *	Jochen Erwied, Mark Gooderum, Ken Lorber, Greg Olson, Mike Passaretti,
 *	and Gregg Wonderly)
 *  refine and extend Macintosh port and documentation (courtesy Johnny Lee,
 *	Kevin Sitze, Michael Sokolov, Andy Swanson, Jon Watte, and Tom West)
 *  refine VMS documentation
 *  continuing ANSIfication, this time of const usage
 *  teach '/' about differences within monster classes
 *  smarter eating code (yet again), death messages, and treatment of
 *	non-animal monsters, monster unconsciousness, and naming
 *  extended version command to give compilation options
 *  and the usual bug fixes and hole plugs
 */

/*
 *  Patch 6, November 19, 1989
 *  add overlay support for MS-DOS (courtesy Pierre Martineau, Stephen
 *	Spackman, and Norm Meluch)
 *  refine Macintosh port
 *  different door states show as different symbols (courtesy Ari Huttunen)
 *  smarter drawbridges (courtesy Kevin Darcy)
 *  add CLIPPING and split INFERNO off HARD
 *  further refine eating code wrt picking up and resumption
 *  make first few levels easier, by adding :x monsters and increasing initial
 *	attribute points and hitting probability
 *  teach '/' about configurable symbols
 */

/*
 *  Patch 5, October 15, 1989
 *  add support for Macintosh OS (courtesy Johnny Lee)
 *  fix annoying dependency loop via new color.h file
 *  allow interruption while eating -- general handling of partially eaten food
 *  smarter treatment of iron balls (courtesy Kevin Darcy)
 *  a handful of other bug fixes
 */

/*
 *  Patch 4, September 27, 1989
 *  add support for VMS (courtesy David Gentzel)
 *  move monster-on-floor references into functions and implement the new
 *	lookup structure for both objects and monsters
 *  extend the definitions of objects and monsters to provide "living color"
 *	in the dungeon, instead of a single monster color
 *  ifdef varargs usage to satisfy ANSI compilers
 *  standardize on the color 'gray'
 *  assorted bug fixes
 */

/*
 *  Patch 3, September 6, 1989
 *  add war hammers and revise object prices
 *  extend prototypes to ANSI compilers in addition to the previous MSDOS ones
 *  move object-on-floor references into functions in preparation for planned
 *	data structures to allow faster access and better colors
 *  fix some more bugs, and extend the portability of things added in earlier
 *	patches
 */

/*
 *  Patch 2, August 16, 1989
 *  add support for OS/2 (courtesy Timo Hakulinen)
 *  add a better makefile for MicroSoft C (courtesy Paul Gyugyi)
 *  more accomodation of compilers and preprocessors
 *  add better screen-size sensing
 *  expand color use for PCs and introduce it for SVR3 UNIX machines
 *  extend '/' to multiple identifications
 *  allow meta key to be used to invoke extended commands
 *  fix various minor bugs, and do further code cleaning
 */

/*
 *  Patch 1, July 31, 1989
 *  add support for Atari TOS (courtesy Eric Smith) and Andrew File System
 *	(courtesy Ralf Brown)
 *  include the uuencoded version of termcap.arc for the MSDOS versions that
 *	was included with 2.2 and 2.3
 *  make a number of simple changes to accommodate various compilers
 *  fix a handful of bugs, and do some code cleaning elsewhere
 *  add more instructions for new environments and things commonly done wrong
 */

/*
 *  NetHack 3.0 baseline release, July, 1989
 */

/* Version 3.0 */

/*****************************************************************************/

/*patchlevel.h*/
