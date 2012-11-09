AM_CXXFLAGS := \
	-Wall -std=c++0x -shared \
	$(OPENMP_CXXFLAGS) \
	-I$(top_srcdir)/src \
	-fPIC

noinst_LIBRARIES := \
	liba.a
liba_a_SOURCES := \
	forward-decls.hh \
	page-metrics-base.hh \
	page-metrics-base.cc \
	psd.cc \
	psd.hh \
	mc.cc \
	mc.hh \
	mc-artifacts.cc \
	mc-artifacts.hh

if DO_PCH
BUILT_SOURCES := \
	forward-decls.hh.gch \
	page-metrics-base.hh \
	psd.hh.gch \
	mc.hh.gch \
	mc-artifacts.hh.gch
%.hh.gch: %.hh
	$(CXXCOMPILE) -c $<
CLEANFILES = \
	*.gch
endif
