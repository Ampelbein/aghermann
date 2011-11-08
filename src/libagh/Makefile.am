AM_CXXFLAGS = \
	-Wall -std=c++0x \
	$(AGHCORE_CFLAGS) $(OPENMP_CXXFLAGS) \
	-fno-show-column -Wsuggest-attribute=const -Wsuggest-attribute=pure -Wnoexcept
 #-Wunused-but-set-variable

lib_LTLIBRARIES = libaghcore.la

libaghcore_la_SOURCES = \
	forward-decls.hh \
	misc.hh \
	boost-config-validate.hh \
	primaries.cc \
	primaries-loadsave.cc \
	primaries.hh \
	page.cc \
	page.hh \
	source-slowwave.cc \
	psd.cc \
	psd.hh \
	scoring-assistant.cc \
	tunable.cc \
	tunable.hh \
	model.cc \
	model.hh \
	siman.cc

libaghcoreincdir = $(includedir)/libaghcore
libaghcoreinc_HEADERS = \
	forward-decls.hh \
	misc.hh \
	model.hh \
	page.hh \
	primaries.hh \
	psd.hh \
	tunable.hh

install-exec-hook:
	$(mkinstalldirs) $(DESTDIR)$(libaghcoreincdir)
