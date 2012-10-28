AM_CXXFLAGS := \
	-std=c++0x -Wdeprecated-declarations -Wall \
	$(GTK_CFLAGS) $(LIBCONFIGXX_CFLAGS) \
	-DPACKAGE_DATADIR=\"$(datadir)\" \
	$(OPENMP_CXXFLAGS) \
	-I$(top_srcdir)/src

noinst_LIBRARIES := liba.a

liba_a_SOURCES := \
	sm.cc \
	sm-construct.cc \
	sm.hh \
	sm_cb.cc \
	sm_cb.hh

if DO_PCH
BUILT_SOURCES := \
	sm.hh.gch \
	sm_cb.hh.gch
%.hh.gch: %.hh
	$(CXXCOMPILE) -c $<
CLEANFILES := \
	$(BUILT_SOURCES)
endif
