
INCLUDES = @GTK_CFLAGS@ @GNOME_INCLUDEDIR@ -DHACKDIR=\"@HACKLIBDIR@\"

noinst_LIBRARIES = libwingnome.a

nethackdir = @HACKLIBDIR@
nethack_DATA = mapbg.xpm

Gamesdir = @DESKTOP_DIR@
Games_DATA = gnomehack.desktop

EXTRA_DIST = $(Games_DATA)

libwingnome_a_SOURCES = \
	GnomeHackAskStringDialog.c GnomeHackAskStringDialog.h \
	GnomeHackBind.c GnomeHackBind.h \
	GnomeHackGlyph.c GnomeHackGlyph.h \
	GnomeHackMainWindow.c GnomeHackMainWindow.h \
	GnomeHackMapWindow.c GnomeHackMapWindow.h \
	GnomeHackMessageWindow.c GnomeHackMessageWindow.h \
	GnomeHackMenuWindow.c GnomeHackMenuWindow.h \
	GnomeHackPlayerSelDialog.c GnomeHackPlayerSelDialog.h \
	GnomeHackSignals.c GnomeHackSignals.h \
	GnomeHackStatusWindow.c GnomeHackStatusWindow.h \
	GnomeHackTextWindow.c GnomeHackTextWindow.h \
	GnomeHackYesNoDialog.c GnomeHackYesNoDialog.h \
	GnomeHackSettings.c GnomeHackSettings.h \
	GnomeHack.h xpms.h mapbg.xpm


