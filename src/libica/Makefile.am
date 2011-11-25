AM_CXXFLAGS = -Wall -std=c++0x \
	$(AGHCORE_CFLAGS) $(AGHICA_CFLAGS) $(OPENMP_CFLAGS) \
	-fno-show-column

# lib_LTLIBRARIES = libica.la

# libica_la_SOURCES = \
# 	ica.cc

libicaincdir = $(includedir)/libica
libicainc_HEADERS = \
	ica.hh

install-exec-hook:
	$(mkinstalldirs) $(DESTDIR)$(libaghcoreincdir)
