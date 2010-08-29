DAT_DATA_SOURCES := $(CURDIR)/data.base
DAT_DATA_OBJECTS := $(DAT_DATA_SOURCES:.base=)

DAT_ORACLES_SOURCES := $(CURDIR)/oracles.txt
DAT_ORACLES_OBJECTS := $(DAT_ORACLES_SOURCES:.txt=)

DAT_OPTIONS_OBJECTS := $(CURDIR)/options

DAT_QUEST_SOURCES := $(CURDIR)/quest.txt
DAT_QUEST_OBJECTS := $(DAT_QUEST_SOURCES:.txt=.dat)

DAT_RUMORS_SOURCES := rumors.tru rumors.fal rumors.pot
DAT_RUMORS_SOURCES := $(DAT_RUMORS_SOURCES:%=$(CURDIR)/%)
DAT_RUMORS_OBJECTS := $(CURDIR)/rumors

DAT_DUNGEON_SOURCES := $(CURDIR)/dungeon.def
DAT_DUNGEON_GENERATED := $(DAT_DUNGEON_SOURCES:.def=.pdf)
DAT_DUNGEON_OBJECTS := $(DAT_DUNGEON_SOURCES:.def=)

DAT_SPEC_SOURCES := bigroom.des castle.des endgame.des gehennom.des knox.des medusa.des mines.des Potter.des sokoban.des tower.des yendor.des
DAT_SPEC_SOURCES := $(DAT_SPEC_SOURCES:%=$(CURDIR)/%)
DAT_SPEC_OBJECTS := $(DAT_SPEC_SOURCES:.des=.lev)

DAT_QUEST_LEVEL_SOURCES := Arch.des Barb.des Caveman.des Healer.des Knight.des Monk.des Priest.des Ranger.des Rogue.des Samurai.des Tourist.des Valkyrie.des Wizard.des
DAT_QUEST_LEVEL_SOURCES := $(DAT_QUEST_LEVEL_SOURCES:%=$(CURDIR)/%)
DAT_QUEST_LEVEL_OBJECTS := $(DAT_QUEST_LEVEL_SOURCES:.des=.lev)

DAT_INSTALL_OBJECTS := help hh cmdhelp history opthelp wizhelp asmodeus.lev baalz.lev bigrm-?.lev castle-?.lev cthulhu.lev fakewiz?.lev juiblex.lev knox.lev lilith.lev lolth.lev mammon.lev medusa-?.lev minend-?.lev minefill.lev minetn-?.lev Potter.lev orcus.lev sammael.lev sanctum.lev soko?-?.lev tower?.lev valley.lev wizard?.lev astral.lev air.lev earth.lev fire.lev water.lev ???-goal.lev ???-fil?.lev ???-loca.lev ???-strt.lev license
DAT_INSTALL_OBJECTS := $(DAT_INSTALL_OBJECTS:%=$(CURDIR)/%)
DAT_INSTALL_OBJECTS += $(DAT_DATA_OBJECTS) $(DAT_ORACLES_OBJECTS) $(DAT_OPTIONS_OBJECTS) $(DAT_QUEST_OBJECTS) $(DAT_RUMORS_OBJECTS) $(DAT_DUNGEON_OBJECTS)
