doc_DATA = ../README ../ChangeLog

if DO_EMACS_ORGMODE_CONV
orgconv_ORGFILES = aghermann.org
orgconv_DATA = aghermann.html
orgconvdir = $(docdir)
doc_DATA += ${orgconv_DATA}

install-data-hook:
	for F in ${orgconv_ORGFILES}; do $(ORGMODE_EMACS) --batch -Q -l org $$F -f org-export-as-html; done
endif

EXTRA_DIST = $(doc_DATA)
