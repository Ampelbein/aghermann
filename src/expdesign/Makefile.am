AM_CXXFLAGS = \
	-Wall -std=c++0x

noinst_LIBRARIES = libexpdesign.a

libexpdesign_a_SOURCES = \
	forward-decls.hh \
	primaries.cc \
	primaries-loadsave.cc \
	primaries.hh \
	assisted-score.cc

if DO_PCH
BUILT_SOURCES = \
	forward-decls.hh.gch \
	primaries.hh.gch
%.hh.gch: %.hh
	$(CXXCOMPILE) -c $<
endif

CLEANFILES = \
	*.gch
