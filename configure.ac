dnl Cmod is the legal property of its developers. Please refer to the
dnl COPYRIGHT file distributed with this source distribution.
dnl
dnl This program is free software: you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation, either version 3 of the License, or
dnl (at your option) any later version.
dnl
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with this program.  If not, see <http://www.gnu.org/licenses/>.
dnl

AC_PREREQ(2.59)
AC_REVISION($Revision$)

AC_INIT(Cmod, 0.01, http://redbrain.co.uk)
AC_CONFIG_SRCDIR([src/mm-cmod.c])
AC_CONFIG_HEADER([src/config.h])
dnl keep the autoconf generated stuff neatly in a folder :)
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_MACRO_DIR([m4])
AM_SANITY_CHECK

dnl Last slash shouldn't be stripped if prefix=/
if test "$prefix" != "/"; then
    prefix=`echo "$prefix" | sed -e 's/\/$//g'`
fi 

dnl Checks for programs.
AC_PROG_CC
AC_DEFINE_UNQUOTED([CONFIG_CC], "$CC", [C Compiler used])

AC_PROG_AWK
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB
AC_CHECK_PROGS(AR, ar aal, ar)
AC_PROG_CC_C99
AC_PROG_CXX

AC_EXEEXT
AC_GNU_SOURCE

AC_PROG_INSTALL

AC_PROG_LIBTOOL
AM_PROG_LIBTOOL

AM_PROG_CC_C_O

AC_PROG_YACC
AC_PROG_LEX

AC_C_BIGENDIAN
AC_C_INLINE
AC_C_VOLATILE

AC_CANONICAL_HOST

case "$host" in
  *apple*)
    AC_DEFINE([CRAPLE], 1, [Apple Mac Osx Specifics])
    ;;
esac

AC_CHECK_LIB([m], [pow], , 
  [AC_MSG_ERROR([System Math library is required!])])

#it would be nice to get their versions here...
bignum="no"
AC_ARG_WITH(bignum,
  [AS_HELP_STRING([--with-bignum=yes/no],
    [With GNU MP and MPFR multi-precision support, default no.])],
  [bignum="$withval"])
if test "x$bignum" == "xyes"; then
  AC_CHECK_LIB([gmp], [__gmpz_init], ,
    [AC_MSG_ERROR([GNU MP not found, see http://gmplib.org/])])
  AC_CHECK_LIB([mpfr],[mpfr_init] , ,
    [AC_MSG_ERROR([MPFR not found, see http://www.mpfr.org/])])
  AC_DEFINE([CONFIG_BIGNUM], 1, [Bignum multi-percision support])
  AC_CHECK_SIZEOF(mp_limb_t, , [#include <gmp.h>])
  CFLAGS="$CFLAGS -DBIGNUM"
fi

AC_SUBST(CFLAGS)
AC_SUBST(LDFLAGS)

AC_PATH_PROG(MV, mv, mv)
AC_PATH_PROG(RM, rm, rm)
AC_PATH_PROG(CP, cp, cp)
AC_PATH_PROG(SED, sed, sed)
AC_PATH_PROG(CMP, cmp, cmp)
AC_PATH_PROG(CHMOD, chmod, chmod)
AC_PATH_PROG(HOSTNAME, hostname, hostname)

# Check whether --with-system-type or --without-system-type was given.
AC_ARG_WITH([system-type],
    [AS_HELP_STRING([--with-system-type],
       [Set the system type, like "sun-solaris10"])],
    [SYSTEM_TYPE="$withval"],
    [SYSTEM_TYPE="$host_vendor-$host_os"])
AC_ARG_WITH([machine-type],
    [AS_HELP_STRING([--with-machine-type],
       [Set the machine type, like "sparc"])],
    [MACHINE_TYPE="$withval"],
    [MACHINE_TYPE="$host_cpu"])
AC_SUBST(SYSTEM_TYPE)
AC_DEFINE_UNQUOTED([SYSTEM_TYPE], ["$SYSTEM_TYPE"],
                   [Name of system, eg sun-solaris])
AC_SUBST(MACHINE_TYPE)
AC_DEFINE_UNQUOTED([MACHINE_TYPE], ["$MACHINE_TYPE"],
                   [Machine type name, eg sparc])

AM_INIT_AUTOMAKE

AC_CHECK_SIZEOF(long)
AC_CHECK_SIZEOF(char)
AC_CHECK_SIZEOF(int)
AC_CHECK_SIZEOF(void*)
AC_CHECK_SIZEOF(short)
AC_CHECK_SIZEOF(double)
AC_CHECK_SIZEOF(float)
AC_CHECK_SIZEOF(size_t)
AC_CHECK_SIZEOF(long int)
AC_CHECK_SIZEOF(long long)

# Checks for header files.
AC_HEADER_SYS_WAIT
AC_HEADER_STDBOOL
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h stdarg.h unistd.h \
		  stdint.h stdio.h getopt.h \
		  assert.h sys/types.h signal.h \
		  fcntl.h pthread.h sys/wait.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_CHECK_FUNCS([popen fopen fclose sprintf fprintf strdup \
		strlen strcpy strcmp getopt_long \
		memcpy calloc system sysconf atoi \
		getpid execl fork wait exit atof \
		vfprintf memcmp getc fgets pipe \
		waitpid fdopen])

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_UTIME_NULL
AC_FUNC_VPRINTF
AC_CONFIG_FILES([Makefile
                 include/Makefile
                 src/Makefile])
AC_OUTPUT

echo "---"
echo "Configuration summary for $PACKAGE_NAME version $VERSION"
echo ""
echo "   * Installation prefix:       $prefix"
echo "   * System type:               $SYSTEM_TYPE"
echo "   * Host CPU:                  $host_cpu"
echo "   * C Compiler:                $CC"
echo "   * CFLAGS:                    $CFLAGS"
echo "   * LDFLAGS:                   $LDFLAGS"
echo "   * YACC version:              $YACC"
echo "   * LEX version:               $LEX"
echo "   * Multi-Precision:           $bignum"
echo ""
echo "---"

dnl libtoolize scans configure.ac  and needs to see some text
m4_define([LIBTOOLIZE_AC_INIT], [])
