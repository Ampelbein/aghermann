AM_CXXFLAGS = \
	-Wall -std=c++0x \
	$(AGHCORE_CFLAGS) $(OPENMP_CXXFLAGS) \
	-fno-show-column -Wsuggest-attribute=const -Wsuggest-attribute=pure -Wnoexcept

lib_LTLIBRARIES = libsigfile.la

libsigfile_la_SOURCES = \
	channel.hh \
	source.hh \
	edf.cc \
	edf.hh \
	page.cc \
	page.hh \
	psd.cc \
	psd.hh

libsigfileincdir = $(includedir)/libsigfile
libsigfileinc_HEADERS = \
	forward-decls.hh \
	channel.hh \
	source.hh \
	edf.hh \
	page.hh \
	psd.hh

install-exec-hook:
	$(mkinstalldirs) $(DESTDIR)$(libsigfileincdir)
