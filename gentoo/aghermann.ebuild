# Copyright 1999-2013 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

EAPI="2"
inherit eutils

DESCRIPTION="A sleep-research experiment manager, EDF viewer & Achermann's Process S model runner"
HOMEPAGE="http://johnhommer.com/academic/code/aghermann"
SRC_URI="http://johnhommer.com/academic/code/aghermann/source/${PN}-${PV}.tar.bz2"

LICENSE="GPL-2+"
SLOT="0"
KEYWORDS="~amd64 ~x86"

DEPEND="x11-libs/gtk+-3.0
	dev-libs/libconfig
	dev-libs/libunique
	sci-libs/gsl
	sci-libs/itpp
	sci-libs/fftw
	dev-libs/libconfig
	dev-libs/libxml2
	media-libs/samplerate
	x11-libs/vte"

src_install() {
	emake DESTDIR="${D}" install || die "make install failed"
	dodoc AUTHORS ChangeLog* README || die "dodoc failed"
}
