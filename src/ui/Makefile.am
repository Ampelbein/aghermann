AM_CXXFLAGS = \
	-std=c++0x \
	$(AGHUI_CFLAGS) \
	-DPACKAGE_DATADIR=\"$(datadir)\"

noinst_LIBRARIES = libaghui.a

libaghui_a_SOURCES = \
	forward-decls.hh \
	misc.cc \
	misc.hh \
	managed-colour.hh \
	draw-signal-generic.hh \
	draw-signal-generic.cc \
	ui.cc \
	ui.hh \
	expdesign.hh \
	expdesign.cc \
	expdesign_cb.cc \
	expdesign-construct.cc \
	expdesign-measurements.cc \
	expdesign-measurements_cb.cc \
	expdesign-loadsave.cc \
	expdesign-admit-one.cc \
	expdesign-settings_cb.cc \
	expdesign-simulations.cc \
	expdesign-simulations_cb.cc \
	expdesign-chooser.cc \
	expdesign-chooser_cb.cc \
	scoring-facility.hh \
	scoring-facility.cc \
	scoring-facility-ica.cc \
	scoring-facility-ica_cb.cc \
	scoring-facility_cb.cc \
	scoring-facility-montage.cc \
	scoring-facility-montage_cb.cc \
	scoring-facility-hypnogram.cc \
	scoring-facility-patterns.cc \
	scoring-facility-patterns_cb.cc \
	scoring-facility-filter.cc \
	scoring-facility-phasediff.cc \
	modelrun-facility.hh \
	modelrun-facility.cc \
	modelrun-facility-construct.cc \
	modelrun-facility_cb.cc

BUILT_SOURCES = \
	forward-decls.hh.gch \
	misc.hh.gch \
	managed-colour.hh.gch \
	draw-signal-generic.hh.gch \
	ui.hh.gch \
	expdesign.hh.gch \
	scoring-facility.hh.gch \
	modelrun-facility.hh.gch
%.hh.gch: %.hh
	$(CXXCOMPILE) -c $<
