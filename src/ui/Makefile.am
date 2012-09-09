AM_CXXFLAGS = \
	-std=c++0x -Wdeprecated-declarations \
	$(GTK_CFLAGS) $(LIBCONFIGXX_CFLAGS) \
	-DPACKAGE_DATADIR=\"$(datadir)\"

noinst_LIBRARIES = libaghui.a

libaghui_a_SOURCES = \
	forward-decls.hh \
	misc.cc \
	misc.hh \
	managed-colour.hh \
	ui.cc \
	ui.hh \
	session-chooser.cc \
	session-chooser-construct.cc \
	session-chooser.hh \
	session-chooser_cb.cc \
	session-chooser_cb.hh \
	expdesign.hh \
	expdesign_cb.hh \
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
	scoring-facility.hh \
	scoring-facility_cb.hh \
	scoring-facility.cc \
	scoring-facility-construct.cc \
	scoring-facility-channel.cc \
	scoring-facility-ica.cc \
	scoring-facility-ica_cb.cc \
	scoring-facility_cb.cc \
	scoring-facility-montage.cc \
	scoring-facility-montage_cb.cc \
	scoring-facility-hypnogram.cc \
	scoring-facility-hypnogram_cb.cc \
	scoring-facility-patterns.cc \
	scoring-facility-patterns_cb.cc \
	scoring-facility-filter.cc \
	scoring-facility-filter_cb.cc \
	scoring-facility-phasediff.cc \
	scoring-facility-phasediff_cb.cc \
	modelrun-facility.hh \
	modelrun-facility_cb.hh \
	modelrun-facility.cc \
	modelrun-facility-construct.cc \
	modelrun-facility_cb.cc

if DO_PCH
BUILT_SOURCES = \
	forward-decls.hh.gch \
	misc.hh.gch \
	managed-colour.hh.gch \
	ui.hh.gch \
	session-chooser.hh.gch \
	session-chooser_cb.hh.gch \
	expdesign.hh.gch \
	expdesign_cb.hh.gch \
	scoring-facility.hh.gch \
	scoring-facility_cb.hh.gch \
	modelrun-facility.hh.gch \
	modelrun-facility_cb.hh.gch
%.hh.gch: %.hh
	$(CXXCOMPILE) -c $<
endif

CLEANFILES = \
	*.gch
