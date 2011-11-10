SUBDIRS = libexstrom libsigfile libagh tools

AM_CXXFLAGS = \
	-Wall -fno-show-column -Wsuggest-attribute=const -Wsuggest-attribute=pure -Wnoexcept \
	-std=c++0x \
	$(AGHUI_CFLAGS) $(GSL_CFLAGS) \
	-DPACKAGE_DATADIR=\"$(datadir)\" -DBUILT_BY=\"@user@\"

INCLUDES = -I.

bin_PROGRAMS = aghermann

aghermann_SOURCES = \
	libagh/libaghcore.la \
	libedf/libedf.la \
	ui/forward-decls.hh \
	ui/misc.cc ui/misc.hh \
	ui/managed-colour.hh \
	ui/draw-signal-generic.hh \
	ui/draw-signal-generic.cc \
	ui/ui.cc ui/ui.hh \
	ui/expdesign.hh \
	ui/expdesign.cc \
	ui/expdesign_cb.cc \
	ui/expdesign-construct.cc \
	ui/expdesign-measurements.cc \
	ui/expdesign-measurements_cb.cc \
	ui/expdesign-loadsave.cc \
	ui/expdesign-admit-one.cc \
	ui/expdesign-settings_cb.cc \
	ui/expdesign-simulations.cc \
	ui/expdesign-simulations_cb.cc \
	ui/expdesign-chooser.cc \
	ui/expdesign-chooser_cb.cc \
	ui/scoring-facility.hh \
	ui/scoring-facility.cc \
	ui/scoring-facility-ica.cc \
	ui/scoring-facility-ica_cb.cc \
	ui/scoring-facility_cb.cc \
	ui/scoring-facility-montage.cc \
	ui/scoring-facility-montage_cb.cc \
	ui/scoring-facility-hypnogram.cc \
	ui/scoring-facility-patterns.cc \
	ui/scoring-facility-patterns_cb.cc \
	ui/scoring-facility-filter.cc \
	ui/scoring-facility-phasediff.cc \
	ui/modelrun-facility.hh \
	ui/modelrun-facility.cc \
	ui/modelrun-facility-construct.cc \
	ui/modelrun-facility_cb.cc \
	main.cc

aghermann_LDADD = \
	$(AGHCORE_LIBS) $(AGHICA_LIBS) $(AGHUI_LIBS) \
	$(OPENMP_LDADD) $(GSL_LIBS) \
	libagh/libaghcore.la \
	libexstrom/libexstrom.la \
	libsigfile/libsigfile.la
