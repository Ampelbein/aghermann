SUBDIRS = \
	libexstrom libsigfile libica \
	core ui tools

AM_CXXFLAGS = \
	-std=c++0x \
	$(AGHUI_CFLAGS) \
	-DBUILT_BY=\"@user@\" \
	-I.

#AM_LIBTOOLFLAGS = --silent

bin_PROGRAMS = aghermann

aghermann_SOURCES = \
	libica/ica.hh \
	main.cc \
	print_version.cc \
	misc.hh

BUILT_SOURCES = \
	misc.hh.gch \
	libica/ica.hh.gch
%.hh.gch: %.hh
	$(CXXCOMPILE) -c $<

aghermann_LDADD = \
	ui/libaghui.a \
	core/libaghcore.a \
	libexstrom/libexstrom.a \
	libsigfile/libsigfile.la \
	$(AGHCORE_LIBS) $(AGHSIGFILE_LIBS) $(AGHICA_LIBS) $(AGHUI_LIBS) \
	$(OPENMP_LDADD) $(LIBFFTW3_LDADD) $(GSL_LIBS)

CLEANFILES = */*.la */*.gch

main.o:
	touch print_version.cc

dist-hook:
	rm -rf `find $(distdir) -name \*.a`
