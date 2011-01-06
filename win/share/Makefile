.PHONY: win/share/all win/share/clean win/share/depclean

win/share/all: $(WIN_SHARE_OBJECTS)

win/share/clean:
	$(RM) $(WIN_SHARE_OBJECTS)

win/share/depclean:
	$(RM) $(WIN_SHARE_DEPS)

$(WIN_SHARE_DEPS): $(INCLUDE_GENERATED_HEADERS)

-include $(WIN_SHARE_DEPS)
