AM_CXXFLAGS = -Wall -std=c++0x $(AGHCORE_CFLAGS)

noinst_LIBRARIES = libexstrom.a

libexstrom_a_SOURCES = \
	signal.cc exstrom.hh signal.hh

BUILT_SOURCES = \
	exstrom.hh.gch \
	signal.hh.gch
%.hh.gch: %.hh
	$(CXXCOMPILE) -c $<
