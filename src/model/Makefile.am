AM_CXXFLAGS = \
	-Wall -std=c++0x

noinst_LIBRARIES = libmodel.a

libmodel_a_SOURCES = \
	achermann-tunable.cc \
	achermann-tunable.hh \
	achermann.cc \
	achermann.hh \
	achermann-siman.cc \
	ultradian-cycle.cc \
	borbely.cc \
	assisted-score.cc \
	beersma.hh \
	forward-decls.hh

if DO_PCH
BUILT_SOURCES = \
	achermann.hh.gch \
	beersma.hh.gch \
	achermann-tunable.hh.gch \
	forward-decls.hh.gch
%.hh.gch: %.hh
	$(CXXCOMPILE) -c $<
CLEANFILES := \
	$(BUILT_SOURCES)
endif
