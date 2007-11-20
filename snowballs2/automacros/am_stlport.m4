# =========================================================================
# AM_PATH_STLPORT
#
#    Option:      none.
#    Description: check the instalation of the STLPort library and set the
#                 CPPFLAGS and LIBS variables to use it.

AC_DEFUN([AM_PATH_STLPORT], [
    stlport_places=$1

    have_stlport=0

    # Save the original values of the flags we tweak.
    stlport_check_lib_save_ldflags="$LDFLAGS"
    stlport_check_lib_save_cppflags="$CPPFLAGS"

    AC_ARG_WITH( stlport,
    [  --with-stlport=<path>   path to the STLport install files directory.
                          e.g. /usr/local/stlport])

    AC_ARG_WITH( stlport-include,
    [  --with-stlport-include=<path>
                          path to the STLport header files directory.
                          e.g. /usr/local/stlport/include])

    AC_ARG_WITH( stlport-lib,
    [  --with-stlport-lib=<path>
                          path to the STLport library files directory.
                          e.g. /usr/local/stlport/lib])

    if test "$with_stlport"
    then
        stlport_places="$with_stlport"
    fi

    found=not
    for stlport_place in $stlport_places;
    do
      LDFLAGS="$stlport_check_lib_save_ldflags"
      CPPFLAGS="$stlport_check_lib_save_cppflags"

      # If the user specified --with-stlport-include, override the search.
      if test "$with_stlport_include"
      then
        CPPFLAGS="$CPPFLAGS -I$with_stlport_include"
      else
        CPPFLAGS="$CPPFLAGS -I$stlport_place/include/stlport"
      fi

      # If the user specified --with-stlport-lib, override the search.
      if test "$with_stlport_lib"
      then
        LDFLAGS="$LDFLAGS -L$with_stlport_lib"
      else
        LDFLAGS="$LDFLAGS -L$stlport_place/lib"
      fi

      # The description of where we are searching.
      description="$stlport_place"


      AC_MSG_NOTICE([checking for STLport in $description])
      # Clear the header cache variable for each location
      changequote(,)
      cache_id="`echo ac_cv_header_stl__stlport_version_h \
               | sed -e 's/[^a-zA-Z0-9_]/_/g'`"
      changequote([,])
      unset $cache_id
      AC_CHECK_HEADER([stl/_stlport_version.h], [
        # We generate a separate cache variable for each prefix and libname
        # we search under.  That way, we avoid caching information that
        # changes if the user runs `configure' with a different set of
        # switches.
        changequote(,)
        cache_id="`echo nel_cv_check_stlport_stlport_version_h_in_${stlport_place} \
                 | sed -e 's/[^a-zA-Z0-9_]/_/g'`"
        changequote([,])

        AC_CACHE_VAL($cache_id,
          [
            eval "$cache_id=yes"
          ])
        result="`eval echo '$'$cache_id`"
        AC_MSG_RESULT($result)
      ], [result="no"])

      # If we found it, no need to search any more.
      if test "$result" = "yes"; then
        found="$stlport_place"
        break
      fi
      test "$found" != "not" && break
    done

    # Restore the original values of the flags we tweak.
    LDFLAGS="$stlport_check_lib_save_ldflags"
    CPPFLAGS="$stlport_check_lib_save_cppflags"

    case "$found" in
    "not")
      have_stlport=0
      AC_MSG_ERROR([STLport must be installed ( http://www.stlport.org).])
      ;;
    *)
      if test "$with_stlport_include"
      then
        STLPORT_INCLUDES="-I$with_stlport_include"
      else
        STLPORT_INCLUDES="-I$found/include/stlport"
      fi

      STLPORT_LIBS="-lstlport"

      if test "$with_stlport_lib"
      then
        STLPORT_LDFLAGS="-L$with_stlport_lib"
      else
        STLPORT_LDFLAGS="-L$found/lib"
      fi

      have_stlport=1
      ;;
    esac
    AC_SUBST(STLPORT_INCLUDES)
    AC_SUBST(STLPORT_LIBS)
    AC_SUBST(STLPORT_LDFLAGS)
])

# =========================================================================
# End of file

