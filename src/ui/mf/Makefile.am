AM_CXXFLAGS := \
	-std=c++0x -Wdeprecated-declarations -Wall \
	$(GTK_CFLAGS) $(LIBCONFIGXX_CFLAGS) $(UNIQUE_CFLAGS) \
	-DPACKAGE_DATADIR=\"$(datadir)\" \
	$(OPENMP_CXXFLAGS) \
	-I$(top_srcdir)/src

noinst_LIBRARIES := liba.a

liba_a_SOURCES := \
	mf.hh \
	mf_cb.hh \
	mf.cc \
	mf-construct.cc \
	mf_cb.cc

if DO_PCH
BUILT_SOURCES := \
	mf.hh.gch \
	mf_cb.hh.gch
%.hh.gch: %.hh
	$(CXXCOMPILE) -c $<
CLEANFILES := \
	$(BUILT_SOURCES)
endif
