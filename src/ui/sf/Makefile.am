SUBDIRS := \
	d

AM_CXXFLAGS := \
	-std=c++0x -fno-rtti -Wdeprecated-declarations -Wall \
	$(GTK_CFLAGS)  $(UNIQUE_CFLAGS)  $(CAIRO_CFLAGS) $(LIBCONFIGXX_CFLAGS) \
	-DPACKAGE_DATADIR=\"$(datadir)\" \
	$(OPENMP_CXXFLAGS) \
	-I$(top_srcdir)/src

noinst_LIBRARIES := liba.a

liba_a_SOURCES := \
	channel.cc \
	construct.cc \
	hypnogram.cc \
	hypnogram_cb.cc \
	ica.cc \
	ica_cb.cc \
	montage.cc \
	montage-overlays.cc \
	montage_cb.cc \
	widgets.hh \
	sf.cc \
	sf.hh \
	sf_cb.cc

if DO_PCH
BUILT_SOURCES = \
	sf.hh.gch \
	widgets.hh.gch
%.hh.gch: %.hh
	$(CXXCOMPILE) -c $<
CLEANFILES := \
	$(BUILT_SOURCES)
endif
