SUBDIRS = libagh

AM_CFLAGS = -Wall -std=c99 $(AGH_CFLAGS) $(GSL_CFLAGS)

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
	ui/modelrun-facility.c \
	ui/simulations.c \
	ui/tunables.c \
	ui/statusbar.c \
	ui/loadsave.c \
	ui/settings.h \
	ui/ui.h

aghermann_LDADD = $(AGH_LIBS) $(GSL_LIBS) libagh/libaghcore.la libagh/libaghcore-glib.la
