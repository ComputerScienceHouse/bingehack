.PHONY: src/all src/clean src/depclean

src/all: $(NETHACK)

src/clean:
	$(RM) $(CLEAN_OBJECTS) $(NAMING_OBJECTS) $(NETHACK)

src/depclean:
	$(RM) $(CLEAN_DEPS) $(MONOBJ_DEPS) $(NAMING_DEPS) $(MONOBJ_OBJECTS)

$(CLEAN_DEPS) $(NAMING_DEPS): $(INCLUDE_GENERATED_HEADERS)

# TODO: This is wrong. It is not a multi-output target.
$(CURDIR)/vis_tab.c $(INCDIR)/vis_tab.h: $(MAKEDEFS)
	$(MAKEDEFS) -z

$(CURDIR)/monstr.c: $(MAKEDEFS)
	$(MAKEDEFS) -m

-include $(MONOBJ_DEPS) $(CLEAN_DEPS) $(NAMING_DEPS)

$(NETHACK): $(MONOBJ_OBJECTS) $(CLEAN_OBJECTS) $(NAMING_OBJECTS) $(SYS_SHARE_OBJECTS) $(SYS_UNIX_OBJECTS) $(WIN_TTY_OBJECTS)
	$(CC) $(LIBRARIES) $(LDFLAGS) -o $@ $^
