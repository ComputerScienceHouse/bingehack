.PHONY: sys/unix/all sys/unix/clean sys/unix/depclean

sys/unix/all: $(SYS_UNIX_OBJECTS)

sys/unix/clean:
	$(RM) $(SYS_UNIX_OBJECTS)

sys/unix/depclean:
	$(RM) $(SYS_UNIX_DEPS)

$(SYS_UNIX_DEPS): $(INCLUDE_GENERATED_HEADERS)

-include $(SYS_UNIX_DEPS)
