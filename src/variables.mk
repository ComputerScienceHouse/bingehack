NETHACK := $(CURDIR)/nethack

GENERATED_SOURCES := monstr.c vis_tab.c
GENERATED_SOURCES := $(GENERATED_SOURCES:%=$(CURDIR)/%)

MONOBJ_SOURCES := monst.c objects.c
MONOBJ_SOURCES := $(MONOBJ_SOURCES:%=$(CURDIR)/%)
MONOBJ_OBJECTS := $(MONOBJ_SOURCES:.c=.o)
MONOBJ_DEPS := $(MONOBJ_SOURCES:.c=.d)

NAMING_SOURCES := drawing.c decl.c
NAMING_SOURCES := $(NAMING_SOURCES:%=$(CURDIR)/%)
NAMING_OBJECTS := $(NAMING_SOURCES:.c=.o)
NAMING_DEPS := $(NAMING_SOURCES:.c=.d)

CLEAN_SOURCES := allmain.c alloc.c apply.c artifact.c attrib.c ball.c \
	bones.c botl.c cmd.c dbridge.c detect.c dig.c display.c \
	do.c do_name.c do_wear.c dog.c dogmove.c dokick.c dothrow.c \
	dungeon.c eat.c end.c engrave.c exper.c explode.c \
	extralev.c files.c fountain.c hack.c hacklib.c invent.c light.c \
	lock.c mail.c makemon.c mapglyph.c mcastu.c mhitm.c mhitu.c \
	minion.c mklev.c mkmap.c \
	mkmaze.c mkobj.c mkroom.c mon.c mondata.c monmove.c monstr.c \
	mplayer.c mthrowu.c muse.c music.c o_init.c objnam.c options.c \
	pager.c pickup.c pline.c polyself.c potion.c pray.c priest.c \
	quest.c questpgr.c read.c rect.c region.c restore.c rip.c rnd.c \
	role.c rumors.c save.c shk.c shknam.c sit.c sounds.c sp_lev.c spell.c \
	steal.c steed.c teleport.c timeout.c topten.c track.c trap.c u_init.c \
	uhitm.c vault.c vision.c vis_tab.c weapon.c were.c wield.c windows.c \
	wizard.c worm.c worn.c write.c zap.c version.c
CLEAN_SOURCES := $(CLEAN_SOURCES:%=$(CURDIR)/%)
CLEAN_OBJECTS := $(CLEAN_SOURCES:.c=.o)
CLEAN_DEPS := $(CLEAN_SOURCES:.c=.d)
