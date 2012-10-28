applicationsdir = $(datadir)/applications
applications_DATA = \
	aghermann.desktop edfhed-gtk.desktop

uidir = $(datadir)/${PACKAGE}

ui_DATA := \
	aghermann.png \
	edfhed.glade \
	aghermann.gresource \
	idle-bg.svg

CLEANFILES := \
	aghermann.desktop edfhed-gtk.desktop

EXTRA_DIST := \
	aghermann.png \
	aghermann.desktop.in edfhed-gtk.desktop.in \
	icon.svg \
	aghermann.gresource.xml \
	session-chooser.glade main.glade dialogs.glade \
	sf.glade mf.glade edfhed.glade \
	idle-bg.svg equations1.png equation-uc.png

GRESOURCES := \
	session-chooser.glade main.glade dialogs.glade \
	sf.glade mf.glade edfhed.glade \
	aghermann.png equations1.png equation-uc.png

aghermann.gresource: aghermann.gresource.xml $(GRESOURCES)
	$(glib_compile_resources) aghermann.gresource.xml

#update_desktop_database = update-desktop-database $(datadir)/applications

install-data-hook:
	@if [ x$enable_update_desktop_database = xyes ]; then	\
		$(mkinstalldirs) $(uidir); 			\
	else							\
		echo "*** Desktop database not updated";	\
	fi
