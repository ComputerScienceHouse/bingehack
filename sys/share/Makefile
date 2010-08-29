.PHONY: sys/share/all sys/share/clean sys/share/depclean

sys/share/all: $(SYS_SHARE_OBJECTS)

sys/share/clean:
	$(RM) $(SYS_SHARE_OBJECTS)

sys/share/depclean:
	$(RM) $(SYS_SHARE_DEPS)

$(SYS_SHARE_DEPS): $(INCLUDE_GENERATED_HEADERS)

-include $(SYS_SHARE_DEPS)
