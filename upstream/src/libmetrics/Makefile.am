AM_CXXFLAGS := \
	-Wall -std=c++0x -fno-rtti -shared -fPIC \
	$(OPENMP_CXXFLAGS) \
	-I$(top_srcdir)/src

pkglib_LTLIBRARIES := \
	libmetrics.la

libmetrics_la_SOURCES := \
	forward-decls.hh \
	bands.hh \
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

libmetrics_la_LDFLAGS := \
	-avoid-version \
	-rpath $(libdir)/$(PACKAGE) \
	-shared -module

if DO_PCH
BUILT_SOURCES := \
	forward-decls.hh.gch \
	bands.hh.gch \
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
