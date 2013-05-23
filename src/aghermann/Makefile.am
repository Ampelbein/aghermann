SUBDIRS := \
	ica \
	metrics \
	expdesign \
	patterns \
	rk1968 model \
	ui

AM_CXXFLAGS := \
	-Wall -std=c++0x -fno-rtti \
	$(GTK_CFLAGS) $(UNIQUE_CFLAGS) \
	$(OPENMP_CXXFLAGS) \
	-I$(top_srcdir)/src \
	-DBUILT_BY=\"@user@\"

bin_PROGRAMS := aghermann

aghermann_SOURCES := \
	main.cc \
	ui/sm/sm.hh

aghermann_LDADD := \
	ui/sm/liba.a \
	ui/mw/liba.a \
	ui/sf/liba.a \
	ui/sf/d/liba.a \
	ui/mf/liba.a \
	ui/liba.a \
	model/liba.a \
	rk1968/liba.a \
	patterns/liba.a \
	metrics/liba.a \
	../libsigfile/liba.a \
	../sigproc/liba.a \
	../common/liba.a \
	expdesign/liba.a \
	$(FFTW3_LIBS) $(ITPP_LIBS) $(SAMPLERATE_LIBS) $(GSL_LIBS) \
	$(GTK_LIBS) $(CAIRO_LIBS) $(UNIQUE_LIBS) $(VTE_LIBS) \
	$(LIBCONFIGXX_LIBS) \
	$(LIBFFTW3_LDADD) $(OPENMP_CXXFLAGS)

install-exec-hook:
	rm -f $(DESTDIR)/$(pkglibdir)/*.la
uninstall-hook:
	rm -f $(DESTDIR)/$(pkglibdir)/*.so

