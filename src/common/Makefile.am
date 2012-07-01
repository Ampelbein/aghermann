AM_CXXFLAGS = \
	-Wall -std=c++0x

noinst_LIBRARIES = \
	libcommon.a

libcommon_a_SOURCES = \
	libcommon.cc \
	config-validate.hh \
	string.hh \
	misc.hh \
	fs.hh

if DO_PCH
BUILT_SOURCES = \
	config-validate.hh.gch \
	string.hh.gch \
	misc.hh.gch \
	fs.hh.gch

%.hh.gch: %.hh
	$(CXXCOMPILE) -c $<
endif

CLEANFILES = \
	*.gch
