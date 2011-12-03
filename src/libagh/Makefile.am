AM_CXXFLAGS = \
	-Wall -std=c++0x \
	$(AGHCORE_CFLAGS)

lib_LTLIBRARIES = libaghcore.la

libaghcore_la_SOURCES = \
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

libaghcoreincdir = $(includedir)/libaghcore
libaghcoreinc_HEADERS = \
	forward-decls.hh \
	model.hh \
	primaries.hh \
	tunable.hh

BUILT_SOURCES = \
	forward-decls.hh.gch \
	primaries.hh.gch \
	model.hh.gch \
	tunable.hh.gch
%.hh.gch: %.hh
	$(CXXCOMPILE) -c $<

install-exec-hook:
	$(mkinstalldirs) $(DESTDIR)$(libaghcoreincdir)
