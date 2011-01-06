.PHONY: win/curses/all win/curses/clean win/curses/depclean

win/curses/all: $(WIN_CURSES_OBJECTS)

win/curses/clean:
	$(RM) $(WIN_CURSES_OBJECTS)

win/curses/depclean:
	$(RM) $(WIN_CURSES_DEPS)

$(WIN_CURSES_DEPS): $(INCLUDE_GENERATED_HEADERS)

-include $(WIN_CURSES_DEPS)
