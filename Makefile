PUMP := pump
CC := gcc
DEPGEN := gcc
YACC := bison
LEX := flex
INSTALL ?= install
MV ?= mv
TOUCH ?= touch
CSCOPE ?= cscope

NCURSESW_CONFIG ?= ncursesw5-config
MYSQL_CONFIG ?= mysql_config
PKG_CONFIG ?= pkg-config

PREFIX ?= /usr/local
GAMEDIR ?= $(PREFIX)/nethack
VARDIR ?= $(GAMEDIR)/var

-include config.mk

SUBDIRS := include util sys/share sys/unix win/tty win/curses win/share src dat

CSCOPE_FILES := cscope.out cscope.po.out cscope.in.out

TOPDIR := $(PWD)
INCDIR := $(TOPDIR)/include
SRCDIR := $(TOPDIR)/src

CPPFLAGS := $(CPPFLAGS) -I$(INCDIR) -D_GNU_SOURCE
CFLAGS := $(CFLAGS) -fPIC -Wall -Wno-format -Wnonnull -std=gnu99 -O2

UNAME := $(shell uname -s)
ifeq ($(UNAME), OpenBSD)
NCURSESW_LIBRARIES ?= -L/usr/lib -lncursesw
else
NCURSESW_CPPFLAGS ?= $(shell $(NCURSESW_CONFIG) --cflags)
NCURSESW_LIBRARIES ?= $(shell $(NCURSESW_CONFIG) --libs)
endif

ifeq ($(UNAME), Linux)
DYLD_LIBRARIES ?= -ldl
endif

LIBCONFIG_CPPFLAGS ?= $(shell $(PKG_CONFIG) --cflags libconfig)
LIBCONFIG_LIBRARIES ?= $(shell $(PKG_CONFIG) --libs libconfig)
MYSQL_CPPFLAGS ?= $(shell $(MYSQL_CONFIG) --cflags)
GLIB_CPPFLAGS ?= $(shell $(PKG_CONFIG) --cflags glib-2.0)
GLIB_LIBRARIES ?= $(shell $(PKG_CONFIG) --libs glib-2.0)

CPPFLAGS := $(CPPFLAGS) $(NCURSESW_CPPFLAGS) $(LIBCONFIG_CPPFLAGS) $(DYLD_CPPFLAGS) $(MYSQL_CPPFLAGS) $(GLIB_CPPFLAGS)
LIBRARIES := $(DYLD_LIBRARIES) $(LIBRARIES) $(NCURSESW_LIBRARIES) $(LIBCONFIG_LIBRARIES) $(GLIB_LIBRARIES)

CLEAN_TARGETS := $(SUBDIRS:=/clean)
DEPCLEAN_TARGETS := $(SUBDIRS:=/depclean)
ALL_TARGETS := $(SUBDIRS:=/all)

.PHONY: all clean depclean install update cscope pristine cscope-clean
.DEFAULT_GOAL: all

all: $(ALL_TARGETS)
clean: $(CLEAN_TARGETS)
depclean: clean $(DEPCLEAN_TARGETS)
pristine: depclean cscope-clean

cscope-clean:
	$(RM) $(CSCOPE_FILES)

cscope:
	$(CSCOPE) -R -b -q

distcc:
	DISTCC_FALLBACK=0 $(PUMP) $(MAKE) CC="distcc $(CC)" $(DISTCC_JOBS) all

# Define default hooks so a subdir doesn't need to define them.
$(CLEAN_TARGETS):
$(DEPCLEAN_TARGETS):

define variableRule
 CURDIR := $$(TOPDIR)/$$$(1)
 include $$(CURDIR)/variables.mk
endef
$(foreach subdir, $(SUBDIRS), $(eval $(call variableRule, $(subdir))))

# This defines the following for every dir in SUBDIRS:
#   Sets CURDIR to the $(TOPDIR)/$(dir)
#   Includes a makefile in $(CURDIR)/Makefile
define subdirRule
 CURDIR := $$(TOPDIR)/$$$(1)
 $$$(1)/all: CURDIR := $$(CURDIR)
 $$$(1)/clean: CURDIR := $$(CURDIR)
 $$$(1)/depclean: CURDIR := $$(CURDIR)
 include $$(CURDIR)/Makefile
endef
# This is what actually does the work.
# The "call" command replaces every $(1) variable reference in subdirRule with $(subdir)
# The "eval" command parses the result of the "call" command as make syntax
$(foreach subdir, $(SUBDIRS), $(eval $(call subdirRule, $(subdir))))
# Reset CURDIR back to what it should be.
CURDIR := $(TOPDIR)

update: all
	$(MV) $(GAMEDIR)/nethack $(GAMEDIR)/nethack.old
	$(MV) $(GAMEDIR)/quest.dat $(GAMEDIR)/quest.dat.old
	$(MAKE) install
	$(TOUCH) -c $(VARDIR)/{bones*,?lock*,wizard*,save/*}

install: all
	$(INSTALL) -d $(GAMEDIR) $(VARDIR)/save
	$(INSTALL) -m 0644 $(DAT_INSTALL_OBJECTS) $(GAMEDIR)
	$(INSTALL) -m 2755 $(SRCDIR)/nethack $(GAMEDIR)
ifneq ($(UNAME), OpenBSD)
	$(INSTALL) -T $(RECOVER) $(GAMEDIR)/recover
else
	$(INSTALL) $(RECOVER) $(GAMEDIR)/recover
endif
	$(TOUCH) $(VARDIR)/perm $(VARDIR)/record $(VARDIR)/logfile $(VARDIR)/xlogfile

%.exe:
	$(CC) $(LDFLAGS) -o $@ $(EXE_OBJECTS) $(EXE_LIBRARIES)

%.d: %.c
	$(DEPGEN) -MM $(CPPFLAGS) -MQ $(@:.d=.o) -MQ $@ -MF $*.d $<

%.o: %.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) -o $@ $<

# vim:tw=80
