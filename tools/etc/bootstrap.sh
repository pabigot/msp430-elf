#!/bin/sh

# Bootstrapping a compiler.  Should be done in parallel directory
# from sources.

# Variables needed

TRUE=1
FALSE=0
blddir="`pwd`"

# /opt/ansic/bin must be before /usr/ccs/bin for HPUX
# /usr/vac/bin is for AIX
# /opt/SUNWspro/bin and /usr/xpg4/bin are for Solaris
# /usr/xpg4/bin must be before /bin and /usr/bin to find the right grep
PATH=/usr/vac/bin:/opt/ansic/bin:/usr/ccs/bin:/opt/SUNWspro/bin:\
/usr/xpg4/bin:/usr/sbin:/sbin:/bin:/usr/bin:/usr/ucb:/usr/bsd
export PATH

# reasonable umask
umask 0002

# go away on certain signals
trap "echo \"$ProgName: Caught a sig @ \`date\`, exiting.\" ; exit 1"  1 2 3 15

# Shell functions
#
# End Shell Functions

srcdir=${srcdir:="$blddir/gnupro"}

# arg parsing time
while [ $# -ne 0 ]; do
    option=$1
    shift

    optarg=""
    case $option in
    --*=*)
        optarg=`echo $option | sed -e 's/^[^=]*=//'`
        ;;
    esac

    case $option in
    --host*)
        host=$optarg
        targets=$optarg
        build=$optarg
        ;;
    --src*)
        srcdir=$optarg
        ;;
    --bin*)
        bindir=$optarg
        ;;
    *)
        echo "$ProgName: Invalid argument \"$option\""
        usage
        exit 1
        ;;
    esac
done

host=${host:=`$srcdir/config.guess`}

if [ "$host" = "hppa2.0w-hp-hpux11.00" ]; then
    host="hppa64-hp-hpux11.00"
fi

build=${build:=$host}
targets=${build:=$host}
bindir=${bindir:="$blddir/bin"}

PATH=$bindir:$PATH
export PATH

# set these before product specific case so we can change them
case $host in
    *linux*) MAKE=${MAKE:="make"} ;;
    *)       if [ -f "/usr/progressive/bin/make" ]
             then
               MAKE=${MAKE:="/usr/progressive/bin/make"}
             elif type gmake > /dev/null
	     then
               MAKE=${MAKE:="gmake"}
	     else
               MAKE=${MAKE:="make"}
             fi
             ;;

esac
export MAKE

case $host in
    *cygwin*)
	MAKE_1_TARGETS="all-gas all-ld all-binutils all-gcc all-target-libiberty all-target-libtermcap all-target-libstdc++-v3 all-target-newlib all-target-winsup"
	INSTALL_1_TARGETS="install-gas install-ld install-binutils install-gcc install-target-libiberty install-target-libtermcap install-target-libstdc++-v3 install-target-newlib install-target-winsup"
        STAGE1_LANGUAGE_OPT="--enable-languages=c,c++"
	;;
    *aix*)
        if type bash > /dev/null
	then
	  CONFIG_SHELL=bash
          export CONFIG_SHELL
	fi
        MAKE_1_TARGETS="all-gcc"
	INSTALL_1_TARGETS="install-gcc"
	STAGE1_LANGUAGE_OPT="--enable-languages=c"
	;;
    *)
	MAKE_1_TARGETS="all-gas all-ld all-binutils all-gcc"
	INSTALL_1_TARGETS="install-gas install-ld install-binutils install-gcc"
        STAGE1_LANGUAGE_OPT="--enable-languages=c"
	;;
esac

case $host in
    *aix*)
	MAKE_23_TARGETS="all"
	INSTALL_23_TARGETS="install"
	;;
    *)
	MAKE_23_TARGETS="all-gas all-ld all-binutils all"
	INSTALL_23_TARGETS="install-gas install-ld install-binutils install"
	;;
esac

# Enable these definitions for a faster/smaller bootstrap test.
#
# MAKE_23_TARGETS="all-gas all-ld all-binutils all-gcc"
# INSTALL_23_TARGETS="install-gas install-ld install-binutils install-gcc"

# Host specific tweaks and options
CONFIG_OPTS=${CONFIG_OPTS:=""}
case $host in
    *aix*)
        MSGFMT=/bin/true export MSGFMT
	CONFIG_OPTS="${CONFIG_OPTS} --disable-nls"
	;;
    *cygwin*)
	CONFIG_OPTS="${CONFIG_OPTS} --disable-nls"
	;;
esac

# main routine
for targ in $targets; do
    cd $blddir

# Stage 1
    test -d stage1 || \
        $Echo mkdir stage1
    $Echo cd stage1

# Change CC to gcc for everything...

    if [ "$host" = "hppa64-hp-hpux11.00" ]; then
        CC="gcc"
        PATH=/usr/cygnus/hppa-000310/H-hppa2.0w-hp-hpux11.00/bin:$PATH
        export CC PATH
    #else
    #    CC=gcc
    #    export CC
    fi

    echo "begin configuring stage1 ${host} @ `date` on `hostname`"
    $Echo $srcdir/configure -v \
        --host=$host \
        --build=$build \
        --prefix=$blddir/install-stage1 \
        --target=$targ \
	--bootstrap \
        ${STAGE1_LANGUAGE_OPT} \
	${CONFIG_OPTS} \
        >> configure.out 2>&1
        status=$?

        if [ $status != 0 ]; then
            echo "Error configuring stage1"
            Errors=1
            Errmsg="configure failed in stage 1 for $host-x-$targ in $blddir"
            exit 1
        else
            $Echo touch "./---CONFIGURED---"
        fi
    echo "finished configuring stage1 ${host} @ `date` on `hostname`"

    echo "begin building stage1 ${host} @ `date` on `hostname`"
    $Echo eval $MAKE $MAKE_1_TARGETS >> make.log 2>&1
    status=$?
    echo "finished building stage1 ${host} @ `date` on `hostname`"

        if [ $status != 0 ]; then
            echo "Error building stage1"
            Errors=1
            Errmsg="build failed in stage 1 for $host-x-$targ in $blddir"
            exit 1
        else
            $Echo touch "./---BUILT---"
        fi

    echo "begin installing stage1 ${host} @ `date` on `hostname`"
    $Echo eval $MAKE $INSTALL_1_TARGETS >> install.log 2>&1
    status=$?

    case $host in
      *cygwin*)	mv $blddir/install-stage1/bin/cygwin1.dll $blddir/install-stage1/bin/new-cygwin1.dll ;;
      *) ;;
    esac

    echo "finished installing stage1 ${host} @ `date` on `hostname`"

        if [ $status != 0 ]; then
            echo "Error install stage1"
            Errors=1
            Errmsg="install failed in stage 1 for $host-x-$targ in $blddir"
            exit 1
        else
            $Echo touch "./---INSTALLED---"
        fi


#stage 2

    CC="gcc"
    PATH=$blddir/install-stage1/bin:$PATH
    export CC PATH

    cd $blddir

    # Note - we call the directory 'stageX' not 'stage2'.  When we are
    # done we will rename this directory to stage2 and then build stage3
    # in a directory named 'stageX'.  This is so that any path names
    # embeded in the binaries created by stage2 and stage3 should be
    # exactly the same (and hash to same value if stored in a hash table,
    # as are debug strings genreated by gcc for the -g switch).
    
    test -d stageX || \
        $Echo mkdir stageX
    $Echo cd stageX

    echo "begin configuring stage2 ${host} @ `date` on `hostname`"
    $Echo $srcdir/configure -v \
        --host=$host \
        --build=$build \
	--bootstrap \
        --prefix=$blddir/install-stage2 \
        --enable-languages=c,c++ \
        --target=$targ \
	${CONFIG_OPTS} \
        >> configure.out 2>&1
    status=$?
    echo "finished configuring stage2 ${host} @ `date` on `hostname`"

        if [ $status != 0 ]; then
            echo "Error configuring stage2"
            Errors=1
            Errmsg="configure failed in stage 2 for $host-x-$targ in $blddir"
            exit 2
        else
            $Echo touch "./---CONFIGURED---"
        fi

    echo "begin building stage2 ${host} @ `date` on `hostname`"
    $Echo eval $MAKE $MAKE_23_TARGETS >> make.log 2>&1
    status=$?
    echo "finished building stage2 ${host} @ `date` on `hostname`"

        if [ $status != 0 ]; then
            echo "Error building stage2"
            Errors=1
            Errmsg="build failed in stage 2 for $host-x-$targ in $blddir"
            exit 2
        else
            $Echo touch "./---BUILT---"
        fi

    echo "begin installing stage2 ${host} @ `date` on `hostname`"
    $Echo eval $MAKE $INSTALL_23_TARGETS >> install.log 2>&1
    status=$?

    case $host in
      *cygwin*)	mv $blddir/install-stage2/bin/cygwin1.dll $blddir/install-stage2/bin/new-cygwin1.dll ;;
      *) ;;
    esac

    echo "finished installing stage2 ${host} @ `date` on `hostname`"

        if [ $status != 0 ]; then
            echo "Error install stage2"
            Errors=1
            Errmsg="install failed in stage 2 for $host-x-$targ in $blddir"
            exit 2
        else
            $Echo touch "./---INSTALLED---"
        fi

    # Rename the stageX directory to stage2 so that it
    # can be found/does not get overwritten by stage3.
    
    cd $blddir
    mv stageX stage2
    
    # stage 3

    CC=gcc
    PATH=$blddir/install-stage2/bin:$PATH
    export CC PATH

    cd $blddir

    # See comment at start of stage 2.
    
    test -d stageX || \
        $Echo mkdir stageX
    $Echo cd stageX

    echo "begin configuring stage3 ${host} @ `date` on `hostname`"
    $Echo $srcdir/configure -v \
        --host=$host \
        --build=$build \
        --prefix=$blddir/install-stage3 \
        --enable-languages=c,c++ \
        --target=$targ \
	${CONFIG_OPTS} \
        >> configure.out 2>&1
    status=$?
    echo "finished configuring stage3 ${host} @ `date` on `hostname`"

        if [ $status != 0 ]; then
            echo "Error configuring stage3"
            Errors=1
            Errmsg="configure failed in stage 3 for $host-x-$targ in $blddir"
            exit 3
        else
            $Echo touch "./---CONFIGURED---"

        fi

    echo "begin building stage3 ${host} @ `date` on `hostname`"
    $Echo eval $MAKE $MAKE_23_TARGETS >> make.log 2>&1
    status=$?
    echo "finished building stage3 ${host} @ `date` on `hostname`"

        if [ $status != 0 ]; then
            echo "Error building stage3"
            Errors=1
            Errmsg="build failed in stage 3 for $host-x-$targ in $blddir"
            exit 3
        else
            $Echo touch "./---BUILT---"
        fi

    echo "begin installing stage3 ${host} @ `date` on `hostname`"
    $Echo eval $MAKE $INSTALL_23_TARGETS >> install.log 2>&1
    status=$?

    case $host in
      *cygwin*)	mv $blddir/install-stage3/bin/cygwin1.dll $blddir/install-stage3/bin/new-cygwin1.dll ;;
      *) ;;
    esac

    echo "finished installing stage3 ${host} @ `date` on `hostname`"

        if [ $status != 0 ]; then
            echo "Error install stage3"
            Errors=1
            Errmsg="install failed in stage 3 for $host-x-$targ in $blddir"
            exit 3
        else
            $Echo touch "./---INSTALLED---"
        fi

    # See comment at end of stage 2.
    
    cd $blddir
    mv stageX stage3
    
    # Checking

    echo "begin checking 3stage ${host} @ `date` on `hostname`"

    TmpFile1=/tmp/cmp1$$
    TmpFile2=/tmp/cmp2$$
    CompareLog=$blddir/$host-compare.log
    Message1=" present in stage2 but not in stage3"
    Message2=" present in stage3 but not in stage2"
    Message3=" differs between stage2 and stage3"
    $Echo rm -f $CompareLog
    $Echo cd $blddir

    ( cd $blddir/stage2 ; find . -name '*.[o]' -print ) | while read filename
    do
	if [ ! -f $blddir/stage3/$filename ]
	then
	    echo $filename $Message1 >> $CompareLog 2>&1
	else
	    tail +16c stage2/$filename > $TmpFile1
	    tail +16c stage3/$filename > $TmpFile2
	    err="`cmp -l $TmpFile1 $TmpFile2 | grep -v -e ' 61  *62$' -e ' 62  *63$' | wc -l`"
	    if [ $err != 0 ]; then
		echo $filename $Message3 >> $CompareLog 2>&1
	    fi
	fi
    done
    rm $Tmpfile1 $TmpFile2

    if [ -f $CompareLog ] ; then
	    echo "stage2 and stage3 are different"
	    exit 1
    fi

    echo "finished checking 3stage ${host} @ `date` on `hostname`"

done
exit $Errors
