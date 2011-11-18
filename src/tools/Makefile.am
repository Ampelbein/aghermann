AM_CXXFLAGS = -Wall -fno-show-column -std=c++0x $(AGHUI_CFLAGS) -DPACKAGE_DATADIR=\"$(datadir)\"

INCLUDES = -I..

bin_PROGRAMS = \
	edfhed \
	edfhed-gtk

edfhed_SOURCES = \
	../libsigfile/libsigfile.la \
	edfhed.cc
edfhed_LDADD = \
	$(AGHSIGFILE_LIBS) $(OPENMP_LDADD) $(GSL_LIBS) \
	../libsigfile/libsigfile.la

edfhed_gtk_SOURCES = \
	../libsigfile/libsigfile.la \
	edfhed-gtk.cc \
	../ui/misc.cc
edfhed_gtk_LDADD = \
	$(AGHSIGFILE_LIBS) $(OPENMP_LDADD) $(GSL_LIBS) $(AGHUI_LIBS) \
	../libsigfile/libsigfile.la
