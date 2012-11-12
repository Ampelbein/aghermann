AM_CXXFLAGS = \
	-std=c++0x -Wdeprecated-declarations -Wall \
	$(GTK_CFLAGS) $(VTE_CFLAGS) $(CAIRO_CFLAGS) $(UNIQUE_CFLAGS) $(LIBCONFIGXX_CFLAGS) \
	-DPACKAGE_DATADIR=\"$(datadir)\" \
	$(OPENMP_CXXFLAGS) \
	-I$(top_srcdir)/src

noinst_LIBRARIES = liba.a

liba_a_SOURCES = \
	mw-admit-one.cc \
	mw-construct.cc \
	mw-loadsave.cc \
	mw-mainmenu_cb.cc \
	mw-measurements.cc \
	mw-measurements_cb.cc \
	mw-populate.cc \
	mw-settings_cb.cc \
	mw-simulations.cc \
	mw-simulations_cb.cc \
	mw-splash.cc \
	mw-widgets.hh \
	mw.cc \
	mw.hh \
	mw_cb.cc \
	mw_cb.hh

if DO_PCH
BUILT_SOURCES = \
	mw.hh.gch \
	mw-widgets.hh.gch \
	mw_cb.hh.gch
%.hh.gch: %.hh
	$(CXXCOMPILE) -c $<
CLEANFILES := \
	$(BUILT_SOURCES)
endif
