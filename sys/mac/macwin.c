/*	SCCS Id: @(#)macwin.c	3.3	96/01/15	*/
/* Copyright (c) Jon W{tte, Hao-Yang Wang, Jonathan Handler 1992. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "func_tab.h"
#include "macwin.h"
#include "mactty.h"
#include "wintty.h"

#if defined(applec)
#include <sysequ.h>
#else
#include <LowMem.h>
#endif
#include <AppleEvents.h>
#include <Gestalt.h>
#include <TextUtils.h>
#include <DiskInit.h>

static Boolean kApplicInFront = TRUE;

NhWindow *theWindows = (NhWindow *) 0;

#ifndef USESROUTINEDESCRIPTORS /* not using universal headers */
  /* Cast everything in terms of the new Low Memory function calls. */
# if defined(applec)
#  define LMGetCurStackBase()	(*(long *) CurStackBase)
#  define LMGetDefltStack()		(*(long *) DefltStack)
# elif defined(THINK_C)
#  define LMGetCurStackBase()	CurStackBase
#  define LMGetDefltStack()		(*(long *) DefltStack)
# elif defined(__MWERKS__)
# else
#  error /* need to define LM functions for this compiler */
# endif
#endif /* !USEROUTINEDESCRIPTORS (universal headers) */

/* Borrowed from the Mac tty port */
extern WindowPtr _mt_window;

/* Some useful #defines for the scroll bar width and height */
#define		SBARWIDTH	15
#define		SBARHEIGHT	15

/*
 * We put a TE on the message window for the "top line" queries.
 * top_line is the TE that holds both the query and the user's
 * response.  The first topl_query_len characters in top_line are
 * the query, the rests are the response.  topl_resp is the valid
 * response to a yn query, while topl_resp[topl_def_idx] is the
 * default response to a yn query.
 */
static TEHandle top_line = (TEHandle) nil;
static int	topl_query_len;
static int	topl_def_idx = -1;
static char	topl_resp[10] = "";

#define CHAR_ANY '\n'

/*
 * inSelect means we have a menu window up for selection or
 * something similar. It makes the window with win number ==
 * inSelect a movable modal (unfortunately without the border)
 * and clicking the close box forces an RET into the key
 * buffer. Don't forget to set inSelect to WIN_ERR when you're
 * done...
 */
static winid inSelect = WIN_ERR;

/*
 * The key queue ring buffer where Read is where to take from, 
 * Write is where next char goes and count is queue depth.
 */
static unsigned char keyQueue [QUEUE_LEN];
static int keyQueueRead = 0,
	keyQueueWrite = 0,
	keyQueueCount = 0;

static Boolean gClickedToMove = 0;	/* For ObscureCursor */

static Point clicked_pos;	/* For nh_poskey */
static int clicked_mod;
static Boolean cursor_locked = false;

static ControlActionUPP MoveScrollUPP;		/* scrolling callback, init'ed in InitMac */

void
lock_mouse_cursor(Boolean new_cursor_locked) {
	cursor_locked = new_cursor_locked;
}


/*
 * Add key to input queue, force means flush left and replace if full
 */
void
AddToKeyQueue (unsigned char ch, Boolean force) {
	if (keyQueueCount < QUEUE_LEN) {
		keyQueue [keyQueueWrite++] = ch;
		keyQueueCount++;
	}
	else if (force) {
		keyQueue [keyQueueWrite++] = ch;
		keyQueueRead++;
		if (keyQueueRead >= QUEUE_LEN)
			keyQueueRead = 0;
		keyQueueCount = QUEUE_LEN;
	}
	if (keyQueueWrite >= QUEUE_LEN)
		keyQueueWrite = 0;
}


/*
 * Get key from queue
 */
unsigned char
GetFromKeyQueue (void) {
	unsigned char ret;

	if (keyQueueCount) {
		ret = keyQueue [keyQueueRead++];
		keyQueueCount--;
		if (keyQueueRead >= QUEUE_LEN)
			keyQueueRead = 0;
	}
	else
		ret = 0;
	return ret;
}


/*
 * Cursor movement
 */
static RgnHandle gMouseRgn = (RgnHandle) 0;

/*
 * _Gestalt madness - we rely heavily on the _Gestalt glue, since we
 * don't check for the trap...
 */
MacFlags macFlags;

/*
 * The screen layouts on the small 512x342 screen need special cares.
 */
Boolean small_screen;

#ifdef NHW_BASE
# undef NHW_BASE
#endif
#define NHW_BASE 0

static int FDECL(filter_scroll_key,(const int, NhWindow *));

static void FDECL(GeneralKey, (EventRecord *, WindowPtr));
static void FDECL(macKeyMenu, (EventRecord *, WindowPtr));
static void FDECL(macKeyText, (EventRecord *, WindowPtr));

static void FDECL(macClickMessage, (EventRecord *, WindowPtr));
static void FDECL(macClickTerm, (EventRecord *, WindowPtr));
static void FDECL(macClickMenu, (EventRecord *, WindowPtr));
static void FDECL(macClickText, (EventRecord *, WindowPtr));

static short FDECL(macDoNull, (EventRecord *, WindowPtr));
static short FDECL(macUpdateMessage, (EventRecord *, WindowPtr));
static short FDECL(macUpdateMenu, (EventRecord *, WindowPtr));
static short FDECL(GeneralUpdate, (EventRecord *, WindowPtr));

static void FDECL(macCursorTerm, (EventRecord *, WindowPtr, RgnHandle));
static void FDECL(GeneralCursor, (EventRecord *, WindowPtr, RgnHandle));

static void FDECL(DoScrollBar,(Point, short, ControlHandle, NhWindow *));
static pascal void FDECL(MoveScrollBar, (ControlHandle, short));

typedef void (*CbFunc) (EventRecord *, WindowPtr);
typedef short (*CbUpFunc) (EventRecord *, WindowPtr);
typedef void (*CbCursFunc) (EventRecord *, WindowPtr, RgnHandle);

#define NUM_FUNCS 6
static const CbFunc winKeyFuncs [NUM_FUNCS] = {
	GeneralKey, GeneralKey, GeneralKey, GeneralKey, macKeyMenu, macKeyText
};

static const CbFunc winClickFuncs [NUM_FUNCS] = {
	(CbFunc)macDoNull, macClickMessage, macClickTerm, macClickTerm, macClickMenu ,
	macClickText
};

static const CbUpFunc winUpdateFuncs [NUM_FUNCS] = {
	macDoNull, macUpdateMessage, image_tty, image_tty ,
	macUpdateMenu, GeneralUpdate
};

static const CbCursFunc winCursorFuncs [NUM_FUNCS] = {
	(CbCursFunc) macDoNull, GeneralCursor, macCursorTerm, macCursorTerm ,
	GeneralCursor, GeneralCursor
};


static NhWindow *
GetNhWin(WindowPtr mac_win) {
	if (mac_win == _mt_window)	/* term window is still maintained by both systems, and */
		return theWindows;		/* WRefCon still refers to tty struct, so we have to map it */
	else {
		NhWindow *aWin = (NhWindow *)GetWRefCon (mac_win);
		if (aWin >= theWindows && aWin < &theWindows[NUM_MACWINDOWS])
			return aWin;
	}
	return ((NhWindow *) nil);
}


Boolean CheckNhWin (WindowPtr mac_win) {
	return GetNhWin (mac_win) != nil;
}


static pascal OSErr AppleEventHandler (
	const AppleEvent*	inAppleEvent,
	AppleEvent*			outAEReply,
	long				inRefCon)
{
#if defined(applec) || defined(__MWERKS__)
# pragma unused(outAEReply,inRefCon)
#endif
	Size     actualSize;
	DescType typeCode;
	AEEventID EventID;
	OSErr    err;

	/* Get Event ID */
	err = AEGetAttributePtr (inAppleEvent, keyEventIDAttr,
								typeType, &typeCode,
								&EventID, sizeof (EventID), &actualSize);
	if (err == noErr) {
		switch (EventID) {
			default :
			case kAEOpenApplication :
				macFlags.gotOpen = 1;
				/* fall through */
			case kAEPrintDocuments :
				err = errAEEventNotHandled;
				break;
			case kAEQuitApplication :
				/* Flush key queue */
				keyQueueCount = keyQueueWrite = keyQueueRead = 0;
				AddToKeyQueue ( 'S' , 1 ) ;
				break;
			case kAEOpenDocuments : {
				FSSpec      fss;
				FInfo fndrInfo;
				AEKeyword   keywd;
				AEDescList  docList;
				long     index, itemsInList;

				if((err = AEGetParamDesc(inAppleEvent, keyDirectObject, typeAEList, &docList)) != noErr ||
					(err = AECountItems(&docList, &itemsInList)) != noErr){
					if (err == errAEDescNotFound)
						itemsInList = 0;
					else
						break;
				}

				for(index = 1; index <= itemsInList; index++){
					err = AEGetNthPtr(&docList, index, typeFSS, &keywd, &typeCode, (Ptr)&fss,
								sizeof(FSSpec), &actualSize);
					if(noErr != err)
						break;

					err = FSpGetFInfo (&fss, &fndrInfo);
					if (noErr != err)
						break;

					if (fndrInfo.fdType != SAVE_TYPE) 
						continue;	/* only look at save files */

					process_openfile (fss.vRefNum, fss.parID, fss.name, fndrInfo.fdType);
					if (macFlags.gotOpen)
						break;	/* got our save file */
				}
				err = AEDisposeDesc(&docList);
				break;
			}
		}
	}			

	/* Check to see if all required parameters for this type of event are present */
	if (err == noErr) {
		err = AEGetAttributePtr (inAppleEvent, keyMissedKeywordAttr, 
						  		typeWildCard, &typeCode, NULL, 0, &actualSize);
		if (err == errAEDescNotFound)											 
			err = noErr;		/* got all the required parameters */
		else if (err == noErr)	/* missed a required parameter */
			err = errAEEventNotHandled;
	}

	return err;
}


short win_fonts [NHW_TEXT + 1];

void
InitMac(void) {
	short i;
	long l;
	Str255 volName;

	if (LMGetDefltStack() < 50 * 1024L) {
		SetApplLimit ((void *) ((long) LMGetCurStackBase() - (50 * 1024L)));
	}
	MaxApplZone ();
	for (i = 0; i < 5; i ++)
		MoreMasters ();

	InitGraf (&qd.thePort);
	InitFonts ();
	InitWindows ();
	InitMenus ();
	InitDialogs (0L);
	TEInit ();

	memset (&macFlags, 0, sizeof(macFlags));
	if (!Gestalt (gestaltOSAttr, & l)) {
		macFlags.processes = (l & (1 << gestaltLaunchControl)) ? 1 : 0;
		macFlags.tempMem = (l & (1 << gestaltRealTempMemory)) ? 1 : 0;
		macFlags.hasDebugger = (l & (1 << gestaltSysDebuggerSupport)) ? 1 : 0;
	}
	if (!Gestalt (gestaltQuickdrawVersion, & l))
		macFlags.color = (l >= gestalt8BitQD) ? 1 : 0;

	if (!Gestalt (gestaltFindFolderAttr, & l))
		macFlags.folders = (l & (1 << gestaltFindFolderPresent)) ? 1 : 0;
		
	if (!Gestalt (gestaltHelpMgrAttr, & l))
		macFlags.help = (l & (1 << gestaltHelpMgrPresent)) ? 1 : 0;

	if (!Gestalt (gestaltFSAttr, & l))
		macFlags.fsSpec = (l & (1 << gestaltHasFSSpecCalls)) ? 1 : 0;

	if (!Gestalt (gestaltFontMgrAttr, & l))
		macFlags.trueType = (l & (1 << gestaltOutlineFonts)) ? 1 : 0;

	if (!Gestalt (gestaltAUXVersion, & l))
		macFlags.aux = (l >= 0x200) ? 1 : 0;

	if (!Gestalt (gestaltAliasMgrAttr, & l))
		macFlags.alias = (l & (1 << gestaltAliasMgrPresent)) ? 1 : 0;

	if (!Gestalt (gestaltStandardFileAttr, & l))
		macFlags.standardFile = (l & (1 << gestaltStandardFile58)) ? 1 : 0;

	gMouseRgn = NewRgn ();
	InitCursor ();
	ObscureCursor ();
	
	MoveScrollUPP = NewControlActionProc(MoveScrollBar);

	/* set up base fonts for all window types */
	GetFNum ("\pHackFont", &i);
	if (i == 0)
		i = monaco;
	win_fonts [NHW_BASE] = win_fonts [NHW_MAP] = win_fonts [NHW_STATUS] = i;
	GetFNum ("\pPSHackFont", &i);
	if (i == 0)
		i = geneva;
	win_fonts [NHW_MESSAGE] = i;
	win_fonts [NHW_TEXT] = geneva;
	
	macFlags.hasAE = 0;
	if(!Gestalt(gestaltAppleEventsAttr, &l) && (l & (1L << gestaltAppleEventsPresent))){
		if (AEInstallEventHandler (kCoreEventClass, typeWildCard,
							NewAEEventHandlerProc (AppleEventHandler),
							0,
							FALSE) == noErr)
			macFlags.hasAE = 1;
	}

	/*
	 * We should try to get this data from a rsrc, in the profile file
	 * the user double-clicked...  This data should be saved with the
	 * save file in the resource fork, AND be saveable in "stationary"
	 */
	GetVol (volName, &theDirs.dataRefNum );
	GetWDInfo (theDirs.dataRefNum, &theDirs.dataRefNum, &theDirs.dataDirID, &l);
	if (volName [0] > 31) volName [0] = 31;
	for (l = 1; l <= volName [0]; l++) {
		if (volName [l] == ':') {
			volName [l] = 0;
			volName [0] = l - 1;
			break;
		}
	}
	BlockMove (volName, theDirs.dataName, l);
	BlockMove (volName, theDirs.saveName, l);
	BlockMove (volName, theDirs.levelName, l);
	theDirs.saveRefNum = theDirs.levelRefNum = theDirs.dataRefNum;
	theDirs.saveDirID = theDirs.levelDirID = theDirs.dataDirID;

	/* Create the "record" file, if necessary */
	check_recordfile("");
	return;
}


/*
 * Change default window fonts.
 */
short
set_tty_font_name (int window_type, char *font_name) {
	short fnum;
	Str255 new_font;
	
	if (window_type < NHW_BASE || window_type > NHW_TEXT)
		return general_failure;
		
	C2P (font_name, new_font);
	GetFNum (new_font, &(fnum));
	if (!fnum)
		return general_failure;
	win_fonts [window_type] = fnum;
	return noErr;
}


static void
DrawScrollbar (NhWindow *aWin, WindowPtr theWindow) {
Rect r = theWindow->portRect;
Boolean vis;
short val, lin, win_height;

	if (! aWin->scrollBar) {
		return;
	}
	win_height = r.bottom - r.top;

	if ((*aWin->scrollBar)->contrlRect.top != r.top - 1 ||
		 (*aWin->scrollBar)->contrlRect.left != r.right - SBARWIDTH) {
		MoveControl (aWin->scrollBar, r.right - SBARWIDTH, r.top - 1);
	}
	if ((*aWin->scrollBar)->contrlRect.bottom != r.bottom - SBARHEIGHT ||
		 (*aWin->scrollBar)->contrlRect.right != r.right + 1) {
		SizeControl (aWin->scrollBar, SBARWIDTH+1, win_height - SBARHEIGHT + 2);
	}
	vis = (win_height > (50 + SBARHEIGHT));
	if (vis != (*aWin->scrollBar)->contrlVis) {
		/* current status != control */
		if (vis)/* if visible, show */
			ShowControl (aWin->scrollBar);
		else	/* else hide */
			HideControl (aWin->scrollBar);
	}
	lin = aWin->y_size;
	if (aWin == theWindows + WIN_MESSAGE) {
		/* calculate how big scroll bar is for message window */
		lin -= (win_height - SBARHEIGHT) / aWin->row_height;
		if (lin < 0)
			lin = 0;		
		val = 0;			/* always have message scrollbar active */
	}
	else if (lin) {
		/* calculate how big scroll bar is for other windows */
		lin -= win_height / aWin->row_height;
		if (lin < 0)
			lin = 0;
		if (lin) 	val = 0;	/* if there are 1+ screen lines, activate scrollbar */
		else		val = 255;	/* else grey it out */
	} else {
		val = 255;
	}
	SetControlMaximum (aWin->scrollBar, lin);
	HiliteControl (aWin->scrollBar, val);
	val = GetControlValue (aWin->scrollBar);
	if (val != aWin->scrollPos) {
		InvalRect (&(theWindow->portRect));
		aWin->scrollPos = val;
	}
}


#define MAX_HEIGHT 100
#define MIN_HEIGHT 50
#define MIN_WIDTH 300

/*
 * This function could be overloaded with any amount of intelligence...
 */
int
SanePositions (void) {
	short left, top, width, height;
	short ix, numText = 0, numMenu = 0;
	Rect screenArea = qd.thePort->portBits.bounds;
	WindowPtr theWindow;
	NhWindow *nhWin;

	OffsetRect (&screenArea, - screenArea.left, - screenArea.top);

/* Map Window */
	height = _mt_window->portRect.bottom - _mt_window->portRect.top;
	width = _mt_window->portRect.right - _mt_window->portRect.left;

	if (! RetrievePosition (kMapWindow, &top, &left)) {
		top = GetMBarHeight () + (small_screen ? 2 : 20);
		left = (screenArea.right - width) / 2;
	}
	MoveWindow (_mt_window, left, top, 1);

/* Message Window */
	if (! RetrievePosition (kMessageWindow, &top, &left)) {
		top += height;
		if (! small_screen)
			top += 20;
	}

	if (! RetrieveSize (kMessageWindow, top, left, &height, &width)) {
		height = screenArea.bottom - top - (small_screen ? 2-SBARHEIGHT : 2);
		if (height > MAX_HEIGHT) {
			height = MAX_HEIGHT;
		} else if (height < MIN_HEIGHT) {
			height = MIN_HEIGHT;
			width = MIN_WIDTH;
			left = screenArea.right - width;
			top = screenArea.bottom - MIN_HEIGHT;
		}
	}

/* Move these windows */
	nhWin = theWindows + WIN_MESSAGE;
	theWindow = nhWin->its_window;

	MoveWindow (theWindow, left, top, 1);
	SizeWindow (theWindow, width, height, 1);
	if (nhWin->scrollBar)
		DrawScrollbar (nhWin, theWindow);

	/* Handle other windows */
	for (ix = 0; ix < NUM_MACWINDOWS; ix ++) {
		if (ix != WIN_STATUS && ix != WIN_MESSAGE && ix != WIN_MAP && ix != BASE_WINDOW) {
			theWindow = theWindows [ix].its_window;
			if (theWindow && ((WindowPeek) theWindow)->visible) {
				int shift;
				if (((WindowPeek)theWindow)->windowKind == WIN_BASE_KIND + NHW_MENU) {
					if (! RetrievePosition (kMenuWindow, &top, &left)) {
						top = GetMBarHeight () * 2;
						left = 2;
					}
					top += (numMenu * GetMBarHeight ());
					shift = 20;
					numMenu ++;
				} else {
					if (! RetrievePosition (kTextWindow, &top, &left)) {
						top = GetMBarHeight () * 2;
						left = screenArea.right - 3 - 
							(theWindow->portRect.right - theWindow->portRect.left);
					}
					top += (numText * GetMBarHeight ());
					shift = -20;
					numText ++;
				}
				while (top > screenArea.bottom - MIN_HEIGHT) {
					top -= screenArea.bottom - GetMBarHeight () * 2;
					left += shift;
				}
				MoveWindow (theWindow, left, top, 1);
			}
		}
	}
	
	SetCursor(&qd.arrow);
	return 0;
}


winid
mac_create_nhwindow (int kind) {
	int i;
	NhWindow *aWin;
	FontInfo fi;

	if (kind < NHW_BASE || kind > NHW_TEXT) {
		error ("cre_win: Invalid kind %d.", kind);
		return WIN_ERR;
	}

	for (i = 0; i < NUM_MACWINDOWS; i ++) {
		if (! theWindows [i].its_window)
			break;
	}
	if (i >= NUM_MACWINDOWS) {
		error ("cre_win: Win full; freeing extras");
		for (i = 0; i < NUM_MACWINDOWS; i ++) {
			WindowPeek w = (WindowPeek) theWindows [i].its_window;
			if (w->visible || i == WIN_INVEN ||
				 w->windowKind != WIN_BASE_KIND + NHW_MENU &&
				 w->windowKind != WIN_BASE_KIND + NHW_TEXT)
				continue;
			mac_destroy_nhwindow (i);
			goto got1;
		}
		error ("cre_win: Out of ids (Max=%d)", NUM_MACWINDOWS);
		return WIN_ERR;
	}

got1 :
	aWin = &theWindows [i];
	aWin->windowTextLen = 0L;
	aWin->scrollBar = (ControlHandle) 0;
	aWin->menuInfo = 0;
	aWin->menuSelected = 0;
	aWin->miLen = 0;
	aWin->miSize = 0;
	aWin->menuChar = 'a';
	
	dprintf ("cre_win: New kind %d", kind);

	if (kind == NHW_BASE || kind == NHW_MAP || kind == NHW_STATUS) {
		short x_sz, x_sz_p, y_sz, y_sz_p;
		if (kind != NHW_BASE) {
			if (i != tty_create_nhwindow(kind)) {
				dprintf ("cre_win: error creating kind %d", kind);
			}
			if (kind == NHW_MAP) {
				wins[i]->offy = 0;	/* the message box is in a separate window */
			}
		}
		aWin->its_window = _mt_window;
		get_tty_metrics(aWin->its_window, &x_sz, &y_sz, &x_sz_p, &y_sz_p,
					 &aWin->font_number, &aWin->font_size,
					 &aWin->char_width, &aWin->row_height);
		return i;
	}

	aWin->its_window = GetNewWindow (WIN_BASE_RES + kind, (WindowPtr) 0L, (WindowPtr) -1L);
	((WindowPeek) aWin->its_window)->windowKind = WIN_BASE_KIND + kind;
	SetWRefCon (aWin->its_window, (long) aWin);
	if (!(aWin->windowText = NewHandle (TEXT_BLOCK))) {
		error ("cre_win: NewHandle fail(%ld)", (long) TEXT_BLOCK);
		DisposeWindow (aWin->its_window);
		aWin->its_window = (WindowPtr) 0;
		return WIN_ERR;
	}
	aWin->x_size = aWin->y_size = 0;
	aWin->x_curs = aWin->y_curs = 0;
	aWin->drawn = TRUE;
	mac_clear_nhwindow (i);

	/*HARDCODED*/
	SetPort (aWin->its_window);

	if (kind == NHW_MESSAGE) {
		aWin->font_number = win_fonts [NHW_MESSAGE];
		aWin->font_size = iflags.large_font ? 12 : 9;
		if (!top_line) {
			const Rect out_of_scr = {10000, 10000, 10100, 10100};
			TextFace(bold);
			top_line = TENew(&out_of_scr, &out_of_scr);
			TEActivate(top_line);
			TextFace(normal);
		}
	} else {
		aWin->font_number = win_fonts [NHW_TEXT];
		aWin->font_size = 9;
	}

	TextFont (aWin->font_number); 
	TextSize (aWin->font_size);

	GetFontInfo (&fi);
	aWin->ascent_height = fi.ascent + fi.leading;
	aWin->row_height = aWin->ascent_height + fi.descent;
	aWin->char_width = fi.widMax;

	if (kind == NHW_MENU || kind == NHW_TEXT || kind == NHW_MESSAGE) {
		Rect r = aWin->its_window->portRect;
		r.left = r.right - SBARWIDTH;
		r.bottom -= SBARHEIGHT;
		r.top -= 1;
		r.right += 1;
		aWin->scrollBar = NewControl (aWin->its_window, &r, "\p", (r.bottom > r.top + 50), 0, 0, 0, 16, 0L);
		aWin->scrollPos = 0;
	}
	/* set initial position */
	MoveWindow (aWin->its_window, 30 + rn2 (20), 250 + rn2 (50), FALSE);

	return i;
}


void
mac_init_nhwindows (int *argcp, char **argv) {
	Rect scr = (*GetGrayRgn())->rgnBBox;
	small_screen = scr.bottom - scr.top <= (iflags.large_font ? 12*40 : 9*40);

	InitMenuRes ();

	theWindows = (NhWindow *) NewPtrClear (NUM_MACWINDOWS * sizeof (NhWindow));
	mustwork(MemError());

	DimMenuBar ();

	tty_init_nhwindows(argcp, argv);
	iflags.window_inited = TRUE;

	/* Some ugly hacks to make both interfaces happy:
	 * Mac port uses both tty interface (for main map) and extra windows.  The winids need to
	 * be kept in synch for both interfaces to map.  Also, the "blocked" display_nhwindow case
	 * for the map automatically calls the tty interface for the message box, so some version
	 * of the message box has to exist in the tty world to prevent a meltdown, even though most
	 * messages are handled in mac window.
	 */
	mac_create_nhwindow(NHW_BASE);
	tty_create_nhwindow(NHW_MESSAGE);
}


void
mac_clear_nhwindow (winid win) {
	long l;
	Rect r;
	WindowPtr theWindow;
	NhWindow *aWin = &theWindows [win];

	if (win < 0 || win >= NUM_MACWINDOWS || !(theWindow = aWin->its_window)) {
		error ("clr_win: Invalid win %d.", win);
		return;
	}
	if (theWindow == _mt_window) {
		tty_clear_nhwindow(win);
		return;
	}
	if (!aWin->drawn)
		return;

	SetPort (theWindow);
	r = theWindow->portRect;
	if (aWin->scrollBar)
		r.right -= SBARWIDTH;

	switch (((WindowPeek) theWindow)->windowKind - WIN_BASE_KIND) {
	case NHW_MESSAGE :
		if (aWin->scrollPos == aWin->y_size - 1)	/* if no change since last clear */
			return;									/* don't bother with redraw */
		r.bottom -= SBARHEIGHT;
		for (l = 0; aWin->y_size > iflags.msg_history;) {
			const char cr = CHAR_CR;
			l = Munger(aWin->windowText, l, &cr, 1, nil, 0) + 1;
			--aWin->y_size;
		}
		if (l) {
			aWin->windowTextLen -= l;
			BlockMove(*aWin->windowText + l, *aWin->windowText, aWin->windowTextLen);
		}
		aWin->last_more_lin = aWin->y_size;
		aWin->save_lin	= aWin->y_size;
		aWin->scrollPos = aWin->y_size ? aWin->y_size - 1 : 0;
		break;
	case NHW_MENU:
		if (aWin->menuInfo) {
			DisposeHandle((Handle)aWin->menuInfo);
			aWin->menuInfo = NULL;
		}
		if (aWin->menuSelected) {
			DisposeHandle((Handle)aWin->menuSelected);
			aWin->menuSelected = NULL;
		}
		aWin->menuChar = 'a';
		aWin->miSelLen = 0;
		aWin->miLen = 0;
		aWin->miSize = 0;
		/* Fall-Through */
	default :
		SetHandleSize (aWin->windowText, TEXT_BLOCK);
		aWin->windowTextLen = 0L;
		aWin->x_size = 0;
		aWin->y_size = 0;
		aWin->scrollPos = 0;
		break;
	}
	if (aWin->scrollBar) {
		SetControlMaximum (aWin->scrollBar, aWin->y_size);
		SetControlValue(aWin->scrollBar, aWin->scrollPos);
	}
	aWin->y_curs = 0;
	aWin->x_curs = 0;
	aWin->drawn = FALSE;
	InvalRect (&r);
}


static Boolean
ClosingWindowChar(const int c) {
	return c == CHAR_ESC || c == CHAR_BLANK || c == CHAR_LF || c == CHAR_CR ||
			c == 'q';
}


static Boolean
in_topl_mode(void) {
	return WIN_MESSAGE != WIN_ERR && top_line &&
		(*top_line)->viewRect.left < theWindows[WIN_MESSAGE].its_window->portRect.right;
}


#define BTN_IND 2
#define BTN_W	40
#define BTN_H	(SBARHEIGHT-3)

static void
topl_resp_rect(int resp_idx, Rect *r) {
	r->left	  = (BTN_IND + BTN_W) * resp_idx + BTN_IND;
	r->right  = r->left + BTN_W;
	r->bottom = theWindows[WIN_MESSAGE].its_window->portRect.bottom - 1;
	r->top	  = r->bottom - BTN_H;
}


void
enter_topl_mode(char *query) {
	if (in_topl_mode())
		Debugger();

	putstr(WIN_MESSAGE, ATR_BOLD, query);

	topl_query_len = strlen(query);
	(*top_line)->selStart = topl_query_len;
	(*top_line)->selEnd = topl_query_len;
	(*top_line)->viewRect.left = 0;
	PtrToXHand(query, (*top_line)->hText, topl_query_len);
	TECalText(top_line);

	DimMenuBar();
	mac_display_nhwindow(WIN_MESSAGE, FALSE);
}


void
leave_topl_mode(char *answer) {
	char *ap, *bp;

	int ans_len = (*top_line)->teLength - topl_query_len;
	NhWindow *aWin = theWindows + WIN_MESSAGE;

	if (!in_topl_mode())
		Debugger();

	BlockMove(*(*top_line)->hText + topl_query_len, answer, ans_len);
	answer[ans_len] = '\0';

	/* remove unprintables from the answer */
	for (ap = bp = answer; *ap; ap++) {
		if (*ap >= ' ' && *ap < 128) {
			if (bp != ap)
				*bp = *ap;
			bp++;
		}
	}
	*bp = 0;
	
	if (aWin->windowTextLen && (*aWin->windowText)[aWin->windowTextLen-1] == CHAR_CR) {
		-- aWin->windowTextLen;
		-- aWin->y_size;
	}
	putstr(WIN_MESSAGE, ATR_BOLD, answer);

	(*top_line)->viewRect.left += 10000;
	UndimMenuBar();
}

/*
 * TESetSelect flushes out all the pending key strokes.  I hate it.
 */
static void
topl_set_select(short selStart, short selEnd) {
	TEDeactivate(top_line);
	(*top_line)->selStart	= selStart;
	(*top_line)->selEnd	= selEnd;
	TEActivate(top_line);
}


static void
topl_replace(char *new_ans) {
	topl_set_select(topl_query_len, (*top_line)->teLength);
	TEDelete(top_line);
	TEInsert(new_ans, strlen(new_ans), top_line);
}


Boolean
topl_key(unsigned char ch) {
	switch (ch) {
		case CHAR_ESC:
			topl_replace("\x1b");
		case CHAR_ENTER: case CHAR_CR: case CHAR_LF:
			return false;

		case 0x1f & 'P':
			mac_doprev_message();
			return true;
		case '\x1e'/* up arrow */:
			topl_set_select(topl_query_len, topl_query_len);
			return true;
		case CHAR_BS: case '\x1c'/* left arrow */:
			if ((*top_line)->selEnd <= topl_query_len)
				return true;
		default:
			TEKey(ch, top_line);
			return true;
	}
}


Boolean
topl_ext_key(unsigned char ch) {
	switch (ch) {
		case CHAR_ESC:
			topl_replace("\x1b");
		case CHAR_ENTER: case CHAR_CR: case CHAR_LF:
			return false;

		case 0x1f & 'P':
			mac_doprev_message();
			return true;
		case CHAR_BS:
			topl_replace("");
			return true;
		default: {
			int com_index = -1, oindex = 0;
			TEKey (ch, top_line);
			while(extcmdlist[oindex].ef_txt != (char *)0) {
				if(!strncmpi(*(*top_line)->hText + topl_query_len,
							 extcmdlist[oindex].ef_txt,
							 (*top_line)->teLength - topl_query_len)) {
					if(com_index == -1) /* No matches yet*/
						com_index = oindex;
					else /* More than 1 match */ {
						com_index = -2;
						break;
					}
				}
				oindex++;
			}
			if(com_index >= 0)
				topl_replace((char *) extcmdlist[com_index].ef_txt);
			return true;
		}
	}
}


static void
topl_flash_resp(int resp_idx) {
	unsigned long dont_care;
	Rect frame;
	SetPort(theWindows[WIN_MESSAGE].its_window);
	topl_resp_rect(resp_idx, &frame);
	InsetRect(&frame, 1, 1);
	InvertRect(&frame);
	Delay(GetDblTime() / 2, &dont_care);
	InvertRect(&frame);
}


static void
topl_set_def(int new_def_idx) {
	Rect frame;
	SetPort(theWindows[WIN_MESSAGE].its_window);
	topl_resp_rect(topl_def_idx, &frame);
	InvalRect(&frame);
	topl_def_idx = new_def_idx;
	topl_resp_rect(new_def_idx, &frame);
	InvalRect(&frame);
}


void
topl_set_resp(char *resp, char def) {
	char *loc;
	Rect frame;
	int r_len, r_len1;

	if (!resp) {
		const char any_str[2] = {CHAR_ANY, '\0'};
		resp = (char *) any_str;
		def = CHAR_ANY;
	}

	SetPort(theWindows[WIN_MESSAGE].its_window);
	r_len1 = strlen(resp);
	r_len  = strlen(topl_resp);
	if (r_len < r_len1)
		r_len = r_len1;
	topl_resp_rect(0, &frame);
	frame.right = (BTN_IND + BTN_W) * r_len;
	InvalRect(&frame);

	strcpy(topl_resp, resp);
	loc = strchr (resp, def);
	topl_def_idx = loc ? loc - resp : -1;
}


static char
topl_resp_key(char ch) {
	if (strlen(topl_resp) > 0) {
		char *loc = strchr(topl_resp, ch);

		if (!loc) {
			if (ch == '\x9'/* tab */) {
				topl_set_def(topl_def_idx<=0 ? strlen(topl_resp)-1 : topl_def_idx-1);
				ch = '\0';
			} else if (ch == CHAR_ESC) {
				loc = strchr(topl_resp, 'q');
				if (!loc) {
					loc = strchr(topl_resp, 'n');
					if (!loc && topl_def_idx >= 0)
						loc = topl_resp + topl_def_idx;
				}
			} else if (ch == (0x1f & 'P')) {
				mac_doprev_message();
				ch = '\0';
			} else if (topl_def_idx >= 0) {
				if (ch == CHAR_ENTER || ch == CHAR_CR || ch == CHAR_LF ||
					 ch == CHAR_BLANK || topl_resp[topl_def_idx] == CHAR_ANY)
					loc = topl_resp + topl_def_idx;

				else if (strchr(topl_resp, '#')) {
					if (digit(ch)) {
						topl_set_def(strchr(topl_resp, '#') - topl_resp);
						TEKey(ch, top_line);
						ch = '\0';

					} else if (topl_resp[topl_def_idx] == '#') {
						if (ch == '\x1e'/* up arrow */) {
							topl_set_select(topl_query_len, topl_query_len);
							ch = '\0';
						} else if (ch == '\x1d'/* right arrow */ ||
							   ch == '\x1f'/* down arrow */ ||
							   ch == CHAR_BS || ch == '\x1c'/* left arrow */ &&
							   (*top_line)->selEnd > topl_query_len) {
							TEKey(ch, top_line);
							ch = '\0';
						}
					}
				}
			}
		}

		if (loc) {
			topl_flash_resp(loc - topl_resp);
			if (*loc != CHAR_ANY)
				ch = *loc;
			TEKey(ch, top_line);
		}
	}

	return ch;
}


static void
adjust_window_pos(NhWindow *aWin, short width, short height) {
	WindowPtr theWindow = aWin->its_window;
	Rect scr_r = (*GetGrayRgn())->rgnBBox;
	const Rect win_ind = {2, 2, 3, 3};
	const short	min_w = theWindow->portRect.right - theWindow->portRect.left,
				max_w = scr_r.right - scr_r.left - win_ind.left - win_ind.right;
	Point pos;
	short max_h;

	SetPort(theWindow);
	if (!RetrieveWinPos(theWindow, &pos.v, &pos.h)) {
		pos.v = 0;	/* take window's existing position */
		pos.h = 0;
		LocalToGlobal(&pos);
	}

	max_h = scr_r.bottom - win_ind.bottom - pos.v;
	if (height > max_h)		height = max_h;
	if (height < MIN_HEIGHT)	height = MIN_HEIGHT;
	if (width < min_w)		width = min_w;
	if (width > max_w)		width = max_w;
	SizeWindow(theWindow, width, height, true);

	if (pos.v + height + win_ind.bottom > scr_r.bottom)
		pos.v = scr_r.bottom - height - win_ind.bottom;
	if (pos.h + width + win_ind.right > scr_r.right)
		pos.h = scr_r.right	 - width - win_ind.right;
	MoveWindow(theWindow, pos.h, pos.v, false);
	if (aWin->scrollBar)	
		DrawScrollbar (aWin, theWindow);
}


/*
 * display/select/update the window.
 * If f is true, this window should be "modal" - don't return
 * until presumed seen.
 */
void
mac_display_nhwindow (winid win, BOOLEAN_P f) {
	WindowPtr theWindow;
	NhWindow *aWin = &theWindows [win];

	if (win < 0 || win >= NUM_MACWINDOWS || !(theWindow = aWin->its_window)) {
		error ("disp_win: Invalid window %d.", win);
		return;
	}

	if (theWindow == _mt_window) {
		tty_display_nhwindow(win, f);
		return;
	}

	if (f && inSelect == WIN_ERR && win == WIN_MESSAGE) {
		topl_set_resp ((char *)0, 0);
		if (aWin->windowTextLen > 0 &&
			 (*aWin->windowText) [aWin->windowTextLen - 1] == CHAR_CR) {
			-- aWin->windowTextLen;
			-- aWin->y_size;
		}
		putstr (win, flags.standout ? ATR_INVERSE : ATR_NONE, " --More--");
	}

	if (! ((WindowPeek) theWindow)->visible) {
		if (win != WIN_MESSAGE) {
			adjust_window_pos(aWin, aWin->x_size + SBARWIDTH+1, aWin->y_size *aWin->row_height);
		}

		if (! small_screen || win != WIN_MESSAGE || f)
			SelectWindow (theWindow);
		ShowWindow (theWindow);
	}

	if (f && inSelect == WIN_ERR) {
		int ch;

		DimMenuBar();
		inSelect = win;
		do {
			ch = mac_nhgetch ();
		} while (! ClosingWindowChar (ch));
		inSelect = WIN_ERR;
		UndimMenuBar();

		if (win == WIN_MESSAGE)
			topl_set_resp ("", '\0');
		else
			HideWindow (theWindow);

	} else {
		mac_get_nh_event ();
	}
}


void
mac_destroy_nhwindow (winid win) {
	WindowPtr theWindow;
	NhWindow *aWin = &theWindows [win];
	int kind, visible;

	if (win < 0 || win >= NUM_MACWINDOWS) {
		if (iflags.window_inited)
			error ("dest_win: Invalid win %d (Max %d).", win, NUM_MACWINDOWS);
		return;
	}
	theWindow = aWin->its_window;
	if (! theWindow) {
		error ("dest_win: Not allocated win %d.", win);
		return;
	}

	/*
	 * Check special windows.
	 * The base window should never go away.
	 * The other "standard" windows should not go away
	 * unless we've exitted nhwindows.
	 */
	if (win == BASE_WINDOW) {
		return;
	}
	if (win == WIN_INVEN || win == WIN_MESSAGE) {
		if (iflags.window_inited) {
			if (flags.tombstone && killer) {
				/* Prepare for the coming of the tombstone window. */
				win_fonts [NHW_TEXT] = monaco;
			}
			return;
		}
		if (win == WIN_MESSAGE)
			WIN_MESSAGE = WIN_ERR;
	}

	kind = ((WindowPeek) theWindow)->windowKind - WIN_BASE_KIND;
	visible = ((WindowPeek) theWindow)->visible;

	if ((! visible || (kind != NHW_MENU && kind != NHW_TEXT)) &&
		theWindow != _mt_window) {
		DisposeWindow (theWindow);
		if (aWin->windowText) {
			DisposHandle (aWin->windowText);
		}
		aWin->its_window = (WindowPtr) 0;
		aWin->windowText = (Handle) 0;
	}
}


void
mac_number_pad (int pad) {
	iflags.num_pad = pad;
}


void
trans_num_keys(EventRecord *theEvent) {
#if defined(applec) || defined(__MWERKS__)
# pragma unused(theEvent)
#endif
/* KMH -- Removed this translation.
 * Number pad keys should always emit digit characters.
 * That's consistent with the default MacOS behavior.
 * The number_pad option controls how digits are interpreted.
 */
#if 0
	if (iflags.num_pad) {
		Handle h = GetResource('Nump', theEvent->modifiers & shiftKey ? 129 : 128);
		if (h) {
			short inkey = (theEvent->message & keyCodeMask), *ab = (short *)*h;
			int i = ab[0];
			for (; i; i--) {
				if (inkey == (ab[i] & keyCodeMask)) {
					theEvent->message = ab[i];
					break;
				}
			}
		}
	}
#endif
}


/*
 * Note; theWindow may very well be null here, since keyDown may call
 * it when theres no window !!!
 */
void
GeneralKey (EventRecord *theEvent, WindowPtr theWindow) {
#if defined(applec) || defined(__MWERKS__)
# pragma unused(theWindow)
#endif
	trans_num_keys (theEvent);
	AddToKeyQueue (topl_resp_key (theEvent->message & 0xff), TRUE);
}


/*
 * Routine used to select and de-select elements in a menu window, used by KeyMenu,
 * ClickMenu, and UpdateMenu.  Takes the NhWindow and a line ref relative to the scrollbar.
 */
static void ToggleMenuSelect (NhWindow *aWin, int line) {
	Rect r;
	WindowPtr theWindow = aWin->its_window;
	
	r = theWindow->portRect;
	if (aWin->scrollBar)
		r.right -= SBARWIDTH;			
	r.top = line * aWin->row_height;
	r.bottom = r.top + aWin->row_height;

	LMSetHiliteMode((UInt8) (LMGetHiliteMode() & 0x7F));
	InvertRect(&r);
}

/*
 * Check to see if given item is selected
 */
static Boolean
IsListItemSelected (NhWindow *aWin, int item) {
	int		i;

	HLock ((char**)aWin->menuSelected);
	/* Find item in selection list */
	for (i = aWin->miSelLen - 1; i >= 0; i--) {
		if ((*aWin->menuSelected) [i] == item)
			break;
	}
	HUnlock ((char**)aWin->menuSelected);
	return i >= 0;
}

/*
 * Add item to selection list if it's not selected already
 * If it is selected already, remove it from the list.
 */
static void
ToggleMenuListItemSelected (NhWindow *aWin, short item) {
	int i;

	HLock ((char**)aWin->menuSelected);
	/* Find item in selection list */
	for (i = aWin->miSelLen - 1; i >= 0; i--)
		if ((*aWin->menuSelected) [i] == item)
			break;

	if (i < 0) {
		(*aWin->menuSelected) [aWin->miSelLen] = item;
		aWin->miSelLen++;
	}
	else {
		short *mi = &(*aWin->menuSelected)[i];
		aWin->miSelLen --;
		memcpy (mi, mi + 1, (aWin->miSelLen - i)*sizeof(short));
	}
	HUnlock ((char**)aWin->menuSelected);
}


/*
 * Find menu item in list given a line number on the window
 */
static short
ListCoordinateToItem (NhWindow *aWin, short Row) {
	int					i, item = -1;
	MacMHMenuItem *		mi;

	HLock ((char**)aWin->menuInfo);
	for (i = 0, mi = *aWin->menuInfo; i < aWin->miLen; i++, mi++) {
		if (mi->line == Row + aWin->scrollPos) {
			item = i;
			break;
		}
	}
	HUnlock ((char**)aWin->menuInfo);

	return item;
}


static void
macKeyMenu (EventRecord *theEvent, WindowPtr theWindow) {
	NhWindow *aWin = GetNhWin(theWindow);
	MacMHMenuItem *mi;
	int l, ch = theEvent->message & 0xff;

	if (aWin) {
		HLock ((char**)aWin->menuInfo);
		for (l = 0, mi = *aWin->menuInfo; l < aWin->miLen; l++, mi++) {
			if (mi->accelerator == ch) {
				ToggleMenuListItemSelected (aWin, l);
				if (mi->line >= aWin->scrollPos && mi->line <= aWin->y_size) {
					SetPort(theWindow);
					ToggleMenuSelect (aWin, mi->line - aWin->scrollPos);
				}				
				/* Dismiss window if only picking one item */
				if (aWin->how != PICK_ANY)
					AddToKeyQueue(CHAR_CR, 1);
				break;
			}
		}
		HUnlock ((char**)aWin->menuInfo);
		/* add key if didn't find it in menu and not filtered */
		if (l == aWin->miLen && filter_scroll_key (ch, aWin))
			GeneralKey (theEvent, theWindow);
	}
}


static void
macClickMenu (EventRecord *theEvent, WindowPtr theWindow) {
	Point p;
	NhWindow *aWin = GetNhWin (theWindow);

	if (aWin->scrollBar && (*aWin->scrollBar)->contrlVis) {
		short code;
		ControlHandle theBar;

		p = theEvent->where;
		GlobalToLocal (&p);
		code = FindControl (p, theWindow, &theBar);
		if (code) {
			DoScrollBar (p, code, theBar, aWin);
			return;
		}
		if (p.h >= theWindow->portRect.right - SBARWIDTH)
			return;
	}
	if (inSelect != WIN_ERR && aWin->how != PICK_NONE) {
		short		currentRow = -1, previousRow = -1;
		short		previousItem = -1, item = -1;
		Boolean		majorSelectState, firstRow = TRUE;

		do {
			SystemTask ();
			GetMouse (&p);
			currentRow = p.v / aWin->row_height;
			if (p.h < theWindow->portRect.left || p.h > theWindow->portRect.right ||
				p.v < 0 || p.v > theWindow->portRect.bottom || currentRow >= aWin->y_size) {
				continue;	/* not in window range */
			}

			item = ListCoordinateToItem (aWin, currentRow);

			if (item != previousItem) {
				/* Implement typical Mac multiple-selection behavior
				 * (ie, not the UI implemented by the Finder)
				 */
				Boolean	itemIsSelected = IsListItemSelected (aWin,item);

				if (firstRow) {
					/* this is first valid row, so major state is opposite of what this row is */
					majorSelectState = !itemIsSelected;
					firstRow = FALSE;
				}

				if (aWin->how == PICK_ONE && previousItem != -1) {
					/* if previous row was selected and we're only selecting one object,
					 * deselect previous row!
					 */
					ToggleMenuListItemSelected (aWin, previousItem);
					ToggleMenuSelect (aWin, previousRow);
					previousItem = -1;
				}

				if (item == -1)
					continue;	/* header line */
					
				if (majorSelectState != itemIsSelected) {
					ToggleMenuListItemSelected (aWin, item);
					ToggleMenuSelect (aWin, currentRow);
				}

				previousRow		= currentRow;
				previousItem	= item;
			}
		} while (StillDown ());

		/* Dismiss window if only picking one item */
		if (aWin->how == PICK_ONE)
			AddToKeyQueue(CHAR_CR, 1);
	}
}


static void
macKeyText (EventRecord *theEvent, WindowPtr theWindow) {
	char c = filter_scroll_key (theEvent->message & 0xff, GetNhWin (theWindow));
	if (c) {
		if (inSelect == WIN_ERR && ClosingWindowChar (c)) {
			HideWindow (theWindow);
			mac_destroy_nhwindow (GetNhWin (theWindow) - theWindows);
		} else {
			GeneralKey (theEvent, theWindow);
		}
	}
}


static void
macClickText (EventRecord *theEvent, WindowPtr theWindow) {
	NhWindow *aWin = GetNhWin (theWindow);

	if (aWin->scrollBar && (*aWin->scrollBar)->contrlVis) {
		short code;
		Point p = theEvent->where;
		ControlHandle theBar;

		GlobalToLocal (&p);
		code = FindControl (p, theWindow, &theBar);
		if (code) {
			DoScrollBar (p, code, theBar, aWin);
		}
	}
}


static void
macClickMessage (EventRecord *theEvent, WindowPtr theWindow) {
	int r_idx = 0;
	Point mouse = theEvent->where;

	GlobalToLocal(&mouse);
	while (topl_resp[r_idx]) {
		Rect frame;
		topl_resp_rect(r_idx, &frame);
		InsetRect(&frame, 1, 1);
		if (PtInRect(mouse, &frame)) {
			Boolean in_btn = true;

			InvertRect(&frame);
			while (WaitMouseUp()) {
				SystemTask();
				GetMouse(&mouse);
				if (PtInRect(mouse, &frame) != in_btn) {
					in_btn = !in_btn;
					InvertRect(&frame);
				}
			}
			if (in_btn) {
				InvertRect(&frame);
				AddToKeyQueue (topl_resp [r_idx], 1);
			}
			return;

		}
		++r_idx;
	}

	macClickText(theEvent, theWindow);
}


static void
macClickTerm (EventRecord *theEvent, WindowPtr theWindow) {
	NhWindow *nhw = GetNhWin(theWindow);
	Point where = theEvent->where;

	GlobalToLocal(&where);
	where.h = where.h / nhw->char_width + 1;
	where.v = where.v / nhw->row_height;
	clicked_mod = (theEvent->modifiers & shiftKey) ? CLICK_2 : CLICK_1;

	if (strchr(topl_resp, click_to_cmd(where.h, where.v, clicked_mod)))
		nhbell();
	else {
		if (cursor_locked)
			while (WaitMouseUp())
				SystemTask();

		gClickedToMove = TRUE;
		clicked_pos = where;
	}
}

static pascal void
MoveScrollBar (ControlHandle theBar, short part) {
	EventRecord fake;
	Rect r;
	RgnHandle rgn;
	int now, amtToScroll;
	WindowPtr theWin;
	NhWindow *winToScroll;
	
	if (!part)
		return;

	theWin = (*theBar)->contrlOwner;
	winToScroll = (NhWindow*)(GetWRefCon(theWin));
	now = GetControlValue (theBar);
		
	if (part == kControlPageUpPart || part == kControlPageDownPart)	
		amtToScroll = (theWin->portRect.bottom - theWin->portRect.top) / winToScroll->row_height;
	else
		amtToScroll = 1;

	if (part == kControlPageUpPart || part == kControlUpButtonPart) {
		int bound = GetControlMinimum (theBar);
		if (now - bound < amtToScroll)
			amtToScroll = now - bound;
		amtToScroll = -amtToScroll;
	} else {
		int bound = GetControlMaximum (theBar);
		if (bound - now < amtToScroll)
			amtToScroll = bound - now;
	}
	
	if (!amtToScroll)
		return;

	SetControlValue (theBar, now + amtToScroll);
	winToScroll->scrollPos = now + amtToScroll;
	r = theWin->portRect;
	r.right -= SBARWIDTH;
	if (winToScroll == theWindows + WIN_MESSAGE)
		r.bottom -= SBARHEIGHT;
	rgn = NewRgn ();
	ScrollRect (&r, 0, -amtToScroll * winToScroll->row_height, rgn);
	if (rgn) {
		InvalRgn (rgn);
		BeginUpdate (theWin);
	}
	winUpdateFuncs [((WindowPeek)theWin)->windowKind - WIN_BASE_KIND] (&fake, theWin);
	if (rgn) {
		EndUpdate (theWin);
		DisposeRgn (rgn);
	}
}


static void
DoScrollBar (Point p, short code, ControlHandle theBar, NhWindow *aWin) {
	ControlActionUPP func = NULL;

	if (code == kControlUpButtonPart || code == kControlPageUpPart ||
		code == kControlDownButtonPart || code == kControlPageDownPart)
		func = MoveScrollUPP;
	(void) TrackControl (theBar, p, func);
	if (!func) {
		if (aWin->scrollPos != GetControlValue (theBar)) {
			aWin->scrollPos = GetControlValue (theBar);
			InvalRect (&(aWin->its_window)->portRect);
		}
	}
}


static int
filter_scroll_key(const int ch, NhWindow *aWin) {
	if (aWin->scrollBar && GetControlValue(aWin->scrollBar) < GetControlMaximum(aWin->scrollBar)) {
		short part = 0;
		if (ch == CHAR_BLANK) {
			part = kControlPageDownPart;
		}
		else if (ch == CHAR_CR || ch == CHAR_LF) {
			part = kControlDownButtonPart;
		}
		if (part) {
			SetPort(aWin->its_window);
			MoveScrollBar(aWin->scrollBar, part);
			return 0;
		}
	}
	return ch;
}


int
mac_doprev_message(void) {
	if (WIN_MESSAGE) {
		NhWindow *winToScroll = &theWindows[WIN_MESSAGE];
		mac_display_nhwindow(WIN_MESSAGE, FALSE);
		SetPort(winToScroll->its_window);
		MoveScrollBar(winToScroll->scrollBar, kControlUpButtonPart);
	}	
	return 0;
}


static short
macDoNull (EventRecord *theEvent, WindowPtr theWindow) {
	if (!theEvent || !theWindow)
		Debugger ();
	return 0;
}


static void
draw_growicon_vert_only(WindowPtr wind) {
	GrafPtr org_port;
	RgnHandle org_clip = NewRgn();
	Rect r = wind->portRect;
	r.left = r.right - SBARWIDTH;

	GetPort(&org_port);
	SetPort(wind);
	GetClip(org_clip);
	ClipRect(&r);
	DrawGrowIcon(wind);
	SetClip(org_clip);
	DisposeRgn(org_clip);
	SetPort(org_port);
}


static short
macUpdateMessage (EventRecord *theEvent, WindowPtr theWindow) {
	RgnHandle org_clip = NewRgn(), clip = NewRgn();
	Rect r = theWindow->portRect;
	NhWindow *aWin = GetNhWin (theWindow);
	int l;

	if (!theEvent) {
		Debugger ();
	}

	GetClip(org_clip);

	DrawControls(theWindow);
	DrawGrowIcon(theWindow);

	for (l = 0; topl_resp[l]; l++) {
		StringPtr name;
		unsigned char tmp[2];
		FontInfo font;
		Rect frame;
		topl_resp_rect(l, &frame);
		switch (topl_resp[l]) {
			case 'y':
				name = "\pyes";
				break;
			case 'n':
				name = "\pno";
				break;
			case 'N':
				name = "\pNone";
				break;
			case 'a':
				name = "\pall";
				break;
			case 'q':
				name = "\pquit";
				break;
			case CHAR_ANY:
				name = "\pany key";
				break;
			default:
				tmp[0] = 1;
				tmp[1] = topl_resp[l];
				name = tmp;
				break;
		}
		TextFont(geneva);
		TextSize(9);
		GetFontInfo(&font);
		MoveTo ((frame.left + frame.right - StringWidth(name)) / 2,
			(frame.top + frame.bottom + font.ascent-font.descent-font.leading-1) / 2);
		DrawString(name);
		PenNormal();
		if (l == topl_def_idx)
			PenSize(2, 2);
		FrameRoundRect(&frame, 4, 4);
	}

	r.right -= SBARWIDTH;
	r.bottom -= SBARHEIGHT;
	/* Clip to the portrect - scrollbar/growicon *before* adjusting the rect
		to be larger than the size of the window (!) */
	RectRgn(clip, &r);
	SectRgn(clip, org_clip, clip);
	if (r.right < MIN_RIGHT)
		r.right = MIN_RIGHT;
	r.top -= aWin->scrollPos * aWin->row_height;

#if 0
	/* If you enable this band of code (and disable the next band), you will get
	   fewer flickers but a slower performance while drawing the dot line. */
	{	RgnHandle dotl_rgn = NewRgn();
		Rect dotl;
		dotl.left	= r.left;
		dotl.right	= r.right;
		dotl.bottom = r.top + aWin->save_lin * aWin->row_height;
		dotl.top	= dotl.bottom - 1;
		FillRect(&dotl, &qd.gray);
		RectRgn(dotl_rgn, &dotl);
		DiffRgn(clip, dotl_rgn, clip);
		DisposeRgn(dotl_rgn);
		SetClip(clip);
	}
#endif

	if (in_topl_mode()) {
		RgnHandle topl_rgn = NewRgn();
		Rect topl_r = r;
		topl_r.top += (aWin->y_size - 1) * aWin->row_height;
		l = (*top_line)->destRect.right - (*top_line)->destRect.left;
		(*top_line)->viewRect = topl_r;
		(*top_line)->destRect = topl_r;
		if (l != topl_r.right - topl_r.left)
			TECalText(top_line);
		TEUpdate(&topl_r, top_line);
		RectRgn(topl_rgn, &topl_r);
		DiffRgn(clip, topl_rgn, clip);
		DisposeRgn(topl_rgn);
		SetClip(clip);
	}

	DisposeRgn(clip);

	TextFont (aWin->font_number);
	TextSize (aWin->font_size);
	HLock (aWin->windowText);
	TETextBox (*aWin->windowText, aWin->windowTextLen, &r, teJustLeft);
	HUnlock (aWin->windowText);

#if 1
	r.bottom = r.top + aWin->save_lin * aWin->row_height;
	r.top	 = r.bottom - 1;
	FillRect(&r, (void *) &qd.gray);
#endif

	SetClip(org_clip);
	DisposeRgn(org_clip);
	return 0;
}


static short 
macUpdateMenu (EventRecord *theEvent, WindowPtr theWindow) {
	NhWindow *aWin = GetNhWin (theWindow);
	int i, line;
	MacMHMenuItem *mi;
	
	GeneralUpdate (theEvent, theWindow);
	HLock ((char**)aWin->menuInfo);
	HLock ((char**)aWin->menuSelected);
	for (i = 0; i < aWin->miSelLen; i++) {
		mi = &(*aWin->menuInfo) [(*aWin->menuSelected) [i]];
		line = mi->line;
		if (line > aWin->scrollPos && line <= aWin->y_size)
			ToggleMenuSelect (aWin, line - aWin->scrollPos);
	}
	HUnlock ((char**)aWin->menuInfo);
	HUnlock ((char**)aWin->menuSelected);
	return 0;
}


static short
GeneralUpdate (EventRecord *theEvent, WindowPtr theWindow) {
	Rect r = theWindow->portRect;
	Rect r2 = r;
	NhWindow *aWin = GetNhWin (theWindow);
	RgnHandle h;
	Boolean vis;

	if (! theEvent) {
		Debugger ();
	}

	r2.left = r2.right - SBARWIDTH;
	r2.right += 1;
	r2.top -= 1;
	vis = (r2.bottom > r2.top + 50);

	draw_growicon_vert_only(theWindow);
	DrawControls (theWindow);

	h = (RgnHandle) 0;
	if (vis && (h = NewRgn ())) {
		RgnHandle tmp = NewRgn ();
		if (!tmp) {
			DisposeRgn (h);
			h = (RgnHandle) 0;
		} else {
			GetClip (h);
			RectRgn (tmp, &r2);
			DiffRgn (h, tmp, tmp);
			SetClip (tmp);
			DisposeRgn (tmp);
		}
	}
	if (r.right < MIN_RIGHT)
		r.right = MIN_RIGHT;
	r.top -= aWin->scrollPos * aWin->row_height;
	r.right -= SBARWIDTH;
	HLock (aWin->windowText);
	TETextBox (*aWin->windowText, aWin->windowTextLen, &r, teJustLeft);
	HUnlock (aWin->windowText);
	if (h) {
		SetClip (h);
		DisposeRgn (h);
	}
	return 0;
}


static void
macCursorTerm (EventRecord *theEvent, WindowPtr theWindow, RgnHandle mouseRgn) {
	char *dir_bas, *dir;
	CursHandle ch;
	GrafPtr gp;
	NhWindow *nhw = GetNhWin (theWindow);
	Rect r = {0, 0, 1, 1};

	GetPort (&gp);
	SetPort (theWindow);

	if (cursor_locked)
		dir = (char *)0;
	else {
		Point where = theEvent->where;

		GlobalToLocal (&where);
		dir_bas = iflags.num_pad ? (char *) ndir : (char *) sdir;
		dir = strchr (dir_bas, click_to_cmd (where.h / nhw->char_width + 1 ,
							where.v / nhw->row_height, CLICK_1));
	}
	ch = GetCursor (dir ? dir - dir_bas + 513 : 512);
	if (ch) {
		HLock ((Handle) ch);
		SetCursor (*ch);
		HUnlock ((Handle) ch);

	} else {
		SetCursor(&qd.arrow);
	}
	OffsetRect (&r, theEvent->where.h, theEvent->where.v);
	RectRgn (mouseRgn, &r);
	SetPort (gp);
}


static void
GeneralCursor (EventRecord *theEvent, WindowPtr theWindow, RgnHandle mouseRgn) {
#if defined(applec) || defined(__MWERKS__)
# pragma unused(theWindow)
#endif
	Rect r = {-1, -1, 2, 2};

	SetCursor(&qd.arrow);
	OffsetRect (&r, theEvent->where.h, theEvent->where.v);
	RectRgn (mouseRgn, &r);
}


static void
HandleKey (EventRecord *theEvent) {
	WindowPtr theWindow = FrontWindow ();

	if (theEvent->modifiers & cmdKey) {
		if (theEvent->message & 0xff == '.') {
			/* Flush key queue */
			keyQueueCount = keyQueueWrite = keyQueueRead = 0;
			theEvent->message = '\033';
			goto dispatchKey;
		} else {
			UndimMenuBar ();
			DoMenuEvt (MenuKey (theEvent->message & 0xff));
		}
	} else {

dispatchKey :
		if (theWindow) {
			int kind = ((WindowPeek)theWindow)->windowKind - WIN_BASE_KIND;
			winKeyFuncs [kind] (theEvent, theWindow);
		} else {
			GeneralKey (theEvent, (WindowPtr) 0);
		}
	}
}


static void
WindowGoAway (EventRecord *theEvent, WindowPtr theWindow) {
	NhWindow *aWin = GetNhWin(theWindow);

	if (! theEvent || TrackGoAway (theWindow, theEvent->where)) {
		if (aWin - theWindows == BASE_WINDOW && ! iflags.window_inited) {
			AddToKeyQueue ('\033', 1);
			return;
		} else {
			HideWindow (theWindow);
		}
		if (inSelect == WIN_ERR || aWin - theWindows != inSelect) {
			mac_destroy_nhwindow (aWin - theWindows);
		} else {
			AddToKeyQueue (CHAR_CR, 1);
		}
	}
}


static void
HandleClick (EventRecord *theEvent) {
	int code;
	unsigned long l;
	WindowPtr theWindow;
	NhWindow *aWin;
	Rect r = (*GetGrayRgn ())->rgnBBox;

	InsetRect (&r, 4, 4);

	code = FindWindow (theEvent->where, &theWindow);
	aWin = GetNhWin (theWindow);
	
	switch (code) {
	case inContent :
		if (inSelect == WIN_ERR || aWin - theWindows == inSelect) {
			int kind = ((WindowPeek)theWindow)->windowKind - WIN_BASE_KIND;
			winCursorFuncs [kind] (theEvent, theWindow, gMouseRgn);
			SelectWindow (theWindow);
			SetPort (theWindow);
			winClickFuncs [kind] (theEvent, theWindow);
		} else {
			nhbell ();
		}
		break;

	case inDrag :
		if (inSelect == WIN_ERR || aWin - theWindows == inSelect) {
			SetCursor(&qd.arrow);
			DragWindow (theWindow, theEvent->where, &r);
			SaveWindowPos (theWindow);
		} else {
			nhbell ();
		}
		break;

	case inGrow :
		if (inSelect == WIN_ERR || aWin - theWindows == inSelect) {
			SetCursor(&qd.arrow);
			SetRect (&r, 80, 2 * aWin->row_height + 1, r.right, r.bottom);
			if (aWin == theWindows + WIN_MESSAGE)
				r.top += SBARHEIGHT;
			l = GrowWindow (theWindow, theEvent->where, &r);
			SizeWindow (theWindow, l & 0xffff, l >> 16, FALSE);
			SaveWindowSize (theWindow);
			SetPort (theWindow);
			InvalRect (&(theWindow->portRect));
			if (aWin->scrollBar) {
				DrawScrollbar (aWin, theWindow);
			}
		} else {
			nhbell ();
		}
		break;

	case inGoAway :
		WindowGoAway(theEvent, theWindow);
		break;

	case inMenuBar :
		UndimMenuBar();
		DoMenuEvt (MenuSelect (theEvent->where));
		break;

	case inSysWindow :
		SystemClick(theEvent, theWindow);
	default :
		break;
	}
}


static void
HandleUpdate (EventRecord *theEvent) {
	WindowPtr theWindow = (WindowPtr) theEvent->message;
	NhWindow *aWin = GetNhWin (theWindow);

	char existing_update_region = FALSE;
	Rect rect;
	
	if (theWindow == _mt_window) {
		existing_update_region = (get_invalid_region (theWindow, &rect) == noErr);
	}
	BeginUpdate (theWindow);
	SetPort (theWindow);
	EraseRect (&(theWindow->portRect));
	winUpdateFuncs [((WindowPeek)theWindow)->windowKind - WIN_BASE_KIND] 
				(theEvent, theWindow);

	if (theWindow == _mt_window && existing_update_region) {
		set_invalid_region (theWindow, &rect);
	}
	aWin->drawn = TRUE;
	EndUpdate (theWindow);
}


static void
DoOsEvt (EventRecord *theEvent) {
	WindowPtr wp;
	short code;

	if ((theEvent->message & 0xff000000) == 0xfa000000) {
		/* Mouse Moved */

		code = FindWindow (theEvent->where, &wp);
		if (code != inContent) {
			Rect r = {-1, -1, 2, 2};

			SetCursor(&qd.arrow);
			OffsetRect (&r, theEvent->where.h, theEvent->where.v);
			RectRgn (gMouseRgn, &r);

		} else {
			int kind = ((WindowPeek)wp)->windowKind - WIN_BASE_KIND;
			if (kind >= 0 && kind <= NHW_TEXT) {
				winCursorFuncs [kind] (theEvent, wp, gMouseRgn);
			}
		}
	} else {
		/* Suspend/resume */
		if (((theEvent->message & osEvtMessageMask) >> 24) == suspendResumeMessage)
			kApplicInFront = (theEvent->message & resumeFlag);
	}
}


void
HandleEvent (EventRecord *theEvent) {
	switch (theEvent->what) {
	case autoKey :
	case keyDown :
		HandleKey (theEvent);
		break;
	case updateEvt :
		HandleUpdate (theEvent);
		break;
	case mouseDown :
		HandleClick (theEvent);
		break;
	case diskEvt :
		if ((theEvent->message & 0xffff0000) != 0) {
			Point p = {150, 150};
			(void) DIBadMount (p, theEvent->message);
		}
		break;
	case osEvt :
		DoOsEvt (theEvent);
		break;
	case kHighLevelEvent:
		AEProcessAppleEvent(theEvent);
	default :
		break;
	}
}


void
mac_get_nh_event(void) {
	EventRecord anEvent;

	/* KMH -- Don't proceed if the window system isn't set up */
	if (!iflags.window_inited)
		return;

	(void) WaitNextEvent (everyEvent, &anEvent, 0, gMouseRgn);
	HandleEvent (&anEvent);
}


int
mac_nhgetch(void) {
	int ch;
	long doDawdle;
	EventRecord anEvent;

	/* We want to take care of keys in the buffer as fast as
	 * possible
	 */
	if (keyQueueCount)
		doDawdle = 0L;
	else {
		long total, contig;
		static char warn = 0;

		doDawdle = (in_topl_mode() ? GetCaretTime () : 120L);
		/* Since we have time, check memory */
		PurgeSpace (&total, &contig);
		if (contig < 25000L || total < 50000L) {
			if (!warn) {
				pline ("Low Memory!");
				warn = 1;
			}
		} else {
			warn = 0;
		}
	}

	do {
		(void) WaitNextEvent (everyEvent, &anEvent, doDawdle, gMouseRgn);
		HandleEvent (&anEvent);
		ch = GetFromKeyQueue ();
	} while (!ch && !gClickedToMove);

	if (!gClickedToMove)
		ObscureCursor ();
	else
		gClickedToMove = 0;

#ifdef THINK_C
	if (ch == '\r') ch = '\n';
#endif

	return ch;
}


void
mac_delay_output(void) {
	long destTicks = TickCount () + 1;

	while (TickCount () < destTicks) {
		mac_get_nh_event ();
	}
}


#ifdef CLIPPING
static void
mac_cliparound (int x, int y) {
#if defined(applec) || defined(__MWERKS__)
# pragma unused(x,y)
#endif
	/* TODO */
}
#endif

void
mac_exit_nhwindows (const char *s) {
	clear_screen ();
	tty_exit_nhwindows (s);
	mac_destroy_nhwindow (WIN_MESSAGE);
	mac_destroy_nhwindow (WIN_INVEN);
}


/*
 * Don't forget to decrease in_putstr before returning...
 */
void
mac_putstr (winid win, int attr, const char *str) {
	long len, slen;
	NhWindow *aWin = &theWindows [win];
	static char in_putstr = 0;
	short newWidth, maxWidth;
	Rect r;
	char *src, *sline, *dst, ch;

	if (win < 0 || win >= NUM_MACWINDOWS || ! aWin->its_window) {
		error ("putstr: Invalid win %d (Max %d).", win, NUM_MACWINDOWS, attr);
		return;
	}

	if (aWin->its_window == _mt_window) {
		tty_putstr(win, attr, str);
		return;
	}

	if (in_putstr > 3)
		return;

	in_putstr ++;
	slen = strlen (str);

	SetPort (aWin->its_window);
	r = aWin->its_window->portRect;
	if (win == WIN_MESSAGE) {
		r.right  -= SBARWIDTH;
		r.bottom -= SBARHEIGHT;
		if (flags.page_wait && 
			aWin->last_more_lin <= aWin->y_size - (r.bottom - r.top) / aWin->row_height) {
			aWin->last_more_lin = aWin->y_size;
			mac_display_nhwindow(win, TRUE);
		}
	}

	/*
	 * A "default" text window - uses TETextBox
	 * We just add the text, without attributes for now
	 */
	len = GetHandleSize (aWin->windowText);
	while (aWin->windowTextLen + slen + 1 > len) {
		len = (len > 2048) ? (len + 2048) : (len * 2);
		SetHandleSize (aWin->windowText, len);
		if (MemError ()) {
			error ("putstr: SetHandleSize");
			aWin->windowTextLen = 0L;
			aWin->save_lin = 0;
			aWin->y_curs = 0;
			aWin->y_size = 0;
		}
	}
	
	len = aWin->windowTextLen;
	dst = *(aWin->windowText) + len;
	sline = src = (char *)str;
	maxWidth = newWidth = 0;
	for (ch = *src; ch; ch = *src) {
		if (ch == CHAR_LF)
			ch = CHAR_CR;
		*dst++ = ch;
		if (ch == CHAR_CR) {
			aWin->y_curs ++;
			aWin->y_size ++;
			aWin->x_curs = 0;
			newWidth = TextWidth (sline, 0, src - sline);
			if (newWidth > maxWidth) {
				maxWidth = newWidth;
			}
			sline = src+1;	/* keep track of where new line begins */
		}
		else
			aWin->x_curs ++;
		src++;
	}

	newWidth = TextWidth (sline, 0, src - sline);
	if (newWidth > maxWidth) {
		maxWidth = newWidth;
	}

	aWin->windowTextLen += slen;
	
	if (ch != CHAR_CR) {
		(*(aWin->windowText)) [len + slen] = CHAR_CR;
		aWin->windowTextLen ++;
		aWin->y_curs ++;
		aWin->y_size ++;
		aWin->x_curs = 0;
	}
	
	if (win == WIN_MESSAGE) {
		short min = aWin->y_size - (r.bottom - r.top) / aWin->row_height;
		if (aWin->scrollPos < min) {
			aWin->scrollPos = min;
			SetControlMaximum (aWin->scrollBar, aWin->y_size);
			SetControlValue(aWin->scrollBar, min);
		}
		InvalRect (&r);
	}
	else	/* Message has a fixed width, other windows base on content */
		if (maxWidth > aWin->x_size)
			aWin->x_size = maxWidth;
	in_putstr --;
}


void
mac_curs (winid win, int x, int y) {
	NhWindow *aWin = &theWindows [win];

	if (aWin->its_window == _mt_window) {
		tty_curs(win, x, y);
		return;
	}

	SetPort (aWin->its_window);
	MoveTo (x * aWin->char_width, (y * aWin->row_height) + aWin->ascent_height);
	aWin->x_curs = x;
	aWin->y_curs = y;
}


int
mac_nh_poskey (int *a, int *b, int *c) {
	int ch = mac_nhgetch();
	*a = clicked_pos.h;
	*b = clicked_pos.v;
	*c = clicked_mod;
	return ch;
}


void
mac_start_menu (winid win) {
	HideWindow (theWindows [win].its_window);
	mac_clear_nhwindow (win);
}


void
mac_add_menu (winid win, int glyph, const anything *any, CHAR_P menuChar, CHAR_P groupAcc, int attr, const char *inStr, int preselected) {
#if defined(applec) || defined(__MWERKS__)
# pragma unused(glyph)
#endif
	NhWindow *aWin = &theWindows [win];
	const char *str;
	char locStr[4+BUFSZ];
	MacMHMenuItem *item;

	if (!inStr) return;

	if (any->a_void != 0) {

#define kMenuSizeBump 26
		if (!aWin->miSize) {
			aWin->menuInfo = (MacMHMenuItem **)NewHandle(sizeof(MacMHMenuItem) * kMenuSizeBump);
			if (!aWin->menuInfo) {
				error("Can't alloc menu handle");
				return;
			}
			aWin->menuSelected = (short **)NewHandle(sizeof(short) * kMenuSizeBump);
			if (!aWin->menuSelected) {
				error("Can't alloc menu select handle");
				return;
			}
			aWin->miSize = kMenuSizeBump;
		}

		if (aWin->miLen + 1 >= aWin->miSize) {
			SetHandleSize((Handle)aWin->menuInfo, sizeof(MacMHMenuItem) * (aWin->miLen+kMenuSizeBump));
			if (MemError()) {
				error("Can't resize menu handle");
				return;
			}
			SetHandleSize((Handle)aWin->menuSelected, sizeof(short) * (aWin->miLen+kMenuSizeBump));
			if (MemError()) {
				error("Can't resize menu select handle");
				return;
			}
			aWin->miSize += kMenuSizeBump;
		}

		if (menuChar == 0) {	
			if (('a' <= aWin->menuChar && aWin->menuChar <= 'z') ||
				('A' <= aWin->menuChar && aWin->menuChar <= 'Z')) {
				menuChar = aWin->menuChar++;
				if (menuChar == 'z')
					aWin->menuChar = 'A';
			}
		}
		
		Sprintf(locStr, "%c - %s", (menuChar ? menuChar : ' '), inStr);
		str = locStr;
		HLock ((char**)aWin->menuInfo);
		HLock ((char**)aWin->menuSelected);
		(*aWin->menuSelected)[aWin->miLen] = preselected;
		item = &(*aWin->menuInfo)[aWin->miLen];
		aWin->miLen++;
		item->id = *any;
		item->accelerator = menuChar;
		item->groupAcc = groupAcc;
		item->line = aWin->y_size;
		HUnlock ((char**)aWin->menuInfo);
		HUnlock ((char**)aWin->menuSelected);
	} else
		str = inStr;

	putstr (win, attr, str);
}


/*
 * End a menu in this window, window must a type NHW_MENU.
 * str is a list of cancel characters (values that may be input)
 * morestr is a prompt to display, rather than the default.
 * str and morestr might be ignored by some ports.
 */
void
mac_end_menu (winid win, const char *morestr) {
	unsigned char buf [256];
	int len;
	NhWindow *aWin = &theWindows [win];

	buf [0] = 0;
	if (morestr) {
		len = strlen (morestr);
		len = (len > 255) ? 255 : len;
		strncpy ((char *)&buf [1], morestr, len);
	}
	SetWTitle (aWin->its_window, buf);
}


int
mac_select_menu (winid win, int how, menu_item **selected_list) {
	int c;
	NhWindow *aWin = &theWindows [win];
	WindowPtr theWin = aWin->its_window;

	inSelect = win;

	mac_display_nhwindow (win, FALSE);

	aWin->how = (short) how;
	for (;;) {
		c = map_menu_cmd (mac_nhgetch());
		if (c == CHAR_ESC) {
			/* deselect everything */
			aWin->miSelLen = 0;
			break;
		} else if (ClosingWindowChar(c)) {
			break;
		} else {
			nhbell();
		}
	}

	HideWindow (theWin);

	if (aWin->miSelLen) {
		menu_item *mp;
		MacMHMenuItem *mi;
		*selected_list = mp = (menu_item *) alloc(aWin->miSelLen * sizeof(menu_item));
		HLock ((char**)aWin->menuInfo);
		HLock ((char**)aWin->menuSelected);
		for (c = 0; c < aWin->miSelLen; c++) {
			mi = &(*aWin->menuInfo)[(*aWin->menuSelected) [c]];
			mp->item = mi->id;
			mp->count = -1L;
			mp++;
		}
		HUnlock ((char**)aWin->menuInfo);
		HUnlock ((char**)aWin->menuSelected);
	} else
		*selected_list = 0;

	inSelect = WIN_ERR;

	return aWin->miSelLen;
}

#include "dlb.h"

static void
mac_display_file (name, complain)
const char *name;	/* not ANSI prototype because of boolean parameter */
boolean complain;
{
	Ptr buf;
	int win;
	dlb *fp = dlb_fopen(name, "r");
	
	if (fp) {
		long l = dlb_fseek(fp, 0, SEEK_END);
		(void) dlb_fseek(fp, 0, 0L);
		buf = NewPtr(l+1);
		if (buf) {
			l = dlb_fread(buf, 1, l, fp);
			if (l > 0) {
				buf[l] = '\0';
				win = create_nhwindow(NHW_TEXT);
				if (WIN_ERR == win) {
					if (complain) error ("Cannot make window.");
				} else {
					putstr(win, 0, buf);
					display_nhwindow(win, FALSE);
				}
			}
			DisposePtr(buf);
		}
		dlb_fclose(fp);
	} else if (complain)
		error("Cannot open %s.", name);
}


void
port_help () {
	display_file (PORT_HELP, TRUE);
}


static void
mac_unimplemented (void) {
}


static void
mac_suspend_nhwindows (const char *foo) {
#if defined(applec) || defined(__MWERKS__)
# pragma unused(foo)
#endif
	/*	Can't really do that :-)		*/
}


int
try_key_queue (char *bufp) {
	if (keyQueueCount) {
		char ch;
		for (ch = GetFromKeyQueue(); ; ch = GetFromKeyQueue()) {
			if (ch == CHAR_LF || ch == CHAR_CR)
				ch = 0;
			*bufp++ = ch;
			if (ch == 0)
				break;
		}
		return 1;
	}
	return 0;
}

/* Interface definition, for windows.c */
struct window_procs mac_procs = {
	"mac",
	mac_init_nhwindows,
	mac_unimplemented,	/* see macmenu.c:mac_askname() for player selection */
	mac_askname,
	mac_get_nh_event,
	mac_exit_nhwindows,
	mac_suspend_nhwindows,
	mac_unimplemented,
	mac_create_nhwindow,
	mac_clear_nhwindow,
	mac_display_nhwindow,
	mac_destroy_nhwindow,
	mac_curs,
	mac_putstr,
	mac_display_file,
	mac_start_menu,
	mac_add_menu,
	mac_end_menu,
	mac_select_menu,
	genl_message_menu,
	mac_unimplemented,
	mac_get_nh_event,
	mac_get_nh_event,
#ifdef CLIPPING
	mac_cliparound,
#endif
#ifdef POSITIONBAR
	donull,
#endif
	tty_print_glyph,
	tty_raw_print,
	tty_raw_print_bold,
	mac_nhgetch,
	mac_nh_poskey,
	tty_nhbell,
	mac_doprev_message,
	mac_yn_function,
	mac_getlin,
	mac_get_ext_cmd,
	mac_number_pad,
	mac_delay_output,
#ifdef CHANGE_COLOR
	tty_change_color,
	tty_change_background,
	set_tty_font_name,
	tty_get_color_string,
#endif
/* other defs that really should go away (they're tty specific) */
	0, //    mac_start_screen,
	0, //    mac_end_screen,
	genl_outrip,
};

/*macwin.c*/
