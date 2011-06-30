SUBDIRS = libexstrom libagh

AM_CXXFLAGS = -Wall -fno-show-column -std=c++0x $(AGH_CFLAGS) $(GSL_CFLAGS) -DPACKAGE_DATADIR=\"$(datadir)\" -DBUILT_BY=\"@user@\"

INCLUDES = -I.

bin_PROGRAMS = aghermann

aghermann_SOURCES = \
	libagh/libaghcore.la \
	ui/misc.cc ui/misc.hh \
	ui/managed-colour.hh \
	ui/draw-signal-generic.hh \
	ui/draw-signal-generic.cc \
	ui/ui.cc ui/ui.hh \
	ui/expdesign.hh \
	ui/expdesign.cc ui/expdesign_cb.cc \
	ui/expdesign-loadsave.cc \
	ui/expdesign-admit-one.cc \
	ui/expdesign-statusbar.cc \
	ui/expdesign-settings.cc \
	ui/expdesign-simulations.cc \
	ui/expdesign-selector.cc \
	ui/scoring-facility.hh \
	ui/scoring-facility.cc \
	ui/scoring-facility_cb.cc \
	ui/scoring-facility-montage.cc \
	ui/scoring-facility-hypnogram.cc \
	ui/scoring-facility-patterns.cc \
	ui/scoring-facility-filter.cc \
	ui/scoring-facility-phasediff.cc \
	ui/modelrun-facility.cc \
	ui/modelrun-facility.hh \
	main.cc

aghermann_LDFLAGS = --export-dynamic
aghermann_LDADD = \
	$(AGH_LIBS) $(GSL_LIBS) \
	libagh/libaghcore.la \
	libexstrom/libexstrom.la
