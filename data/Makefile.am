applicationsdir = $(datadir)/applications
applications_DATA = aghermann.desktop edfhed-gtk.desktop
uidir = $(datadir)/${PACKAGE}/ui

ui_DATA = ui/agh-ui-main.glade ui/agh-ui-sf.glade ui/agh-ui-mf.glade ui/edf-header-editor.glade \
	ui/idle-bg.svg ui/aghermann.png ui/equations1.png

EXTRA_DIST = ui/agh-ui-main.glade ui/agh-ui-sf.glade ui/agh-ui-mf.glade ui/edf-header-editor.glade \
	ui/idle-bg.svg ui/aghermann.png ui/equations1.png \
	aghermann.desktop edfhed-gtk.desktop

#update_desktop_database = update-desktop-database $(datadir)/applications

install-data-hook:
	if [ x$enable_update_desktop_database = xyes ]; then	\
		$(mkinstalldirs) $(uidir); 				\
	else								\
		echo "*** Desktop database not updated";		\
	fi
