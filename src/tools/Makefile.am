AM_CXXFLAGS = -Wall -std=c++0x

bin_PROGRAMS = \
	edfhed \
	edfhed-gtk

edfhed_SOURCES = \
	edfhed.cc
edfhed_LDADD = \
	../libsigfile/libsigfile.la \
	../libsigproc/libsigproc.a \
	$(FFTW3_LIBS) $(LIBFFTW3_LDADD) $(OPENMP_LDADD) $(GSL_LIBS)

edfhed_gtk_SOURCES = \
	edfhed-gtk.cc \
	../ui/misc.cc
edfhed_gtk_CXXFLAGS = \
	$(AM_CXXFLAGS) 	$(GTK_CFLAGS) -DPACKAGE_DATADIR=\"$(datadir)\"
edfhed_gtk_LDADD = \
	../libsigfile/libsigfile.la \
	../libsigproc/libsigproc.a \
	$(FFTW3_LIBS) $(ITPP_LIBS) $(GTK_LIBS) $(GSL_LIBS) \
	$(OPENMP_LDADD) $(LIBFFTW3_LDADD)
