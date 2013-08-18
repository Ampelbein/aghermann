AM_CXXFLAGS := \
	-Wall -std=c++0x -fno-rtti \
	$(OPENMP_CXXFLAGS) \
	-I$(top_srcdir)/src

noinst_LIBRARIES := liba.a

liba_a_SOURCES := \
	forward-decls.hh \
	expdesign.hh \
	profile.hh \
	expdesign.cc \
	loadsave.cc \
	recording.cc \
	recording.hh \
	subject.cc \
	tree-scanner.cc

if DO_PCH
BUILT_SOURCES := \
	forward-decls.hh.gch \
	profile.hh.gch \
	recording.hh.gch \
	subject.hh.gch \
	expdesign.hh.gch
%.hh.gch: %.hh
	$(CXXCOMPILE) -c $<
CLEANFILES = \
	$(BUILT_SOURCES)
endif
