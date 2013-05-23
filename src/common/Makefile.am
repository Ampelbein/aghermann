AM_CXXFLAGS := \
	-Wall -std=c++0x -fno-rtti \
	-I$(top_srcdir)/src

noinst_LIBRARIES := \
	liba.a

liba_a_SOURCES := \
	libcommon.cc \
	subject_id.cc \
	config-validate.hh \
	string.hh \
	globals.hh \
	alg.hh \
	fs.hh \
	lang.hh \
	subject_id.hh

CLEANFILES := \
	*.la

if DO_PCH
BUILT_SOURCES := \
	config-validate.hh.gch \
	string.hh.gch \
	globals.hh.gch \
	alg.hh.gch \
	containers.hh.gch \
	fs.hh.gch \
	subject_id.hh.gch

%.hh.gch: %.hh
	$(CXXCOMPILE) -c $<
CLEANFILES += \
	$(BUILT_SOURCES)
endif
