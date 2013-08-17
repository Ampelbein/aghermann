ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src data doc

EXTRA_DIST = \
	ChangeLog \
	autogen.sh

man_MANS = \
	man/aghermann.1 \
	man/edfcat.1 \
	man/edfhed.1 \
	man/edfhed-gtk.1

ebuild:
	cp gentoo/aghermann.ebuild aghermann-$(VERSION).ebuild
