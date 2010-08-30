%option nounput
%option noinput

%{
/*	SCCS Id: @(#)dgn_lex.c	3.4	2002/03/27	*/
/*	Copyright (c) 1989 by Jean-Christophe Collet */
/*	Copyright (c) 1990 by M. Stephenson	     */
/* NetHack may be freely redistributed.  See license for details. */

#define DGN_COMP

#include "config.h"
#include "dgn_comp.h"
#include "dgn_file.h"

/*
 * Most of these don't exist in flex, yywrap is macro and
 * yyunput is properly declared in flex.skel.
 */
#if !defined(FLEX_SCANNER) && !defined(FLEXHACK_SCANNER)
int FDECL(yyback, (int *,int));
int NDECL(yylook);
int NDECL(yyinput);
int NDECL(yywrap);
int NDECL(yylex);
void FDECL(yyunput, (int));
void FDECL(yyoutput, (int));
#endif	/* !FLEX_SCANNER && !FLEXHACK_SCANNER */

#ifdef FLEX_SCANNER
#define YY_MALLOC_DECL \
	       genericptr_t FDECL(malloc, (size_t)); \
	       genericptr_t FDECL(realloc, (genericptr_t,size_t));
#endif


void FDECL(init_yyin, (FILE *));
void FDECL(init_yyout, (FILE *));

/* this doesn't always get put in dgn_comp.h
 * (esp. when using older versions of bison)
 */

extern YYSTYPE yylval;

int line_number = 1;

%}
%%
DUNGEON		return(A_DUNGEON);
up		{ yylval.i=1; return(UP_OR_DOWN); }
down		{ yylval.i=0; return(UP_OR_DOWN); }
ENTRY		return(ENTRY);
stair		return(STAIR);
no_up		return(NO_UP);
no_down		return(NO_DOWN);
portal		return(PORTAL);
PROTOFILE	return(PROTOFILE);
DESCRIPTION	return(DESCRIPTION);
LEVELDESC	return(LEVELDESC);
ALIGNMENT       return(ALIGNMENT);
LEVALIGN        return(LEVALIGN);
town		{ yylval.i=TOWN ; return(DESCRIPTOR); }
hellish		{ yylval.i=HELLISH ; return(DESCRIPTOR); }
mazelike	{ yylval.i=MAZELIKE ; return(DESCRIPTOR); }
roguelike	{ yylval.i=ROGUELIKE ; return(DESCRIPTOR); }
unaligned       { yylval.i=D_ALIGN_NONE ; return(DESCRIPTOR); }
noalign         { yylval.i=D_ALIGN_NONE ; return(DESCRIPTOR); }
lawful          { yylval.i=D_ALIGN_LAWFUL ; return(DESCRIPTOR); }
neutral         { yylval.i=D_ALIGN_NEUTRAL ; return(DESCRIPTOR); }
chaotic         { yylval.i=D_ALIGN_CHAOTIC ; return(DESCRIPTOR); }
BRANCH		return(BRANCH);
CHAINBRANCH	return(CHBRANCH);
LEVEL		return(LEVEL);
RNDLEVEL	return(RNDLEVEL);
CHAINLEVEL	return(CHLEVEL);
RNDCHLEVEL	return(RNDCHLEVEL);
[-0-9]+		{ yylval.i=atoi(yytext); return(INTEGER); }
\"[^"]*\"	{ yytext[yyleng-1] = 0; /* Discard the trailing \" */
		  yylval.str = (char *) alloc(strlen(yytext+1)+1);
		  Strcpy(yylval.str, yytext+1); /* Discard the first \" */
		  return(STRING); }
^#.*\n		{ line_number++; }
\r?\n		{ line_number++; }
[ \t]+		;	/* skip trailing tabs & spaces */
.		{ return yytext[0]; }
%%

/* routine to switch to another input file; needed for flex */
void init_yyin( input_f )
FILE *input_f;
{
#if defined(FLEX_SCANNER) || defined(FLEXHACK_SCANNER)
	if (yyin)
	    yyrestart(input_f);
	else
#endif
	    yyin = input_f;
}
/* analogous routine (for completeness) */
void init_yyout( output_f )
FILE *output_f;
{
	yyout = output_f;
}

/*dgn_comp.l*/
