SUBDIRS = libexstrom libagh

AM_CFLAGS = -Wall -std=gnu99 $(AGH_CFLAGS) $(GSL_CFLAGS) -fno-show-column

INCLUDES = -I.

bin_PROGRAMS = aghermann

aghermann_SOURCES = \
	main.c \
	common.h \
	structures.c structures.h \
	libagh/libaghcore.la \
	libagh/libaghcore-glib.la \
	ui/misc.h \
	ui/misc.c \
	ui/ui.c \
	ui/settings-tab.c \
	ui/measurements.c \
	ui/scoring-facility.c \
	ui/scoring-facility-patterns.c \
	ui/scoring-facility-filter.c \
	ui/scoring-facility-phasediff.c \
	ui/modelrun-facility.c \
	ui/simulations.c \
	ui/tunables.c \
	ui/statusbar.c \
	ui/loadsave.c \
	ui/settings.h \
	ui/ui.h

aghermann_LDADD = $(AGH_LIBS) $(GSL_LIBS) libagh/libaghcore.la libagh/libaghcore-glib.la libexstrom/libexstrom.la
