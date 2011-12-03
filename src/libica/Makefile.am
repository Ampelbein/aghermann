AM_CXXFLAGS = -Wall -std=c++0x \
	$(AGHCORE_CFLAGS) $(AGHICA_CFLAGS) $(OPENMP_CFLAGS)

# lib_LTLIBRARIES = libica.la

# libica_la_SOURCES = \
# 	ica.cc

libicaincdir = $(includedir)/libica
libicainc_HEADERS = \
	ica.hh
BUILT_SOURCES = \
	ica.hh.gch
%.hh.gch: %.hh
	$(CXXCOMPILE) -c $<


install-exec-hook:
	$(mkinstalldirs) $(DESTDIR)$(libaghcoreincdir)
