AM_CXXFLAGS := \
	-Wall -std=c++0x -fno-rtti -shared -fPIC \
	-I$(top_srcdir)/src

pkglib_LTLIBRARIES := \
	libsigfile.la

libsigfile_la_SOURCES := \
	forward-decls.hh \
	channel.cc \
	channel.hh \
	source-base.cc \
	source-base.hh \
	typed-source.cc \
	typed-source.hh \
	edf.cc \
	edf-io.cc \
	edf.hh \
	tsv.cc \
	tsv.hh \
	page.cc \
	page.hh \
	all.hh

libsigfile_la_LDFLAGS := \
	-avoid-version \
	-rpath $(libdir)/$(PACKAGE) \
	-shared -module

# libsigfileincdir = $(includedir)/libsigfile
# libsigfileinc_HEADERS = \
# 	forward-decls.hh \
# 	channel.hh \
# 	source-base.hh \
# 	source.hh \
# 	edf.ii \
# 	edf.hh \
# 	page.hh


if DO_UPDATE_MIME_DB
xdgdatadir := $(XDGDATADIR)
mimedir := $(xdgdatadir)/mime
xmldir := $(mimedir)/packages
xml_DATA := edf-file.xml

EXTRA_DIST := edf-file.xml

endif
