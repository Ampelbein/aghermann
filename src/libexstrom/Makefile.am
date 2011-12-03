AM_CXXFLAGS = -Wall -std=c++0x $(AGHCORE_CFLAGS)

lib_LTLIBRARIES = libexstrom.la
#libexstrom_la_LDFLAGS = -lgomp

libexstrom_la_SOURCES = \
	signal.cc exstrom.hh signal.hh

libexstromincdir = $(includedir)/libexstrom
libexstrominc_HEADERS = \
	exstrom.hh \
	signal.hh
BUILT_SOURCES = \
	exstrom.hh.gch \
	signal.hh.gch
%.hh.gch: %.hh
	$(CXXCOMPILE) -c $<


install-exec-hook:
	$(mkinstalldirs) $(DESTDIR)$(libexstromincdir)
