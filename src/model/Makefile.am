AM_CXXFLAGS = \
	-Wall -std=c++0x

noinst_LIBRARIES = libmodel.a

libmodel_a_SOURCES = \
	tunable.cc \
	tunable.hh \
	achermann.cc \
	achermann.hh \
	borbely.cc \
	borbely.hh \
	siman.cc \
	forward-decls.hh

if DO_PCH
BUILT_SOURCES = \
	achermann.hh.gch \
	borbely.hh.gch \
	tunable.hh.gch \
	forward-decls.hh.gch
%.hh.gch: %.hh
	$(CXXCOMPILE) -c $<
endif

CLEANFILES = \
	*.gch
