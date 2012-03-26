SUBDIRS = \
	libsigproc libsigfile libica \
	core ui tools

AM_CXXFLAGS = \
	-Wall -std=c++0x \
	$(AGHUI_CFLAGS) \
	-DBUILT_BY=\"@user@\"

bin_PROGRAMS = aghermann

aghermann_SOURCES = \
	libica/ica.hh \
	main.cc \
	common/misc.hh \
	common/string.hh \
	common/fs.hh

aghermann_LDADD = \
	ui/libaghui.a \
	core/libaghcore.a \
	libsigproc/libsigproc.a \
	libsigfile/libsigfile.la \
	$(AGHCORE_LIBS) $(AGHSIGFILE_LIBS) $(AGHICA_LIBS) $(AGHUI_LIBS) \
	$(OPENMP_LDADD) $(LIBFFTW3_LDADD)


BUILT_SOURCES = \
	common/misc.hh.gch \
	common/string.hh.gch \
	common/fs.hh.gch \
	libica/ica.hh.gch
%.hh.gch: %.hh
	$(CXXCOMPILE) -c $<


CLEANFILES = */*.la */*.gch

dist-hook:
	rm -rf `find $(distdir) -name \*.a`
