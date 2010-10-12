AM_CXXFLAGS = -Wall -std=c++0x $(AGH_CFLAGS) -fopenmp

INCLUDES = -I..

lib_LTLIBRARIES = libaghcore.la libaghcore-glib.la
libaghcore_la_LDFLAGS = -lgomp

libaghcore_la_SOURCES = \
	misc.hh \
	primaries.cc  primaries-loadsave.cc  primaries.hh \
	page.cc  page.hh \
	edf.cc  edf.hh \
	psd.cc psd.hh \
	tunable.cc tunable.hh \
	model.cc model.hh \
	siman.cc \
	iface.h \
	iface-expdesign.cc \
	iface-simulations.cc \
	iface-tunables.cc

libaghcore_glib_la_SOURCES = \
	iface-glib.h \
	iface-glib.cc
