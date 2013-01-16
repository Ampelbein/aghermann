AM_CXXFLAGS := \
	-std=c++11 -fno-rtti -Wdeprecated-declarations -Wall \
	$(GTK_CFLAGS)  $(UNIQUE_CFLAGS)  $(CAIRO_CFLAGS) $(LIBCONFIGXX_CFLAGS) \
	-DPACKAGE_DATADIR=\"$(datadir)\" \
	$(OPENMP_CXXFLAGS) \
	-I$(top_srcdir)/src

noinst_LIBRARIES := liba.a

liba_a_SOURCES := \
	sf-artifacts.cc \
	sf-artifacts_cb.cc \
	sf-channel.cc \
	sf-construct.cc \
	sf-filter.cc \
	sf-filter_cb.cc \
	sf-hypnogram.cc \
	sf-hypnogram_cb.cc \
	sf-ica.cc \
	sf-ica_cb.cc \
	sf-montage.cc \
	sf-montage-overlays.cc \
	sf-montage_cb.cc \
	sf-patterns.cc \
	sf-patterns-draw.cc \
	sf-patterns-enumerate.cc \
	sf-patterns_cb.cc \
	sf-phasediff.cc \
	sf-phasediff_cb.cc \
	sf-phasic-events.cc \
	sf-widgets.hh \
	sf.cc \
	sf.hh \
	sf_cb.cc \
	sf_cb.hh

if DO_PCH
BUILT_SOURCES = \
	sf.hh.gch \
	sf-widgets.hh.gch \
	sf_cb.hh.gch
%.hh.gch: %.hh
	$(CXXCOMPILE) -c $<
CLEANFILES := \
	$(BUILT_SOURCES)
endif
