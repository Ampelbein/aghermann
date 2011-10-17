AM_CXXFLAGS = -Wall -fno-show-column -std=c++0x $(AGHCORE_CFLAGS) $(AGHUI_CFLAGS) -DPACKAGE_DATADIR=\"$(datadir)\"

INCLUDES = -I..

bin_PROGRAMS = \
	edfhed \
	edfhed-gtk

edfhed_SOURCES = \
	../libagh/libaghcore.la \
	edfhed.hh \
	edfhed.cc
edfhed_LDADD = \
	$(AGHCORE_LIBS) $(OPENMP_LDADD) $(GSL_LIBS) \
	../libagh/libaghcore.la \
	../libexstrom/libexstrom.la

edfhed_gtk_SOURCES = \
	../libagh/libaghcore.la \
	edfhed.hh \
	edfhed-gtk.cc \
	../ui/ui.cc
edfhed_gtk_LDADD = \
	$(AGHCORE_LIBS) $(OPENMP_LDADD) $(AGHUI_LIBS) $(GSL_LIBS) \
	../libagh/libaghcore.la \
	../libexstrom/libexstrom.la
