AM_CXXFLAGS = \
	-std=c++0x -Wdeprecated-declarations \
	$(GTK_CFLAGS) $(LIBCONFIGXX_CFLAGS) \
	-DPACKAGE_DATADIR=\"$(datadir)\" \
	$(OPENMP_CXXFLAGS)

noinst_LIBRARIES = libaghui.a

libaghui_a_SOURCES = \
	forward-decls.hh \
	ui.hh \
	ui++.hh \
	misc.hh \
	libcommon.cc \
	globals.hh \
	sc.cc \
	sc-construct.cc \
	sc.hh \
	sc_cb.cc \
	sc_cb.hh \
	ed.hh \
	ed-widgets.hh \
	ed_cb.hh \
	ed.cc \
	ed_cb.cc \
	ed-construct.cc \
	ed-populate.cc \
	ed-measurements.cc \
	ed-measurements_cb.cc \
	ed-loadsave.cc \
	ed-admit-one.cc \
	ed-settings_cb.cc \
	ed-simulations.cc \
	ed-simulations_cb.cc \
	sf.hh \
	sf-widgets.hh \
	sf_cb.hh \
	sf.cc \
	sf-construct.cc \
	sf-channel.cc \
	sf-artifacts.cc \
	sf-artifacts_cb.cc \
	sf-ica.cc \
	sf-ica_cb.cc \
	sf_cb.cc \
	sf-montage.cc \
	sf-montage_cb.cc \
	sf-hypnogram.cc \
	sf-hypnogram_cb.cc \
	sf-patterns.cc \
	sf-patterns_cb.cc \
	sf-filter.cc \
	sf-filter_cb.cc \
	sf-phasediff.cc \
	sf-phasediff_cb.cc \
	mf.hh \
	mf_cb.hh \
	mf.cc \
	mf-construct.cc \
	mf_cb.cc

if DO_PCH
BUILT_SOURCES = \
	ui.hh.gch \
	ui++.hh.gch \
	misc.hh.gch \
	globals.hh.gch \
	forward-decls.hh.gch \
	sc.hh.gch \
	sc_cb.hh.gch \
	ed.hh.gch \
	ed-widgets.hh.gch \
	ed_cb.hh.gch \
	sf.hh.gch \
	sf-widgets.hh.gch \
	sf_cb.hh.gch \
	mf.hh.gch \
	mf_cb.hh.gch
%.hh.gch: %.hh
	$(CXXCOMPILE) -c $<
CLEANFILES := \
	$(BUILT_SOURCES)
endif
