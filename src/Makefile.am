SUBDIRS := \
	common sigproc ica libsigfile \
	metrics model \
	expdesign \
	ui \
	tools

AM_CXXFLAGS := \
	-Wall -std=c++0x \
	$(GTK_CFLAGS) $(UNIQUE_CFLAGS) \
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
	expdesign/liba.a \
	metrics/liba.a \
	libsigfile/libsigfile.a \
	sigproc/liba.a \
	common/liba.a \
	$(FFTW3_LIBS) $(ITPP_LIBS) $(SAMPLERATE_LIBS) $(GSL_LIBS) \
	$(GTK_LIBS) $(CAIRO_LIBS) $(UNIQUE_LIBS) $(VTE_LIBS) \
	$(LIBCONFIGXX_LIBS) \
	$(OPENMP_LDADD) $(LIBFFTW3_LDADD)

CLEANFILES := \
	*/*.la */*.gch

install-exec-hook:
	rm -f $(DESTDIR)/$(pkglibdir)/*.la
uninstall-hook:
	rm -f $(DESTDIR)/$(pkglibdir)/*.so

