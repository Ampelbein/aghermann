AM_CXXFLAGS := \
	-Wall -std=c++0x -fno-rtti -shared \
	-I$(top_srcdir)/src \
	-fPIC

pkglib_LTLIBRARIES := \
	libsigfile.la

noinst_LIBRARIES := \
	liba.a
liba_a_CXXFLAGS = $(AM_CXXFLAGS)
# see http://www.gnu.org/software/automake/manual/html_node/Objects-created-both-with-libtool-and-without.html#Objects-created-both-with-libtool-and-without

# noinst_RELOCATABLES = \
# 	libsigfile.o
# two days knocking about, googling it: no, thank you

libsigfile_la_SOURCES := \
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
	page.hh
liba_a_SOURCES := \
	$(libsigfile_la_SOURCES)

# libsigfile_la_parts = \
# 	source-base.o \ ...

libsigfile_la_LDFLAGS := \
	-avoid-version \
	-rpath $(libdir)/$(PACKAGE) \
	-module

# libsigfile.la: $(libsigfile_la_parts)
# 	$(LINK) -shared -o $@ -Wl,-whole-archive $(libsigfile_la_parts) -no-whole-archive \
# 	-avoid-version \
# 	-rpath $(libdir)/$(PACKAGE) \
# 	$(FFTW3_LIBS) $(LIBFFTW3_LDADD) $(GSL_LIBS) \
# 	-Ur
# I will give $10 to anyone knows how to do it

# libsigfileincdir = $(includedir)/libsigfile
# libsigfileinc_HEADERS = \
# 	forward-decls.hh \
# 	channel.hh \
# 	source-base.hh \
# 	source.hh \
# 	edf.ii \
# 	edf.hh \
# 	page.hh

if DO_PCH
BUILT_SOURCES := \
	forward-decls.hh.gch \
	channel.hh.gch \
	source-base.hh.gch \
	source.hh.gch \
	edf.hh.gch \
	page.hh.gch
%.hh.gch: %.hh
	$(CXXCOMPILE) -c $<
CLEANFILES = \
	*.gch
endif

if DO_UPDATE_MIME_DB
xdgdatadir := $(XDGDATADIR)
mimedir := $(xdgdatadir)/mime
xmldir := $(mimedir)/packages
xml_DATA := edf-file.xml

EXTRA_DIST := edf-file.xml

endif
