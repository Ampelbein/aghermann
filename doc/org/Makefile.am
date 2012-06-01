doc_DATA = \
	aghermann.html \
	mc.html \
	model.html \
	usage.html

%.html: %.org
	$(ORGMODE_EMACS) --batch -Q -l org $< -f org-export-as-html

EXTRA_DIST = $(doc_DATA)
