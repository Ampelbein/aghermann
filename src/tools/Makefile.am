AM_CXXFLAGS = -Wall -std=c++0x

bin_PROGRAMS = \
	edfcat \
	edfhed \
	edfhed-gtk

edfcat_SOURCES = \
	edfcat.cc
edfcat_LDADD = \
	../libsigfile/libsigfile.a \
	../sigproc/libsigproc.a \
	../common/libcommon.a \
	$(FFTW3_LIBS) $(ITPP_LIBS) $(SAMPLERATE_LIBS) $(GSL_LIBS) \
	$(OPENMP_LDADD) $(LIBFFTW3_LDADD)

edfhed_SOURCES = \
	edfhed.cc
edfhed_LDADD = \
	../libsigfile/libsigfile.a \
	../sigproc/libsigproc.a \
	../common/libcommon.a \
	$(FFTW3_LIBS) $(ITPP_LIBS) $(SAMPLERATE_LIBS) $(GSL_LIBS) \
	$(OPENMP_LDADD) $(LIBFFTW3_LDADD)

edfhed_gtk_SOURCES = \
	edfhed-gtk.cc
edfhed_gtk_CXXFLAGS = \
	$(AM_CXXFLAGS) 	$(GTK_CFLAGS) -DPACKAGE_DATADIR=\"$(datadir)\"
edfhed_gtk_LDADD = \
	../libsigfile/libsigfile.a \
	../sigproc/libsigproc.a \
	../common/libcommon.a \
	../ui/libaghui.a \
	$(FFTW3_LIBS) $(ITPP_LIBS) $(SAMPLERATE_LIBS) $(GTK_LIBS) $(GSL_LIBS) \
	$(OPENMP_LDADD) $(LIBFFTW3_LDADD)

