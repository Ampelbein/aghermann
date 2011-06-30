AM_CXXFLAGS = -Wall -std=c++0x $(AGH_CFLAGS) -fopenmp -fno-show-column

INCLUDES = -I..

lib_LTLIBRARIES = libaghcore.la # libaghcore-glib.la
libaghcore_la_LDFLAGS = -lgomp

libaghcore_la_SOURCES = \
	misc.hh boost-ptree-validator.hh \
	primaries.cc  primaries-loadsave.cc  primaries.hh \
	page.cc  page.hh \
	edf.cc  edf.hh \
	psd.cc psd.hh \
	scoring-assistant.cc \
	tunable.cc tunable.hh \
	model.cc model.hh \
	siman.cc

#libaghcore_glib_la_SOURCES = \
#	iface-glib.h \
#	iface-glib.cc

libaghcoreincdir = $(includedir)/libaghcore
libaghcoreinc_HEADERS = \
	misc.hh \
	edf.hh \
	model.hh \
	page.hh \
	primaries.hh \
	psd.hh \
	tunable.hh

install-exec-hook:
	$(mkinstalldirs) $(DESTDIR)$(libaghcoreincdir)
