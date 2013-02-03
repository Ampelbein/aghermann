AM_CXXFLAGS := \
	-Wall -std=c++0x -fno-rtti \
	$(OPENMP_CXXFLAGS) \
	-I$(top_srcdir)/src

noinst_LIBRARIES := liba.a

liba_a_SOURCES := \
	forward-decls.hh \
	loadsave.cc \
	tree-scanner.cc \
	primaries.cc \
	primaries.hh \
	recording.cc \
	recording.hh \
	profile.hh

if DO_PCH
BUILT_SOURCES := \
	forward-decls.hh.gch \
	profile.hh.gch \
	recording.hh.gch \
	primaries.hh.gch
%.hh.gch: %.hh
	$(CXXCOMPILE) -c $<
CLEANFILES := \
	$(BUILT_SOURCES)
endif
