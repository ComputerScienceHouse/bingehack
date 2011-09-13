INCLUDE_INSTALLED_HEADERS := flag.h you.h decl.h quest.h spell.h dgn_file.h rm.h dungeon.h sp_lev.h monst.h trap.h objclass.h
INCLUDE_INSTALLED_HEADERS := $(INCLUDE_INSTALLED_HEADERS:%=$(CURDIR)/%)
INCLUDE_GENERATED_HEADERS := date.h onames.h pm.h vis_tab.h
INCLUDE_GENERATED_HEADERS := $(INCLUDE_GENERATED_HEADERS:%=$(CURDIR)/%)
