dnl ************* <auto-copyright.pl BEGIN do not edit this line> *************
dnl
dnl VR Juggler is (C) Copyright 1998-2002 by Iowa State University
dnl
dnl Original Authors:
dnl   Allen Bierbaum, Christopher Just,
dnl   Patrick Hartling, Kevin Meinert,
dnl   Carolina Cruz-Neira, Albert Baker
dnl
dnl This library is free software; you can redistribute it and/or
dnl modify it under the terms of the GNU Library General Public
dnl License as published by the Free Software Foundation; either
dnl version 2 of the License, or (at your option) any later version.
dnl
dnl This library is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
dnl Library General Public License for more details.
dnl
dnl You should have received a copy of the GNU Library General Public
dnl License along with this library; if not, write to the
dnl Free Software Foundation, Inc., 59 Temple Place - Suite 330,
dnl Boston, MA 02111-1307, USA.
dnl
dnl -----------------------------------------------------------------
dnl File:          $RCSfile$
dnl Date modified: $Date$
dnl Version:       $Revision$
dnl -----------------------------------------------------------------
dnl
dnl ************** <auto-copyright.pl END do not edit this line> **************

dnl ---------------------------------------------------------------------------
dnl NOTE: This should not be called by external code.
dnl ---------------------------------------------------------------------------
AC_DEFUN(_JCCL_PATH_SETUP,
[
    dnl Get the cflags and libraries from the jccl-config script
    AC_ARG_WITH(jccl-prefix,
                [  --with-jccl-prefix=<PATH>
                          Prefix where JCCL is installed
                          (optional)                      [No default]],
                jccl_config_prefix="$withval", jccl_config_prefix="")
    AC_ARG_WITH(jccl-exec-prefix,
                [  --with-jccl-exec-prefix=<PATH>
                          Exec prefix where JCCL is
                          installed (optional)            [No default]],
                jccl_config_exec_prefix="$withval",
                jccl_config_exec_prefix="")
dnl    AC_ARG_ENABLE(jccltest,
dnl                  [  --disable-jccltest     Do not try to compile and run a
dnl                          test JCCL program], , enable_jccltest=yes)

    if test "x$jccl_config_exec_prefix" != "x" ; then
        jccl_config_args="$jccl_config_args --exec-prefix=$jccl_config_exec_prefix"

        if test x${JCCL_CONFIG+set} != xset ; then
            JCCL_CONFIG="$jccl_config_exec_prefix/bin/jccl-config"
        fi
    fi

    if test "x$jccl_config_prefix" != "x" ; then
        jccl_config_args="$jccl_config_args --prefix=$jccl_config_prefix"

        if test x${JCCL_CONFIG+set} != xset ; then
            JCCL_CONFIG="$jccl_config_prefix/bin/jccl-config"
        fi
    fi

    if test "x$JCCL_BASE_DIR" != "x" ; then
        jccl_config_args="$jccl_config_args --prefix=$JCCL_BASE_DIR"

        if test x${JCCL_CONFIG+set} != xset ; then
            JCCL_CONFIG="$JCCL_BASE_DIR/bin/jccl-config"
        fi
    fi

    AC_PATH_PROG(JCCL_CONFIG, jccl-config, no)

    dnl Do a sanity check to ensure that $JCCL_CONFIG actually works.
    if ! (eval $JCCL_CONFIG --cxxflags >/dev/null 2>&1) 2>&1 ; then
        JCCL_CONFIG='no'
        echo "*** The jccl-config script installed by JCCL could not be found"
        echo "*** If JCCL was installed in PREFIX, make sure PREFIX/bin is in"
        echo "*** your path, or set the JCCL_CONFIG environment variable to the"
        echo "*** full path to jccl-config."
    fi
])

dnl ---------------------------------------------------------------------------
dnl _JCCL_VERSION_CHECK(MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl NOTE: This should not be called by external code.
dnl ---------------------------------------------------------------------------
AC_DEFUN(_JCCL_VERSION_CHECK,
[
   AC_REQUIRE([_JCCL_PATH_SETUP])

   if test "x$JCCL_CONFIG" = "xno" ; then
      ifelse([$3], , :, [$3])
   else
      JCCL_VERSION=`$JCCL_CONFIG --version`

      min_jccl_version=ifelse([$1], , 0.0.1, $1)
      AC_MSG_CHECKING([whether JCCL version is >= $min_jccl_version])
      AC_MSG_RESULT([$JCCL_VERSION])
      DPP_VERSION_CHECK([$JCCL_VERSION], [$min_jccl_version], $2, $3)
   fi
])

dnl ---------------------------------------------------------------------------
dnl JCCL_PATH_CXX([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND [, MODULES]]]])
dnl
dnl Tests for JCCL C++ API and then defines the following variables:
dnl     JCCL_CXXFLAGS
dnl     JCCL_CXXFLAGS_MIN
dnl     JCCL_INCLUDES
dnl     JCCL_INCLUDES_MIN
dnl     JCCL_LIBS_LD
dnl     JCCL_LIBS_LD_MIN
dnl     JCCL_LIBS_STATIC_LD
dnl     JCCL_LIBS_STATIC_LD_MIN
dnl     JCCL_LIBS_CC
dnl     JCCL_LIBS_CC_MIN
dnl     JCCL_LIBS_STATIC_CC
dnl     JCCL_LIBS_STATIC_CC_MIN
dnl     JCCL_PROF_LIBS_LD
dnl     JCCL_PROF_LIBS_LD_MIN
dnl     JCCL_PROF_LIBS_STATIC_LD
dnl     JCCL_PROF_LIBS_STATIC_LD_MIN
dnl     JCCL_PROF_LIBS_CC
dnl     JCCL_PROF_LIBS_CC_MIN
dnl     JCCL_PROF_LIBS_STATIC_CC
dnl     JCCL_PROF_LIBS_STATIC_CC_MIN
dnl ---------------------------------------------------------------------------
AC_DEFUN(JCCL_PATH_CXX,
[
   AC_REQUIRE([_JCCL_PATH_SETUP])

   JCCL_CXXFLAGS=""
   JCCL_CXXFLAGS_MIN=""
   JCCL_INCLUDES=""
   JCCL_INCLUDES_MIN=""
   JCCL_LIBS_LD=""
   JCCL_LIBS_LD_MIN=""
   JCCL_LIBS_STATIC_LD=""
   JCCL_LIBS_STATIC_LD_MIN=""
   JCCL_LIBS_CC=""
   JCCL_LIBS_CC_MIN=""
   JCCL_LIBS_STATIC_CC=""
   JCCL_LIBS_STATIC_CC_MIN=""
   JCCL_PROF_LIBS_LD=""
   JCCL_PROF_LIBS_LD_MIN=""
   JCCL_PROF_LIBS_STATIC_LD=""
   JCCL_PROF_LIBS_STATIC_LD_MIN=""
   JCCL_PROF_LIBS_CC=""
   JCCL_PROF_LIBS_CC_MIN=""
   JCCL_PROF_LIBS_STATIC_CC=""
   JCCL_PROF_LIBS_STATIC_CC_MIN=""

   if test "x$JCCL_CONFIG" = "xno" ; then
      ifelse([$3], , :, [$3])
   else
      AC_MSG_CHECKING([whether JCCL C++ API is available])
      has_cxx=`$JCCL_CONFIG --too-much-pressure`

      if test "x$has_cxx" = "xY" ; then
         AC_MSG_RESULT([yes])
         if test "x$JCCL_VERSION" = "x" ; then
            _JCCL_VERSION_CHECK($1, [jccl_version_okay='yes'],
                                 [jccl_version_okay='no'
                                  $3])
         fi

         if test "x$jccl_version_okay" = "xyes" ; then
            JCCL_CXXFLAGS=`$JCCL_CONFIG $jccl_config_args --cxxflags $ABI`
            JCCL_INCLUDES=`$JCCL_CONFIG $jccl_config_args --includes`
            JCCL_EXTRA_LIBS_CC=`$JCCL_CONFIG $jccl_config_args --extra-libs $ABI`
            JCCL_EXTRA_LIBS_LD=`$JCCL_CONFIG $jccl_config_args --extra-libs $ABI --linker`
            JCCL_LIBS_CC="`$JCCL_CONFIG $jccl_config_args --libs $ABI` $JCCL_EXTRA_LIBS_CC"
            JCCL_LIBS_LD="`$JCCL_CONFIG $jccl_config_args --libs $ABI --linker` $JCCL_EXTRA_LIBS_LD"
            JCCL_PROF_LIBS_CC="`$JCCL_CONFIG $jccl_config_args --libs $ABI --profiled` $JCCL_EXTRA_LIBS_CC"
            JCCL_PROF_LIBS_LD="`$JCCL_CONFIG $jccl_config_args --libs $ABI --linker --profiled` $JCCL_EXTRA_LIBS_LD"
            JCCL_LIBS_STATIC_CC="`$JCCL_CONFIG $jccl_config_args --libs $ABI --static` $JCCL_EXTRA_LIBS_CC"
            JCCL_LIBS_STATIC_LD="`$JCCL_CONFIG $jccl_config_args --libs $ABI --linker --static` $JCCL_EXTRA_LIBS_LD"
            JCCL_PROF_LIBS_STATIC_CC="`$JCCL_CONFIG $jccl_config_args --libs $ABI --static --profiled` $JCCL_EXTRA_LIBS_CC"
            JCCL_PROF_LIBS_STATIC_LD="`$JCCL_CONFIG $jccl_config_args --libs $ABI --linker --static --profiled` $JCCL_EXTRA_LIBS_LD"

            JCCL_CXXFLAGS_MIN=`$JCCL_CONFIG $jccl_config_args --cxxflags $ABI --min`
            JCCL_INCLUDES_MIN=`$JCCL_CONFIG $jccl_config_args --includes --min`
            JCCL_EXTRA_LIBS_CC_MIN=`$JCCL_CONFIG $jccl_config_args --extra-libs $ABI --min`
            JCCL_EXTRA_LIBS_LD_MIN=`$JCCL_CONFIG $jccl_config_args --extra-libs $ABI --linker --min`
            JCCL_LIBS_CC_MIN="`$JCCL_CONFIG $jccl_config_args --libs $ABI --min` $JCCL_EXTRA_LIBS_CC_MIN"
            JCCL_LIBS_LD_MIN="`$JCCL_CONFIG $jccl_config_args --libs $ABI --linker --min` $JCCL_EXTRA_LIBS_LD_MIN"
            JCCL_LIBS_LD_MIN="`$JCCL_CONFIG $jccl_config_args --libs $ABI --linker --min` $JCCL_EXTRA_LIBS_LD_MIN"
            JCCL_PROF_LIBS_CC_MIN="`$JCCL_CONFIG $jccl_config_args --libs $ABI --min --profiled` $JCCL_EXTRA_LIBS_CC_MIN"
            JCCL_PROF_LIBS_LD_MIN="`$JCCL_CONFIG $jccl_config_args --libs $ABI --linker --min --profiled` $JCCL_EXTRA_LIBS_LD_MIN"
            JCCL_LIBS_STATIC_CC_MIN="`$JCCL_CONFIG $jccl_config_args --libs $ABI --static --min` $JCCL_EXTRA_LIBS_CC_MIN"
            JCCL_LIBS_STATIC_LD_MIN="`$JCCL_CONFIG $jccl_config_args --libs $ABI --linker --static --min` $JCCL_EXTRA_LIBS_LD_MIN"
            JCCL_PROF_LIBS_STATIC_CC_MIN="`$JCCL_CONFIG $jccl_config_args --libs $ABI --static --min --profiled` $JCCL_EXTRA_LIBS_CC_MIN"
            JCCL_PROF_LIBS_STATIC_LD_MIN="`$JCCL_CONFIG $jccl_config_args --libs $ABI --linker --static --min --profiled` $JCCL_EXTRA_LIBS_LD_MIN"

            ifelse([$2], , :, [$2])
         fi
      else
         AC_MSG_RESULT([no])
         ifelse([$3], , :, [$3])
      fi
   fi

   AC_SUBST([JCCL_CXXFLAGS])
   AC_SUBST([JCCL_LIBS_CC])
   AC_SUBST([JCCL_LIBS_LD])
   AC_SUBST([JCCL_PROF_LIBS_CC])
   AC_SUBST([JCCL_PROF_LIBS_LD])
   AC_SUBST([JCCL_LIBS_STATIC_LD])
   AC_SUBST([JCCL_LIBS_STATIC_CC])
   AC_SUBST([JCCL_PROF_LIBS_STATIC_LD])
   AC_SUBST([JCCL_PROF_LIBS_STATIC_CC])

   AC_SUBST([JCCL_CXXFLAGS_MIN])
   AC_SUBST([JCCL_INCLUDES_MIN])
   AC_SUBST([JCCL_LIBS_CC_MIN])
   AC_SUBST([JCCL_LIBS_LD_MIN])
   AC_SUBST([JCCL_PROF_LIBS_CC_MIN])
   AC_SUBST([JCCL_PROF_LIBS_LD_MIN])
   AC_SUBST([JCCL_LIBS_STATIC_CC_MIN])
   AC_SUBST([JCCL_LIBS_STATIC_LD_MIN])
   AC_SUBST([JCCL_PROF_LIBS_STATIC_CC_MIN])
   AC_SUBST([JCCL_PROF_LIBS_STATIC_LD_MIN])
])

dnl ---------------------------------------------------------------------------
dnl JCCL_PATH_JAVA([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND [, MODULES]]]])
dnl
dnl Tests for JCCL Java API and then defines the following variables:
dnl     JCCL_JARS
dnl ---------------------------------------------------------------------------
AC_DEFUN(JCCL_PATH_JAVA,
[
   AC_REQUIRE([_JCCL_PATH_SETUP])

   JCCL_JARS=''

   if test "x$JCCL_CONFIG" = "xno" ; then
      ifelse([$3], , :, [$3])
   else
      AC_MSG_CHECKING([whether JCCL Java API is available])
      has_java=`$JCCL_CONFIG --is-jittery`

      if test "x$has_java" = "xY" ; then
         AC_MSG_RESULT([yes])
         if test "x$JCCL_VERSION" = "x" ; then
            _JCCL_VERSION_CHECK($1, [jccl_version_okay='yes'],
                                 [jccl_version_okay='no'
                                  $3])
         fi

         if test "x$jccl_version_okay" = "xyes" ; then
            JCCL_JARS="`$JCCL_CONFIG $jccl_config_args --jars`"

            ifelse([$2], , :, [$2])
         fi
      else
         AC_MSG_RESULT([no])
         ifelse([$3], , :, [$3])
      fi
   fi

   AC_SUBST([JCCL_JARS])
])

dnl ---------------------------------------------------------------------------
dnl JCCL_PATH([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND [, MODULES]]]])
dnl
dnl Tests for JCCL C++ and Java APIs and then defines the following
dnl variables:
dnl     JCCL_CXXFLAGS
dnl     JCCL_CXXFLAGS_MIN
dnl     JCCL_LIBS_LD
dnl     JCCL_LIBS_LD_MIN
dnl     JCCL_LIBS_STATIC_LD
dnl     JCCL_LIBS_STATIC_LD_MIN
dnl     JCCL_LIBS_CC
dnl     JCCL_LIBS_CC_MIN
dnl     JCCL_LIBS_STATIC_CC
dnl     JCCL_LIBS_STATIC_CC_MIN
dnl     JCCL_PROF_LIBS_LD
dnl     JCCL_PROF_LIBS_LD_MIN
dnl     JCCL_PROF_LIBS_STATIC_LD
dnl     JCCL_PROF_LIBS_STATIC_LD_MIN
dnl     JCCL_PROF_LIBS_CC
dnl     JCCL_PROF_LIBS_CC_MIN
dnl     JCCL_PROF_LIBS_STATIC_CC
dnl     JCCL_PROF_LIBS_STATIC_CC_MIN
dnl     JCCL_JARS
dnl ---------------------------------------------------------------------------
AC_DEFUN(JCCL_PATH,
[
   JCCL_PATH_CXX($1, [jccl_have_cxx='yes'], $3, $4)

   if test "x$jccl_have_cxx" = "xyes" ; then
      JCCL_PATH_JAVA($1, $2, $3, $4)
   fi
])
