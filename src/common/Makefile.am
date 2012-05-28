AM_CXXFLAGS = \
	-Wall -std=c++0x

# noinst_LIBRARIES = libcommon.a

# libcommon_a_SOURCES = \
# 	config-validate.hh \
# 	config-validate.cc

BUILT_SOURCES = \
	misc.hh.gch \
	string.hh.gch \
	fs.hh.gch

CLEANFILES = *.gch

%.hh.gch: %.hh
	$(CXXCOMPILE) -c $<
