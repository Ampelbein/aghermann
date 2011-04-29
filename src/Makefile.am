SUBDIRS = libexstrom libagh

AM_CFLAGS = -Wall -std=gnu99 $(AGH_CFLAGS) $(GSL_CFLAGS) -DPACKAGE_DATADIR=\"$(docdir)\" -DBUILT_BY=\"@user@\"
AM_CXXFLAGS = -Wall -fno-show-column -std=c++0x $(AGH_CFLAGS) $(GSL_CFLAGS) -DPACKAGE_DATADIR=\"$(docdir)\" -DBUILT_BY=\"@user@\"

INCLUDES = -I.

bin_PROGRAMS = aghermann

aghermann_SOURCES = \
	main.cc \
	structures.cc structures.hh \
	libagh/libaghcore.la \
	ui/misc.hh \
	ui/misc.cc \
	ui/ui.cc \
	ui/settings-tab.cc \
	ui/measurements.hh \
	ui/measurements.cc \
	ui/measurements-admit-one.cc \
	ui/scoring-facility.hh \
	ui/scoring-facility.cc \
	ui/scoring-facility-da_page.cc \
	ui/scoring-facility-da_power.cc \
	ui/scoring-facility-da_emg.cc \
	ui/scoring-facility-patterns.cc \
	ui/scoring-facility-filter.cc \
	ui/scoring-facility-phasediff.cc \
	ui/modelrun-facility.cc \
	ui/simulations.cc \
	ui/statusbar.cc \
	ui/loadsave.cc \
	ui/settings.hh \
	ui/ui.hh

aghermann_LDADD = \
	$(AGH_LIBS) $(GSL_LIBS) \
	libagh/libaghcore.la \
	libexstrom/libexstrom.la
