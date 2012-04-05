AM_CXXFLAGS = -Wall -std=c++0x $(AGHCORE_CFLAGS)

noinst_LIBRARIES = libsigproc.a

libsigproc_a_SOURCES = \
	exstrom.hh \
	ext-filters.cc ext-filters.hh \
	sigproc.cc sigproc.hh

BUILT_SOURCES = \
	ext-filters.hh.gch \
	exstrom.hh.gch \
	sigproc.hh.gch
%.hh.gch: %.hh
	$(CXXCOMPILE) -c $<

EXTRA_DIST = sigproc.ii
