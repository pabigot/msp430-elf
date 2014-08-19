#!/bin/bash

# Shell script to automatically update a GNUPro source tree with changes
# made to the (branches in the) relevant projects in FSF repositories.
#
# Copyright (c) 2007-2013 Red Hat.
#
# Ideally this would be written in Perl or Python, but I just do not have
# the time to do learn those languages right now.  One day, when there is
# more time...
#
# The idea is to use CVS/Subversion to handle all of the synchronisation
# for us by keeping copies of all of their control files and directories.
# Conceptually what the script does is this:
#
#  * Move the FSF control files into place (and the GNUPro ones out of the
#    way if necessary).
#  * Run a "svn/cvs update" to get all of the changes that have been made.
#  * Move the GNUPro control files back into place (and save the FSF ones
#    if necessary).
#  * Run a "cvs commit" to add all of the changes to the GNUPro repository.
#
# There is more to it of course, but this is the basic idea.
#
# In order to start using this script you need a checked out copy of the
# GNUPro repository which has specially saved copies of the CVS control
# files.  You could do this as follows:
#
#  1. Check out *read-only* copies of the most recent stable branches from
#     the relevant projects in FSF repositories.  Eg:
#
#      mkdir /FSF
#      pushd /FSF
#      cvs -z 9 -d :pserver:anoncvs@sourceware.org:/cvs/src login
#      cvs -d :pserver:anoncvs@sources.redhat.com:/cvs/src co -r binutils-2_23-branch binutils
#      cvs -d :pserver:anoncvs@sources.redhat.com:/cvs/src co -r newlib-2_0 newlib
#      cvs -d :pserver:anoncvs@sources.redhat.com:/cvs/src co -r gdb-7_0 gdb
#      svn -q checkout svn://gcc.gnu.org/svn/gcc/branches/gcc-4_8-branch gcc
#
#     [Using uberbaum might help here....]
#
#  2. Now go through these source tree(s) and rename the CVS directories to
#      .CVS.FSF.  eg:
#
#       #!/bin/tcsh
#       foreach A (`find . -type d -name CVS`)
#        mv $A `dirname $A`/.CVS.FSF
#       end
#
#  3. Next check out a (separate) GNUPro source tree.
#
#        mkdir /GNUPro
#        pushd /GNUPro
#        cvs -d :ext:cvs.ges.redhat.com:/cvs/cvsfiles co gnupro-cross
#
#   4. Go through and make copies of the CVS directories in the GNUPro
#       sources, calling the copies .CVS.GNUPro.
#
#       #!/bin/tcsh
#       foreach A (`find . -type d -name CVS`)
#        cp -r $A `dirname $A`/.CVS.GNUPro
#       end
#
#   5. Copy all of the files from the FSF directories into the GNUPro
#       source tree.
#
#        cp -r /FSF/* /GNUPro
#
#     Then check to see if there are any directories which do not contain a
#      a CVS directory.  These will have been added to the FSF repository and
#      not the GNUPro repository, so you will need to add them by hand.
#      You can use this script with the -f option to find the directories and
#      the add-non-devo-cvs-direcrtory script to add them.  [Make sure that
#      you have set the correct pathnames in the init() function below first].
#
#        GNUProImporter.sh -f | tee new-dirs
#
#      Note - it is worth checking the contents of this list of new
#      directories before going on to add them to the GNUPro repository:
#
#        foreach A (`cat new-dirs`)
#           cd `dirname $A` ; add-non-devo-cvs-directory `basename $A` ; cd -
#        end
#        rm new-dirs
#
#     Similarly you must check for and add any files which are not mentioned
#      in their corresponding CVS/Entries file:
#
#        cvs -q update | grep \? > new-files
#        cvs add `cat new-files`
#        rm new-files
#
#     Next you must find any files or directories in the GNUPro repository
#      which are no longer in the corresponding FSF repository and decide
#      whether to delete them or not.
#
#        GNUProImporter.sh -F | tee old-dirs
#
#      Note - it is worth checking the contents of this list of
#      directories before removing them from the GNUPro repository:
#
#        foreach A (`cat old-dirs`)
#           cd `dirname $A` ; rm-cvs-directory `basename $A` ; cd -
#        end
#        rm old-dirs
#
#      Note - currently the GNUProImporter script does not have the ability
#      to locate obsolete files, so you will have to find them by hand.
#      FIXME: This should be implemented.
#
#     Once you have done this you do not need the FSF source trees any more.
#      You can delete them if you want to.
#
#        rm -r /FSF
#
#  6. Apply the GNUPro local patches.  Decide if any of them have become
#      obsolete and can be deleted.  You can leave this step until later if
#      you want to.
#
#  7. Build toolchains and fix any problems.
#
#  8. Commit any changes to the GNUPro repository.
#
# In order to use this script you will need to modify the paths in the init()
# function to match your setup.

init ()
{
  # Where the checked out sources are located.
  SRC_ROOT=/work/sources/gnupro/current

  # Where the checked out sources are built.
  BUILD_ROOT=/work/builds/gnupro/current

  # Temporary files.
  TMPFILE_1=$TMPDIR/gnupro-update.tmp1
  TMPFILE_2=$TMPDIR/gnupro-update.tmp2

  # Names of alternative CVS directories.
  CVS_GNUPRO=.CVS.GNUPro
  CVS_FSF=.CVS.FSF

  # Programs used to regenerate configure and makefiles.
  AUTOCONF=autoconf-2.64
  AUTORECONF=autoreconf-2.64
  AUTOMAKE=automake-1.11
  ACLOCAL=aclocal-1.11

  # FSF GCC sources use Subversion which only maintains .svn file at
  # the top level of directory heirarchy.  Therefore we cannot assume
  # that just because a sub-directory in the GNUPro source tree does
  # not contain a .svn direcory (or a CVS directory) that it is therefore
  # orphaned.  Instead we use a list of top level directories that are
  # known to use CVS and check them.

  CVS_DIRS="bfd binutils config cpu elfcpp gas gdb gold gprof include intl ld libgloss libiberty newlib opcodes readline sim texinfo"
  
  
#
# -------------------------------------------------------------
#   You should not need to modify anything below this line.
# -------------------------------------------------------------
#

  if [ ! -d $SRC_ROOT ]
  then
    fail "Checked out sources are not located at $SRC_ROOT"
  fi

  # Multi-state variable:
  # -2 => Do not do anything, be verbose about what would be done.
  # -1 => Do not do anything, just show what would be done.
  #  0 => Do not report anything, just return an exit code.
  #  1 => Provide a basic desription of what is going on.
  #  2 => Be verbose about what is going on.
  SHOW_PROGRESS=1

  ACTION=import-build-commit

  VERSION=1.4
}

main ()
{
  init

  parse_args ${1+"$@"}

  case "$ACTION" in
    import-build-commit)
      do_updates FSF
      do_updates GNUPro
      remove_generated_files
      check_updated_sources
      regenerate_files
      check_toolchains_build
      commit_changes
      finish
      ;;
    import-build)
      do_updates FSF
      do_updates GNUPro
      remove_generated_files
      check_updated_sources
      regenerate_files
      check_toolchains_build
      finish
      ;;
    import)
      do_updates FSF
      do_updates GNUPro
      remove_generated_files
      check_updated_sources
      regenerate_files
      finish
      ;;
    restore)
      restore_tree
      ;;
    find-conflicts)
      look_for_conflict_markers
      ;;
    verify-tree)
      look_for_bogus_VC_info
      ;;
    regenerate-files)
      regenerate_files
      ;;
    find-nonrepository-directories)
      find-nonrepository-directories
      ;;
    find-deleted-files)
      find-deleted-dirs
      find-deleted-files
      ;;
      
    *)
      fail "bad value for ACTION: $ACTION"
      ;;
  esac

  exit 0
}

finish ()
{
  rm -f $TMPFILE_1 $TMPFILE_2 
}

parse_args ()
{
  local optname optarg

  while [ $# -gt 0 ]
  do
    optname="`echo $1 | sed 's,=.*,,'`"
    optarg="`echo $1 | sed 's,^[^=]*=,,'`"
    case "$1" in
      --srctop*)
           SRC_ROOT="$optarg"
           ;;
      --buildtop*)
           BUILD_ROOT="$optarg"
           ;;
	   
      --import-build-commit | -i)
           ACTION=import-build-commit
	   ;;
      --import-and-build)
           ACTION=import-build
	   ;;
      --import-only)
           ACTION=import
	   ;;
      --restore-tree | -r)
           ACTION=restore
	   ;;
      --find-conflicts | -c)
           ACTION=find-conflicts
	   ;;
      --verify-tree | -V)
           ACTION=verify-tree
	   ;;
      --regenerate-files | -R)
           ACTION=regenerate-files
	   ;;
      --find-nonrepository-directories | -f)
           ACTION=find-nonrepository-directories
	   ;;
      --find-deleted-files | -F)
           ACTION=find-deleted-files
	   ;;

      --verbose | -v)
	   if [ $SHOW_PROGRESS -eq -1 ]
	   then
	     SHOW_PROGRESS=-2
	   else
             SHOW_PROGRESS=2
	   fi
           ;;
      --progress | -p)
           if [ $SHOW_PROGRESS -lt 0 ]
	   then
             SHOW_PROGRESS=-1
	   else
             SHOW_PROGRESS=1
	   fi
           ;;
      --dry-run | -n)
           if [ $SHOW_PROGRESS -eq 2 ]
	   then
	     SHOW_PROGRESS=-2
	   else
             SHOW_PROGRESS=-1
	   fi
	   ;;
      --quiet | -q)
           if [ $SHOW_PROGRESS -gt 0 ]
	   then
             SHOW_PROGRESS=0
	   fi
           ;;
	   
      -h | --help)
           help
           exit 0
           ;;
      --version)
           report "`basename $0`: version: $VERSION"
	   exit 0
	   ;;
      *)
           warn "`basename $0`: Unrecognized option \"$1\""
           ;;
    esac
    shift
  done
}

help ()
{
  # The following exec goop is so that we don't have to manually
  # redirect every message to stderr in this function.

  exec 4>&1    # save stdout fd to fd #4
  exec 1>&2    # redirect stdout to stderr

  cat <<__EOM__
  This is a shell script to update the GNUPro sources with any
  changes that have been made on the corresponding FSF branches.

Usage: `basename $0` [options] [action]

[action] is one of:
  -i --import-build-commit  Import changes, rebuild the toolchains and then commit.  [Default]
     --import-and-build     Import the changes and then rebuild toolchains.
     --import-only          Import the changes from the FSF repositories.
  -r --restore-tree         Restore GNUPro CVS directories.
                             (Use after terminating an in-progress import).
  -c --find-conflicts       Look for merge conflict markers in the source tree.
  -V --verify-tree          Look for bogus CVS directories.
  -R --regenerate-files     Rebuild auto-generated files.
  -f --find-nonrepository-directories
                            Produce a list of directories not checked into the GNUPro repository
  -F --find-deleted-files   Produce a list of files & dirs that are not in the FSF repositories

[options] are:
  -q --quiet                Do not show any messages.
  -p --progress             Show basic progress messages as the script runs.  [Default]
  -v --verbose              Show all messages as the script runs.
  
  -n --dry-run              Show the actions that would be taken, but do not do them.
  -h --help                 Display this information.
     --version              Report the version number of this script.
     
  --srctop <path>           Absolute path to top of checked out sources.
  --buildtop <path>         Absolute path to top of build tree.

Default paths are:
  Sources: $SRC_ROOT
  Builds:  $BUILD_ROOT

__EOM__
  exec 1>&4   # Copy stdout fd back from temporary save fd, #4
}

report ()
{
  if [ $SHOW_PROGRESS -ne 0 ]
  then
    echo ${1+"$@"}
  fi
}

verbose ()
{
  if [ $SHOW_PROGRESS -gt 1 ] || [ $SHOW_PROGRESS -lt -1 ]
  then
    echo ${1+"$@"}
  fi
}

fail ()
{
  if test "x$1" != "x" ;
  then
    report "  Internal error: $1"
  fi

  exit 1  
}

warn ()
{
  if test "x$1" != "x" ;
  then
    report "  Warning: $1"
  fi
}

restore_tree ()
{
  local A current_dir

  report " Restoring the GNUPro repository meta-files."

  if [ $SHOW_PROGRESS -lt 0 ]
  then
    return
  fi

  pushd $SRC_ROOT > /dev/null

  verbose "  Locating GNUPro meta-files."
  for A in `find . -type d -name $CVS_GNUPRO`
  do
    current_dir=`dirname $A`
    verbose "  Restoring in: $current_dir"
    
    pushd $current_dir > /dev/null

    if [ -d CVS ]
    then
      mv CVS .CVS.bad
    else
      report "CVS directory missing in: $current_dir"
    fi

    cp -r  $CVS_GNUPRO  CVS
    rm -fr .CVS.bad

    popd > /dev/null
    
  done
  popd > /dev/null
}

rename_repository ()
{
  local A name new_cvs_dir old_cvs_dir current_dir

  if test "x$3" = "x" ;
  then
    fail "rename_repository called with too few arguments"
  fi

  name=$1
  new_cvs_dir=$2
  old_cvs_dir=$3

  report " Selecting the $name repository."

  if [ $SHOW_PROGRESS -lt 0 ]
  then
    return
  fi
  
  pushd $SRC_ROOT > /dev/null
  for A in `find . -type d -name $new_cvs_dir`
  do
    current_dir=`dirname $A`
    pushd $current_dir > /dev/null

    # Paranoia 1:
    #  Only rename the CVS directory if it contains
    #  both a .CVS.GNUPro and a .CVS.FSF duplicate.
    if [ -d $old_cvs_dir ]
    then
      # Paranoia 2:
      #  Only rename the CVS directory if the current
      #  CVS directory matches the old CVS directory.
      if cmp -s CVS/Root $old_cvs_dir/Root
      then
        rm -fr $old_cvs_dir
        mv     CVS           $old_cvs_dir
        cp -r  $new_cvs_dir  CVS
      else
        verbose "  $current_dir's CVS directory does not match $old_cvs_dir's."
      fi
    elif [ $old_cvs_dir != $CVS_FSF ] || [ ! -d .svn ]
    then
      verbose "  $current_dir does not contain a $old_cvs_dir directory."
    fi
    
    popd > /dev/null
  done
  popd > /dev/null
}

update_from_repository ()
{
  local A name cvs_dir
  
  if test "x$2" = "x" ;
  then
    fail "update_from_repository called with too few arguments."
  fi

  name=$1
  cvs_dir=$2
  
  report -n " Updating from the $name repository using CVS: "

  pushd $SRC_ROOT > /dev/null

  if [ $SHOW_PROGRESS -lt 0 ]
  then
    verbose ""
    verbose -n "  "
  fi
  
  verbose -n "toplevel "
  if [ $SHOW_PROGRESS -gt -1 ]
  then
    cvs -q update -l > /dev/null 2>/dev/null
  fi

  for A in *
  do
    if [ -d "$A" ]
    then
      pushd $A > /dev/null
      if [ -d $cvs_dir ]
      then
        verbose -n "$A "
	if [ $SHOW_PROGRESS -gt -1 ]
	then
          cvs -q update > /dev/null 2>&1
	fi
      else
        if [ x$A != "xCVS" ]
	then
          verbose -n "(skip: $A) "
	fi
      fi
      popd > /dev/null
    fi
  done

  popd > /dev/null
  report ""
}

do_updates ()
{
  local A name new_cvs_dir old_cvs_dir
  
  if test "x$1" = "x" ;
  then
    fail "do_updates called with too few arguments."
  fi

  if test "x$1" = "xFSF"
  then
    name="FSF"
    new_cvs_dir=$CVS_FSF
    old_cvs_dir=$CVS_GNUPRO

    rename_repository       $name $new_cvs_dir $old_cvs_dir
    update_from_repository  $name $new_cvs_dir

    report " Updating from the FSF repository using subversion:"

    pushd $SRC_ROOT > /dev/null

    if [ $SHOW_PROGRESS -lt 0 ]
    then
      verbose -n "  "
      for A in *
      do
        if [ -d $A/.svn ]
        then
          verbose -n "$A "
        fi
      done
      verbose ""
    else
      svn -q update
    fi

    popd > /dev/null

  elif test "x$1" = "xGNUPro"
  then
    name="GNUPro"
    new_cvs_dir=$CVS_GNUPRO
    old_cvs_dir=$CVS_FSF

    rename_repository       $name $new_cvs_dir $old_cvs_dir
    update_from_repository  $name $new_cvs_dir

  else
    fail "do_updates called with invalid agrument."
  fi
}

remove_generated_files ()
{
  report " Removing generated and redundant files from the GNUPro repository."
  
  if [ $SHOW_PROGRESS -lt 0 ]
  then
    return
  fi

  pushd $SRC_ROOT > /dev/null

  rm -f \
    bfd/doc/bfd.info* \
    gas/doc/as.info* \
    gas/doc/as.1 \
    ld/ld.info \
    ld/ld.1 \
    binutils/doc/binutils.info \
    binutils/doc/*.1

  rm -f \
    gdb/configure.in \
    sim/arm/configure.in \
    sim/common/configure.in \
    sim/d10v/configure.in \
    sim/erc32/configure.in \
    sim/frv/configure.in \
    sim/h8300/configure.in \
    sim/igen/configure.in \
    sim/m32r/configure.in \
    sim/m68hc11/configure.in \
    sim/mcore/configure.in \
    sim/mips/configure.in \
    sim/mn10300/configure.in \
    sim/ppc/configure.in \
    sim/sh/configure.in \
    sim/v850/configure.in \
    intl/configure.in

  # Eliminate extraneous files created by Subversion when a conflict occurs.
  find . -name "*.mine" -exec rm {} \;
  find . -name "*.r[0-9]+" -exec rm {} \;
  
  # Eliminate extraneous files created by autoconf
  find . -name "autom4te.cache" -exec rm -fr {} \;

  # Ignore changes to header files that just contain a timestamp.
  rm bfd/version.h gdb/version.in
  cvs update bfd/version.h gdb/version.in

  cvsclean | xargs rm -f
  
  popd > /dev/null
}

look_for_conflict_markers ()
{
  report " Looking for conflict markers in local source tree."

  # Note - recording the "C" status messages of the FSF
  # update is unreliable.  CVS will frequently produce this
  # status for files which do not contain merge conflicts.
  # So instead we grep for merge markers.
  #
  # Note - the following are known to contain false positives:
  #
  #   itcl/itcl/doc/license.terms
  #   gcc/doc/fr.po
    
  pushd $SRC_ROOT > /dev/null
  grep -l -r ">>>>>>>" . | xargs grep -l "=======" | grep -v -e "cvs/FAQ" -e "GNUProImporter.sh" -e "license.terms" -e ".svn" -e "fr.po"
  popd > /dev/null
}

look_for_bogus_VC_info ()
{
  local A B dir skip_dir
  
  report " Looking for directories with bogus or missing version control data."
    
  pushd $SRC_ROOT > /dev/null

  skip_dir="zz"

  for A in `find $CVS_DIRS -type d`
  do
    dir="`basename $A`"

    if [[ x$A =~ x$skip_dir* ]]
    then
      continue;
    else
      skip_dir="zz"
    fi

    if [ x$dir == xCVS ] || [ x$dir == x$CVS_GNUPRO ] || [ x$dir == x$CVS_FSF ]
    then
      for B in `find $A/* -type d`
      do
        report "  Directory $B found inside $A"
      done
    elif [ x$dir == x.svn ]
    then
      for B in CVS $CVS_GNUPRO $CVS_FSF .svn
      do
        if [ -d $A/$B ]
        then
          report "  Directory $B found inside $A"
        fi
      done
    elif [ ! -d $A/CVS ]
    then
      dir=`dirname $A`
      B=`dirname $dir`
      dir=`basename $dir`
      B=`basename $B`
	
      if [ x$dir != x.svn ] && [ x$B != x.svn ]
      then
        report "  $A does not have a CVS directory."
      fi
    elif [ -f $A/.FSF_ignore ]
    then
      verbose "  skipping non-FSF directory: $A"
      skip_dir=$A
    elif [ ! -d $A/$CVS_GNUPRO ]
    then
      report "  $A does not have a $CVS_GNUPRO."
    elif [ -d $A/$CVS_FSF ] && [ -d $A/.svn ]
    then
      report "  $A has both .svn and $CVS_FSF directories."
    elif `grep --silent --invert-match gnupro $A/CVS/Repository` ;
    then
      report "  CVS file in $A is not from the GNUPro repository."
    elif [ ! -d $A/$CVS_FSF ]
    then
      report "  $A does not have a $CVS_FSF directory."
      skip_dir=$A
    else
      verbose "  $A OK."
    fi
  done
  
  report " Finished"
  
  popd > /dev/null
}

check_updated_sources ()
{
  report " Adding new files to the GNUPro repository."

  if [ $SHOW_PROGRESS -lt 0 ]
  then
    return
  fi

  pushd $SRC_ROOT > /dev/null
  cvs -q update | grep "? " > $TMPFILE_1
  if [ -s $TMPFILE_1 ]
  then
    sed -e 's/\? //' < $TMPFILE_1 > $TMPFILE_2
    xargs cvs -q add < $TMPFILE_2
  fi
  popd > /dev/null

  if [ $SHOW_PROGRESS -gt -1 ]
  then
    look_for_conflict_markers
  fi
}

check_toolchains_build ()
{
  report " Checking that toolchains still build."

  if [ $SHOW_PROGRESS -lt 0 ]
  then
    return
  fi

  pushd $BUILD_ROOT > /dev/null
  # make
  make all-gcc
  popd > /dev/null
}

commit_changes ()
{
  report " Commiting changes to the GNUPro repository"

  if [ $SHOW_PROGRESS -lt 0 ]
  then
    return
  fi

  pushd $SRC_ROOT > /dev/null
  cvs -q commit -m"Latest imports from gcc 4.5, gdb 7.0, binutils 2.20 and newlib 1.18"
  popd > /dev/null
}

# Check to see if the argument is an executable program in the user's PATH.
# Set RETURN_found to 1 if found, 0 if not found.
# Set RETURN_fullname to the found path + filename or "".
find_in_path ()
{
  local OFS i target dir

  if test "x$1" = "x" ;
  then
    fail "find_in_path called with too few arguments."
  fi
  if test "x$2" != "x" ;
  then
    fail "find_in_path called with too many arguments."
  fi

  # Check to see if we have been given an absolute path.
  if test "x${1:0:1}" == "x/" ;
  then
    if [ -x $1 ]
    then
      RETURN_filename=$1
      RETURN_found=1
    fi
  else
    target="$1"
    RETURN_fullname=""
    RETURN_found=0
    OFS="$IFS"
    IFS=:
    for i in $PATH
    do
      [ -z "$i" ] && i="."
      if [ -x "$i/$target" ]
      then
        RETURN_fullname="$i/$target"
        RETURN_found=1
        break
      fi
    done
    IFS="$OFS"
  fi
}

# syntax: run <command> [<args>]
#  If being verbose report the command being run, and
#   the directory in which it is run.
#  If not performing a dry run execute the command and
#   issue a warning message if the command fails.
run ()
{
  local where

  if test "x$1" = "x" ;
  then
    fail "run() called without an argument."
  fi

  where=${PWD##$SRC_ROOT/}

  verbose "  Regenerate in $where (using $1)"

  if [ $SHOW_PROGRESS -lt 0 ]
  then
    return
  fi

  ( ${1+$@} ) || warn "Problems encountered whilst regenerating in $where using ${1+$@}"
}

arg_regenerate ()
{
  local A command arg

  if test "x$3" = "x" ;
  then
    fail "arg_regenerate called with too few arguments."
  fi
  command="$1"
  shift
  arg="$1"
  shift
  
  find_in_path $command
  if [ $RETURN_found -eq 1 ]
  then
    for A in ${1+"$@"}
    do
      if [ ! -d $A ]
      then
        verbose "  Could not regenerate in $A - directory not found."
      else
        cd $A
        verbose "  Regenerate in $A (using $command)"
        if [ $SHOW_PROGRESS -gt -1 ]
	then
          run $command $arg
        fi
        cd - > /dev/null
      fi
    done
  else
    warn "Could not find $command in PATH."
  fi
}

regenerate ()
{
  local command

  if test "x$2" = "x" ;
  then
    fail "regenerate called with too few arguments."
  fi
  command="$1"
  shift

  arg_regenerate $command "-I .. -I ../config" ${1+"$@"}
}

# This function is cribbed from Jeff Johnston's reconf.sh script.

regenerate_in_newlib ()
{
  local A

  report " Rebuilding in newlib..."
  
  pushd $SRC_ROOT/newlib > /dev/null

  # run $AUTORECONF
  run $ACLOCAL -I . -I ..
  run $AUTOCONF
  run $AUTOMAKE Makefile
  
  cd doc

  # run $AUTORECONF
  run $ACLOCAL -I ..
  run $AUTOCONF
  run $AUTOMAKE Makefile

  cd ../iconvdata

  # run $AUTORECONF
  run $ACLOCAL -I .. -I ../..
  run $AUTOCONF
  run $AUTOMAKE Makefile
  
  cd ../libc
  
  # run $AUTORECONF
  run $ACLOCAL -I .. -I ../..
  run $AUTOCONF
  run $AUTOMAKE Makefile
  run $AUTOMAKE argz/Makefile ctype/Makefile errno/Makefile iconv/Makefile \
    iconv/lib/Makefile iconv/ccs/Makefile iconv/ccs/binary/Makefile \
    iconv/ces/Makefile locale/Makefile misc/Makefile posix/Makefile \
    reent/Makefile search/Makefile signal/Makefile stdio/Makefile \
    stdio64/Makefile stdlib/Makefile string/Makefile syscalls/Makefile \
    time/Makefile unix/Makefile 

  cd machine
  
  # run $AUTORECONF
  run $ACLOCAL -I ../.. -I ../../..
  run $AUTOCONF
  run $AUTOMAKE  Makefile

  for A in *;
  do
    if test -d $A;
    then
      if [ x$A == xCVS ] || [ x$A == xautom4te.cache ]
      then
        continue
      fi
      
      cd $A
      # run $AUTORECONF
      run $ACLOCAL -I ../../.. -I ../../../..
      run $AUTOCONF
      run $AUTOMAKE Makefile
      cd - > /dev/null
    fi
  done
  
  cd ../sys

  # run $AUTORECONF
  run $ACLOCAL -I ../.. -I ../../..
  run $AUTOCONF
  run $AUTOMAKE Makefile
  
  for A in *;
  do
    if test -d $A;
    then
      if [ x$A == xCVS ] || [ x$A == xautom4te.cache ]
      then
        continue
      fi
      
      cd $A
      # run $AUTORECONF
      run $ACLOCAL -I ../../.. -I ../../../..
      run $AUTOCONF
      run $AUTOMAKE Makefile
      cd - > /dev/null
    fi
  done

  cd linux

  run $AUTOMAKE argp/Makefile cmath/Makefile dl/Makefile iconv/Makefile \
    intl/Makefile net/Makefile stdlib/Makefile
    
  cd machine

  # run $AUTORECONF
  run $ACLOCAL -I ../../../.. -I ../../../../..
  run $AUTOCONF
  run $AUTOMAKE Makefile
  
  cd i386

  # run $AUTORECONF
  run $ACLOCAL -I ../../../../.. -I ../../../../../..
  run $AUTOCONF
  run $AUTOMAKE Makefile
  
  cd ../../linuxthreads
  
  # run $AUTORECONF
  run $ACLOCAL -I ../../../.. -I ../../../../..
  run $AUTOCONF
  run $AUTOMAKE Makefile

  cd machine

  # run $AUTORECONF
  run $ACLOCAL -I ../../../../.. -I ../../../../../..
  run $AUTOCONF
  run $AUTOMAKE Makefile

  cd i386

  # run $AUTORECONF
  run $ACLOCAL -I ../../../../../.. -I ../../../../../../..
  run $AUTOCONF
  run $AUTOMAKE Makefile
  
  cd $SRC_ROOT/newlib/libm

  # run $AUTORECONF
  run $ACLOCAL -I .. -I ../..
  run $AUTOCONF
  run $AUTOMAKE Makefile
  run $AUTOMAKE common/Makefile math/Makefile mathfp/Makefile

  cd machine

  # run $AUTORECONF
  run $ACLOCAL -I ../.. -I ../../..
  run $AUTOCONF
  run $AUTOMAKE Makefile

  cd i386

  # run $AUTORECONF
  run $ACLOCAL -I ../../.. -I ../../../..
  run $AUTOCONF
  run $AUTOMAKE Makefile

  cd ../spu

  # run $AUTORECONF
  run $ACLOCAL -I ../../..
  run $AUTOCONF
  run $AUTOMAKE Makefile

  verbose "  Done"

  popd > /dev/null
}

regenerate_for_binutils ()
{
  run $ACLOCAL -I .. -I ../bfd -I ../config
  run $AUTOCONF
  run $AUTOMAKE
}

regenerate_files ()
{
  local A

  report " Rebuilding auto-generated files..."

  pushd $SRC_ROOT > /dev/null

  regenerate_in_newlib

  report " Rebuilding in binutils directories..."

  for A in bfd binutils gas gprof ld opcodes 
  do
    if [ ! -d $A ]
    then
      warn "Could not regenerate $A - directory not found."
    else
      cd $A
      regenerate_for_binutils
      cd ..
    fi
  done

  verbose "  Done"

  report " Rebuilding top level directories..."
    
  regenerate $AUTOCONF \
      bison boehm-gc cgen dejagnu dejagnu etc fastjar fixincludes \
      gcc gdb gmp gold guile intl libada libcpp libdecnumber libffi \
      libgcc libgfortran libgloss libgomp libiberty libjava \
      libmudflap libssp libstdc++-v3 make mmalloc mpfr readline \
      sid sim texinfo utils zlib .

  # regenerate autoconf-2.13 \
  #    expect itcl libtermcap tcl tk

  verbose "  Done"
  report " Rebuilding in libgloss sub-directories..."
  
  for A in  arm  bfin  cris  crx  d30v  fr30  frv  i386  i960 \
            iq2000  libnosys  lm32  m32c  m32r  m68hc11  m68k \
            mcore  mep  mips  mn10200  mn10300  mt pa  rs6000 \
	    rx  sparc  spu  xstormy16
  do
    if [ ! -d libgloss/$A ]
    then
      warn "Could not regenerate libgloss/$A - directory not found."
    else
      cd libgloss/$A
      run $AUTOCONF -I ..
      cd - > /dev/null
    fi
  done

  verbose "  Done"
  report " Rebuilidng in simulator sub-directories..."

  for A in  arm  common  cris  d10v  erc32 fr30  frv \
            h8300   igen  iq2000  m32c   m32r \
	    m68hc11  mcore  mips  mn10300 ppc  rx \
            sh   sh64  testsuite  v850  
  do
    if [ ! -d sim/$A ]
    then
      warn "Could not regenerate sim/$A - directory not found."
    else
      cd sim/$A
      run $AUTOCONF -I ..
      cd - > /dev/null
    fi
  done

  verbose "  Done"
  report " Rebuilding top level makefiles..."  
 
  regenerate $AUTOMAKE \
      bfd binutils bison boehm-gc dejagnu gas gprof ld libffi \
      libgfortran libgomp libjava libmudflap libssp libstdc++-v3 \
      mpfr newlib opcodes zlib

  # skip texinfo for now, regenerating it breaks lots of builds.
  
  arg_regenerate autogen Makefile.def .

  # FIXME: Regenerate Makefile.in for: bison cgen fastjar gmp guile libgui libtermcap make sid texinfo
  # FIXME: Regenerate BFD headers ?
  # FIXME: Regenerate yacc and lex files ?

  verbose "  Done"
  popd > /dev/null
}

find-nonrepository-directories ()
{
  local A

  report "The following directories are not checked into the GNUPro repository:"
  
  if [ $SHOW_PROGRESS -lt 0 ]
  then
    return
  fi

  pushd $SRC_ROOT > /dev/null

  for A in `find . -type d \( -name .svn -prune -o -name CVS -prune -o -name $CVS_GNUPRO -prune -o -name $CVS_FSF -prune -o -print \)`
  do
    if [ ! -d $A/$CVS_GNUPRO ]
    then
      report $A 
    fi
  done

  popd > /dev/null
}

find-deleted-dirs ()
{
  local A B=xxx

  report "The following directories are not checked into any FSF repository:"
  
  if [ $SHOW_PROGRESS -lt 0 ]
  then
    return
  fi

  pushd $SRC_ROOT > /dev/null

  # FIXME: We should also parse the output from "svn update" at
  # the top level to find deleted directories in the GCC sources.
  
  for A in `find $CVS_DIRS -type d \( -name CVS -prune -o -name $CVS_GNUPRO -prune -o -name $CVS_FSF -prune -o -print \)`
  do
    if [ ! -d $A/$CVS_FSF ] && [ ! -d $A/.svn ]
    then
      if  [ -f $A/.FSF_ignore ]
      then
	verbose "Ignoring $A"
	B=$A
      elif [[ $A =~ $B/* ]]
      then
	  # Only report this directory in verbose mode:
	  # It is a sub-directory of a previously reported directory.
	  verbose " ($A)"
      else
          report $A
	  B=$A
      fi
    fi
  done

  popd > /dev/null
}

find-deleted-files ()
{
  local A current_dir
    
  report "The following files are not checked in to any FSF repository"
  
  if [ $SHOW_PROGRESS -lt 0 ]
  then
    return
  fi

  pushd $SRC_ROOT > /dev/null

  for A in `find . -type d -name $CVS_FSF`
  do
    if [ -d CVS ] && [ -d $CVS_GNUPRO ]
    then
      current_dir=`dirname $A`
      verbose "  Checking: $current_dir"
      pushd $current_dir > /dev/null
      mv CVS ../.CVS.current
      mv $CVS_FSF CVS
      cvs -q update -l . | grep \? | awk --assign dir=$current_dir 'FS=" " { print dir"/"$2 }'
      mv CVS $CVS_FSF
      mv ../.CVS.current CVS
      popd > /dev/null
    fi
  done

  for A in `find . -type d -name .svn`
  do
    if [ -d CVS ] && [ -d $CVS_GNUPRO ]
    then
      current_dir=`dirname $A`
      verbose "  Checking: $current_dir"
      svn status --non-interactive --depth=files $current_dir \
       | grep \? | awk 'FS=" " { print $2 }'
    fi
  done
  
  popd > /dev/null
}

# Invoke main

main ${1+"$@"}
