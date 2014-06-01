dnl This provides configure definitions used by all the bsp
dnl configure.in files.

dnl This calls basic introductory stuff, including AM_INIT_AUTOMAKE
dnl and AC_CANONICAL_HOST.  It also runs configure.host.  The only
dnl argument is the relative path to the top bsp directory.

AC_DEFUN(BSP_CONFIGURE,
[
dnl Default to --enable-multilib
AC_ARG_ENABLE(multilib,
[  --enable-multilib         build many library versions (default)],
[case "${enableval}" in
  yes) multilib=yes ;;
  no)  multilib=no ;;
  *)   AC_MSG_ERROR(bad value ${enableval} for multilib option) ;;
 esac], [multilib=yes])dnl

dnl We may get other options which we don't document:
dnl --with-target-subdir, --with-multisrctop, --with-multisubdir

test -z "[$]{with_target_subdir}" && with_target_subdir=.

if test "[$]{srcdir}" = "."; then
  if test "[$]{with_target_subdir}" != "."; then
    bsp_basedir="[$]{srcdir}/[$]{with_multisrctop}../$1"
  else
    bsp_basedir="[$]{srcdir}/[$]{with_multisrctop}$1"
  fi
else
  bsp_basedir="[$]{srcdir}/$1"
fi
AC_SUBST(bsp_basedir)

AC_CANONICAL_HOST

AM_INIT_AUTOMAKE(bsp, 1.0)

# FIXME: We temporarily define our own version of AC_PROG_CC.  This is
# copied from autoconf 2.12, but does not call AC_PROG_CC_WORKS.  We
# are probably using a cross compiler, which will not be able to fully
# link an executable.  This should really be fixed in autoconf
# itself.

AC_DEFUN(LIB_AC_PROG_CC,
[AC_BEFORE([$0], [AC_PROG_CPP])dnl
AC_CHECK_PROG(CC, gcc, gcc)
if test -z "$CC"; then
  AC_CHECK_PROG(CC, cc, cc, , , /usr/ucb/cc)
  test -z "$CC" && AC_MSG_ERROR([no acceptable cc found in \$PATH])
fi

AC_PROG_CC_GNU

if test $ac_cv_prog_gcc = yes; then
  GCC=yes
dnl Check whether -g works, even if CFLAGS is set, in case the package
dnl plays around with CFLAGS (such as to build both debugging and
dnl normal versions of a library), tasteless as that idea is.
  ac_test_CFLAGS="${CFLAGS+set}"
  ac_save_CFLAGS="$CFLAGS"
  CFLAGS=
  AC_PROG_CC_G
  if test "$ac_test_CFLAGS" = set; then
    CFLAGS="$ac_save_CFLAGS"
  elif test $ac_cv_prog_cc_g = yes; then
    CFLAGS="-g -O2"
  else
    CFLAGS="-O2"
  fi
else
  GCC=
  test "${CFLAGS+set}" = set || CFLAGS="-g"
fi
])

LIB_AC_PROG_CC

# AC_CHECK_TOOL does AC_REQUIRE (AC_CANONICAL_BUILD).  If we don't
# run it explicitly here, it will be run implicitly before
# BSP_CONFIGURE, which doesn't work because that means that it will
# be run before AC_CANONICAL_HOST.
AC_CANONICAL_BUILD

AC_CHECK_TOOL(AS, as)
AC_CHECK_TOOL(AR, ar)
AC_CHECK_TOOL(RANLIB, ranlib, :)

AC_PROG_INSTALL

AM_MAINTAINER_MODE

# We need AC_EXEEXT to keep automake happy in cygnus mode.  However,
# at least currently, we never actually build a program, so we never
# need to use $(EXEEXT).  Moreover, the test for EXEEXT normally
# fails, because we are probably configuring with a cross compiler
# which can't create executables.  So we include AC_EXEEXT to keep
# automake happy, but we don't execute it, since we don't care about
# the result.
if false; then
  AC_EXEEXT
fi

. [$]{bsp_basedir}/configure.host

AC_SUBST(archdir)
AC_SUBST(arch_obj)
AC_SUBST(board_list)

dnl We have to include this fragment like this rather than an include
dnl statement in the Makefile.am files because it uses some gnu make
dnl extensions.
COMMON_IN=[$]{bsp_basedir}/common/common.in
AC_SUBST_FILE(COMMON_IN)

case [$]{bsp_basedir} in
/* | [A-Za-z]:[/\\]*) bsp_flagbasedir=[$]{bsp_basedir} ;;
*) bsp_flagbasedir='[$](top_builddir)/'[$]{bsp_basedir} ;;
esac

BSP_INCLUDES="[$]{bsp_includes} -I[$]{bsp_flagbasedir}/include -I[$]{bsp_flagbasedir}/[$]{archdir}/include"
AC_SUBST(BSP_INCLUDES)

tooldir='$(exec_prefix)/$(host_alias)'
toollibdir='$(tooldir)/lib$(multisubdir)'
AC_SUBST(tooldir)
AC_SUBST(toollibdir)

multisubdir=
if test x[$]{with_multisubdir} != x; then
  multisubdir=/[$]{with_multisubdir}
fi
AC_SUBST(multisubdir)

])
