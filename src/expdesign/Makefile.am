AM_CXXFLAGS = \
	-Wall -std=c++0x \
	$(OPENMP_CXXFLAGS)

noinst_LIBRARIES = libexpdesign.a

libexpdesign_a_SOURCES = \
	forward-decls.hh \
	primaries.cc \
	primaries-loadsave.cc \
	primaries-tree-scanner.cc \
	primaries.hh \
	recording.cc \
	recording.hh

if DO_PCH
BUILT_SOURCES = \
	forward-decls.hh.gch \
	recording.hh.gch \
	primaries.hh.gch
%.hh.gch: %.hh
	$(CXXCOMPILE) -c $<
endif

CLEANFILES = \
	*.gch
