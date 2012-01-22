#!/bin/sh

test -d /usr/share/autoconf-archive && AUTOCONF_ARCHIVE="-I /usr/share/autoconf-archive"
aclocal $AUTOCONF_ARCHIVE
grep "AC_PROG_LIBTOOL" configure.in && libtoolize --force --copy
autoheader
automake --gnu --add-missing --copy
autoconf
