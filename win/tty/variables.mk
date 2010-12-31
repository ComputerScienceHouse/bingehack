WIN_TTY_SOURCES := getline.c termcap.c topl.c wintty.c unicode.c
WIN_TTY_SOURCES := $(WIN_TTY_SOURCES:%=$(CURDIR)/%)

WIN_TTY_OBJECTS := $(WIN_TTY_SOURCES:.c=.o)
WIN_TTY_DEPS := $(WIN_TTY_SOURCES:.c=.d)
