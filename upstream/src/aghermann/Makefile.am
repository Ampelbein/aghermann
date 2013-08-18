SUBDIRS := \
	expdesign \
	patterns \
	rk1968 model ica \
	ui

AM_CXXFLAGS := \
	-Wall -std=c++0x -fno-rtti \
	$(GTK_CFLAGS) $(UNIQUE_CFLAGS) \
	$(OPENMP_CXXFLAGS) \
	-I$(top_srcdir)/src \
	-DBUILT_BY=\"@user@\"

bin_PROGRAMS := aghermann

aghermann_SOURCES := \
	print_version.cc \
	main.cc \
	globals.cc \
	globals.hh

print_version.o: FORCE
FORCE:

aghermann_LDADD := \
	ui/sm/liba.a \
	ui/mw/liba.a \
	ui/sf/liba.a \
	ui/mf/liba.a \
	ui/liba.a \
	model/liba.a \
	rk1968/liba.a \
	patterns/liba.a \
	../libmetrics/libmetrics.la \
	../libsigproc/libsigproc.la \
	../common/liba.a \
	expdesign/liba.a \
	../libsigfile/libsigfile.la \
	$(FFTW3_LIBS) $(ITPP_LIBS) $(SAMPLERATE_LIBS) $(GSL_LIBS) \
	$(GTK_LIBS) $(CAIRO_LIBS) $(UNIQUE_LIBS) $(VTE_LIBS) \
	$(LIBCONFIGXX_LIBS) \
	$(LIBFFTW3_LDADD) $(OPENMP_CXXFLAGS)

aghermann_LDFLAGS := \
	-shared

install-exec-hook:
	rm -f $(DESTDIR)/$(pkglibdir)/*.la
uninstall-hook:
	rm -f $(DESTDIR)/$(pkglibdir)/*.so

