/*	SCCS Id: @(#)dogmove.c	3.3	97/05/25	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

#include "mfndpos.h"
#include "edog.h"

extern boolean notonhead;

#ifdef OVL0

STATIC_DCL boolean FDECL(dog_hunger,(struct monst *,struct edog *));
STATIC_DCL int FDECL(dog_invent,(struct monst *,struct edog *,int));
STATIC_DCL int FDECL(dog_goal,(struct monst *,struct edog *,int,int,int));

STATIC_DCL struct obj *FDECL(DROPPABLES, (struct monst *));

STATIC_OVL struct obj *
DROPPABLES(mon)
register struct monst *mon;
{
	register struct obj *obj;
	struct obj *wep = MON_WEP(mon);
	boolean item1 = FALSE, item2 = FALSE;

	if (is_animal(mon->data) || mindless(mon->data))
		item1 = item2 = TRUE;
	if (!tunnels(mon->data) || !needspick(mon->data))
		item1 = TRUE;
	for(obj = mon->minvent; obj; obj = obj->nobj) {
		if (!item1 && is_pick(obj)) {
			item1 = TRUE;
			continue;
		}
		if (!item2 && obj->otyp == UNICORN_HORN && !obj->cursed) {
			item2 = TRUE;
			continue;
		}
		if (!obj->owornmask && obj != wep) return obj;
	}
	return (struct obj *)0;
}

static NEARDATA const char nofetch[] = { BALL_CLASS, CHAIN_CLASS, ROCK_CLASS, 0 };

#endif /* OVL0 */

STATIC_OVL boolean FDECL(cursed_object_at, (int, int));

STATIC_VAR xchar gtyp, gx, gy;	/* type and position of dog's current goal */

STATIC_PTR void FDECL(wantdoor, (int, int, genericptr_t));

#ifdef OVLB
STATIC_OVL boolean
cursed_object_at(x, y)
int x, y;
{
	struct obj *otmp;

	for(otmp = level.objects[x][y]; otmp; otmp = otmp->nexthere)
		if (otmp->cursed) return TRUE;
	return FALSE;
}

int
dog_nutrition(mtmp, obj)
struct monst *mtmp;
struct obj *obj;
{
	int nutrit;

	/*
	 * It is arbitrary that the pet takes the same length of time to eat
	 * as a human, but gets more nutritional value.
	 */
	if (obj->oclass == FOOD_CLASS) {
	    if(obj->otyp == CORPSE) {
		mtmp->meating = 3 + (mons[obj->corpsenm].cwt >> 6);
		nutrit = mons[obj->corpsenm].cnutrit;
	    } else {
		mtmp->meating = objects[obj->otyp].oc_delay;
		nutrit = objects[obj->otyp].oc_nutrition;
	    }
	    switch(mtmp->data->msize) {
		case MZ_TINY: nutrit *= 8; break;
		case MZ_SMALL: nutrit *= 6; break;
		default:
		case MZ_MEDIUM: nutrit *= 5; break;
		case MZ_LARGE: nutrit *= 4; break;
		case MZ_HUGE: nutrit *= 3; break;
		case MZ_GIGANTIC: nutrit *= 2; break;
	    }
	    if(obj->oeaten) {
		mtmp->meating = eaten_stat(mtmp->meating, obj);
		nutrit = eaten_stat(nutrit, obj);
	    }
	} else if (obj->oclass == GOLD_CLASS) {
	    mtmp->meating = (int)(obj->quan/2000) + 1;
	    if (mtmp->meating < 0) mtmp->meating = 1;
	    nutrit = (int)(obj->quan/20);
	    if (nutrit < 0) nutrit = 0;
	} else {
	    /* Unusual pet such as gelatinous cube eating odd stuff.
	     * meating made consistent with wild monsters in mon.c.
	     * nutrit made consistent with polymorphed player nutrit in
	     * eat.c.  (This also applies to pets eating gold.)
	     */
	    mtmp->meating = obj->owt/20 + 1;
	    nutrit = 5*objects[obj->otyp].oc_nutrition;
	}
	return nutrit;
}

/* returns 2 if pet dies, otherwise 1 */
int
dog_eat(mtmp, obj, x, y, devour)
register struct monst *mtmp;
register struct obj * obj;
int x, y;
boolean devour;
{
	register struct edog *edog = EDOG(mtmp);
	boolean poly = FALSE, grow = FALSE, heal = FALSE;
	int nutrit;

	if(edog->hungrytime < monstermoves)
	    edog->hungrytime = monstermoves;
	nutrit = dog_nutrition(mtmp, obj);
	poly = polyfodder(obj);
	grow = mlevelgain(obj);
	heal = mhealup(obj);
	if (devour) {
	    if (mtmp->meating > 1) mtmp->meating /= 2;
	    if (nutrit > 1) nutrit = (nutrit * 3) / 4;
	}
	edog->hungrytime += nutrit;
	mtmp->mconf = 0;
	if (mtmp->mflee && mtmp->mfleetim > 1) mtmp->mfleetim /= 2;
	if (mtmp->mtame < 20) mtmp->mtame++;
	if (x != mtmp->mx || y != mtmp->my) {	/* moved & ate on same turn */
	    newsym(x, y);
	    newsym(mtmp->mx, mtmp->my);
	}
	if (is_pool(x, y) && !Underwater) {
	    /* Don't print obj */
	    /* TODO: Reveal presence of sea monster (especially sharks) */
	} else
	/* hack: observe the action if either new or old location is in view */
	if (cansee(x, y) || cansee(mtmp->mx, mtmp->my))
	    pline("%s %s %s.", Monnam(mtmp),
		  devour ? "devours" : "eats",
		  (obj->oclass == FOOD_CLASS) ?
			singular(obj, doname) : doname(obj));
	/* It's a reward if it's DOGFOOD and the player dropped/threw it. */
	/* We know the player had it if invlet is set -dlc */
	if(dogfood(mtmp,obj) == DOGFOOD && obj->invlet)
#ifdef LINT
	    edog->apport = 0;
#else
	    edog->apport += (int)(200L/
		((long)edog->dropdist + monstermoves - edog->droptime));
#endif
	if (mtmp->data == &mons[PM_RUST_MONSTER] && obj->oerodeproof) {
	    /* The object's rustproofing is gone now */
	    obj->oerodeproof = 0;
	    mtmp->mstun = 1;
	    if (canseemon(mtmp) && flags.verbose) {
		pline("%s spits %s out in disgust!",
		      Monnam(mtmp), distant_name(obj,doname));
	    }
	} else if (obj == uball) {
	    unpunish();
	    delobj(obj);
	} else if (obj == uchain)
	    unpunish();
	else if (obj->quan > 1L && obj->oclass == FOOD_CLASS)
	    obj->quan--;
	else
	    delobj(obj);

	if (poly) {
	    char oldpet[BUFSZ];
#ifdef STEED
	    long mw = mtmp->misc_worn_check;

	    mtmp->misc_worn_check &= ~W_SADDLE;
#endif
	    Strcpy(oldpet, Monnam(mtmp));
#ifdef STEED
	    mtmp->misc_worn_check = mw;
#endif
	    if (newcham(mtmp, (struct permonst *)0) &&
			cansee(mtmp->mx, mtmp->my)) {
		uchar save_mnamelth = mtmp->mnamelth;
		mtmp->mnamelth = 0;
		pline("%s turns into %s!", oldpet, a_monnam(mtmp));
		mtmp->mnamelth = save_mnamelth;
	    }
	}
	/* limit "instant" growth to prevent potential abuse */
	if (grow && (int) mtmp->m_lev < (int)mtmp->data->mlevel + 15) {
	    if (!grow_up(mtmp, (struct monst *)0)) return 2;
	}
	if (heal) mtmp->mhp = mtmp->mhpmax;
	return 1;
}

#endif /* OVLB */
#ifdef OVL0

/* hunger effects -- returns TRUE on starvation */
STATIC_OVL boolean
dog_hunger(mtmp, edog)
register struct monst *mtmp;
register struct edog *edog;
{
	if (monstermoves > edog->hungrytime + 500) {
	    if (!carnivorous(mtmp->data) && !herbivorous(mtmp->data)) {
		edog->hungrytime = monstermoves + 500;
		/* but not too high; it might polymorph */
	    } else if (!mtmp->mconf) {
		mtmp->mconf = 1;
		mtmp->mhpmax /= 3;
		if (mtmp->mhp > mtmp->mhpmax)
		    mtmp->mhp = mtmp->mhpmax;
		if (mtmp->mhp < 1) goto dog_died;
		if (cansee(mtmp->mx, mtmp->my))
		    pline("%s is confused from hunger.", Monnam(mtmp));
		else if (couldsee(mtmp->mx, mtmp->my))
		    beg(mtmp);
		else
		    You_feel("worried about %s.", y_monnam(mtmp));
	    } else if (monstermoves > edog->hungrytime + 750 || mtmp->mhp < 1) {
	    dog_died:
		if (mtmp->mleashed)
		    Your("leash goes slack.");
		else if (cansee(mtmp->mx, mtmp->my))
		    pline("%s dies%s.", Monnam(mtmp),
			    (mtmp->mhp >= 1) ? "" : " from hunger");
		else
		    You_feel("%s for a moment.",
			Hallucination ? "bummed" : "sad");
		mondied(mtmp);
		return(TRUE);
	    }
	}
	return(FALSE);
}

/* do something with object (drop, pick up, eat) at current position
 * returns 1 if object eaten (since that counts as dog's move), 2 if died
 */
STATIC_OVL int
dog_invent(mtmp, edog, udist)
register struct monst *mtmp;
register struct edog *edog;
int udist;
{
	register int omx, omy;
	struct obj *obj;

	if (mtmp->msleeping || !mtmp->mcanmove) return(0);

	omx = mtmp->mx;
	omy = mtmp->my;

	/* if we are carrying sth then we drop it (perhaps near @) */
	/* Note: if apport == 1 then our behaviour is independent of udist */
	/* Use udist+1 so steed won't cause divide by zero */
	if(DROPPABLES(mtmp) || mtmp->mgold) {
	    if (!rn2(udist+1) || !rn2(edog->apport))
		if(rn2(10) < edog->apport){
		    relobj(mtmp, (int)mtmp->minvis, TRUE);
		    if(edog->apport > 1) edog->apport--;
		    edog->dropdist = udist;		/* hpscdi!jon */
		    edog->droptime = monstermoves;
		}
	} else {
	    if((obj=level.objects[omx][omy]) && !index(nofetch,obj->oclass)
#ifdef MAIL
			&& obj->otyp != SCR_MAIL
#endif
									){
		if (dogfood(mtmp, obj) <= CADAVER)
		    return dog_eat(mtmp, obj, omx, omy, FALSE);

		if(can_carry(mtmp, obj) && !obj->cursed)
		    if(rn2(20) < edog->apport+3)
			if (rn2(udist) || !rn2(edog->apport)) {
			    if (cansee(omx, omy) && flags.verbose)
				pline("%s picks up %s.", Monnam(mtmp),
				    distant_name(obj, doname));
			    obj_extract_self(obj);
			    newsym(omx,omy);
			    mpickobj(mtmp,obj);
			    if (attacktype(mtmp->data, AT_WEAP)) {
				mtmp->weapon_check = NEED_HTH_WEAPON;
				(void) mon_wield_item(mtmp);
			    }
			}
	    }
	}
	return 0;
}

/* set dog's goal -- gtyp, gx, gy
 * returns -1/0/1 (dog's desire to approach player) or -2 (abort move)
 */
STATIC_OVL int
dog_goal(mtmp, edog, after, udist, whappr)
register struct monst *mtmp;
struct edog *edog;
int after, udist, whappr;
{
	register int omx, omy;
	boolean in_masters_sight;
	register struct obj *obj;
	xchar otyp;
	int appr;


#ifdef STEED
	/* Steeds don't move on their own will */
	if (mtmp == u.usteed)
		return (-2);
#endif

	omx = mtmp->mx;
	omy = mtmp->my;

	in_masters_sight = couldsee(omx, omy);

	if (!edog || mtmp->mleashed) {	/* he's not going anywhere... */
	    gtyp = APPORT;
	    gx = u.ux;
	    gy = u.uy;
	} else {
#define DDIST(x,y) (dist2(x,y,omx,omy))
#define SQSRCHRADIUS 5
	    int min_x, max_x, min_y, max_y;
	    register int nx, ny;

	    gtyp = UNDEF;	/* no goal as yet */
	    gx = gy = 0;	/* suppress 'used before set' message */

	    if ((min_x = omx - SQSRCHRADIUS) < 0) min_x = 0;
	    if ((max_x = omx + SQSRCHRADIUS) >= COLNO) max_x = COLNO - 1;
	    if ((min_y = omy - SQSRCHRADIUS) < 0) min_y = 0;
	    if ((max_y = omy + SQSRCHRADIUS) >= ROWNO) max_y = ROWNO - 1;

	    /* nearby food is the first choice, then other objects */
	    for (obj = fobj; obj; obj = obj->nobj) {
		nx = obj->ox;
		ny = obj->oy;
		if (nx >= min_x && nx <= max_x && ny >= min_y && ny <= max_y) {
		    otyp = dogfood(mtmp, obj);
		    if (otyp > gtyp || otyp == UNDEF)
			continue;
		    if (cursed_object_at(nx, ny))
			continue;
		    if (otyp < MANFOOD) {
			if (otyp < gtyp || DDIST(nx,ny) < DDIST(gx,gy)) {
			    gx = nx;
			    gy = ny;
			    gtyp = otyp;
			}
		    } else if(gtyp == UNDEF && in_masters_sight &&
			      !mtmp->minvent &&
			      (!levl[omx][omy].lit || levl[u.ux][u.uy].lit) &&
			      (otyp == MANFOOD || m_cansee(mtmp, nx, ny)) &&
			      edog->apport > rn2(8) &&
			      can_carry(mtmp,obj)) {
			gx = nx;
			gy = ny;
			gtyp = APPORT;
		    }
		}
	    }
	}

	/* follow player if appropriate */
	if (gtyp == UNDEF ||
	    (gtyp != DOGFOOD && gtyp != APPORT && monstermoves < edog->hungrytime)) {
		gx = u.ux;
		gy = u.uy;
		if (after && udist <= 4 && gx == u.ux && gy == u.uy)
			return(-2);
		appr = (udist >= 9) ? 1 : (mtmp->mflee) ? -1 : 0;
		if (udist > 1) {
			if (!IS_ROOM(levl[u.ux][u.uy].typ) || !rn2(4) ||
			   whappr ||
			   (mtmp->minvent && rn2(edog->apport)))
				appr = 1;
		}
		/* if you have dog food it'll follow you more closely */
		if (appr == 0) {
			obj = invent;
			while (obj) {
				if(dogfood(mtmp, obj) == DOGFOOD) {
					appr = 1;
					break;
				}
				obj = obj->nobj;
			}
		}
	} else
	    appr = 1;	/* gtyp != UNDEF */
	if(mtmp->mconf)
	    appr = 0;

#define FARAWAY (COLNO + 2)		/* position outside screen */
	if (gx == u.ux && gy == u.uy && !in_masters_sight) {
	    register coord *cp;

	    cp = gettrack(omx,omy);
	    if (cp) {
		gx = cp->x;
		gy = cp->y;
		if(edog) edog->ogoal.x = 0;
	    } else {
		/* assume master hasn't moved far, and reuse previous goal */
		if(edog && edog->ogoal.x &&
		   ((edog->ogoal.x != omx) || (edog->ogoal.y != omy))) {
		    gx = edog->ogoal.x;
		    gy = edog->ogoal.y;
		    edog->ogoal.x = 0;
		} else {
		    int fardist = FARAWAY * FARAWAY;
		    gx = gy = FARAWAY; /* random */
		    do_clear_area(omx, omy, 9, wantdoor,
				  (genericptr_t)&fardist);

		    /* here gx == FARAWAY e.g. when dog is in a vault */
		    if (gx == FARAWAY || (gx == omx && gy == omy)) {
			gx = u.ux;
			gy = u.uy;
		    } else if(edog) {
			edog->ogoal.x = gx;
			edog->ogoal.y = gy;
		    }
		}
	    }
	} else if(edog) {
	    edog->ogoal.x = 0;
	}
	return appr;
}

/* return 0 (no move), 1 (move) or 2 (dead) */
int
dog_move(mtmp, after)
register struct monst *mtmp;
register int after;	/* this is extra fast monster movement */
{
	int omx, omy;		/* original mtmp position */
	int appr, whappr, udist;
	int i, j, k;
	register struct edog *edog = EDOG(mtmp);
	struct obj *obj = (struct obj *) 0;
	xchar otyp;
	boolean has_edog, cursemsg[9], do_eat = FALSE;
	xchar nix, niy;		/* position mtmp is (considering) moving to */
	register int nx, ny;	/* temporary coordinates */
	xchar cnt, uncursedcnt, chcnt;
	int chi = -1, nidist, ndist;
	coord poss[9];
	long info[9], allowflags;
#define GDIST(x,y) (dist2(x,y,gx,gy))

	/*
	 * Tame Angels have isminion set and an ispriest structure instead of
	 * an edog structure.  Fortunately, guardian Angels need not worry
	 * about mundane things like eating and fetching objects, and can
	 * spend all their energy defending the player.  (They are the only
	 * monsters with other structures that can be tame.)
	 */
	has_edog = !mtmp->isminion;

	omx = mtmp->mx;
	omy = mtmp->my;
	if (has_edog && dog_hunger(mtmp, edog)) return(2);	/* starved */

	udist = distu(omx,omy);
#ifdef STEED
	/* Let steeds eat */
	if (mtmp == u.usteed)
		udist = 1;
	else
#endif
	/* maybe we tamed him while being swallowed --jgm */
	if (!udist) return(0);

	nix = omx;	/* set before newdogpos */
	niy = omy;
	cursemsg[0] = FALSE;	/* lint suppression */
	info[0] = 0;		/* ditto */

	if (has_edog) {
	    j = dog_invent(mtmp, edog, udist);
	    if (j == 2) return 2;		/* died */
	    else if (j == 1) goto newdogpos;	/* eating something */

	    whappr = (monstermoves - edog->whistletime < 5);
	} else
	    whappr = 0;

	appr = dog_goal(mtmp, has_edog ? edog : (struct edog *)0,
							after, udist, whappr);
	if (appr == -2) return(0);

	allowflags = ALLOW_M | ALLOW_TRAPS | ALLOW_SSM | ALLOW_SANCT;
	if (passes_walls(mtmp->data)) allowflags |= (ALLOW_ROCK|ALLOW_WALL);
	if (throws_rocks(mtmp->data)) allowflags |= ALLOW_ROCK;
	if (Conflict && !resist(mtmp, RING_CLASS, 0, 0)) {
	    allowflags |= ALLOW_U;
	    if (!has_edog) {
		coord mm;
		/* Guardian angel refuses to be conflicted; rather,
		 * it disappears, angrily, and sends in some nasties
		 */
		if (canspotmon(mtmp)) {
		    pline("%s rebukes you, saying:", Monnam(mtmp));
		    verbalize("Since you desire conflict, have some more!");
		}
		mongone(mtmp);
		i = rnd(4);
		while(i--) {
		    mm.x = u.ux;
		    mm.y = u.uy;
		    if(enexto(&mm, mm.x, mm.y, &mons[PM_ANGEL]))
			(void) mk_roamer(&mons[PM_ANGEL], u.ualign.type,
					 mm.x, mm.y, FALSE);
		}
		return(2);

	    }
	}
	if (!nohands(mtmp->data) && !verysmall(mtmp->data)) {
		allowflags |= OPENDOOR;
		if (m_carrying(mtmp, SKELETON_KEY)) allowflags |= BUSTDOOR;
	}
	if (is_giant(mtmp->data)) allowflags |= BUSTDOOR;
	if (tunnels(mtmp->data) && !needspick(mtmp->data))
		allowflags |= ALLOW_DIG;
	cnt = mfndpos(mtmp, poss, info, allowflags);

	/* Normally dogs don't step on cursed items, but if they have no
	 * other choice they will.  This requires checking ahead of time
	 * to see how many uncursed item squares are around.
	 */
	uncursedcnt = 0;
	for (i = 0; i < cnt; i++) {
		nx = poss[i].x; ny = poss[i].y;
		if (MON_AT(nx,ny) && !(info[i] & ALLOW_M)) continue;
		if (cursed_object_at(nx, ny)) continue;
		uncursedcnt++;
	}

	chcnt = 0;
	chi = -1;
	nidist = GDIST(nix,niy);

	for (i = 0; i < cnt; i++) {
		nx = poss[i].x;
		ny = poss[i].y;
		cursemsg[i] = FALSE;

		/* if leashed, we drag him along. */
		if (mtmp->mleashed && distu(nx, ny) > 4) continue;

		/* if a guardian, try to stay close by choice */
		if (!has_edog &&
		    (j = distu(nx, ny)) > 16 && j >= udist) continue;

		if ((info[i] & ALLOW_M) && MON_AT(nx, ny)) {
		    int mstatus;
		    register struct monst *mtmp2 = m_at(nx,ny);

		    if ((int)mtmp2->m_lev >= (int)mtmp->m_lev+2 ||
			(mtmp2->data == &mons[PM_FLOATING_EYE] && rn2(10) &&
			 mtmp->mcansee && haseyes(mtmp->data) && mtmp2->mcansee
			 && (perceives(mtmp->data) || !mtmp2->minvis)) ||
			(mtmp2->data==&mons[PM_GELATINOUS_CUBE] && rn2(10)) ||
			(max_passive_dmg(mtmp2, mtmp) >= mtmp->mhp) ||
			((mtmp->mhp*4 < mtmp->mhpmax
			  || mtmp2->data->msound == MS_GUARDIAN
			  || mtmp2->data->msound == MS_LEADER) &&
			 mtmp2->mpeaceful && !Conflict) ||
			   (touch_petrifies(mtmp2->data) &&
				!resists_ston(mtmp)))
			continue;

		    if (after) return(0); /* hit only once each move */

		    notonhead = 0;
		    mstatus = mattackm(mtmp, mtmp2);

		    /* aggressor (pet) died */
		    if (mstatus & MM_AGR_DIED) return 2;

		    if ((mstatus & MM_HIT) && !(mstatus & MM_DEF_DIED) &&
			    rn2(4) && mtmp2->mlstmv != monstermoves &&
			    !onscary(mtmp->mx, mtmp->my, mtmp2) &&
			    /* monnear check needed: long worms hit on tail */
			    monnear(mtmp2, mtmp->mx, mtmp->my)) {
			mstatus = mattackm(mtmp2, mtmp);  /* return attack */
			if (mstatus & MM_DEF_DIED) return 2;
		    }

		    return 0;
		}

		{   /* Dog avoids harmful traps, but perhaps it has to pass one
		     * in order to follow player.  (Non-harmful traps do not
		     * have ALLOW_TRAPS in info[].)  The dog only avoids the
		     * trap if you've seen it, unlike enemies who avoid traps
		     * if they've seen some trap of that type sometime in the
		     * past.  (Neither behavior is really realistic.)
		     */
		    struct trap *trap;

		    if ((info[i] & ALLOW_TRAPS) && (trap = t_at(nx,ny))) {
			if (mtmp->mleashed) {
			    if (flags.soundok) whimper(mtmp);
			} else
			    /* 1/40 chance of stepping on it anyway, in case
			     * it has to pass one to follow the player...
			     */
			    if (trap->tseen && rn2(40)) continue;
		    }
		}

		/* dog eschews cursed objects, but likes dog food */
		/* (minion isn't interested; `cursemsg' stays FALSE) */
		if (has_edog)
		for (obj = level.objects[nx][ny]; obj; obj = obj->nexthere) {
		    if (obj->cursed) cursemsg[i] = TRUE;
		    else if ((otyp = dogfood(mtmp, obj)) < MANFOOD &&
			     (otyp < ACCFOOD || edog->hungrytime <= monstermoves)) {
			/* Note: our dog likes the food so much that he
			 * might eat it even when it conceals a cursed object */
			nix = nx;
			niy = ny;
			chi = i;
			do_eat = TRUE;
			cursemsg[i] = FALSE;	/* not reluctant */
			goto newdogpos;
		    }
		}
		/* didn't find something to eat; if we saw a cursed item and
		   aren't being forced to walk on it, usually keep looking */
		if (cursemsg[i] && !mtmp->mleashed && uncursedcnt > 0 &&
		    rn2(13 * uncursedcnt)) continue;

		/* lessen the chance of backtracking to previous position(s) */
		k = has_edog ? uncursedcnt : cnt;
		for (j = 0; j < MTSZ && j < k - 1; j++)
			if (nx == mtmp->mtrack[j].x && ny == mtmp->mtrack[j].y)
				if (rn2(MTSZ * (k - j))) goto nxti;

		j = ((ndist = GDIST(nx,ny)) - nidist) * appr;
		if ((j == 0 && !rn2(++chcnt)) || j < 0 ||
			(j > 0 && !whappr &&
				((omx == nix && omy == niy && !rn2(3))
					|| !rn2(12))
			)) {
			nix = nx;
			niy = ny;
			nidist = ndist;
			if(j < 0) chcnt = 0;
			chi = i;
		}
	nxti:	;
	}
newdogpos:
	if (nix != omx || niy != omy) {
		if (info[chi] & ALLOW_U) {
			if (mtmp->mleashed) { /* play it safe */
				pline("%s breaks loose of %s leash!",
				      Monnam(mtmp), his[pronoun_gender(mtmp)]);
				m_unleash(mtmp);
			}
			(void) mattacku(mtmp);
			return(0);
		}
		if (!m_in_out_region(mtmp, nix, niy))
		    return 1;
		/* insert a worm_move() if worms ever begin to eat things */
		remove_monster(omx, omy);
		place_monster(mtmp, nix, niy);
		if (cursemsg[chi] && (cansee(omx,omy) || cansee(nix,niy)))
			pline("%s moves only reluctantly.", Monnam(mtmp));
		for (j=MTSZ-1; j>0; j--) mtmp->mtrack[j] = mtmp->mtrack[j-1];
		mtmp->mtrack[0].x = omx;
		mtmp->mtrack[0].y = omy;
		/* We have to know if the pet's gonna do a combined eat and
		 * move before moving it, but it can't eat until after being
		 * moved.  Thus the do_eat flag.
		 */
		if (do_eat) {
		    if (dog_eat(mtmp, obj, omx, omy, FALSE) == 2) return 2;
		}
	} else if (mtmp->mleashed && distu(omx, omy) > 4) {
		/* an incredible kludge, but the only way to keep pooch near
		 * after it spends time eating or in a trap, etc.
		 */
		coord cc;

		nx = sgn(omx - u.ux);
		ny = sgn(omy - u.uy);
		cc.x = u.ux + nx;
		cc.y = u.uy + ny;
		if (goodpos(cc.x, cc.y, mtmp)) goto dognext;

		i  = xytod(nx, ny);
		for (j = (i + 7)%8; j < (i + 1)%8; j++) {
			dtoxy(&cc, j);
			if (goodpos(cc.x, cc.y, mtmp)) goto dognext;
		}
		for (j = (i + 6)%8; j < (i + 2)%8; j++) {
			dtoxy(&cc, j);
			if (goodpos(cc.x, cc.y, mtmp)) goto dognext;
		}
		cc.x = mtmp->mx;
		cc.y = mtmp->my;
dognext:
		if (!m_in_out_region(mtmp, nix, niy))
		  return 1;
		remove_monster(mtmp->mx, mtmp->my);
		place_monster(mtmp, cc.x, cc.y);
		newsym(cc.x,cc.y);
		set_apparxy(mtmp);
	}
	return(1);
}

#endif /* OVL0 */
#ifdef OVLB

/*ARGSUSED*/	/* do_clear_area client */
STATIC_PTR void
wantdoor(x, y, distance)
int x, y;
genericptr_t distance;
{
    int ndist;

    if (*(int*)distance > (ndist = distu(x, y))) {
	gx = x;
	gy = y;
	*(int*)distance = ndist;
    }
}

#endif /* OVLB */

/*dogmove.c*/
