SUBDIRS := sm mw sf mf
AM_CXXFLAGS := \
	-std=c++0x -fno-rtti -Wdeprecated-declarations \
	$(GTK_CFLAGS) $(UNIQUE_CFLAGS) \
	-DPACKAGE_DATADIR=\"$(datadir)\" \
	$(OPENMP_CXXFLAGS) \
	-I$(top_srcdir)/src

noinst_LIBRARIES := liba.a

liba_a_SOURCES := \
	forward-decls.hh \
	globals.hh \
	libcommon.cc \
	misc.hh \
	ui++.hh \
	ui.hh

if DO_PCH
BUILT_SOURCES := \
	forward-decls.hh.gch \
	globals.hh.gch \
	misc.hh.gch \
	ui++.hh.gch \
	ui.hh.gch
%.hh.gch: %.hh
	$(CXXCOMPILE) -c $<
CLEANFILES = \
	$(BUILT_SOURCES)
endif
