/*	SCCS Id: @(#)allmain.c	3.3	1999/11/30	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* various code that was replicated in *main.c */

#include "hack.h"

#ifndef NO_SIGNAL
#include <signal.h>
#endif

#ifdef POSITIONBAR
STATIC_DCL void NDECL(do_positionbar);
#endif

#ifdef OVL0

void
moveloop()
{
#ifdef MICRO
	char ch;
	int abort_lev;
#endif
	int moveamt = 0, wtcap = 0, change = 0;
	boolean didmove = FALSE, monscanmove = FALSE;

	flags.moonphase = phase_of_the_moon();
	if(flags.moonphase == FULL_MOON) {
		You("are lucky!  Full moon tonight.");
		change_luck(1);
	} else if(flags.moonphase == NEW_MOON) {
		pline("Be careful!  New moon tonight.");
	}
	flags.friday13 = friday_13th();
	if (flags.friday13) {
		pline("Watch out!  Bad things can happen on Friday the 13th.");
		change_luck(-1);
	}

	initrack();


	/* Note:  these initializers don't do anything except guarantee that
		we're linked properly.
	*/
	decl_init();
	monst_init();
	monstr_init();	/* monster strengths */
	objects_init();

#ifdef WIZARD
	if (wizard) add_debug_extended_commands();
#endif

	(void) encumber_msg(); /* in case they auto-picked up something */

	u.uz0.dlevel = u.uz.dlevel;

	for(;;) {
#ifdef CLIPPING
		cliparound(u.ux, u.uy);
#endif
		get_nh_event();
#ifdef POSITIONBAR
		do_positionbar();
#endif

		didmove = flags.move;
		if(didmove) {
		    /* actual time passed */
		    if (youmonst.movement >= NORMAL_SPEED) {
			youmonst.movement -= NORMAL_SPEED;
			++moves;
		    }

		    if (u.utotype) deferred_goto();
		    wtcap = encumber_msg();
		    dosounds();

		    flags.mon_moving = TRUE;
		    do {
			monscanmove = movemon();
			if (youmonst.movement > NORMAL_SPEED)
			    break;	/* it's now your turn */
		    } while (monscanmove);
		    flags.mon_moving = FALSE;

		    if (!monscanmove && youmonst.movement < NORMAL_SPEED) {
			/* both you and the monsters are out of steam this round */
			struct monst *mtmp;
			mcalcdistress();	/* adjust monsters' trap, blind, etc */

			/* reallocate movement rations to monsters */
			for (mtmp = fmon; mtmp; mtmp = mtmp->nmon)
			    mcalcmove(mtmp);

			if(!rn2(u.uevent.udemigod ? 25 :
				(depth(&u.uz) > depth(&stronghold_level)) ? 50 : 70))
			    (void) makemon((struct permonst *)0, 0, 0, NO_MM_FLAGS);

			/* calculate how much time passed. */
#ifdef STEED
		      if (u.usteed && flags.mv) {
			/* your speed doesn't augment steed's speed */
			moveamt = u.usteed->movement;
		      } else {
#endif
			moveamt = youmonst.data->mmove;

			if (Very_fast) {	/* speed boots or potion */
			    /* average movement is 1.67 times normal */
			    moveamt += NORMAL_SPEED / 2;
			    if (rn2(3) == 0) moveamt += NORMAL_SPEED / 2;
			} else if (Fast) {
			    /* average movement is 1.33 times normal */
			    if (rn2(3) != 0) moveamt += NORMAL_SPEED / 2;
			}
#ifdef STEED
		      }
#endif
			switch (wtcap) {
			case UNENCUMBERED: break;
			case SLT_ENCUMBER: moveamt -= (moveamt / 4); break;
			case MOD_ENCUMBER: moveamt -= (moveamt / 2); break;
			case HVY_ENCUMBER: moveamt -= ((moveamt * 3) / 4); break;
			case EXT_ENCUMBER: moveamt -= ((moveamt * 7) / 8); break;
			default: break;
			}

			youmonst.movement += moveamt;
			if (youmonst.movement < 0) youmonst.movement = 0;
			settrack();

			monstermoves++;
		    }			

		    if(Glib) glibr();
		    nh_timeout();
		    run_regions();

		    if (u.ublesscnt)  u.ublesscnt--;
		    if(flags.time && !flags.run)
			    flags.botl = 1;

		    /* One possible result of prayer is healing.  Whether or
		     * not you get healed depends on your current hit points.
		     * If you are allowed to regenerate during the prayer, the
		     * end-of-prayer calculation messes up on this.
		     * Another possible result is rehumanization, which requires
		     * that encumbrance and movement rate be recalculated.
		     */
		    if (u.uinvulnerable) {
			/* for the moment at least, you're in tiptop shape */
			wtcap = UNENCUMBERED;
		    } else if (Upolyd && u.mh < u.mhmax) {
			if (u.mh < 1)
			   rehumanize();
			else if (Regeneration ||
				    (wtcap < MOD_ENCUMBER && !(moves%20))) {
			    flags.botl = 1;
			    u.mh++;
			}
		    } else if (u.uhp < u.uhpmax &&
			 (wtcap < MOD_ENCUMBER || !flags.mv || Regeneration)) {
			if (u.ulevel > 9 && !(moves % 3)) {
			    int heal, Con = (int) ACURR(A_CON);

			    if (Con <= 12) {
				heal = 1;
			    } else {
				heal = rnd(Con);
				if (heal > u.ulevel-9) heal = u.ulevel-9;
			    }
			    flags.botl = 1;
			    u.uhp += heal;
			    if(u.uhp > u.uhpmax)
				u.uhp = u.uhpmax;
			} else if (Regeneration ||
			     (u.ulevel <= 9 &&
			      !(moves % ((MAXULEV+12) / (u.ulevel+2) + 1)))) {
			    flags.botl = 1;
			    u.uhp++;
			}
		    }

		    if (wtcap > MOD_ENCUMBER && flags.mv) {
			if(!(wtcap < EXT_ENCUMBER ? moves%30 : moves%10)) {
			    if (Upolyd && u.mh > 1) {
				u.mh--;
			    } else if (!Upolyd && u.uhp > 1) {
				u.uhp--;
			    } else {
				You("pass out from exertion!");
				exercise(A_CON, FALSE);
				fall_asleep(-10, FALSE);
			    }
			}
		    }

		    if ((u.uen < u.uenmax) &&
			((wtcap < MOD_ENCUMBER &&
			  (!(moves%((MAXULEV + 8 - u.ulevel) *
				    (Role_if(PM_WIZARD) ? 3 : 4) / 6))))
			 || Energy_regeneration)) {
			u.uen += rn1((int)(ACURR(A_WIS) + ACURR(A_INT)) / 15 + 1,1);
			if (u.uen > u.uenmax)  u.uen = u.uenmax;
			flags.botl = 1;
		    }

		    if(!u.uinvulnerable) {
			if(Teleportation && !rn2(85)) {
#ifdef REDO
			    xchar old_ux = u.ux, old_uy = u.uy;
#endif
			    tele();
#ifdef REDO
			    if (u.ux != old_ux || u.uy != old_uy) {
			    	/* clear doagain keystrokes */
				pushch(0);
				savech(0);
			    }
#endif
			}
			if(Polymorph && !rn2(100))
			    change = 1;
			else if (u.ulycn >= LOW_PM && !rn2(80 - (20 * night())))
			    change = 2;
			if (change && !Unchanging) {
			    if (multi >= 0) {
				if (occupation)
				    stop_occupation();
				else
				    nomul(0);
				if (change == 1) polyself();
				else you_were();
				change = 0;
			    }
			}
		    }

		    if(Searching && multi >= 0) (void) dosearch0(1);
		    do_storms();
		    gethungry();
		    age_spells();
		    exerchk();
		    invault();
		    if (u.uhave.amulet) amulet();
		    if (!rn2(40+(int)(ACURR(A_DEX)*3)))
			u_wipe_engr(rnd(3));
		    if (u.uevent.udemigod && !u.uinvulnerable) {
			if (u.udg_cnt) u.udg_cnt--;
			if (!u.udg_cnt) {
			    intervene();
			    u.udg_cnt = rn1(200, 50);
			}
		    }
		    restore_attrib();
		    /* underwater and waterlevel vision are done here */
		    if (Is_waterlevel(&u.uz))
			movebubbles();
		    else if (Underwater)
		    	under_water(0);
		    /* vision while buried done here */
		    else if (u.uburied) under_ground(0);
		}
		if(multi < 0) {
		    if (++multi == 0)	/* finished yet? */
			unmul((char *)0);
		}

		find_ac();
		if(!flags.mv || Blind) {
		    /* redo monsters if hallu or wearing a helm of telepathy */
		    if (Hallucination) {	/* update screen randomly */
			see_monsters();
			see_objects();
			see_traps();
			if (u.uswallow) swallowed(0);
		    } else if (Unblind_telepat) {
			see_monsters();
		    }
		    if (vision_full_recalc) vision_recalc(0);	/* vision! */
		}
		if(flags.botl || flags.botlx) bot();

		flags.move = 1;

		if(multi >= 0 && occupation) {
#ifdef MICRO
		    abort_lev = 0;
		    if (kbhit()) {
			if ((ch = Getchar()) == ABORT)
			    abort_lev++;
# ifdef REDO
			else
			    pushch(ch);
# endif /* REDO */
		    }
		    if (!abort_lev && (*occupation)() == 0)
#else
		    if ((*occupation)() == 0)
#endif
			occupation = 0;
		    if(
#ifdef MICRO
			   abort_lev ||
#endif
			   monster_nearby()) {
			stop_occupation();
			reset_eat();
		    }
#ifdef MICRO
		    if (!(++occtime % 7))
			display_nhwindow(WIN_MAP, FALSE);
#endif
		    continue;
		}

		if ((u.uhave.amulet || Clairvoyant) &&
		    !In_endgame(&u.uz) && !BClairvoyant &&
		    !(moves % 15) && !rn2(2))
			do_vicinity_map();

		if(u.utrap && u.utraptype == TT_LAVA) {
		    if(!is_lava(u.ux,u.uy))
			u.utrap = 0;
		    else {
			u.utrap -= 1<<8;
			if(u.utrap < 1<<8) {
			    killer_format = KILLED_BY;
			    killer = "molten lava";
			    You("sink below the surface and die.");
			    done(DISSOLVED);
			} else if(didmove && !u.umoved) {
			    Norep("You sink deeper into the lava.");
			    u.utrap += rnd(4);
			}
		    }
		}

#ifdef WIZARD
		if (iflags.sanity_check)
		    sanity_check();
#endif

		u.umoved = FALSE;

		if (multi > 0) {
		    lookaround();
		    if (!multi) {
			/* lookaround may clear multi */
			flags.move = 0;
			continue;
		    }
		    if (flags.mv) {
			if(multi < COLNO && !--multi)
			    flags.mv = flags.run = 0;
			domove();
		    } else {
			--multi;
			rhack(save_cm);
		    }
		} else if (multi == 0) {
#ifdef MAIL
		    ckmailstatus();
#endif
		    rhack((char *)0);
		}
		if (u.utotype)		/* change dungeon level */
		    deferred_goto();	/* after rhack() */
		/* !flags.move here: multiple movement command stopped */
		else if (flags.time && (!flags.move || !flags.mv))
		    flags.botl = 1;

		if (vision_full_recalc) vision_recalc(0);	/* vision! */
		if (multi && multi%7 == 0)
		    display_nhwindow(WIN_MAP, FALSE);
	}
}

#endif /* OVL0 */
#ifdef OVL1

void
stop_occupation()
{
	if(occupation) {
		You("stop %s.", occtxt);
		occupation = 0;
/* fainting stops your occupation, there's no reason to sync.
		sync_hunger();
*/
#ifdef REDO
		nomul(0);
		pushch(0);
#endif
	}
}

#endif /* OVL1 */
#ifdef OVLB

void
display_gamewindows()
{
    WIN_MESSAGE = create_nhwindow(NHW_MESSAGE);
    WIN_STATUS = create_nhwindow(NHW_STATUS);
    WIN_MAP = create_nhwindow(NHW_MAP);
    WIN_INVEN = create_nhwindow(NHW_MENU);

#ifdef MAC
    /*
     * This _is_ the right place for this - maybe we will
     * have to split display_gamewindows into create_gamewindows
     * and show_gamewindows to get rid of this ifdef...
     */
	if ( ! strcmp ( windowprocs . name , "mac" ) ) {
	    SanePositions ( ) ;
	}
#endif

    /*
     * The mac port is not DEPENDENT on the order of these
     * displays, but it looks a lot better this way...
     */
    display_nhwindow(WIN_STATUS, FALSE);
    display_nhwindow(WIN_MESSAGE, FALSE);
    clear_glyph_buffer();
    display_nhwindow(WIN_MAP, FALSE);
}

void
newgame()
{
	int i;

#ifdef MFLOPPY
	gameDiskPrompt();
#endif

	flags.ident = 1;

	for (i = 0; i < NUMMONS; i++)
		mvitals[i].mvflags = mons[i].geno & G_NOCORPSE;

	init_objects();		/* must be before u_init() */
	u_init();
	init_dungeons();	/* must be after u_init() */
	init_artifacts();	/* must be after u_init() */

#ifndef NO_SIGNAL
	(void) signal(SIGINT, (SIG_RET_TYPE) done1);
#endif
#ifdef NEWS
	if(iflags.news) display_file(NEWS, FALSE);
#endif
	load_qtlist();	/* load up the quest text info */
/*	quest_init();*/	/* Now part of role_init() */

	mklev();
	u_on_upstairs();
	vision_reset();		/* set up internals for level (after mklev) */
	check_special_room(FALSE);

	flags.botlx = 1;

	/* Move the monster from under you or else
	 * makedog() will fail when it calls makemon().
	 *			- ucsfcgl!kneller
	 */
	if(MON_AT(u.ux, u.uy)) mnexto(m_at(u.ux, u.uy));
	(void) makedog();
	docrt();

	if (flags.legacy) {
		flush_screen(1);
		com_pager(1);
	}

#ifdef INSURANCE
	save_currentstate();
#endif
	program_state.something_worth_saving++;	/* useful data now exists */

	/* Success! */
	welcome(TRUE);
	return;
}

/* show "welcome [back] to nethack" message at program startup */
void
welcome(new_game)
boolean new_game;	/* false => restoring an old game */
{
    char buf[BUFSZ];
    boolean currentgend = Upolyd ? u.mfemale : flags.female;

    /*
     * The "welcome back" message always describes your innate form
     * even when polymorphed or wearing a helm of opposite alignment.
     * Alignment is shown unconditionally for new games; for restores
     * it's only shown if it has changed from its original value.
     * Sex is shown for new games except when it is redundant; for
     * restores it's only shown if different from its original value.
     */
    *buf = '\0';
    if (new_game || u.ualignbase[1] != u.ualignbase[0])
	Sprintf(eos(buf), " %s", align_str(u.ualignbase[1]));
    if (!urole.name.f &&
	    (new_game ? (urole.allow & ROLE_GENDMASK) == (ROLE_MALE|ROLE_FEMALE) :
	     currentgend != flags.initgend))
	Sprintf(eos(buf), " %s", genders[currentgend].adj);

    pline(new_game ? "%s %s, welcome to NetHack!  You are a%s %s %s."
		   : "%s %s, the%s %s %s, welcome back to NetHack!",
	  Hello(), plname, buf, urace.adj,
	  (currentgend && urole.name.f) ? urole.name.f : urole.name.m);
}

#ifdef POSITIONBAR
STATIC_DCL void
do_positionbar()
{
	static char pbar[COLNO];
	char *p;
	
	p = pbar;
	/* up stairway */
	if (upstair.sx &&
	   (glyph_to_cmap(level.locations[upstair.sx][upstair.sy].glyph) ==
	    S_upstair ||
 	    glyph_to_cmap(level.locations[upstair.sx][upstair.sy].glyph) ==
	    S_upladder)) {
		*p++ = '<';
		*p++ = upstair.sx;
	}
	if (sstairs.sx &&
	   (glyph_to_cmap(level.locations[sstairs.sx][sstairs.sy].glyph) ==
	    S_upstair ||
 	    glyph_to_cmap(level.locations[sstairs.sx][sstairs.sy].glyph) ==
	    S_upladder)) {
		*p++ = '<';
		*p++ = sstairs.sx;
	}

	/* down stairway */
	if (dnstair.sx &&
	   (glyph_to_cmap(level.locations[dnstair.sx][dnstair.sy].glyph) ==
	    S_dnstair ||
 	    glyph_to_cmap(level.locations[dnstair.sx][dnstair.sy].glyph) ==
	    S_dnladder)) {
		*p++ = '>';
		*p++ = dnstair.sx;
	}
	if (sstairs.sx &&
	   (glyph_to_cmap(level.locations[sstairs.sx][sstairs.sy].glyph) ==
	    S_dnstair ||
 	    glyph_to_cmap(level.locations[sstairs.sx][sstairs.sy].glyph) ==
	    S_dnladder)) {
		*p++ = '>';
		*p++ = sstairs.sx;
	}

	/* hero location */
	if (u.ux) {
		*p++ = '@';
		*p++ = u.ux;
	}
	/* fence post */
	*p = 0;

	update_positionbar(pbar);
}
#endif

#endif /* OVLB */

/*allmain.c*/
