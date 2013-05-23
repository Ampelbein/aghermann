AM_CXXFLAGS := \
	-Wall -std=c++0x -fno-rtti \
	-I$(top_srcdir)/src

noinst_LIBRARIES := liba.a

liba_a_SOURCES := \
	rk1968.cc \
	rk1968.hh

CLEANFILES := \
	liba.la

if DO_PCH
BUILT_SOURCES := \
	rk1968.hh.gch
%.hh.gch: %.hh
	$(CXXCOMPILE) -c $<
CLEANFILES += \
	$(BUILT_SOURCES)
endif
