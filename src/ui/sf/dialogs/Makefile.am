AM_CXXFLAGS := \
	-std=c++11 -fno-rtti -Wdeprecated-declarations -Wall \
	$(GTK_CFLAGS)  $(UNIQUE_CFLAGS)  $(CAIRO_CFLAGS) $(LIBCONFIGXX_CFLAGS) \
	-DPACKAGE_DATADIR=\"$(datadir)\" \
	$(OPENMP_CXXFLAGS) \
	-I$(top_srcdir)/src

noinst_LIBRARIES := liba.a

liba_a_SOURCES := \
	artifacts.hh \
	artifacts.cc \
	artifacts-construct.cc \
	artifacts_cb.cc \
	filters.hh \
	filters.cc \
	filters-construct.cc \
	filters_cb.cc \
	patterns.hh \
	patterns.cc \
	patterns-construct.cc \
	patterns-draw.cc \
	patterns-enumerate.cc \
	patterns_cb.cc \
	phasediff.cc \
	phasediff-construct.cc \
	phasediff_cb.cc

if DO_PCH
BUILT_SOURCES = \
	artifacts.hh.gch \
	patterns.hh.gch \
	filters.hh.gch \
	phasediff.hh.gch
%.hh.gch: %.hh
	$(CXXCOMPILE) -c $<
CLEANFILES := \
	$(BUILT_SOURCES)
endif
