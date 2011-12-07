AM_CXXFLAGS = \
	-Wall -std=c++0x \
	$(AGHCORE_CFLAGS)

noinst_LIBRARIES = libaghcore.a

libaghcore_a_SOURCES = \
	forward-decls.hh \
	boost-config-validate.hh \
	primaries.cc \
	primaries-loadsave.cc \
	primaries.hh \
	assisted-score.cc \
	tunable.cc \
	tunable.hh \
	model.cc \
	model.hh \
	siman.cc

BUILT_SOURCES = \
	forward-decls.hh.gch \
	primaries.hh.gch \
	model.hh.gch \
	tunable.hh.gch
%.hh.gch: %.hh
	$(CXXCOMPILE) -c $<
