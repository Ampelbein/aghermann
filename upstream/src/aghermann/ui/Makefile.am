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
	globals.cc \
	globals.hh \
	misc.cc \
	misc.hh \
	ui++.cc \
	ui++.hh \
	ui.cc \
	ui.hh
