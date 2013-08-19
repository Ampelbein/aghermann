AM_CXXFLAGS := \
	-std=c++0x -fno-rtti -Wdeprecated-declarations -Wall \
	$(GTK_CFLAGS)  $(UNIQUE_CFLAGS) \
	-DPACKAGE_DATADIR=\"$(datadir)\" \
	$(OPENMP_CXXFLAGS) \
	-I$(top_srcdir)/src

noinst_LIBRARIES := liba.a

liba_a_SOURCES := \
	sm-construct.cc \
	sm.cc \
	sm.hh \
	sm_cb.cc \
	sm_cb.hh

CLEANFILES := \
	liba.la
