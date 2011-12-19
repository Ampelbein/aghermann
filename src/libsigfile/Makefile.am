AM_CXXFLAGS = \
	-Wall -std=c++0x \
	$(OPENMP_CXXFLAGS)

pkglib_LTLIBRARIES = libsigfile.la

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

libsigfile_la_LDFLAGS = \
	-avoid-version \
	-module -rpath $(libdir)/$(PACKAGE)

# libsigfileincdir = $(includedir)/libsigfile
# libsigfileinc_HEADERS = \
# 	forward-decls.hh \
# 	channel.hh \
# 	source-base.hh \
# 	source.hh \
# 	edf.ii \
# 	edf.hh \
# 	page.hh \
# 	psd.hh

BUILT_SOURCES = \
	forward-decls.hh.gch \
	channel.hh.gch \
	source-base.hh.gch \
	source.hh.gch \
	edf.ii \
	edf.hh.gch \
	page.hh.gch \
	psd.hh.gch
%.hh.gch: %.hh
	$(CXXCOMPILE) -c $<

# install-exec-hook:
# 	$(mkinstalldirs) $(DESTDIR)$(libsigfileincdir)

if DO_UPDATE_MIME_DB
xdgdatadir = $(XDGDATADIR)
mimedir = $(xdgdatadir)/mime
xmldir = $(mimedir)/packages
xml_DATA = edf-file.xml

EXTRA_DIST = edf-file.xml

install-data-hook:
	rm -f $(DESTDIR)$(pkglibdir)/*.la; \
	$(UPDATE_MIME_DATABASE) "$(DESTDIR)$(mimedir)"

uninstall-hook:
	$(UPDATE_MIME_DATABASE) "$(DESTDIR)$(mimedir)"
else

install-data-hook:
	rm -f $(DESTDIR)$(pkgverlibdir)/*.la;

endif
