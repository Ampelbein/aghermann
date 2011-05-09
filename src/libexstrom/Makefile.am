AM_CXXFLAGS = -Wall -std=c++0x  $(AGH_CFLAGS) -fopenmp -fno-show-column

lib_LTLIBRARIES = libexstrom.la
libexstrom_la_LDFLAGS = -lgomp

libexstrom_la_SOURCES = \
	signal.cc exstrom.hh signal.hh

libexstromincdir = $(includedir)/libexstrom
libexstrominc_HEADERS = \
	exstrom.hh \
	signal.hh

install-exec-hook:
	$(mkinstalldirs) $(DESTDIR)$(libexstromincdir)
