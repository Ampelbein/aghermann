AM_CXXFLAGS = \
	-Wall -std=c++0x

noinst_LIBRARIES = libaghcore.a

libaghcore_a_SOURCES = \
	forward-decls.hh \
	primaries.cc \
	primaries-loadsave.cc \
	primaries.hh \
	assisted-score.cc \
	tunable.cc \
	tunable.hh \
	model.cc \
	model.hh \
	siman.cc

if DO_PCH
BUILT_SOURCES = \
	forward-decls.hh.gch \
	primaries.hh.gch \
	model.hh.gch \
	tunable.hh.gch
%.hh.gch: %.hh
	$(CXXCOMPILE) -c $<
endif

CLEANFILES = \
	*.gch
