AM_CXXFLAGS := \
	-Wall -std=c++0x -fno-rtti \
	-I$(top_srcdir)/src

bin_PROGRAMS := \
	edfcat \
	edfhed \
	edfhed-gtk \
	agh-profile-gen

edfcat_SOURCES := \
	edfcat.cc
edfcat_LDADD := \
	../libsigfile/libsigfile.la \
	../common/liba.a \
	../libsigproc/libsigproc.la \
	$(FFTW3_LIBS) $(ITPP_LIBS) $(SAMPLERATE_LIBS) $(GSL_LIBS) \
	$(OPENMP_LDADD) $(LIBFFTW3_LDADD)

edfhed_SOURCES := \
	edfhed.cc
edfhed_LDADD := \
	../libsigfile/libsigfile.la \
	../common/liba.a \
	../libsigproc/libsigproc.la \
	$(FFTW3_LIBS) $(ITPP_LIBS) $(SAMPLERATE_LIBS) $(GSL_LIBS) \
	$(OPENMP_LDADD) $(LIBFFTW3_LDADD)

edfhed_gtk_SOURCES := \
	edfhed-gtk.cc
edfhed_gtk_CXXFLAGS := \
	$(AM_CXXFLAGS) 	$(GTK_CFLAGS) -DPACKAGE_DATADIR=\"$(datadir)\"
edfhed_gtk_LDADD := \
	../libsigfile/libsigfile.la \
	../common/liba.a \
	../libsigproc/libsigproc.la \
	$(FFTW3_LIBS) $(ITPP_LIBS) $(SAMPLERATE_LIBS) $(GSL_LIBS) \
	$(GTK_LIBS) \
	$(OPENMP_LDADD) $(LIBFFTW3_LDADD)


agh_profile_gen_SOURCES := \
	agh-profile-gen.cc
agh_profile_gen_LDADD := \
	../libsigfile/libsigfile.la \
	../common/liba.a \
	../libsigproc/libsigproc.la \
	../libmetrics/libmetrics.la \
	$(FFTW3_LIBS) $(ITPP_LIBS) $(SAMPLERATE_LIBS) $(GSL_LIBS) \
	$(OPENMP_LDADD) $(LIBFFTW3_LDADD)

