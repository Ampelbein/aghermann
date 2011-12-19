AM_CXXFLAGS = -Wall -std=c++0x $(AGHUI_CFLAGS) -DPACKAGE_DATADIR=\"$(datadir)\" -I..

bin_PROGRAMS = \
	edfhed \
	edfhed-gtk

edfhed_SOURCES = \
	edfhed.cc
edfhed_LDADD = \
	../libsigfile/libsigfile.la \
	$(AGHSIGFILE_LIBS) $(LIBFFTW3_LDADD) $(OPENMP_LDADD) $(GSL_LIBS)

edfhed_gtk_SOURCES = \
	edfhed-gtk.cc \
	../ui/misc.cc
edfhed_gtk_LDADD = \
	../libsigfile/libsigfile.la \
	$(AGHSIGFILE_LIBS) $(LIBFFTW3_LDADD) $(OPENMP_LDADD) $(GSL_LIBS) $(AGHUI_LIBS)
