/*	SCCS Id: @(#)dlb.h	3.4	1997/07/29	*/
/* Copyright (c) Kenneth Lorber, Bethesda, Maryland, 1993. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef DLB_H
#define DLB_H

# define dlb FILE

# define dlb_init()
# define dlb_cleanup()

# define dlb_fopen	fopen
# define dlb_fclose	fclose
# define dlb_fread	fread
# define dlb_fseek	fseek
# define dlb_fgets	fgets
# define dlb_fgetc	fgetc
# define dlb_ftell	ftell

/* various other I/O stuff we don't want to replicate everywhere */

#ifndef SEEK_SET
# define SEEK_SET 0
#endif
#ifndef SEEK_CUR
# define SEEK_CUR 1
#endif
#ifndef SEEK_END
# define SEEK_END 2
#endif

#define RDTMODE "r"
#if (defined(MSDOS) || defined(WIN32) || defined(TOS) || defined(OS2)) && defined(DLB)
#define WRTMODE "w+b"
#else
#define WRTMODE "w+"
#endif
#if (defined(MICRO) && !defined(AMIGA)) || defined(THINK_C) || defined(__MWERKS__) || defined(WIN32)
# define RDBMODE "rb"
# define WRBMODE "w+b"
#else
# define RDBMODE "r"
# define WRBMODE "w+"
#endif

#endif	/* DLB_H */
