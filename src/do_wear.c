/*	SCCS Id: @(#)do_wear.c	3.3	1999/08/16	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

#ifndef OVLB

STATIC_DCL long takeoff_mask, taking_off;

#else /* OVLB */

STATIC_OVL NEARDATA long takeoff_mask = 0L, taking_off = 0L;

static NEARDATA int todelay;

static NEARDATA const char see_yourself[] = "see yourself";
static NEARDATA const char unknown_type[] = "Unknown type of %s (%d)";
static NEARDATA const char *c_armor  = "armor",
			   *c_suit   = "suit",
#ifdef TOURIST
			   *c_shirt  = "shirt",
#endif
			   *c_cloak  = "cloak",
			   *c_gloves = "gloves",
			   *c_boots  = "boots",
			   *c_helmet = "helmet",
			   *c_shield = "shield",
			   *c_weapon = "weapon",
			   *c_sword  = "sword",
			   *c_axe    = "axe",
			   *c_that_  = "that";

static NEARDATA const long takeoff_order[] = { WORN_BLINDF, W_WEP,
	WORN_SHIELD, WORN_GLOVES, LEFT_RING, RIGHT_RING, WORN_CLOAK,
	WORN_HELMET, WORN_AMUL, WORN_ARMOR,
#ifdef TOURIST
	WORN_SHIRT,
#endif
	WORN_BOOTS, W_SWAPWEP, W_QUIVER, 0L };

STATIC_DCL void FDECL(on_msg, (struct obj *));
STATIC_PTR int NDECL(Armor_on);
STATIC_PTR int NDECL(Boots_on);
STATIC_DCL int NDECL(Cloak_on);
STATIC_PTR int NDECL(Helmet_on);
STATIC_PTR int NDECL(Gloves_on);
STATIC_DCL void NDECL(Amulet_on);
STATIC_DCL void FDECL(Ring_off_or_gone, (struct obj *, BOOLEAN_P));
STATIC_PTR int FDECL(select_off, (struct obj *));
STATIC_DCL struct obj *NDECL(do_takeoff);
STATIC_PTR int NDECL(take_off);
STATIC_DCL int FDECL(menu_remarm, (int));
STATIC_DCL void FDECL(already_wearing, (const char*));

void
off_msg(otmp)
register struct obj *otmp;
{
	if(flags.verbose)
	    You("were wearing %s.", doname(otmp));
}

/* for items that involve no delay */
STATIC_OVL void
on_msg(otmp)
register struct obj *otmp;
{
	if(flags.verbose)
	    You("are now wearing %s.",
		obj_is_pname(otmp) ? the(xname(otmp)) : an(xname(otmp)));
}

/*
 * The Type_on() functions should be called *after* setworn().
 * The Type_off() functions call setworn() themselves.
 */

STATIC_PTR
int
Boots_on()
{
	long oldprop = u.uprops[objects[uarmf->otyp].oc_oprop].extrinsic & ~WORN_BOOTS;


    switch(uarmf->otyp) {
	case LOW_BOOTS:
	case IRON_SHOES:
	case HIGH_BOOTS:
	case JUMPING_BOOTS:
	case KICKING_BOOTS:
		break;
	case WATER_WALKING_BOOTS:
		if (u.uinwater) spoteffects();
		break;
	case SPEED_BOOTS:
		/* Speed boots are still better than intrinsic speed, */
		/* though not better than potion speed */
		if (!oldprop && !(HFast & TIMEOUT)) {
			makeknown(uarmf->otyp);
			You_feel("yourself speed up%s.",
				(oldprop || HFast) ? " a bit more" : "");
		}
		break;
	case ELVEN_BOOTS:
		if (!oldprop && !HStealth && !BStealth) {
			makeknown(uarmf->otyp);
			You("walk very quietly.");
		}
		break;
	case FUMBLE_BOOTS:
		if (!oldprop && !(HFumbling & ~TIMEOUT))
			incr_itimeout(&HFumbling, rnd(20));
		break;
	case LEVITATION_BOOTS:
		if (!oldprop && !HLevitation) {
			makeknown(uarmf->otyp);
			float_up();
		}
		break;
	default: impossible(unknown_type, c_boots, uarmf->otyp);
    }
    return 0;
}

int
Boots_off()
{
    int otyp = uarmf->otyp;
	long oldprop = u.uprops[objects[otyp].oc_oprop].extrinsic & ~WORN_BOOTS;


	/* For levitation, float_down() returns if Levitation, so we
	 * must do a setworn() _before_ the levitation case.
	 */
    setworn((struct obj *)0, W_ARMF);
    switch (otyp) {
	case SPEED_BOOTS:
		if (!Very_fast) {
			makeknown(otyp);
			You_feel("yourself slow down%s.",
				Fast ? " a bit" : "");
		}
		break;
	case WATER_WALKING_BOOTS:
		if (is_pool(u.ux,u.uy) && !Levitation
			    && !Flying && !is_clinger(youmonst.data)) {
			makeknown(otyp);
			/* make boots known in case you survive the drowning */
			spoteffects();
		}
		break;
	case ELVEN_BOOTS:
		if (!oldprop && !HStealth && !BStealth) {
			makeknown(otyp);
			You("sure are noisy.");
		}
		break;
	case FUMBLE_BOOTS:
		if (!oldprop && !(HFumbling & ~TIMEOUT))
			HFumbling = EFumbling = 0;
		break;
	case LEVITATION_BOOTS:
		if (!oldprop && !HLevitation) {
			(void) float_down(0L, 0L);
			makeknown(otyp);
		}
		break;
	case LOW_BOOTS:
	case IRON_SHOES:
	case HIGH_BOOTS:
	case JUMPING_BOOTS:
	case KICKING_BOOTS:
		break;
	default: impossible(unknown_type, c_boots, otyp);
    }
    return 0;
}

STATIC_OVL int
Cloak_on()
{
    long oldprop = u.uprops[objects[uarmc->otyp].oc_oprop].extrinsic & ~WORN_CLOAK;


    switch(uarmc->otyp) {
	case ELVEN_CLOAK:
	case CLOAK_OF_PROTECTION:
	case CLOAK_OF_DISPLACEMENT:
		makeknown(uarmc->otyp);
		break;
	case ORCISH_CLOAK:
	case DWARVISH_CLOAK:
	case CLOAK_OF_MAGIC_RESISTANCE:
	case ROBE:
		break;
	case MUMMY_WRAPPING:
		/* Note: it's already being worn, so we have to cheat here. */
		if ((HInvis || EInvis || pm_invisible(youmonst.data)) && !Blind) {
		    newsym(u.ux,u.uy);
		    You("can %s!",
			See_invisible ? "no longer see through yourself"
			: see_yourself);
		}
		break;
	case CLOAK_OF_INVISIBILITY:
		/* since cloak of invisibility was worn, we know mummy wrapping
		   wasn't, so no need to check `oldprop' against blocked */
		if (!oldprop && !HInvis && !Blind) {
		    makeknown(uarmc->otyp);
		    newsym(u.ux,u.uy);
		    pline("Suddenly you can%s yourself.",
			See_invisible ? " see through" : "not see");
		}
		break;
	case OILSKIN_CLOAK:
		pline("%s fits very tightly.",The(xname(uarmc)));
		break;
	/* Alchemy smock gives poison _and_ acid resistance */
	case ALCHEMY_SMOCK:
		EAcid_resistance |= WORN_CLOAK;
  		break;
	default: impossible(unknown_type, c_cloak, uarmc->otyp);
    }
    return 0;
}

int
Cloak_off()
{
    int otyp = uarmc->otyp;
    long oldprop = u.uprops[objects[otyp].oc_oprop].extrinsic & ~WORN_CLOAK;


	/* For mummy wrapping, taking it off first resets `Invisible'. */
    setworn((struct obj *)0, W_ARMC);
    switch (otyp) {
	case ELVEN_CLOAK:
	case ORCISH_CLOAK:
	case DWARVISH_CLOAK:
	case CLOAK_OF_PROTECTION:
	case CLOAK_OF_MAGIC_RESISTANCE:
	case CLOAK_OF_DISPLACEMENT:
	case OILSKIN_CLOAK:
	case ROBE:
		break;
	case MUMMY_WRAPPING:
		if (Invis && !Blind) {
		    newsym(u.ux,u.uy);
		    You("can %s.",
			See_invisible ? "see through yourself"
			: "no longer see yourself");
		}
		break;
	case CLOAK_OF_INVISIBILITY:
		if (!oldprop && !HInvis && !Blind) {
		    makeknown(CLOAK_OF_INVISIBILITY);
		    newsym(u.ux,u.uy);
		    pline("Suddenly you can %s.",
			See_invisible ? "no longer see through yourself"
			: see_yourself);
		}
		break;
	/* Alchemy smock gives poison _and_ acid resistance */
	case ALCHEMY_SMOCK:
		EAcid_resistance &= ~WORN_CLOAK;
  		break;
	default: impossible(unknown_type, c_cloak, otyp);
    }
    return 0;
}

STATIC_PTR
int
Helmet_on()
{
    switch(uarmh->otyp) {
	case FEDORA:
	case HELMET:
	case DENTED_POT:
	case ELVEN_LEATHER_HELM:
	case DWARVISH_IRON_HELM:
	case ORCISH_HELM:
	case HELM_OF_TELEPATHY:
		break;
	case HELM_OF_BRILLIANCE:
		adj_abon(uarmh, uarmh->spe);
		break;
	case CORNUTHAUM:
		/* people think marked wizards know what they're talking
		 * about, but it takes trained arrogance to pull it off,
		 * and the actual enchantment of the hat is irrelevant.
		 */
		ABON(A_CHA) += (Role_if(PM_WIZARD) ? 1 : -1);
		flags.botl = 1;
		makeknown(uarmh->otyp);
		break;
	case HELM_OF_OPPOSITE_ALIGNMENT:
		if (u.ualign.type == A_NEUTRAL)
		    u.ualign.type = rn2(2) ? A_CHAOTIC : A_LAWFUL;
		else u.ualign.type = -(u.ualign.type);
		u.ublessed = 0; /* lose your god's protection */
	     /* makeknown(uarmh->otyp);   -- moved below, after xname() */
		/*FALLTHRU*/
	case DUNCE_CAP:
		if (!uarmh->cursed) {
		    pline("%s %s%s for a moment.", The(xname(uarmh)),
			  Blind ? "vibrates" : "glows ",
			  Blind ? (const char *)"" : hcolor(Black));
		    curse(uarmh);
		}
		flags.botl = 1;		/* reveal new alignment or INT & WIS */
		if (Hallucination) {
		    pline("My brain hurts!"); /* Monty Python's Flying Circus */
		} else if (uarmh->otyp == DUNCE_CAP) {
		    You_feel("%s.",	/* track INT change; ignore WIS */
		  ACURR(A_INT) <= (ABASE(A_INT) + ABON(A_INT) + ATEMP(A_INT)) ?
			     "like sitting in a corner" : "giddy");
		} else {
		    Your("mind oscillates briefly.");
		    makeknown(HELM_OF_OPPOSITE_ALIGNMENT);
		}
		break;
	default: impossible(unknown_type, c_helmet, uarmh->otyp);
    }
    return 0;
}

int
Helmet_off()
{
    switch(uarmh->otyp) {
	case FEDORA:
	case HELMET:
	case DENTED_POT:
	case ELVEN_LEATHER_HELM:
	case DWARVISH_IRON_HELM:
	case ORCISH_HELM:
		break;
	case DUNCE_CAP:
		flags.botl = 1;
		break;
	case CORNUTHAUM:
		ABON(A_CHA) += (Role_if(PM_WIZARD) ? -1 : 1);
		flags.botl = 1;
		break;
	case HELM_OF_TELEPATHY:
		/* need to update ability before calling see_monsters() */
		setworn((struct obj *)0, W_ARMH);
		see_monsters();
		return 0;
	case HELM_OF_BRILLIANCE:
		adj_abon(uarmh, -uarmh->spe);
		break;
	case HELM_OF_OPPOSITE_ALIGNMENT:
		u.ualign.type = u.ualignbase[0];
		u.ublessed = 0; /* lose the other god's protection */
		flags.botl = 1;
		break;
	default: impossible(unknown_type, c_helmet, uarmh->otyp);
    }
    setworn((struct obj *)0, W_ARMH);
    return 0;
}

STATIC_PTR
int
Gloves_on()
{
    long oldprop =
		u.uprops[objects[uarmg->otyp].oc_oprop].extrinsic & ~WORN_GLOVES;


    switch(uarmg->otyp) {
	case LEATHER_GLOVES:
		break;
	case GAUNTLETS_OF_FUMBLING:
		if (!oldprop && !(HFumbling & ~TIMEOUT))
			incr_itimeout(&HFumbling, rnd(20));
		break;
	case GAUNTLETS_OF_POWER:
		makeknown(uarmg->otyp);
		flags.botl = 1; /* taken care of in attrib.c */
		break;
	case GAUNTLETS_OF_DEXTERITY:
		adj_abon(uarmg, uarmg->spe);
		break;
	default: impossible(unknown_type, c_gloves, uarmg->otyp);
    }
    return 0;
}

int
Gloves_off()
{
    long oldprop =
		u.uprops[objects[uarmg->otyp].oc_oprop].extrinsic & ~WORN_GLOVES;


    switch(uarmg->otyp) {
	case LEATHER_GLOVES:
		break;
	case GAUNTLETS_OF_FUMBLING:
		if (!oldprop && !(HFumbling & ~TIMEOUT))
			HFumbling = EFumbling = 0;
		break;
	case GAUNTLETS_OF_POWER:
		makeknown(uarmg->otyp);
		flags.botl = 1; /* taken care of in attrib.c */
		break;
	case GAUNTLETS_OF_DEXTERITY:
		adj_abon(uarmg, -uarmg->spe);
		break;
	default: impossible(unknown_type, c_gloves, uarmg->otyp);
    }
    setworn((struct obj *)0, W_ARMG);

    /* Prevent wielding cockatrice when not wearing gloves */
    if (uwep && uwep->otyp == CORPSE &&
		touch_petrifies(&mons[uwep->corpsenm])) {
	char kbuf[BUFSZ];

	You("wield the %s corpse in your bare %s.",
	    mons[uwep->corpsenm].mname, makeplural(body_part(HAND)));
	Sprintf(kbuf, "%s corpse", an(mons[uwep->corpsenm].mname));
	instapetrify(kbuf);
	uwepgone();  /* life-saved still doesn't allow touching cockatrice */
    }
	/* KMH -- ...or your secondary weapon when you're wielding it */
	if (u.twoweap && uswapwep && uswapwep->otyp == CORPSE &&
			touch_petrifies(&mons[uswapwep->corpsenm])) {
	char kbuf[BUFSZ];

	You("wield the %s corpse in your bare %s.",
		mons[uswapwep->corpsenm].mname, body_part(HAND));

	Sprintf(kbuf, "%s corpse", an(mons[uswapwep->corpsenm].mname));
	instapetrify(kbuf);
	uswapwepgone();  /* life-saved still doesn't allow touching cockatrice */
	}

    return 0;
}

/*
STATIC_OVL int
Shield_on()
{
    switch(uarms->otyp) {
	case SMALL_SHIELD:
	case ELVEN_SHIELD:
	case URUK_HAI_SHIELD:
	case ORCISH_SHIELD:
	case DWARVISH_ROUNDSHIELD:
	case LARGE_SHIELD:
	case SHIELD_OF_REFLECTION:
		break;
	default: impossible(unknown_type, c_shield, uarms->otyp);
    }
    return 0;
}
*/

int
Shield_off()
{
/*
    switch(uarms->otyp) {
	case SMALL_SHIELD:
	case ELVEN_SHIELD:
	case URUK_HAI_SHIELD:
	case ORCISH_SHIELD:
	case DWARVISH_ROUNDSHIELD:
	case LARGE_SHIELD:
	case SHIELD_OF_REFLECTION:
		break;
	default: impossible(unknown_type, c_shield, uarms->otyp);
    }
*/
    setworn((struct obj *)0, W_ARMS);
    return 0;
}

/* This must be done in worn.c, because one of the possible intrinsics conferred
 * is fire resistance, and we have to immediately set HFire_resistance in worn.c
 * since worn.c will check it before returning.
 */
STATIC_PTR
int
Armor_on()
{
    return 0;
}

int
Armor_off()
{
    setworn((struct obj *)0, W_ARM);
    return 0;
}

/* The gone functions differ from the off functions in that if you die from
 * taking it off and have life saving, you still die.
 */
int
Armor_gone()
{
    setnotworn(uarm);
    return 0;
}

STATIC_OVL void
Amulet_on()
{
    switch(uamul->otyp) {
	case AMULET_OF_ESP:
	case AMULET_OF_LIFE_SAVING:
	case AMULET_VERSUS_POISON:
	case AMULET_OF_REFLECTION:
	case AMULET_OF_MAGICAL_BREATHING:
	case AMULET_OF_UNCHANGING:
	case FAKE_AMULET_OF_YENDOR:
		break;
	case AMULET_OF_CHANGE:
		makeknown(AMULET_OF_CHANGE);
		change_sex();
		/* Don't use same message as polymorph */
		You("are suddenly very %s!", flags.female ? "feminine"
			: "masculine");
		flags.botl = 1;
		pline_The("amulet disintegrates!");
		useup(uamul);
		break;
	case AMULET_OF_STRANGULATION:
		makeknown(AMULET_OF_STRANGULATION);
		pline("It constricts your throat!");
		Strangled = 6;
		break;
	case AMULET_OF_RESTFUL_SLEEP:
		HSleeping = rnd(100);
		break;
	case AMULET_OF_YENDOR:
		break;
    }
}

void
Amulet_off()
{
    switch(uamul->otyp) {
	case AMULET_OF_ESP:
		/* need to update ability before calling see_monsters() */
		setworn((struct obj *)0, W_AMUL);
		see_monsters();
		return;
	case AMULET_OF_LIFE_SAVING:
	case AMULET_VERSUS_POISON:
	case AMULET_OF_REFLECTION:
	case AMULET_OF_UNCHANGING:
	case FAKE_AMULET_OF_YENDOR:
		break;
	case AMULET_OF_MAGICAL_BREATHING:
		if (Underwater) {
		    if (!breathless(youmonst.data) && !amphibious(youmonst.data)
						&& !Swimming)
			You("suddenly inhale an unhealthy amount of water!");
		    /* HMagical_breathing must be set off
		       before calling drown() */
		    setworn((struct obj *)0, W_AMUL);
		    (void) drown();
		    return;
		}
		break;
	case AMULET_OF_CHANGE:
		impossible("Wearing an amulet of change?");
		break;
	case AMULET_OF_STRANGULATION:
		if (Strangled) {
			You("can breathe more easily!");
			Strangled = 0;
		}
		break;
	case AMULET_OF_RESTFUL_SLEEP:
		setworn((struct obj *)0, W_AMUL);
		if (!ESleeping)
			HSleeping = 0;
		return;
	case AMULET_OF_YENDOR:
		break;
    }
    setworn((struct obj *)0, W_AMUL);
    return;
}

void
Ring_on(obj)
register struct obj *obj;
{
    long oldprop = u.uprops[objects[obj->otyp].oc_oprop].extrinsic;
    int old_attrib;


	if (obj == uwep) setuwep((struct obj *) 0);
	if (obj == uswapwep) setuswapwep((struct obj *) 0);
	if (obj == uquiver) setuqwep((struct obj *) 0);

    /* only mask out W_RING when we don't have both
       left and right rings of the same type */
    if ((oldprop & W_RING) != W_RING) oldprop &= ~W_RING;

    switch(obj->otyp){
	case RIN_TELEPORTATION:
	case RIN_REGENERATION:
	case RIN_SEARCHING:
	case RIN_STEALTH:
	case RIN_HUNGER:
	case RIN_AGGRAVATE_MONSTER:
	case RIN_POISON_RESISTANCE:
	case RIN_FIRE_RESISTANCE:
	case RIN_COLD_RESISTANCE:
	case RIN_SHOCK_RESISTANCE:
	case RIN_CONFLICT:
	case RIN_WARNING:
	case RIN_TELEPORT_CONTROL:
	case RIN_POLYMORPH:
	case RIN_POLYMORPH_CONTROL:
	case RIN_FREE_ACTION:                
	case RIN_SLOW_DIGESTION:
	case RIN_SUSTAIN_ABILITY:
	case MEAT_RING:
		break;
	case RIN_SEE_INVISIBLE:
		/* can now see invisible monsters */
		set_mimic_blocking(); /* do special mimic handling */
		see_monsters();
#ifdef INVISIBLE_OBJECTS
		see_objects();
#endif

		if (Invis && !oldprop && !HSee_invisible &&
				!perceives(youmonst.data) && !Blind) {
		    newsym(u.ux,u.uy);
		    pline("Suddenly you are transparent, but there!");
		    makeknown(RIN_SEE_INVISIBLE);
		}
		break;
	case RIN_INVISIBILITY:
		if (!oldprop && !HInvis && !BInvis && !Blind) {
		    makeknown(RIN_INVISIBILITY);
		    newsym(u.ux,u.uy);
		    self_invis_message();
		}
		break;
	case RIN_ADORNMENT:
		old_attrib = ACURR(A_CHA);
		ABON(A_CHA) += obj->spe;
		flags.botl = 1;
		if (ACURR(A_CHA) != old_attrib ||
		    (objects[RIN_ADORNMENT].oc_name_known &&
		     old_attrib != 25 && old_attrib != 3)) {
			makeknown(RIN_ADORNMENT);
			obj->known = TRUE;
		}
		break;
	case RIN_LEVITATION:
		if(!oldprop && !HLevitation) {
			float_up();
			makeknown(RIN_LEVITATION);
			obj->known = TRUE;
		}
		break;
	case RIN_GAIN_STRENGTH:
		old_attrib = ACURR(A_STR);
		ABON(A_STR) += obj->spe;
		flags.botl = 1;
		if (ACURR(A_STR) != old_attrib ||
		    (objects[RIN_GAIN_STRENGTH].oc_name_known &&
		     old_attrib != STR19(25) && old_attrib != 3)) {
			makeknown(RIN_GAIN_STRENGTH);
			obj->known = TRUE;
		}
		break;
	case RIN_GAIN_CONSTITUTION:
		old_attrib = ACURR(A_CON);
		ABON(A_CON) += obj->spe;
		flags.botl = 1;
		if (ACURR(A_CON) != old_attrib ||
		    objects[RIN_GAIN_CONSTITUTION].oc_name_known) {
			makeknown(RIN_GAIN_CONSTITUTION);
			obj->known = TRUE;
		}
		break;
	case RIN_INCREASE_ACCURACY:	/* KMH */
		u.uhitinc += obj->spe;
		break;
	case RIN_INCREASE_DAMAGE:
		u.udaminc += obj->spe;
		break;
	case RIN_PROTECTION_FROM_SHAPE_CHAN:
		rescham();
		break;
	case RIN_PROTECTION:
		flags.botl = 1;
		if (obj->spe || objects[RIN_PROTECTION].oc_name_known) {
			makeknown(RIN_PROTECTION);
			obj->known = TRUE;
		}
		break;
    }
}

STATIC_OVL void
Ring_off_or_gone(obj,gone)
register struct obj *obj;
boolean gone;
{
    register long mask = obj->owornmask & W_RING;
    int old_attrib;

    if(!(u.uprops[objects[obj->otyp].oc_oprop].extrinsic & mask))
	impossible("Strange... I didn't know you had that ring.");
    if(gone) setnotworn(obj);
    else setworn((struct obj *)0, obj->owornmask);
    switch(obj->otyp) {
	case RIN_TELEPORTATION:
	case RIN_REGENERATION:
	case RIN_SEARCHING:
	case RIN_STEALTH:
	case RIN_HUNGER:
	case RIN_AGGRAVATE_MONSTER:
	case RIN_POISON_RESISTANCE:
	case RIN_FIRE_RESISTANCE:
	case RIN_COLD_RESISTANCE:
	case RIN_SHOCK_RESISTANCE:
	case RIN_CONFLICT:
	case RIN_WARNING:
	case RIN_TELEPORT_CONTROL:
	case RIN_POLYMORPH:
	case RIN_POLYMORPH_CONTROL:
	case RIN_FREE_ACTION:                
	case RIN_SLOW_DIGESTION:
	case RIN_SUSTAIN_ABILITY:
	case MEAT_RING:
		break;
	case RIN_SEE_INVISIBLE:
		/* Make invisible monsters go away */
		if (!See_invisible) {
		    set_mimic_blocking(); /* do special mimic handling */
		    see_monsters();
#ifdef INVISIBLE_OBJECTS                
		    see_objects();
#endif
		}

		if (Invisible && !Blind) {
			newsym(u.ux,u.uy);
			pline("Suddenly you cannot see yourself.");
			makeknown(RIN_SEE_INVISIBLE);
		}
		break;
	case RIN_INVISIBILITY:
		if (!Invis && !BInvis && !Blind) {
			newsym(u.ux,u.uy);
			Your("body seems to unfade%s.",
			    See_invisible ? " completely" : "..");
			makeknown(RIN_INVISIBILITY);
		}
		break;
	case RIN_ADORNMENT:
		old_attrib = ACURR(A_CHA);
		ABON(A_CHA) -= obj->spe;
		if (ACURR(A_CHA) != old_attrib) makeknown(RIN_ADORNMENT);
		flags.botl = 1;
		break;
	case RIN_LEVITATION:
		(void) float_down(0L, 0L);
		if (!Levitation) makeknown(RIN_LEVITATION);
		break;
	case RIN_GAIN_STRENGTH:
		old_attrib = ACURR(A_STR);
		ABON(A_STR) -= obj->spe;
		if (ACURR(A_STR) != old_attrib) makeknown(RIN_GAIN_STRENGTH);
		flags.botl = 1;
		break;
	case RIN_GAIN_CONSTITUTION:
		old_attrib = ACURR(A_CON);
		ABON(A_CON) -= obj->spe;
		flags.botl = 1;
		if (ACURR(A_CON) != old_attrib) makeknown(RIN_GAIN_CONSTITUTION);
		break;
	case RIN_INCREASE_ACCURACY:	/* KMH */
		u.uhitinc -= obj->spe;
		break;
	case RIN_INCREASE_DAMAGE:
		u.udaminc -= obj->spe;
		break;
	case RIN_PROTECTION_FROM_SHAPE_CHAN:
		/* If you're no longer protected, let the chameleons
		 * change shape again -dgk
		 */
		restartcham();
		break;
    }
}

void
Ring_off(obj)
struct obj *obj;
{
	Ring_off_or_gone(obj,FALSE);
}

void
Ring_gone(obj)
struct obj *obj;
{
	Ring_off_or_gone(obj,TRUE);
}

void
Blindf_on(otmp)
register struct obj *otmp;
{
	long already_blinded = Blinded;

	if (otmp == uwep)
	    setuwep((struct obj *) 0);
	if (otmp == uswapwep)
		setuswapwep((struct obj *) 0);
	if (otmp == uquiver)
		setuqwep((struct obj *) 0);
	setworn(otmp, W_TOOL);
	if (otmp->otyp == TOWEL && flags.verbose)
	    You("wrap %s around your %s.", an(xname(otmp)), body_part(HEAD));
	on_msg(otmp);
	if (!already_blinded) {
	    if (Punished) set_bc(0);	/* Set ball&chain variables before */
					/* the hero goes blind.		   */
	    if (Blind_telepat || Infravision) see_monsters(); /* sense monsters */
	    vision_full_recalc = 1;	/* recalc vision limits */
	    flags.botl = 1;
	}
}

void
Blindf_off(otmp)
register struct obj *otmp;
{
	long was_blind = Blind;	/* may still be able to see */

	setworn((struct obj *)0, otmp->owornmask);
	off_msg(otmp);

	if (Blind) {
	    if (was_blind)
		You("still cannot see.");
	    else
		You("cannot see anything now!");
	} else if (!Blinded) {
	    if (Blind_telepat || Infravision) see_monsters();
	}
	vision_full_recalc = 1;	/* recalc vision limits */
	flags.botl = 1;
}

/* called in main to set intrinsics of worn start-up items */
void
set_wear()
{
	if (uarm)  (void) Armor_on();
	if (uarmc) (void) Cloak_on();
	if (uarmf) (void) Boots_on();
	if (uarmg) (void) Gloves_on();
	if (uarmh) (void) Helmet_on();
/*	if (uarms) (void) Shield_on(); */
}

boolean
donning(otmp)
register struct obj *otmp;
{
    return((boolean)((otmp == uarmf && (afternmv == Boots_on || afternmv == Boots_off))
	|| (otmp == uarmh && (afternmv == Helmet_on || afternmv == Helmet_off))
	|| (otmp == uarmg && (afternmv == Gloves_on || afternmv == Gloves_off))
	|| (otmp == uarm && (afternmv == Armor_on || afternmv == Armor_off))));
}

void
cancel_don()
{
	/* the piece of armor we were donning/doffing has vanished, so stop
	 * wasting time on it (and don't dereference it when donning would
	 * otherwise finish)
	 */
	afternmv = 0;
	nomovemsg = (char *)0;
	multi = 0;
}

static NEARDATA const char clothes[] = {ARMOR_CLASS, 0};
static NEARDATA const char accessories[] = {RING_CLASS, AMULET_CLASS, TOOL_CLASS, FOOD_CLASS, 0};

int
dotakeoff()
{
	register struct obj *otmp = (struct obj *)0;
	int armorpieces = 0;

#define MOREARM(x) if (x) { armorpieces++; otmp = x; }
	MOREARM(uarmh);
	MOREARM(uarms);
	MOREARM(uarmg);
	MOREARM(uarmf);
	if (uarmc) {
		armorpieces++;
		otmp = uarmc;
	} else if (uarm) {
		armorpieces++;
		otmp = uarm;
#ifdef TOURIST
	} else if (uarmu) {
		armorpieces++;
		otmp = uarmu;
#endif
	}
	if (!armorpieces) {
	     /* assert( GRAY_DRAGON_SCALES > YELLOW_DRAGON_SCALE_MAIL ); */
		if (uskin)
		    pline_The("%s merged with your skin!",
			      uskin->otyp >= GRAY_DRAGON_SCALES ?
				"dragon scales are" : "dragon scale mail is");
		else
		    pline("Not wearing any armor.");
		return 0;
	}
	if (armorpieces > 1)
		otmp = getobj(clothes, "take off");
	if (otmp == 0) return(0);
	if (!(otmp->owornmask & W_ARMOR)) {
		You("are not wearing that.");
		return(0);
	}
	/* note: the `uskin' case shouldn't be able to happen here; dragons
	   can't wear any armor so will end up with `armorpieces == 0' above */
	if (otmp == uskin || ((otmp == uarm) && uarmc)
#ifdef TOURIST
			  || ((otmp == uarmu) && (uarmc || uarm))
#endif
		) {
	    You_cant("take that off.");
	    return 0;
	}
	if (otmp == uarmg && welded(uwep)) {
	    You("seem unable to take off the gloves while holding your %s.",
		is_sword(uwep) ? c_sword : c_weapon);
	    uwep->bknown = TRUE;
	    return 0;
	}
	if (otmp == uarmg && Glib) {
	    You_cant("remove the slippery gloves with your slippery fingers.");
	    return 0;
	}
	if (otmp == uarmf && u.utrap && (u.utraptype == TT_BEARTRAP ||
					u.utraptype == TT_INFLOOR)) { /* -3. */
	    if(u.utraptype == TT_BEARTRAP)
		pline_The("bear trap prevents you from pulling your %s out.",
		      body_part(FOOT));
	    else
		You("are stuck in the %s, and cannot pull your %s out.",
		    surface(u.ux, u.uy), makeplural(body_part(FOOT)));
		return(0);
	}
	reset_remarm();			/* since you may change ordering */
	(void) armoroff(otmp);
	return(1);
}

int
doremring()
{
	register struct obj *otmp = 0;
	int Accessories = 0;

#define MOREACC(x) if (x) { Accessories++; otmp = x; }
	MOREACC(uleft);
	MOREACC(uright);
	MOREACC(uamul);
	MOREACC(ublindf);

	if(!Accessories) {
		pline("Not wearing any accessories.");
		return(0);
	}
	if (Accessories != 1) otmp = getobj(accessories, "take off");
	if(!otmp) return(0);
	if(!(otmp->owornmask & (W_RING | W_AMUL | W_TOOL))) {
		You("are not wearing that.");
		return(0);
	}
	if(cursed(otmp)) return(0);
	if(otmp->oclass == RING_CLASS || otmp->otyp == MEAT_RING) {
		if (nolimbs(youmonst.data)) {
			pline("It seems to be stuck.");
			return(0);
		}
		if (uarmg && uarmg->cursed) {
			uarmg->bknown = TRUE;
			You(
	    "seem unable to remove your ring without taking off your gloves.");
			return(0);
		}
		if (welded(uwep) && bimanual(uwep)) {
			uwep->bknown = TRUE;
			You(
	       "seem unable to remove the ring while your hands hold your %s.",
			    is_sword(uwep) ? c_sword : c_weapon);
			return(0);
		}
		if (welded(uwep) && otmp==uright) {
			uwep->bknown = TRUE;
			You(
	 "seem unable to remove the ring while your right hand holds your %s.",
			    is_sword(uwep) ? c_sword : c_weapon);
			return(0);
		}
		/* Sometimes we want to give the off_msg before removing and
		 * sometimes after; for instance, "you were wearing a moonstone
		 * ring (on right hand)" is desired but "you were wearing a
		 * square amulet (being worn)" is not because of the redundant
		 * "being worn".
		 */
		off_msg(otmp);
		Ring_off(otmp);
	} else if(otmp->oclass == AMULET_CLASS) {
		Amulet_off();
		off_msg(otmp);
	} else Blindf_off(otmp); /* does its own off_msg */
	return(1);
}

/* Check if something worn is cursed _and_ unremovable. */
int
cursed(otmp)
register struct obj *otmp;
{
	/* Curses, like chickens, come home to roost. */
	if((otmp == uwep) ? welded(otmp) : (int)otmp->cursed) {
		You("can't.  %s to be cursed.",
			(is_boots(otmp) || is_gloves(otmp) || otmp->quan > 1L)
			? "They seem" : "It seems");
		otmp->bknown = TRUE;
		return(1);
	}
	return(0);
}

int
armoroff(otmp)
register struct obj *otmp;
{
	register int delay = -objects[otmp->otyp].oc_delay;

	if(cursed(otmp)) return(0);
	if(delay) {
		nomul(delay);
		if (is_helmet(otmp)) {
			nomovemsg = "You finish taking off your helmet.";
			afternmv = Helmet_off;
		     }
		else if (is_gloves(otmp)) {
			nomovemsg = "You finish taking off your gloves.";
			afternmv = Gloves_off;
		     }
		else if (is_boots(otmp)) {
			nomovemsg = "You finish taking off your boots.";
			afternmv = Boots_off;
		     }
		else {
			nomovemsg = "You finish taking off your suit.";
			afternmv = Armor_off;
		}
	} else {
		/* Be warned!  We want off_msg after removing the item to
		 * avoid "You were wearing ____ (being worn)."  However, an
		 * item which grants fire resistance might cause some trouble
		 * if removed in Hell and lifesaving puts it back on; in this
		 * case the message will be printed at the wrong time (after
		 * the messages saying you died and were lifesaved).  Luckily,
		 * no cloak, shield, or fast-removable armor grants fire
		 * resistance, so we can safely do the off_msg afterwards.
		 * Rings do grant fire resistance, but for rings we want the
		 * off_msg before removal anyway so there's no problem.  Take
		 * care in adding armors granting fire resistance; this code
		 * might need modification.
		 * 3.2 (actually 3.1 even): this comment is obsolete since
		 * fire resistance is not needed for Gehennom.
		 */
		if(is_cloak(otmp))
			(void) Cloak_off();
		else if(is_shield(otmp))
			(void) Shield_off();
		else setworn((struct obj *)0, otmp->owornmask & W_ARMOR);
		off_msg(otmp);
	}
	takeoff_mask = taking_off = 0L;
	return(1);
}

STATIC_OVL void
already_wearing(cc)
const char *cc;
{
	You("are already wearing %s%c", cc, (cc == c_that_) ? '!' : '.');
}

/*
 * canwearobj checks to see whether the player can wear a piece of armor
 *
 * inputs: otmp (the piece of armor)
 *         noisy (if TRUE give error messages, otherwise be quiet about it)
 * output: mask (otmp's armor type)
 */
int
canwearobj(otmp,mask,noisy)
struct obj *otmp;
long *mask;
boolean noisy;
{
    int err = 0;
    const char *which;

    which = is_cloak(otmp) ? c_cloak :
#ifdef TOURIST
	    is_shirt(otmp) ? c_shirt :
#endif
	    is_suit(otmp) ? c_suit : 0;
    if (which && cantweararm(youmonst.data) &&
	    /* same exception for cloaks as used in m_dowear() */
	    (which != c_cloak || youmonst.data->msize != MZ_SMALL)) {
	if (noisy) pline_The("%s will not fit on your body.", which);
	return 0;
    } else if (otmp->owornmask & W_ARMOR) {
	if (noisy) already_wearing(c_that_);
	return 0;
    }

    if (is_helmet(otmp)) {
	if (uarmh) {
	    if (noisy) already_wearing(an(c_helmet));
	    err++;
	} else
	    *mask = W_ARMH;
    } else if (is_shield(otmp)) {
	if (uarms) {
	    if (noisy) already_wearing(an(c_shield));
	    err++;
	} else if (uwep && bimanual(uwep)) {
	    if (noisy) 
		You("cannot wear a shield while wielding a two-handed %s.",
		    is_sword(uwep) ? c_sword :
		    (uwep->otyp == BATTLE_AXE) ? c_axe : c_weapon);
	    err++;
	} else if (u.twoweap) {
	    if (noisy)
		You("cannot wear a shield while wielding two weapons.");
	    err++;
	} else
	    *mask = W_ARMS;
    } else if (is_boots(otmp)) {
	if (uarmf) {
	    if (noisy) already_wearing(c_boots);
	    err++;
	} else if (Upolyd && slithy(youmonst.data)) {
	    if (noisy) You("have no feet...");	/* not body_part(FOOT) */
	    err++;
	} else if (u.utrap && (u.utraptype == TT_BEARTRAP ||
				u.utraptype == TT_INFLOOR)) {
	    if (u.utraptype == TT_BEARTRAP) {
		if (noisy) Your("%s is trapped!", body_part(FOOT));
	    } else {
		if (noisy) Your("%s are stuck in the %s!",
				makeplural(body_part(FOOT)),
				surface(u.ux, u.uy));
	    }
	    err++;
	} else
	    *mask = W_ARMF;
    } else if (is_gloves(otmp)) {
	if (uarmg) {
	    if (noisy) already_wearing(c_gloves);
	    err++;
	} else if (welded(uwep)) {
	    if (noisy) You("cannot wear gloves over your %s.",
			   is_sword(uwep) ? c_sword : c_weapon);
	    err++;
	} else
	    *mask = W_ARMG;
#ifdef TOURIST
    } else if (is_shirt(otmp)) {
	if (uarm || uarmc || uarmu) {
	    if (uarmu) {
		if (noisy) already_wearing(an(c_shirt));
	    } else {
		if (noisy) You_cant("wear that over your %s.",
				    (uarm && !uarmc) ? c_armor : c_cloak);
	    }
	    err++;
	} else
	    *mask = W_ARMU;
#endif
    } else if (is_cloak(otmp)) {
	if (uarmc) {
	    if (noisy) already_wearing(an(c_cloak));
	    err++;
	} else
	    *mask = W_ARMC;
    } else {
	if (uarmc) {
	    if (noisy) You("cannot wear armor over a cloak.");
	    err++;
	} else if (uarm) {
	    if (noisy) already_wearing("some armor");
	    err++;
	} else
	    *mask = W_ARM;
    }
/* Unnecessary since now only weapons and special items like pick-axes get
 * welded to your hand, not armor
    if (welded(otmp)) {
	if (!err++) {
	    if (noisy) weldmsg(otmp);
	}
    }
 */
    return !err;
}

/* the 'W' command */
int
dowear()
{
	struct obj *otmp;
	int delay;
	long mask = 0;

	/* cantweararm checks for suits of armor */
	/* verysmall or nohands checks for shields, gloves, etc... */
	if ((verysmall(youmonst.data) || nohands(youmonst.data))) {
		pline("Don't even bother.");
		return(0);
	}

	otmp = getobj(clothes, "wear");
	if(!otmp) return(0);

	if (!canwearobj(otmp,&mask,TRUE)) return(0);

	if (otmp->oartifact && !touch_artifact(otmp, &youmonst))
	    return 1;	/* costs a turn even though it didn't get worn */

	if (otmp->otyp == HELM_OF_OPPOSITE_ALIGNMENT &&
			qstart_level.dnum == u.uz.dnum) {	/* in quest */
		You("narrowly avoid losing all chance at your goal.");
		u.ublessed = 0; /* lose your god's protection */
		makeknown(otmp->otyp);
		flags.botl = 1;
		return 1;
	}

	otmp->known = TRUE;
	if(otmp == uwep)
		setuwep((struct obj *)0);
	if (otmp == uswapwep)
		setuswapwep((struct obj *) 0);
	if (otmp == uquiver)
		setuqwep((struct obj *) 0);
	setworn(otmp, mask);
	delay = -objects[otmp->otyp].oc_delay;
	if(delay){
		nomul(delay);
		if(is_boots(otmp)) afternmv = Boots_on;
		if(is_helmet(otmp)) afternmv = Helmet_on;
		if(is_gloves(otmp)) afternmv = Gloves_on;
		if(otmp == uarm) afternmv = Armor_on;
		nomovemsg = "You finish your dressing maneuver.";
	} else {
		if(is_cloak(otmp)) (void) Cloak_on();
/*		if(is_shield(otmp)) (void) Shield_on(); */
		on_msg(otmp);
	}
	takeoff_mask = taking_off = 0L;
	return(1);
}

int
doputon()
{
	register struct obj *otmp;
	long mask = 0L;

	if(uleft && uright && uamul && ublindf) {
		Your("%s%s are full, and you're already wearing an amulet and %s.",
			humanoid(youmonst.data) ? "ring-" : "",
			makeplural(body_part(FINGER)),
			ublindf->otyp==LENSES ? "some lenses" : "a blindfold");
		return(0);
	}
	otmp = getobj(accessories, "wear");
	if(!otmp) return(0);
	if(otmp->owornmask & (W_RING | W_AMUL | W_TOOL)) {
		already_wearing(c_that_);
		return(0);
	}
	if(welded(otmp)) {
		weldmsg(otmp);
		return(0);
	}
	if(otmp == uwep)
		setuwep((struct obj *)0);
	if(otmp->oclass == RING_CLASS || otmp->otyp == MEAT_RING) {
		if(nolimbs(youmonst.data)) {
			You("cannot make the ring stick to your body.");
			return(0);
		}
		if(uleft && uright){
			pline("There are no more %s%s to fill.",
				humanoid(youmonst.data) ? "ring-" : "",
				makeplural(body_part(FINGER)));
			return(0);
		}
		if(uleft) mask = RIGHT_RING;
		else if(uright) mask = LEFT_RING;
		else do {
			char qbuf[QBUFSZ];
			char answer;

			Sprintf(qbuf, "Which %s%s, Right or Left?",
				humanoid(youmonst.data) ? "ring-" : "",
				body_part(FINGER));
			if(!(answer = yn_function(qbuf, "rl", '\0')))
				return(0);
			switch(answer){
			case 'l':
			case 'L':
				mask = LEFT_RING;
				break;
			case 'r':
			case 'R':
				mask = RIGHT_RING;
				break;
			}
		} while(!mask);
		if (uarmg && uarmg->cursed) {
			uarmg->bknown = TRUE;
		    You("cannot remove your gloves to put on the ring.");
			return(0);
		}
		if (welded(uwep) && bimanual(uwep)) {
			/* welded will set bknown */
	    You("cannot free your weapon hands to put on the ring.");
			return(0);
		}
		if (welded(uwep) && mask==RIGHT_RING) {
			/* welded will set bknown */
	    You("cannot free your weapon hand to put on the ring.");
			return(0);
		}
		setworn(otmp, mask);
		Ring_on(otmp);
	} else if (otmp->oclass == AMULET_CLASS) {
		if(uamul) {
			already_wearing("an amulet");
			return(0);
		}
		setworn(otmp, W_AMUL);
		if (otmp->otyp == AMULET_OF_CHANGE) {
			Amulet_on();
			/* Don't do a prinv() since the amulet is now gone */
			return(1);
		}
		Amulet_on();
	} else {	/* it's a blindfold */
		if (ublindf) {
			if (ublindf->otyp == TOWEL)
				Your("%s is already covered by a towel.",
					body_part(FACE));
			else if (ublindf->otyp == BLINDFOLD)
				already_wearing("a blindfold");
			else if (ublindf->otyp == LENSES)
				already_wearing("some lenses");
			else
				already_wearing("something"); /* ??? */
			return(0);
		}
		if (otmp->otyp != BLINDFOLD && otmp->otyp != TOWEL && otmp->otyp != LENSES) {
			You_cant("wear that!");
			return(0);
		}
		Blindf_on(otmp);
		return(1);
	}
	prinv((char *)0, otmp, 0L);
	return(1);
}

#endif /* OVLB */

#ifdef OVL0

void
find_ac()
{
	int uac = mons[u.umonnum].ac;

	if(uarm) uac -= ARM_BONUS(uarm);
	if(uarmc) uac -= ARM_BONUS(uarmc);
	if(uarmh) uac -= ARM_BONUS(uarmh);
	if(uarmf) uac -= ARM_BONUS(uarmf);
	if(uarms) uac -= ARM_BONUS(uarms);
	if(uarmg) uac -= ARM_BONUS(uarmg);
#ifdef TOURIST
	if(uarmu) uac -= ARM_BONUS(uarmu);
#endif
	if(uleft && uleft->otyp == RIN_PROTECTION) uac -= uleft->spe;
	if(uright && uright->otyp == RIN_PROTECTION) uac -= uright->spe;
	if (HProtection & INTRINSIC) uac -= u.ublessed;
	uac -= u.uspellprot;
	if(uac != u.uac){
		u.uac = uac;
		flags.botl = 1;
	}
}

#endif /* OVL0 */
#ifdef OVLB

void
glibr()
{
	register struct obj *otmp;
	int xfl = 0;
	boolean leftfall, rightfall;

	leftfall = (uleft && !uleft->cursed &&
		    (!uwep || !welded(uwep) || !bimanual(uwep)));
	rightfall = (uright && !uright->cursed && (!welded(uwep)));
	if (!uarmg && (leftfall || rightfall) && !nolimbs(youmonst.data)) {
		/* changed so cursed rings don't fall off, GAN 10/30/86 */
		Your("%s off your %s.",
			(leftfall && rightfall) ? "rings slip" : "ring slips",
			makeplural(body_part(FINGER)));
		xfl++;
		if (leftfall) {
			otmp = uleft;
			Ring_off(uleft);
			dropx(otmp);
		}
		if (rightfall) {
			otmp = uright;
			Ring_off(uright);
			dropx(otmp);
		}
	}
	otmp = uwep;
	if (otmp && !welded(otmp)) {
		/* changed so cursed weapons don't fall, GAN 10/30/86 */
		Your("%s %sslips from your %s.",
			is_sword(otmp) ? c_sword :
				makesingular(oclass_names[(int)otmp->oclass]),
			xfl ? "also " : "",
			makeplural(body_part(HAND)));
		setuwep((struct obj *)0);
		if (otmp->otyp != LOADSTONE || !otmp->cursed)
			dropx(otmp);
	}
}

struct obj *
some_armor(victim)
struct monst *victim;
{
	register struct obj *otmph, *otmp;

	otmph = (victim == &youmonst) ? uarmc : which_armor(victim, W_ARMC);
	if (!otmph)
	    otmph = (victim == &youmonst) ? uarm : which_armor(victim, W_ARM);
#ifdef TOURIST
	if (!otmph)
	    otmph = (victim == &youmonst) ? uarmu : which_armor(victim, W_ARMU);
#endif
	
	otmp = (victim == &youmonst) ? uarmh : which_armor(victim, W_ARMH);
	if(otmp && (!otmph || !rn2(4))) otmph = otmp;
	otmp = (victim == &youmonst) ? uarmg : which_armor(victim, W_ARMG);
	if(otmp && (!otmph || !rn2(4))) otmph = otmp;
	otmp = (victim == &youmonst) ? uarmf : which_armor(victim, W_ARMF);
	if(otmp && (!otmph || !rn2(4))) otmph = otmp;
	otmp = (victim == &youmonst) ? uarms : which_armor(victim, W_ARMS);
	if(otmp && (!otmph || !rn2(4))) otmph = otmp;
	return(otmph);
}

void
erode_armor(victim,acid_dmg)
struct monst *victim;
boolean acid_dmg;
{
	register struct obj *otmph = some_armor(victim);
	int erosion;
	boolean vismon = (victim != &youmonst) && canseemon(victim);

	if (!otmph) return;
	erosion = acid_dmg  ? otmph->oeroded2 : otmph->oeroded;
	if (otmph != uarmf) {
	    if (otmph->greased) {
		grease_protect(otmph,(char *)0,FALSE,victim);
		return;
	    }
	    if (otmph->oerodeproof ||
		(acid_dmg ? !is_corrodeable(otmph) : !is_rustprone(otmph))) {
		if (flags.verbose || !(otmph->oerodeproof && otmph->rknown)) {
		    if (victim == &youmonst)
			Your("%s not affected.", aobjnam(otmph, "are"));
		    else if (vismon)
			pline("%s's %s not affected.", Monnam(victim),
			    aobjnam(otmph, "are"));
		}
		if (otmph->oerodeproof) otmph->rknown = TRUE;
		return;
	    }
	    if (erosion < MAX_ERODE) {
		if (victim == &youmonst)
		    Your("%s%s!", aobjnam(otmph, acid_dmg ? "corrode" : "rust"),
			erosion+1 == MAX_ERODE ? " completely" :
			erosion ? " further" : "");
		else if (vismon)
		    pline("%s's %s%s!", Monnam(victim),
			aobjnam(otmph, acid_dmg ? "corrode" : "rust"),
			erosion+1 == MAX_ERODE ? " completely" :
			erosion ? " further" : "");
		if (acid_dmg)
		    otmph->oeroded2++;
		else
		    otmph->oeroded++;
		return;
	    }
	    if (flags.verbose) {
		if (victim == &youmonst)
		    Your("%s completely %s.",
			 aobjnam(otmph, Blind ? "feel" : "look"),
			 acid_dmg ? "corroded" : "rusty");
		else if (vismon)
		    pline("%s's %s completely %s.", Monnam(victim),
			 aobjnam(otmph, "look"),
			 acid_dmg ? "corroded" : "rusty");
	    }
	}
}

STATIC_PTR
int
select_off(otmp)
register struct obj *otmp;
{
	if(!otmp) return(0);
	if(cursed(otmp)) return(0);
	if((otmp->oclass==RING_CLASS || otmp->otyp == MEAT_RING)
				&& nolimbs(youmonst.data))
		return(0);
	if(welded(uwep) && (otmp==uarmg || otmp==uright || (otmp==uleft
			&& bimanual(uwep))))
		return(0);
	if(uarmg && uarmg->cursed && (otmp==uright || otmp==uleft)) {
		uarmg->bknown = TRUE;
		return(0);
	}
	if(otmp == uarmf && u.utrap && (u.utraptype == TT_BEARTRAP ||
					u.utraptype == TT_INFLOOR)) {
		return (0);
	}
	if((otmp==uarm
#ifdef TOURIST
			|| otmp==uarmu
#endif
					) && uarmc && uarmc->cursed) {
		uarmc->bknown = TRUE;
		return(0);
	}
#ifdef TOURIST
	if(otmp==uarmu && uarm && uarm->cursed) {
		uarm->bknown = TRUE;
		return(0);
	}
#endif

	if(otmp == uarm) takeoff_mask |= WORN_ARMOR;
	else if(otmp == uarmc) takeoff_mask |= WORN_CLOAK;
	else if(otmp == uarmf) takeoff_mask |= WORN_BOOTS;
	else if(otmp == uarmg) takeoff_mask |= WORN_GLOVES;
	else if(otmp == uarmh) takeoff_mask |= WORN_HELMET;
	else if(otmp == uarms) takeoff_mask |= WORN_SHIELD;
#ifdef TOURIST
	else if(otmp == uarmu) takeoff_mask |= WORN_SHIRT;
#endif
	else if(otmp == uleft) takeoff_mask |= LEFT_RING;
	else if(otmp == uright) takeoff_mask |= RIGHT_RING;
	else if(otmp == uamul) takeoff_mask |= WORN_AMUL;
	else if(otmp == ublindf) takeoff_mask |= WORN_BLINDF;
	else if(otmp == uwep) takeoff_mask |= W_WEP;
	else if(otmp == uswapwep) takeoff_mask |= W_SWAPWEP;
	else if(otmp == uquiver) takeoff_mask |= W_QUIVER;

	else impossible("select_off: %s???", doname(otmp));

	return(0);
}

STATIC_OVL struct obj *
do_takeoff()
{
	register struct obj *otmp = (struct obj *)0;

	if (taking_off == W_WEP) {
	  if(!cursed(uwep)) {
	    setuwep((struct obj *) 0);
	    You("are empty %s.", body_part(HANDED));
	    u.twoweap = FALSE;
	  }
	} else if (taking_off == W_SWAPWEP) {
	  setuswapwep((struct obj *) 0);
	  You("no longer have a second weapon readied.");
	  u.twoweap = FALSE;
	} else if (taking_off == W_QUIVER) {
	  setuqwep((struct obj *) 0);
	  You("no longer have ammunition readied.");
	} else if (taking_off == WORN_ARMOR) {
	  otmp = uarm;
	  if(!cursed(otmp)) (void) Armor_off();
	} else if (taking_off == WORN_CLOAK) {
	  otmp = uarmc;
	  if(!cursed(otmp)) (void) Cloak_off();
	} else if (taking_off == WORN_BOOTS) {
	  otmp = uarmf;
	  if(!cursed(otmp)) (void) Boots_off();
	} else if (taking_off == WORN_GLOVES) {
	  otmp = uarmg;
	  if(!cursed(otmp)) (void) Gloves_off();
	} else if (taking_off == WORN_HELMET) {
	  otmp = uarmh;
	  if(!cursed(otmp)) (void) Helmet_off();
	} else if (taking_off == WORN_SHIELD) {
	  otmp = uarms;
	  if(!cursed(otmp)) (void) Shield_off();
#ifdef TOURIST
	} else if (taking_off == WORN_SHIRT) {
	  otmp = uarmu;
	  if(!cursed(otmp))
	    setworn((struct obj *)0, uarmu->owornmask & W_ARMOR);
#endif
	} else if (taking_off == WORN_AMUL) {
	  otmp = uamul;
	  if(!cursed(otmp)) Amulet_off();
	} else if (taking_off == LEFT_RING) {
	  otmp = uleft;
	  if(!cursed(otmp)) Ring_off(uleft);
	} else if (taking_off == RIGHT_RING) {
	  otmp = uright;
	  if(!cursed(otmp)) Ring_off(uright);
	} else if (taking_off == WORN_BLINDF) {
	  if(!cursed(ublindf)) {
	    setworn((struct obj *)0, ublindf->owornmask);
	    if(!Blinded) make_blinded(1L,FALSE); /* See on next move */
	    else	 You("still cannot see.");
	  }
	} else impossible("do_takeoff: taking off %lx", taking_off);

	return(otmp);
}

STATIC_PTR
int
take_off()
{
	register int i;
	register struct obj *otmp;

	if(taking_off) {
	    if(todelay > 0) {

		todelay--;
		return(1);	/* still busy */
	    } else if((otmp = do_takeoff())) off_msg(otmp);

	    takeoff_mask &= ~taking_off;
	    taking_off = 0L;
	}

	for(i = 0; takeoff_order[i]; i++)
	    if(takeoff_mask & takeoff_order[i]) {
		taking_off = takeoff_order[i];
		break;
	    }

	otmp = (struct obj *) 0;
	todelay = 0;

	if (taking_off == 0L) {
	  You("finish disrobing.");
	  return 0;
	} else if (taking_off == W_WEP) {
	  todelay = 1;
	} else if (taking_off == W_SWAPWEP) {
	  todelay = 1;
	} else if (taking_off == W_QUIVER) {
	  todelay = 1;
	} else if (taking_off == WORN_ARMOR) {
	  otmp = uarm;
	  /* If a cloak is being worn, add the time to take it off and put
	   * it back on again.  Kludge alert! since that time is 0 for all
	   * known cloaks, add 1 so that it actually matters...
	   */
	  if (uarmc) todelay += 2 * objects[uarmc->otyp].oc_delay + 1;
	} else if (taking_off == WORN_CLOAK) {
	  otmp = uarmc;
	} else if (taking_off == WORN_BOOTS) {
	  otmp = uarmf;
	} else if (taking_off == WORN_GLOVES) {
	  otmp = uarmg;
	} else if (taking_off == WORN_HELMET) {
	  otmp = uarmh;
	} else if (taking_off == WORN_SHIELD) {
	  otmp = uarms;
#ifdef TOURIST
	} else if (taking_off == WORN_SHIRT) {
	  otmp = uarmu;
	  /* add the time to take off and put back on armor and/or cloak */
	  if (uarm)  todelay += 2 * objects[uarm->otyp].oc_delay;
	  if (uarmc) todelay += 2 * objects[uarmc->otyp].oc_delay + 1;
#endif
	} else if (taking_off == WORN_AMUL) {
	  todelay = 1;
	} else if (taking_off == LEFT_RING) {
	  todelay = 1;
	} else if (taking_off == RIGHT_RING) {
	  todelay = 1;
	} else if (taking_off == WORN_BLINDF) {
	  todelay = 2;
	} else {
	  impossible("take_off: taking off %lx", taking_off);
	  return 0;	/* force done */
	}

	if (otmp) todelay += objects[otmp->otyp].oc_delay;
	set_occupation(take_off, "disrobing", 0);
	return(1);		/* get busy */
}

#endif /* OVLB */
#ifdef OVL1

void
reset_remarm()
{
	taking_off = takeoff_mask = 0L;
}

#endif /* OVL1 */
#ifdef OVLB

int
doddoremarm()
{
    int result = 0;

    if (taking_off || takeoff_mask) {
	You("continue disrobing.");
	set_occupation(take_off, "disrobing", 0);
	return(take_off());
    } else if (!uwep && !uswapwep && !uquiver && !uamul && !ublindf &&
		!uleft && !uright && !wearing_armor()) {
	You("are not wearing anything.");
	return 0;
    }

    add_valid_menu_class(0); /* reset */
    if (flags.menu_style != MENU_TRADITIONAL ||
	    (result = ggetobj("take off", select_off, 0, FALSE)) < -1)
	result = menu_remarm(result);

    return takeoff_mask ? take_off() : 0;
}

STATIC_OVL int
menu_remarm(retry)
int retry;
{
    int n, i = 0;
    menu_item *pick_list;
    boolean all_worn_categories = TRUE;

    if (retry) {
	all_worn_categories = (retry == -2);
    } else if (flags.menu_style == MENU_FULL) {
	all_worn_categories = FALSE;
	n = query_category("What type of things do you want to take off?",
			   invent, WORN_TYPES|ALL_TYPES, &pick_list, PICK_ANY);
	if (!n) return 0;
	for (i = 0; i < n; i++) {
	    if (pick_list[i].item.a_int == ALL_TYPES_SELECTED)
		all_worn_categories = TRUE;
	    else
		add_valid_menu_class(pick_list[i].item.a_int);
	}
	free((genericptr_t) pick_list);
    } else if (flags.menu_style == MENU_COMBINATION) {
	all_worn_categories = FALSE;
	if (ggetobj("take off", select_off, 0, TRUE) == -2)
	    all_worn_categories = TRUE;
    }

    n = query_objlist("What do you want to take off?", invent,
			SIGNAL_NOMENU|USE_INVLET|INVORDER_SORT,
			&pick_list, PICK_ANY,
			all_worn_categories ? is_worn : is_worn_by_type);
    if (n > 0) {
	for (i = 0; i < n; i++)
	    (void) select_off(pick_list[i].item.a_obj);
	free((genericptr_t) pick_list);
    } else if (n < 0) {
	pline("There is nothing else you can remove or unwield.");
    }
    return 0;
}

int
destroy_arm(atmp)
register struct obj *atmp;
{
	register struct obj *otmp;
#define DESTROY_ARM(o) ((otmp = (o)) != 0 && \
			(!atmp || atmp == otmp) && \
			(!obj_resists(otmp, 0, 90)))

	if (DESTROY_ARM(uarmc)) {
		Your("cloak crumbles and turns to dust!");
		(void) Cloak_off();
		useup(otmp);
	} else if (DESTROY_ARM(uarm)) {
		/* may be disintegrated by spell or dragon breath... */
		if (donning(otmp)) cancel_don();
		Your("armor turns to dust and falls to the %s!",
			surface(u.ux,u.uy));
		(void) Armor_gone();
		useup(otmp);
#ifdef TOURIST
	} else if (DESTROY_ARM(uarmu)) {
		Your("shirt crumbles into tiny threads and falls apart!");
		useup(otmp);
#endif
	} else if (DESTROY_ARM(uarmh)) {
		if (donning(otmp)) cancel_don();
		Your("helmet turns to dust and is blown away!");
		(void) Helmet_off();
		useup(otmp);
	} else if (DESTROY_ARM(uarmg)) {
		if (donning(otmp)) cancel_don();
		Your("gloves vanish!");
		(void) Gloves_off();
		useup(otmp);
		selftouch("You");
	} else if (DESTROY_ARM(uarmf)) {
		if (donning(otmp)) cancel_don();
		Your("boots disintegrate!");
		(void) Boots_off();
		useup(otmp);
	} else if (DESTROY_ARM(uarms)) {
		Your("shield crumbles away!");
		(void) Shield_off();
		useup(otmp);
	} else	return(0);		/* could not destroy anything */

#undef DESTROY_ARM
	return(1);
}

void
adj_abon(otmp, delta)
register struct obj *otmp;
register schar delta;
{
	if (uarmg && uarmg == otmp && otmp->otyp == GAUNTLETS_OF_DEXTERITY) {
		if (delta) {
			makeknown(uarmg->otyp);
			ABON(A_DEX) += (delta);
		}
		flags.botl = 1;
	}
	if (uarmh && uarmh == otmp && otmp->otyp == HELM_OF_BRILLIANCE) {
		if (delta) {
			makeknown(uarmh->otyp);
			ABON(A_INT) += (delta);
			ABON(A_WIS) += (delta);
		}
		flags.botl = 1;
	}
}

#endif /* OVLB */

/*do_wear.c*/
