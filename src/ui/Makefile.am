SUBDIRS := sm mw sf mf
AM_CXXFLAGS := \
	-std=c++0x -Wdeprecated-declarations \
	$(GTK_CFLAGS) $(LIBCONFIGXX_CFLAGS) \
	-DPACKAGE_DATADIR=\"$(datadir)\" \
	$(OPENMP_CXXFLAGS) \
	-I..

noinst_LIBRARIES := liba.a

liba_a_SOURCES = \
	forward-decls.hh \
	ui.hh \
	ui++.hh \
	misc.hh \
	globals.hh \
	libcommon.cc

if DO_PCH
BUILT_SOURCES := \
	ui.hh.gch \
	ui++.hh.gch \
	misc.hh.gch \
	globals.hh.gch \
	forward-decls.hh.gch
%.hh.gch: %.hh
	$(CXXCOMPILE) -c $<
CLEANFILES := \
	$(BUILT_SOURCES)
endif
