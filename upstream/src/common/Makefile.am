AM_CXXFLAGS := \
	-Wall -std=c++0x -fno-rtti \
	-I$(top_srcdir)/src

noinst_LIBRARIES := \
	liba.a

liba_a_SOURCES := \
	../libsigfile/source-base.hh \
	alg.hh \
	config-validate.hh \
	containers.hh \
	fs.hh \
	lang.hh \
	libcommon.cc \
	string.hh \
	subject_id.cc \
	subject_id.hh

if DO_PCH
BUILT_SOURCES := \
	config-validate.hh.gch \
	string.hh.gch \
	alg.hh.gch \
	containers.hh.gch \
	fs.hh.gch \
	subject_id.hh.gch

%.hh.gch: %.hh
	$(CXXCOMPILE) -c $<
CLEANFILES = \
	$(BUILT_SOURCES)
endif
