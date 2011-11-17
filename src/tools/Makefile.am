AM_CXXFLAGS = -Wall -fno-show-column -std=c++0x $(AGHCORE_CFLAGS) $(AGHUI_CFLAGS) -DPACKAGE_DATADIR=\"$(datadir)\"

INCLUDES = -I..

bin_PROGRAMS = \
	edfhed \
	edfhed-gtk

edfhed_SOURCES = \
	../libsigfile/libsigfile.la \
	edfhed.cc
edfhed_LDADD = \
	$(AGHCORE_LIBS) $(OPENMP_LDADD) $(GSL_LIBS) \
	../libsigfile/libsigfile.la \
	../libexstrom/libexstrom.la

edfhed_gtk_SOURCES = \
	../libsigfile/libsigfile.la \
	edfhed.hh \
	edfhed-gtk.cc \
	../ui/misc.cc
edfhed_gtk_LDADD = \
	$(AGHCORE_LIBS) $(OPENMP_LDADD) $(AGHUI_LIBS) $(GSL_LIBS) \
	../libsigfile/libsigfile.la \
	../libexstrom/libexstrom.la
