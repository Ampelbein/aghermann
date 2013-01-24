AC_COPYRIGHT([Copyright (c) 2008-12 Andrei Zavada <johnhommer@gmail.com>])

AC_INIT([Aghermann], [0.8_rc], [johnhommer@gmail.com])
AC_CONFIG_SRCDIR([src/main.cc])
AC_CONFIG_MACRO_DIR([m4])
AC_PREREQ(2.61)

AM_INIT_AUTOMAKE([-Wno-portability, silent-rules])
dnl AM_SILENT_RULES([yes])
AC_CONFIG_HEADERS([config.h])

AC_PROG_LD()
AM_DISABLE_STATIC()
LT_INIT()

AC_PROG_CXX([$CXX])
AC_LANG([C++])
AC_OPENMP()

dnl Check for c++11 features
AC_DEFUN([AC_CXX_STDCPP11_FEATURES],
[AC_CACHE_CHECK(whether $CXX has all required c++11 features,
ac_cv_cxx_cpp11_features,
[AC_LANG([C++])
 old_CXX=$CXX
 CXX="$CXX -std=c++11"
 AC_TRY_RUN([
    #include <vector>
    #include <algorithm>
    using namespace std;
    enum class fafa : int { ke, pi };
    int main (int , char **)
    {
      void* p = nullptr;
      vector<vector<int>> v {{1, 2}};
      for ( auto& i : v )
      	;
      unsigned c = count_if( v.begin(), v.end(), [&] (vector<int> &i) { return i == vector<int> ({2,3}); });
      return 0;
    }
 ],
 ac_cv_cxx_cpp11_features=yes, ac_cv_cxx_cpp11_features=no,
 ac_cv_cxx_cpp11_features=yes)
 CXX=$old_CXX
])
])
AC_CXX_STDCPP11_FEATURES()
test $ac_cv_cxx_cpp11_features = no && \
   AC_MSG_ERROR([g++ >= 4.6 is required to build $PACKAGE as we must use -std=c++11 features your compiler doesn't seem to support], 1)

dnl AC_CXX_HAVE_STL # redundant: we check for STL implicitly in AC_CXX_STDCPP0X_FEATURES

dnl Check for some functions
AC_CHECK_FUNC( [mremap], [], AC_MSG_ERROR([Function mremap not available]))
dnl Check for prerequisite lib dependencies
PKG_CHECK_MODULES([LIBCONFIGXX], [libconfig++])
PKG_CHECK_MODULES([SAMPLERATE], [samplerate >= 0.1.7])
PKG_CHECK_MODULES([GSL], [gsl])
PKG_CHECK_MODULES([FFTW3], [fftw3])
PKG_CHECK_MODULES([ITPP], [itpp])
PKG_CHECK_MODULES([GTK], [gtk+-3.0 >= 3.4 glib-2.0 gthread-2.0 gio-unix-2.0 gmodule-export-2.0])
PKG_CHECK_MODULES([CAIRO], [cairo >= 1.8.10])
PKG_CHECK_MODULES([UNIQUE], [unique-3.0])
PKG_CHECK_MODULES([VTE], [vte-2.90])

AC_CHECK_LIB(
	fftw3_omp, fftw_init_threads,
	[AC_DEFINE([HAVE_LIBFFTW3_OMP], [], [we have fftw3 omp-enabled])
	 AC_SUBST(fftw3_ldadd, ["-lfftw3_omp -lpthread"])
	 LIBFFTW3_LDADD="-lfftw3_omp -lpthread"
	 omp_enabled_fftw3=yes
	],
	[AC_SUBST(fftw3_ldadd, [""])
	 LIBFFTW3_LDADD=""
	 omp_enabled_fftw3=no
	],
	[-lpthread -lfftw3_omp -lfftw3])
AC_SUBST(LIBFFTW3_LDADD, $LIBFFTW3_LDADD)


AC_PATH_PROGS([glib_compile_resources], [glib-compile-resources], [:])
test x$glib_compile_resources = x":" && \
     AC_MSG_ERROR([Huh. Isn't glib-compile-resources not part of glib?])

dnl
dnl Own package options
dnl
dnl 1. Main floating-point type
AC_ARG_WITH(
	[fp-type],
	[AS_HELP_STRING( [--with-fp-type=float|double], [use this as main floating-point type (default = float)])],
	[case "$with_fp_type" in
		float)
		       ;;
		double)
		       ;;
		*) AC_MSG_ERROR( [--with-fp-type accepts only `float' or `double' as argument],)
	  esac
	  AC_DEFINE_UNQUOTED( [TFloat], [$with_fp_type], [Main floating-point type])
	  main_fp_type=$with_fp_type
	 ],
	 [main_fp_type=float
	 AC_DEFINE( [TFloat], [float], [Main floating-point type])])


dnl 2. Whether to build local documentation
AC_ARG_ENABLE(
	[docs],
	[AS_HELP_STRING( [--enable-docs], [build html docs from emacs org files (default = no)])],
	[AS_IF([test "x$enableval" = xyes],
		[AC_CACHE_CHECK([for org-mode-enabled emacs], [ac_cv_path_ORGMODE_EMACS],
			[AC_PATH_PROGS_FEATURE_CHECK(
				[ORGMODE_EMACS],
				[emacs],
				[[$ac_path_ORGMODE_EMACS --batch -Q -l org && \
					ac_cv_path_ORGMODE_EMACS=$ac_path_ORGMODE_EMACS \
					build_html_docs=yes \
					ac_path_ORGMODE_EMACS_found=:]])])
		],
		[build_html_docs=no])
	],
	[build_html_docs=no])

dnl 3. Whether to precompile headers
AC_ARG_ENABLE(
	[pch],
	[AS_HELP_STRING( [--enable-pch], [precompile headers (default = no)])],
	[do_pch=$enable_pch],
	[do_pch=no])
dnl defaulting to no to enable make dist-check
AM_CONDITIONAL(DO_PCH, test x$do_pch = xyes)


AC_SUBST([ORGMODE_EMACS], [$ac_cv_path_ORGMODE_EMACS])
AM_CONDITIONAL(DO_EMACS_ORGMODE_CONV, test x$ac_cv_path_ORGMODE_EMACS != x)

dnl 3. Desktop and menu integration
dnl    (usually nicely handled by distro-specific package builder tools (debuild))
AC_ARG_WITH(xdgdatadir,
	AC_HELP_STRING([--with-xdgdatadir=path],
		[Change where the theme icons and mime registrations are installed]),
	[opt_xdgdatadir=$withval])

if test x$opt_xdgdatadir = x; then
	XDGDATADIR=${datadir}
else
	XDGDATADIR="$opt_xdgdatadir"
fi
AC_SUBST(XDGDATADIR)

AC_ARG_ENABLE(update-desktop-database,
	AC_HELP_STRING([--disable-update-desktop-database],
		[do not update desktop database after installation]),,
	enable_update_desktop_database=no)

AM_CONDITIONAL(DO_UPDATE_DESKTOP_DB, test x$enable_update_desktop_database = xyes)

if test x$enable_update_desktop_database = xyes ; then
	AC_PATH_PROG(UPDATE_DESKTOP_DATABASE, [update-desktop-database], no)
	if test $UPDATE_DESKTOP_DATABASE = no; then
		AC_MSG_ERROR([Cannot find update-desktop-database, make sure it is installed
			      and in your PATH, or configure with --disable-update-desktop-database])
	fi
fi

AC_ARG_ENABLE(update-mime-database,
	AC_HELP_STRING([--disable-update-mime-database],
		[do not update mime database after installation]),,
	enable_update_mime_database=no)

AM_CONDITIONAL(DO_UPDATE_MIME_DB, test x$enable_update_mime_database = xyes)

if test x$enable_update_mime_database = xyes ; then
	AC_PATH_PROG(UPDATE_MIME_DATABASE, [update-mime-database], no)
	if test $UPDATE_MIME_DATABASE = no; then
		AC_MSG_ERROR([Cannot find update-mime-database, make sure it is installed
			      and in your PATH, or configure with --disable-update-mime-database])
	fi
fi

PKGDATADIR=$prefix/share/$PACKAGE
AC_SUBST(PKGDATADIR)

dnl Any private defines
AC_SUBST(user, [`whoami`@`hostname`])
AC_SUBST(build_date, [`date +"%F"`])
AC_SUBST(build_datetime, [`date`])

dnl fafa is a powerful debugging tool
AC_DEFINE( [FAFA], [printf( __FILE__ ":%d (%s): fafa\n", __LINE__, __FUNCTION__);], [FAFA])
AC_DEFINE( [FABUF], [printf( __FILE__ ":%d (%s): %s\n", __LINE__, __FUNCTION__, __buf__);], [FABUF])


AC_OUTPUT([
	Makefile
	src/Makefile
	src/sigproc/Makefile
	src/libsigfile/Makefile
	src/patterns/Makefile
	src/metrics/Makefile
	src/ica/Makefile
	src/common/Makefile
	src/expdesign/Makefile
	src/model/Makefile
	src/ui/Makefile
	src/ui/sm/Makefile
	src/ui/mw/Makefile
	src/ui/sf/Makefile
	src/ui/sf/dialogs/Makefile
	src/ui/mf/Makefile
	src/tools/Makefile
	data/Makefile
	data/aghermann.desktop
	data/edfhed-gtk.desktop
	doc/Makefile
	doc/org/Makefile
	doc/org/aghermann.org
	man/aghermann.1
	man/edfhed-gtk.1
	man/edfhed.1
	man/edfcat.1
])

AC_MSG_RESULT([
** Configuration summary for $PACKAGE $VERSION:

   PREFIX:              $prefix

   CXX:			$CXX
   CXXFLAGS:            $CXXFLAGS
   LDFLAGS:             $LDFLAGS
   OPENMP_CXXFLAGS:     $OPENMP_CXXFLAGS

   main fp type:	${main_fp_type}
   fftw3 omp-enabled:	${omp_enabled_fftw3}

   precompile headers:  $do_pch
   build html docs:	${build_html_docs}

])
