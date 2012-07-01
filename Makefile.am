ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src data doc

EXTRA_DIST = \
	ChangeLog \
	autogen.sh \
	debian/changelog \
	debian/compat \
	debian/control \
	debian/copyright \
	debian/rules \
	debian/aghermann.sharedmimeinfo \
	debian/aghermann.manpages \
	debian/README.Debian \
	debian/source/format \
	debian/watch \
	gentoo/aghermann.ebuild

man_MANS = \
	man/aghermann.1 \
	man/edfcat.1 \
	man/edfhed.1 \
	man/edfhed-gtk.1

# dist-hook:
# 	mv $(distdir)/gentoo/aghermann.ebuild $(distdir)/gentoo/aghermann-@VERSION@.ebuild
