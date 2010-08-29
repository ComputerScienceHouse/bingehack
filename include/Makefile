.PHONY: include/depclean include/all

include/all: $(INCLUDE_GENERATED_HEADERS)

include/depclean:
	$(RM) $(INCLUDE_GENERATED_HEADERS)

$(CURDIR)/onames.h: $(MAKEDEFS)
	$(MAKEDEFS) -o

$(CURDIR)/pm.h: $(MAKEDEFS)
	$(MAKEDEFS) -p
