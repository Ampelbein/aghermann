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
	sf_cb.hh \
	sf_cb.cc \
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

if DO_PCH
BUILT_SOURCES = \
	sf.hh.gch \
	sf_cb.hh.gch \
	widgets.hh.gch \
	d/artifacts.hh.gch \
	d/patterns.hh.gch \
	d/filters.hh.gch \
	d/phasediff.hh.gch
%.hh.gch: %.hh
	$(CXXCOMPILE) -c $<
CLEANFILES = \
	$(BUILT_SOURCES)
endif
