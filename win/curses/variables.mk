WIN_CURSES_SOURCES := cursmain.c curswins.c cursmisc.c cursdial.c cursstat.c cursinit.c cursmesg.c
WIN_CURSES_SOURCES := $(WIN_CURSES_SOURCES:%=$(CURDIR)/%)

WIN_CURSES_OBJECTS := $(WIN_CURSES_SOURCES:.c=.o)
WIN_CURSES_DEPS := $(WIN_CURSES_SOURCES:.c=.d)

LIBRARIES += -lncursesw
