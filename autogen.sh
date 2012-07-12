#!/bin/sh

test -d /usr/share/autoconf-archive && AUTOCONF_ARCHIVE="-I /usr/share/autoconf-archive"
libtoolize --force --copy
aclocal $AUTOCONF_ARCHIVE
autoheader
automake --gnu --add-missing --copy
autoconf
# or just use autoreconf?
