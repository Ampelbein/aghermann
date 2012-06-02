SUBDIRS = ui

applicationsdir = $(datadir)/applications
applications_DATA = \
	aghermann.desktop edfhed-gtk.desktop

EXTRA_DIST = \
	aghermann.desktop.in edfhed-gtk.desktop.in

CLEANFILES = \
	aghermann.desktop edfhed-gtk.desktop

#update_desktop_database = update-desktop-database $(datadir)/applications

install-data-hook:
	if [ x$enable_update_desktop_database = xyes ]; then	\
		$(mkinstalldirs) $(uidir); 				\
	else								\
		echo "*** Desktop database not updated";		\
	fi
