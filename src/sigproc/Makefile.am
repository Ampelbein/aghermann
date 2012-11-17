AM_CXXFLAGS := \
	-Wall -std=c++0x \
	-I$(top_srcdir)/src

noinst_LIBRARIES := liba.a

liba_a_SOURCES := \
	exstrom.cc exstrom.hh \
	ext-filters.cc ext-filters.hh \
	sigproc.cc sigproc.hh \
	winfun.cc winfun.hh

if DO_PCH
BUILT_SOURCES := \
	ext-filters.hh.gch \
	exstrom.hh.gch \
	winfun.hh.gch \
	sigproc.hh.gch
%.hh.gch: %.hh
	$(CXXCOMPILE) -c $<
CLEANFILES := \
	$(BUILT_SOURCES)
endif

EXTRA_DIST := \
	sigproc.ii
