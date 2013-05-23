AM_CXXFLAGS := \
	-Wall -std=c++0x -fno-rtti \
	-I$(top_srcdir)/src

noinst_LIBRARIES := liba.a

liba_a_SOURCES := \
	patterns.cc patterns.hh patterns.ii

CLEANFILES := \
	liba.la

if DO_PCH
BUILT_SOURCES := \
	patterns.hh.gch
%.hh.gch: %.hh
	$(CXXCOMPILE) -c $<
CLEANFILES += \
	$(BUILT_SOURCES)
endif

EXTRA_DIST := \
	patterns.ii
