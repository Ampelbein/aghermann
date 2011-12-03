AM_CXXFLAGS = \
	-Wall -std=c++0x \
	$(OPENMP_CXXFLAGS)

lib_LTLIBRARIES = libsigfile.la

libsigfile_la_SOURCES = \
	forward-decls.hh \
	channel.cc \
	channel.hh \
	source-base.cc \
	source-base.hh \
	source.cc \
	source.hh \
	edf.cc \
	edf.ii \
	edf.hh \
	page.cc \
	page.hh \
	psd.cc \
	psd.hh
#	source-slowwave.cc

libsigfileincdir = $(includedir)/libsigfile
libsigfileinc_HEADERS = \
	forward-decls.hh \
	channel.hh \
	source-base.hh \
	source.hh \
	edf.ii \
	edf.hh \
	page.hh \
	psd.hh

install-exec-hook:
	$(mkinstalldirs) $(DESTDIR)$(libsigfileincdir)


xdgdatadir = $(XDGDATADIR)
mimedir = $(xdgdatadir)/mime
xmldir = $(mimedir)/packages
xml_DATA = edf-file.xml

EXTRA_DIST = edf-file.xml

install-data-hook:
	$(UPDATE_MIME_DATABASE) "$(DESTDIR)$(mimedir)"

uninstall-hook:
	$(UPDATE_MIME_DATABASE) "$(DESTDIR)$(mimedir)"
