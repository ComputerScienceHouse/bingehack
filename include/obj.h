/*	SCCS Id: @(#)obj.h	3.3	1999/03/13	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef OBJ_H
#define OBJ_H

/* #define obj obj_nh */ /* uncomment for SCO UNIX, which has a conflicting
			  * typedef for "obj" in <sys/types.h> */

union vptrs {
	    struct obj *v_nexthere;	/* floor location lists */
	    struct obj *v_ocontainer;	/* point back to container */
	    struct monst *v_ocarry;	/* point back to carrying monst */
};

struct obj {
	struct obj *nobj;
	union vptrs v;
#define nexthere	v.v_nexthere
#define ocontainer	v.v_ocontainer
#define ocarry		v.v_ocarry

	struct obj *cobj;	/* contents list for containers */
	unsigned o_id;
	xchar ox,oy;
	short otyp;		/* object class number */
	unsigned owt;
	long quan;		/* number of items */

	schar spe;		/* quality of weapon, armor or ring (+ or -)
				   number of charges for wand ( >= -1 )
				   marks your eggs, spinach tins
				   royal coffers for a court ( == 2)
				   tells which fruit a fruit is
				   special for uball and amulet %% BAH */
	char	oclass;		/* object class */
	char	invlet;		/* designation in inventory */
	char	oartifact;	/* artifact array index */

	xchar where;		/* where the object thinks it is */
#define OBJ_FREE	0		/* object not attached to anything */
#define OBJ_FLOOR	1		/* object on floor */
#define OBJ_CONTAINED	2		/* object in a container */
#define OBJ_INVENT	3		/* object in the hero's inventory */
#define OBJ_MINVENT	4		/* object in a monster inventory */
#define OBJ_MIGRATING	5		/* object sent off to another level */
#define OBJ_BURIED	6		/* object buried */
#define OBJ_ONBILL	7		/* object on shk bill */
#define NOBJ_STATES	8
	xchar timed;		/* # of fuses (timers) attached to this obj */

	Bitfield(cursed,1);
	Bitfield(blessed,1);
	Bitfield(unpaid,1);	/* on some bill */
	Bitfield(no_charge,1);	/* if shk shouldn't charge for this */
	Bitfield(known,1);	/* exact nature known */
	Bitfield(dknown,1);	/* color or text known */
	Bitfield(bknown,1);	/* blessing or curse known */
	Bitfield(rknown,1);	/* rustproof or not known */

	Bitfield(oeroded,2);	/* rusted/burnt weapon/armor */
	Bitfield(oeroded2,2);	/* corroded/rotted weapon/armor */
#define greatest_erosion(otmp) (int)((otmp)->oeroded > (otmp)->oeroded2 ? (otmp)->oeroded : (otmp)->oeroded2)
#define MAX_ERODE 3
#define orotten oeroded		/* rotten food */
#define odiluted oeroded	/* diluted potions */
	Bitfield(oerodeproof,1); /* erodeproof weapon/armor */
	Bitfield(olocked,1);	/* object is locked */
	Bitfield(obroken,1);	/* lock has been broken */
	Bitfield(otrapped,1);	/* container is trapped */
#define opoisoned otrapped	/* object (weapon) is coated with poison */

	Bitfield(recharged,3);	/* number of times it's been recharged */
	Bitfield(lamplit,1);	/* a light-source -- can be lit */
#ifdef INVISIBLE_OBJECTS
	Bitfield(oinvis,1);	/* invisible */
#endif
	Bitfield(greased,1);	/* covered with grease */
	Bitfield(oattached,2);	/* obj struct has special attachment */
#define OATTACHED_NOTHING 0
#define OATTACHED_MONST   1	/* monst struct in oextra */
#define OATTACHED_UNUSED2 2
#define OATTACHED_UNUSED3 3

	Bitfield(in_use,1);	/* for magic items before useup items */
	/* 7 free bits */

	int	corpsenm;	/* type of corpse is mons[corpsenm] */
#define leashmon  corpsenm	/* gets m_id of attached pet */
#define spestudied corpsenm	/* how many times a spellbook has been studied */
	unsigned oeaten;	/* nutrition left in food, if partly eaten */
	long age;		/* creation date */

	uchar onamelth;		/* length of name (following oxlth) */
	short oxlth;		/* length of following data */
	/* in order to prevent alignment problems oextra should
	   be (or follow) a long int */
	long owornmask;
	long oextra[1];		/* used for name of ordinary objects - length
				   is flexible; amount for tmp gold objects */
};

#define newobj(xl)	(struct obj *)alloc((unsigned)(xl) + sizeof(struct obj))
#define ONAME(otmp)	(((char *)(otmp)->oextra) + (otmp)->oxlth)

/* Weapons and weapon-tools */
/* KMH -- now based on skill categories.  Formerly:
 *	#define is_sword(otmp)	(otmp->oclass == WEAPON_CLASS && \
 *			 objects[otmp->otyp].oc_wepcat == WEP_SWORD)
 *	#define is_blade(otmp)	(otmp->oclass == WEAPON_CLASS && \
 *			 (objects[otmp->otyp].oc_wepcat == WEP_BLADE || \
 *			  objects[otmp->otyp].oc_wepcat == WEP_SWORD))
 *	#define is_weptool(o)	((o)->oclass == TOOL_CLASS && \
 *			 objects[(o)->otyp].oc_weptool)
 *	#define is_multigen(otyp) (otyp <= SHURIKEN)
 *	#define is_poisonable(otyp) (otyp <= BEC_DE_CORBIN)
 */
#define is_blade(otmp)	(otmp->oclass == WEAPON_CLASS && \
			 objects[otmp->otyp].oc_skill >= P_DAGGER && \
			 objects[otmp->otyp].oc_skill <= P_SABER)
#define is_axe(otmp)	((otmp->oclass == WEAPON_CLASS || \
			 otmp->oclass == TOOL_CLASS) && \
			 objects[otmp->otyp].oc_skill == P_AXE)
#define is_pick(otmp)	((otmp->oclass == WEAPON_CLASS || \
			 otmp->oclass == TOOL_CLASS) && \
			 objects[otmp->otyp].oc_skill == P_PICK_AXE)
#define is_sword(otmp)	(otmp->oclass == WEAPON_CLASS && \
			 objects[otmp->otyp].oc_skill >= P_SHORT_SWORD && \
			 objects[otmp->otyp].oc_skill <= P_SABER)
#define is_pole(otmp)	((otmp->oclass == WEAPON_CLASS || \
			otmp->oclass == TOOL_CLASS) && \
			 (objects[otmp->otyp].oc_skill == P_POLEARMS || \
			 objects[otmp->otyp].oc_skill == P_LANCE))
#define is_spear(otmp)	(otmp->oclass == WEAPON_CLASS && \
			 objects[otmp->otyp].oc_skill >= P_SPEAR && \
			 objects[otmp->otyp].oc_skill <= P_JAVELIN)
#define is_launcher(otmp)	(otmp->oclass == WEAPON_CLASS && \
			 objects[otmp->otyp].oc_skill >= P_BOW && \
			 objects[otmp->otyp].oc_skill <= P_CROSSBOW)
#define is_ammo(otmp)	((otmp->oclass == WEAPON_CLASS || \
			 otmp->oclass == GEM_CLASS) && \
			 objects[otmp->otyp].oc_skill >= -P_CROSSBOW && \
			 objects[otmp->otyp].oc_skill <= -P_BOW)
#define ammo_and_launcher(otmp,ltmp) \
			 (is_ammo(otmp) && (ltmp) && \
			 objects[(otmp)->otyp].oc_skill == -objects[(ltmp)->otyp].oc_skill)
#define is_missile(otmp)	((otmp->oclass == WEAPON_CLASS || \
			 otmp->oclass == TOOL_CLASS) && \
			 objects[otmp->otyp].oc_skill >= -P_BOOMERANG && \
			 objects[otmp->otyp].oc_skill <= -P_DART)
#define is_weptool(o)	((o)->oclass == TOOL_CLASS && \
			 objects[(o)->otyp].oc_skill != P_NONE)
#define bimanual(otmp)	((otmp->oclass == WEAPON_CLASS || \
			 otmp->oclass == TOOL_CLASS) && \
			 objects[otmp->otyp].oc_bimanual)
#define is_multigen(otmp)	(otmp->oclass == WEAPON_CLASS && \
			 objects[otmp->otyp].oc_skill >= -P_SHURIKEN && \
			 objects[otmp->otyp].oc_skill <= -P_BOW)
#define is_poisonable(otmp)	(otmp->oclass == WEAPON_CLASS && \
			 objects[otmp->otyp].oc_skill >= -P_SHURIKEN && \
			 objects[otmp->otyp].oc_skill <= -P_BOW)

/* Armor */
#define is_shield(otmp) (otmp->oclass == ARMOR_CLASS && \
			 objects[otmp->otyp].oc_armcat == ARM_SHIELD)
#define is_helmet(otmp) (otmp->oclass == ARMOR_CLASS && \
			 objects[otmp->otyp].oc_armcat == ARM_HELM)
#define is_boots(otmp)	(otmp->oclass == ARMOR_CLASS && \
			 objects[otmp->otyp].oc_armcat == ARM_BOOTS)
#define is_gloves(otmp) (otmp->oclass == ARMOR_CLASS && \
			 objects[otmp->otyp].oc_armcat == ARM_GLOVES)
#define is_cloak(otmp)	(otmp->oclass == ARMOR_CLASS && \
			 objects[otmp->otyp].oc_armcat == ARM_CLOAK)
#define is_shirt(otmp)	(otmp->oclass == ARMOR_CLASS && \
			 objects[otmp->otyp].oc_armcat == ARM_SHIRT)
#define is_suit(otmp)	(otmp->oclass == ARMOR_CLASS && \
			 objects[otmp->otyp].oc_armcat == ARM_SUIT)

/* Eggs and other food */
#define MAX_EGG_HATCH_TIME 200	/* longest an egg can remain unhatched */
#define stale_egg(egg)	((monstermoves - (egg)->age) > (2*MAX_EGG_HATCH_TIME))
#define ofood(o) ((o)->otyp == CORPSE || (o)->otyp == EGG || (o)->otyp == TIN)
#define polyfodder(obj) (ofood(obj) && \
			 pm_to_cham((obj)->corpsenm) != CHAM_ORDINARY)
#define mlevelgain(obj) (ofood(obj) && (obj)->corpsenm == PM_WRAITH)
#define mhealup(obj)	(ofood(obj) && (obj)->corpsenm == PM_NURSE)

/* Containers */
#define carried(o)	((o)->where == OBJ_INVENT)
#define mcarried(o)	((o)->where == OBJ_MINVENT)
#define Has_contents(o) (/* (Is_container(o) || (o)->otyp == STATUE) && */ \
			 (o)->cobj != (struct obj *)0)
#define Is_container(o) ((o)->otyp >= LARGE_BOX && (o)->otyp <= BAG_OF_TRICKS)
#define Is_box(otmp)	(otmp->otyp == LARGE_BOX || otmp->otyp == CHEST)
#define Is_mbag(otmp)	(otmp->otyp == BAG_OF_HOLDING || \
			 otmp->otyp == BAG_OF_TRICKS)

/* Light sources */
#define Is_candle(otmp) (otmp->otyp == TALLOW_CANDLE || \
			 otmp->otyp == WAX_CANDLE)
#define MAX_OIL_IN_FLASK 400	/* maximum amount of oil in a potion of oil */

/* Flags for get_obj_location(). */
#define CONTAINED_TOO	0x1
#define BURIED_TOO	0x2

#endif /* OBJ_H */
