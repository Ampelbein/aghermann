SUBDIRS = \
	libexstrom libsigfile libagh libica \
	tools

AM_CXXFLAGS = \
	-std=c++0x \
	$(AGHUI_CFLAGS) \
	-DPACKAGE_DATADIR=\"$(datadir)\" -DBUILT_BY=\"@user@\" \
	-I.

bin_PROGRAMS = aghermann

aghermann_SOURCES = \
	libsigfile/libsigfile.la \
	libagh/libaghcore.a \
	libexstrom/libexstrom.a \
	libica/ica.hh \
	ui/forward-decls.hh \
	ui/misc.cc ui/misc.hh \
	ui/managed-colour.hh \
	ui/draw-signal-generic.hh \
	ui/draw-signal-generic.cc \
	ui/ui.cc ui/ui.hh \
	ui/expdesign.hh \
	ui/expdesign.cc \
	ui/expdesign_cb.cc \
	ui/expdesign-construct.cc \
	ui/expdesign-measurements.cc \
	ui/expdesign-measurements_cb.cc \
	ui/expdesign-loadsave.cc \
	ui/expdesign-admit-one.cc \
	ui/expdesign-settings_cb.cc \
	ui/expdesign-simulations.cc \
	ui/expdesign-simulations_cb.cc \
	ui/expdesign-chooser.cc \
	ui/expdesign-chooser_cb.cc \
	ui/scoring-facility.hh \
	ui/scoring-facility.cc \
	ui/scoring-facility-ica.cc \
	ui/scoring-facility-ica_cb.cc \
	ui/scoring-facility_cb.cc \
	ui/scoring-facility-montage.cc \
	ui/scoring-facility-montage_cb.cc \
	ui/scoring-facility-hypnogram.cc \
	ui/scoring-facility-patterns.cc \
	ui/scoring-facility-patterns_cb.cc \
	ui/scoring-facility-filter.cc \
	ui/scoring-facility-phasediff.cc \
	ui/modelrun-facility.hh \
	ui/modelrun-facility.cc \
	ui/modelrun-facility-construct.cc \
	ui/modelrun-facility_cb.cc \
	main.cc \
	misc.hh

BUILT_SOURCES = \
	ui/forward-decls.hh.gch \
	ui/misc.hh.gch \
	ui/managed-colour.hh.gch \
	ui/draw-signal-generic.hh.gch \
	ui/ui.hh.gch \
	ui/expdesign.hh.gch \
	ui/scoring-facility.hh.gch \
	ui/modelrun-facility.hh.gch \
	misc.hh.gch \
	libica/ica.hh.gch
%.hh.gch: %.hh
	$(CXXCOMPILE) -c $<

aghermann_LDADD = \
	libagh/libaghcore.a \
	libexstrom/libexstrom.a \
	libsigfile/libsigfile.la \
	$(AGHCORE_LIBS) $(AGHSIGFILE_LIBS) $(AGHICA_LIBS) $(AGHUI_LIBS) \
	$(OPENMP_LDADD) $(LIBFFTW3_LDADD) $(GSL_LIBS)
dist-hook:
	rm -rf `find $(distdir) -name \*.a`
