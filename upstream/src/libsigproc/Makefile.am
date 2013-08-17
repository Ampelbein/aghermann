AM_CXXFLAGS := \
	-Wall -std=c++0x -fno-rtti -shared -fPIC \
	-I$(top_srcdir)/src

pkglib_LTLIBRARIES := \
	libsigproc.la

libsigproc_la_SOURCES := \
	exstrom.cc exstrom.hh \
	ext-filters.cc ext-filters.hh ext-filters.ii \
	sigproc.cc sigproc.hh sigproc.ii \
	winfun.cc winfun.hh

libsigproc_la_LDFLAGS := \
	-avoid-version \
	-rpath $(libdir)/$(PACKAGE) \
	-shared -module

if DO_PCH
BUILT_SOURCES := \
	ext-filters.hh.gch \
	exstrom.hh.gch \
	winfun.hh.gch \
	sigproc.hh.gch
%.hh.gch: %.hh
	$(CXXCOMPILE) -c $<
CLEANFILES = \
	$(BUILT_SOURCES)
endif

EXTRA_DIST := \
	sigproc.ii \
	ext-filters.ii
