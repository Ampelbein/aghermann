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
	controls_cb.cc \
	hypnogram.cc \
	hypnogram_cb.cc \
	ica.cc \
	ica_cb.cc \
	mainmenu_cb.cc \
	montage-overlays.cc \
	montage.cc \
	montage_cb.cc \
	sf.cc \
	sf.hh \
	sf_cb.cc \
	sf_cb.hh \
	widgets.hh \
	d/artifacts.hh \
	d/artifacts.cc \
	d/artifacts-construct.cc \
	d/artifacts_cb.cc \
	d/artifacts-simple.hh \
	d/artifacts-simple.cc \
	d/artifacts-simple-construct.cc \
	d/filters.hh \
	d/filters.cc \
	d/filters-construct.cc \
	d/filters_cb.cc \
	d/patterns.hh \
	d/patterns.cc \
	d/patterns-construct.cc \
	d/patterns-draw.cc \
	d/patterns-profiles.cc \
	d/patterns_cb.cc \
	d/patterns-profiles_cb.cc \
	d/phasediff.hh \
	d/phasediff.cc \
	d/phasediff-construct.cc \
	d/phasediff_cb.cc
