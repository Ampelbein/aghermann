EXTRA_DIST = \
	aghermann.org.in \
	mc.org \
	model.org \
	usage.org

if DO_EMACS_ORGMODE_CONV
genhtml_DATA = \
	aghermann.html \
	mc.html \
	model.html \
	usage.html

%.html: %.org
	$(ORGMODE_EMACS) --batch -Q -l org $< -f org-export-as-html

genhtmldir = $(docdir)

img_DATA = \
	img/measurements.png \
	img/measurements-t.png \
	img/annotations.png \
	img/annotations-t.png \
	overview-single-episode-with-uc.png \
	overview-single-episode-with-uc-t.png \
	img/sf.png \
	img/sf-t.png \
	img/ica.png \
	img/ica-t.png \
	img/mf.png \
	img/mf-t.png \
	img/edfhed-gtk.png \
	img/edfhed-gtk-t.png

imgdir = $(docdir)/img

# don't take images into dist
# EXTRA_DIST += $(img_DATA)
endif

