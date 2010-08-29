.PHONY: win/tty/all win/tty/clean win/tty/depclean

win/tty/all: $(WIN_TTY_OBJECTS)

win/tty/clean:
	$(RM) $(WIN_TTY_OBJECTS)

win/tty/depclean:
	$(RM) $(WIN_TTY_DEPS)

$(WIN_TTY_OBJECTS): $(WIN_TTY_SOURCES)
$(WIN_TTY_DEPS): $(INCLUDE_GENERATED_HEADERS)

-include $(WIN_TTY_DEPS)
