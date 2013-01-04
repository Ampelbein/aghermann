SUBDIRS := \
	common sigproc ica libsigfile \
	metrics \
	expdesign model \
	ui \
	tools

AM_CXXFLAGS := \
	-Wall -std=c++11 -fno-rtti \
	$(GTK_CFLAGS) $(UNIQUE_CFLAGS) \
	$(OPENMP_CXXFLAGS) \
	-DBUILT_BY=\"@user@\"

bin_PROGRAMS := aghermann

aghermann_SOURCES := \
	main.cc \
	ui/sm/sm.hh

aghermann_LDADD := \
	ui/sm/liba.a \
	ui/mw/liba.a \
	ui/sf/liba.a \
	ui/mf/liba.a \
	ui/liba.a \
	model/liba.a \
	metrics/liba.a \
	libsigfile/liba.a \
	sigproc/liba.a \
	common/liba.a \
	expdesign/liba.a \
	$(FFTW3_LIBS) $(ITPP_LIBS) $(SAMPLERATE_LIBS) $(GSL_LIBS) \
	$(GTK_LIBS) $(CAIRO_LIBS) $(UNIQUE_LIBS) $(VTE_LIBS) \
	$(LIBCONFIGXX_LIBS) \
	$(LIBFFTW3_LDADD) $(OPENMP_CXXFLAGS)

CLEANFILES := \
	*/*.la */*.gch

install-exec-hook:
	rm -f $(DESTDIR)/$(pkglibdir)/*.la
uninstall-hook:
	rm -f $(DESTDIR)/$(pkglibdir)/*.so

