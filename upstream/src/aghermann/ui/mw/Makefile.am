AM_CXXFLAGS = \
	-std=c++0x -fno-rtti -Wdeprecated-declarations -Wall \
	$(GTK_CFLAGS) $(VTE_CFLAGS) $(CAIRO_CFLAGS) $(UNIQUE_CFLAGS) $(LIBCONFIGXX_CFLAGS) \
	-DPACKAGE_DATADIR=\"$(datadir)\" \
	$(OPENMP_CXXFLAGS) \
	-I$(top_srcdir)/src

noinst_LIBRARIES = liba.a

liba_a_SOURCES = \
	admit-one.cc \
	admit-one_cb.cc \
	construct.cc \
	loadsave.cc \
	mainmenu_cb.cc \
	measurements.cc \
	measurements_cb.cc \
	populate.cc \
	settings_cb.cc \
	simulations.cc \
	simulations_cb.cc \
	splash.cc \
	widgets.hh \
	mw.cc \
	mw.hh \
	mw_cb.cc \
	mw_cb.hh
