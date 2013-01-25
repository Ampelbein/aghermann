SUBDIRS := \
	d

AM_CXXFLAGS := \
	-std=c++11 -fno-rtti -Wdeprecated-declarations -Wall \
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
	phasic-events.cc \
	widgets.hh \
	sf.cc \
	sf.hh \
	sf_cb.cc \
	sf_cb.hh

if DO_PCH
BUILT_SOURCES = \
	sf.hh.gch \
	widgets.hh.gch \
	sf_cb.hh.gch
%.hh.gch: %.hh
	$(CXXCOMPILE) -c $<
CLEANFILES := \
	$(BUILT_SOURCES)
endif
