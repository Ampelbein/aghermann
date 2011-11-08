AM_CXXFLAGS = \
	-Wall -std=c++0x \
	$(AGHCORE_CFLAGS) \
	-fno-show-column -Wsuggest-attribute=const -Wsuggest-attribute=pure -Wnoexcept

lib_LTLIBRARIES = libedf.la

libedf_la_SOURCES = \
	edf.cc \
	edf.hh

libedfincdir = $(includedir)/libedf
libedfinc_HEADERS = \
	edf.hh

install-exec-hook:
	$(mkinstalldirs) $(DESTDIR)$(libaghcoreincdir)
