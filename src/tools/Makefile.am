AM_CXXFLAGS = -Wall -fno-show-column -std=c++0x $(AGHUI_CFLAGS) -DPACKAGE_DATADIR=\"$(datadir)\" -I..

bin_PROGRAMS = \
	edfhed \
	edfhed-gtk

edfhed_SOURCES = \
	../libsigfile/libsigfile.la \
	edfhed.cc
edfhed_LDADD = \
	../libsigfile/libsigfile.la \
	$(AGHSIGFILE_LIBS) $(LIBFFTW3_LDADD) $(OPENMP_LDADD) $(GSL_LIBS)

edfhed_gtk_SOURCES = \
	../libsigfile/libsigfile.la \
	edfhed-gtk.cc \
	../ui/misc.cc
edfhed_gtk_LDADD = \
	../libsigfile/libsigfile.la \
	$(AGHSIGFILE_LIBS) $(LIBFFTW3_LDADD) $(OPENMP_LDADD) $(GSL_LIBS) $(AGHUI_LIBS)
