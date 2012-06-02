doc_DATA = \
	aghermann.html \
	mc.html \
	model.html \
	usage.html

%.html: %.org
	$(ORGMODE_EMACS) --batch -Q -l org $< -f org-export-as-html

imgdir = $(docdir)/img

img_DATA = \
	img/measurements.png \
	img/measurements-t.png \
	img/annotations.png \
	img/annotations-t.png \
	img/sf.png \
	img/sf-t.png \
	img/ica.png \
	img/ica-t.png \
	img/mf.png \
	img/mf-t.png \
	img/edfhed-gtk.png \
	img/edfhed-gtk-t.png

EXTRA_DIST = $(doc_DATA) $(img_DATA)
