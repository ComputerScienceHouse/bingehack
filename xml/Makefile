RNCFILES = dm-message.rnc

RNGFILES = $(RNCFILES:.rnc=.rng)

TRANG = trang
XMLLINT = xmllint

.PHONY: all clean test

all: test

clean:
	$(RM) $(RNGFILES)

test: $(RNGFILES)
	$(XMLLINT) --relaxng $(RNGFILES) --noout *.xml

%.rng: %.rnc
	$(TRANG) $< $@
