SUBDIRS := \
	common sigproc libsigfile ica \
	expdesign model \
	ui tools

AM_CXXFLAGS := \
	-Wall -std=c++0x \
	$(GTK_CFLAGS) $(OPENMP_CXXFLAGS) \
	-DBUILT_BY=\"@user@\"

bin_PROGRAMS := aghermann

aghermann_SOURCES := \
	main.cc \
	ui/expdesign.hh

aghermann_LDADD := \
	ui/sm/liba.a \
	ui/mw/liba.a \
	ui/sf/liba.a \
	ui/mf/liba.a \
	ui/liba.a \
	model/libmodel.a \
	expdesign/libexpdesign.a \
	libsigfile/libsigfile.a \
	sigproc/libsigproc.a \
	common/libcommon.a \
	$(FFTW3_LIBS) $(ITPP_LIBS) $(SAMPLERATE_LIBS) $(GTK_LIBS) $(GSL_LIBS) $(LIBCONFIGXX_LIBS) \
	$(OPENMP_LDADD) $(LIBFFTW3_LDADD)

CLEANFILES := \
	*/*.la */*.gch

install-exec-hook:
	rm -f $(DESTDIR)/$(pkglibdir)/*.la
uninstall-hook:
	rm -f $(DESTDIR)/$(pkglibdir)/*.so

