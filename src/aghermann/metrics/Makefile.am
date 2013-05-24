AM_CXXFLAGS := \
	-Wall -std=c++0x -fno-rtti \
	$(OPENMP_CXXFLAGS) \
	-I$(top_srcdir)/src

noinst_LIBRARIES := \
	liba.a
liba_a_SOURCES := \
	forward-decls.hh \
	page-metrics-base.hh \
	page-metrics-base.cc \
	psd.cc \
	psd.hh \
	swu.cc \
	swu.hh \
	mc.cc \
	mc.hh \
	mc-artifacts.cc \
	mc-artifacts.hh \
	mc-artifacts.ii \
	all.hh

if DO_PCH
BUILT_SOURCES := \
	forward-decls.hh.gch \
	page-metrics-base.hh.gch \
	psd.hh.gch \
	swu.hh.gch \
	mc.hh.gch \
	mc-artifacts.hh.gch
%.hh.gch: %.hh
	$(CXXCOMPILE) -c $<
CLEANFILES = \
	$(BUILT_SOURCES)
endif
