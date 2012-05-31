doc_DATA = ../README ../ChangeLog

if DO_EMACS_ORGMODE_CONV
doc_DATA += aghermann.html

%.html: %.org
	$(ORGMODE_EMACS) --batch -Q -l org $< -f org-export-as-html
endif

EXTRA_DIST = $(doc_DATA)
