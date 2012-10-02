AM_CXXFLAGS = \
	-Wall -std=c++0x

noinst_LIBRARIES = libsigproc.a

libsigproc_a_SOURCES = \
	exstrom.cc exstrom.hh \
	ext-filters.cc ext-filters.hh \
	sigproc.cc sigproc.hh

if DO_PCH
BUILT_SOURCES = \
	ext-filters.hh.gch \
	exstrom.hh.gch \
	sigproc.hh.gch
%.hh.gch: %.hh
	$(CXXCOMPILE) -c $<
endif

EXTRA_DIST = \
	sigproc.ii

CLEANFILES = \
	*.gch
