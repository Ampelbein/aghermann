#pkgdata_DATA = ui/idle-bg.svg

applicationsdir = $(datadir)/applications
applications_DATA = ${PACKAGE}.desktop
uidir = $(pkgdatadir)/ui
ui_DATA = ui/agh-ui.glade ui/idle-bg.svg

EXTRA_DIST = ui/agh-ui.glade ui/idle-bg.svg ${PACKAGE}.desktop

install-exec-hook:
	$(mkinstalldirs) $(uidir) $(datadir)/applications
