CC = gcc
YACC ?= yacc
LEX ?= flex
INSTALL ?= install
MV ?= mv
TOUCH ?= touch
CSCOPE ?= cscope

PREFIX ?= /usr/local
GAMEDIR ?= $(PREFIX)/nethack

-include config.mk

SUBDIRS := include util sys/share sys/unix win/tty win/curses win/share src dat

CSCOPE_FILES := cscope.out cscope.po.out cscope.in.out

TOPDIR := $(PWD)
INCDIR := $(TOPDIR)/include
SRCDIR := $(TOPDIR)/src

CPPFLAGS += -I$(INCDIR) -D_POSIX_C_SOURCE
CFLAGS += -fPIC -Werror -Wall -Wno-format-security -std=c99

CLEAN_TARGETS = $(SUBDIRS:=/clean)
DEPCLEAN_TARGETS = $(SUBDIRS:=/depclean)
ALL_TARGETS = $(SUBDIRS:=/all)

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
	$(TOUCH) -c $(GAMEDIR)/var/{bones*,?lock*,wizard*,save/*}

install: all
	$(INSTALL) -d $(GAMEDIR) $(GAMEDIR)/var/save
	$(INSTALL) -m 0644 $(DAT_INSTALL_OBJECTS) $(GAMEDIR)
	$(INSTALL) -m 2755 $(SRCDIR)/nethack $(GAMEDIR)
	$(INSTALL) -T $(RECOVER) $(GAMEDIR)/recover
	$(TOUCH) $(GAMEDIR)/var/{perm,record,logfile,xlogfile}

%.exe:
	$(CC) $(LDFLAGS) -o $@ $(EXE_OBJECTS)

%.d: %.c
	$(CC) -MM $(CPPFLAGS) -MQ $(@:.d=.o) -MQ $@ -MF $*.d $<

%.o: %.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) -o $@ $<

# vim:tw=80
