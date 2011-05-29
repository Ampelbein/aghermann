SUBDIRS = libexstrom libagh

AM_CXXFLAGS = -Wall -fno-show-column -std=c++0x $(AGH_CFLAGS) $(GSL_CFLAGS) -DPACKAGE_DATADIR=\"$(datadir)\" -DBUILT_BY=\"@user@\"

INCLUDES = -I.

bin_PROGRAMS = aghermann

aghermann_SOURCES = \
	main.cc \
	structures.cc structures.hh \
	libagh/libaghcore.la \
	ui/misc.cc ui/misc.hh \
	ui/ui.cc ui/ui.hh \
	ui/statusbar.cc \
	ui/expdesign-selector.cc \
	ui/loadsave.cc \
	ui/settings-tab.cc \
	ui/settings.hh \
	ui/measurements.hh \
	ui/measurements.cc \
	ui/measurements-admit-one.cc \
	ui/draw-signal-generic.hh \
	ui/draw-signal-generic.cc \
	ui/scoring-facility.hh \
	ui/scoring-facility.cc \
	ui/scoring-facility-da_page.cc \
	ui/scoring-facility-da_power.cc \
	ui/scoring-facility-da_emg.cc \
	ui/scoring-facility-da_hypnogram.cc \
	ui/scoring-facility-patterns.cc \
	ui/scoring-facility-filter.cc \
	ui/scoring-facility-phasediff.cc \
	ui/simulations.cc \
	ui/simulations.hh \
	ui/modelrun-facility.cc \
	ui/modelrun-facility.hh

aghermann_LDFLAGS = --export-dynamic
aghermann_LDADD = \
	$(AGH_LIBS) $(GSL_LIBS) \
	libagh/libaghcore.la \
	libexstrom/libexstrom.la
