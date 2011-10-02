AM_CXXFLAGS = -Wall  -Wextra -std=c++0x -Weffc++  $(AGHCORE_CFLAGS) -fno-show-column # -Wunused-but-set-variable

lib_LTLIBRARIES = libexstrom.la
#libexstrom_la_LDFLAGS = -lgomp

libexstrom_la_SOURCES = \
	signal.cc exstrom.hh signal.hh

libexstromincdir = $(includedir)/libexstrom
libexstrominc_HEADERS = \
	exstrom.hh \
	signal.hh

install-exec-hook:
	$(mkinstalldirs) $(DESTDIR)$(libexstromincdir)
