AM_CXXFLAGS = \
	-std=c++0x -Wdeprecated-declarations -Wall \
	$(GTK_CFLAGS) $(LIBCONFIGXX_CFLAGS) \
	-DPACKAGE_DATADIR=\"$(datadir)\" \
	$(OPENMP_CXXFLAGS) \
	-I../..

noinst_LIBRARIES = liba.a

liba_a_SOURCES = \
	mw.hh \
	mw-widgets.hh \
	mw_cb.hh \
	mw.cc \
	mw_cb.cc \
	mw-construct.cc \
	mw-populate.cc \
	mw-measurements.cc \
	mw-measurements_cb.cc \
	mw-loadsave.cc \
	mw-admit-one.cc \
	mw-settings_cb.cc \
	mw-simulations.cc \
	mw-simulations_cb.cc

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
