AM_CXXFLAGS = \
	-Wall -std=c++0x \
	$(AGHCORE_CFLAGS) \
	-fno-show-column -Wsuggest-attribute=const -Wsuggest-attribute=pure -Wnoexcept
 #-Wunused-but-set-variable

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

install-exec-hook:
	$(mkinstalldirs) $(DESTDIR)$(libaghcoreincdir)
