SUBDIRS := \
	common \
	libsigproc libsigfile libmetrics \
	aghermann \
	tools

# by trial and error, the _CFLAGS mentioned are the minimum required to pre-build all headers
AM_CXXFLAGS := \
	-Wall -std=c++0x -fno-rtti \
	$(GTK_CFLAGS)  $(UNIQUE_CFLAGS) \
	$(OPENMP_CXXFLAGS) \
	-I$(top_srcdir)/src -I.. \
	-DHAVE_CONFIG_H \
	-DBUILT_BY=\"@user@\"

if DO_PCH
BUILT_SOURCES = \
	common/string.hh.gch \
	common/alg.hh.gch \
	common/fs.hh.gch \
	common/subject_id.hh.gch \
	common/lang.hh.gch \
	common/config-validate.hh.gch \
	common/containers.hh.gch \
	libsigproc/sigproc.hh.gch \
	libsigproc/winfun.hh.gch \
	libsigproc/exstrom.hh.gch \
	libsigproc/ext-filters.hh.gch \
	libsigfile/edf.hh.gch \
	libsigfile/page.hh.gch \
	libsigfile/channel.hh.gch \
	libsigfile/typed-source.hh.gch \
	libsigfile/tsv.hh.gch \
	libsigfile/source-base.hh.gch \
	libsigfile/all.hh.gch \
	libsigfile/forward-decls.hh.gch \
	libmetrics/mc-artifacts.hh.gch \
	libmetrics/mc.hh.gch \
	libmetrics/psd.hh.gch \
	libmetrics/swu.hh.gch \
	libmetrics/all.hh.gch \
	libmetrics/page-metrics-base.hh.gch \
	libmetrics/bands.hh.gch \
	libmetrics/forward-decls.hh.gch \
	aghermann/model/achermann-tunable.hh.gch \
	aghermann/model/achermann.hh.gch \
	aghermann/model/beersma.hh.gch \
	aghermann/model/forward-decls.hh.gch \
	aghermann/expdesign/subject.hh.gch \
	aghermann/expdesign/recording.hh.gch \
	aghermann/expdesign/profile.hh.gch \
	aghermann/expdesign/expdesign.hh.gch \
	aghermann/expdesign/forward-decls.hh.gch \
	aghermann/rk1968/rk1968.hh.gch \
	aghermann/globals.hh.gch \
	aghermann/patterns/patterns.hh.gch \
	aghermann/ica/ica.hh.gch \
	aghermann/ui/mf/mf.hh.gch \
	aghermann/ui/mf/mf_cb.hh.gch \
	aghermann/ui/ui.hh.gch \
	aghermann/ui/misc.hh.gch \
	aghermann/ui/sf/d/artifacts-simple.hh.gch \
	aghermann/ui/sf/d/filters.hh.gch \
	aghermann/ui/sf/d/artifacts.hh.gch \
	aghermann/ui/sf/d/patterns.hh.gch \
	aghermann/ui/sf/d/phasediff.hh.gch \
	aghermann/ui/sf/sf_cb.hh.gch \
	aghermann/ui/sf/sf.hh.gch \
	aghermann/ui/sf/widgets.hh.gch \
	aghermann/ui/globals.hh.gch \
	aghermann/ui/sm/sm_cb.hh.gch \
	aghermann/ui/sm/sm.hh.gch \
	aghermann/ui/mw/mw.hh.gch \
	aghermann/ui/mw/mw_cb.hh.gch \
	aghermann/ui/mw/widgets.hh.gch \
	aghermann/ui/ui++.hh.gch \
	aghermann/ui/forward-decls.hh.gch

%.hh.gch: %.hh
# for some reason $(CXXCOMPILE) is just... "c", whereas when seen in
# any sub/Makefile.am, it does the trick alright, so spell it out in full
	$(CXX) $(AM_CXXFLAGS) -c $<

CLEANFILES = \
	$(BUILT_SOURCES)

endif
