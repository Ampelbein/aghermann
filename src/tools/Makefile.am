AM_CXXFLAGS = -Wall -Wextra -fno-show-column -std=c++0x $(AGH_CFLAGS) $(GSL_CFLAGS) -DPACKAGE_DATADIR=\"$(datadir)\"

INCLUDES = -I..

bin_PROGRAMS = edfhed edfhed-gtk

edfhed_SOURCES = \
	../libagh/libaghcore.la \
	edfhed.hh edfhed-common.cc \
	edfhed.cc

edfhed_gtk_SOURCES = \
	../libagh/libaghcore.la \
	edfhed.hh edfhed-common.cc \
	edfhed-gtk.cc \
	../ui/ui.cc

edfhed_LDADD = \
	$(AGH_LIBS) $(GSL_LIBS) \
	../libagh/libaghcore.la \
	../libexstrom/libexstrom.la

edfhed_gtk_LDADD = \
	$(AGH_LIBS) $(GSL_LIBS) \
	../libagh/libaghcore.la \
	../libexstrom/libexstrom.la
