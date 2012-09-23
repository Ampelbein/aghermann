applicationsdir = $(datadir)/applications
applications_DATA = \
	aghermann.desktop edfhed-gtk.desktop

uidir = $(datadir)/${PACKAGE}

ui_DATA = \
	ui/edfhed.glade \
	ui/aghermann.gresource

CLEANFILES = \
	aghermann.desktop edfhed-gtk.desktop \
	ui/dialogs.glade \
	ui/aghermann.gresource

EXTRA_DIST = \
	aghermann.desktop.in edfhed-gtk.desktop.in \
	ui/aghermann.gresource.xml \
	ui/session-chooser.glade ui/main.glade ui/dialogs.glade.in \
	ui/sf.glade ui/mf.glade ui/edfhed.glade \
	ui/idle-bg.svg ui/aghermann.png ui/equations1.png ui/equation-uc.png


GRESOURCES := \
	ui/session-chooser.glade ui/main.glade ui/dialogs.glade \
	ui/sf.glade ui/mf.glade ui/edfhed.glade \
	ui/idle-bg.svg ui/aghermann.png ui/equations1.png ui/equation-uc.png

%.gresource: %.gresource.xml $(GRESOURCES)
	$(glib_compile_resources) $<

#update_desktop_database = update-desktop-database $(datadir)/applications

install-data-hook:
	@if [ x$enable_update_desktop_database = xyes ]; then	\
		$(mkinstalldirs) $(uidir); 			\
	else							\
		echo "*** Desktop database not updated";	\
	fi
