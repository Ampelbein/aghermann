EXTRA_DIST = \
	aghermann.org.in \
	usage.org \
	model.org \
	mc.org \
	swu.org

if DO_EMACS_ORGMODE_CONV
genhtml_DATA = \
	aghermann.html \
	usage.html \
	model.html \
	mc.html \
	swu.html

%.html: %.org
	$(ORGMODE_EMACS) --batch -Q -l org $< -f org-export-as-html

genhtmldir = $(docdir)

img_DATA = \
	img/measurements.png \
	img/measurements-t.png \
	img/annotations.png \
	img/annotations-t.png \
	img/overview-single-episode-with-uc.png \
	img/overview-single-episode-with-uc-t.png \
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

