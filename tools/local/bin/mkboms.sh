#!/bin/bash
#
# mkbom -- Generate boms from old manifests
#
# Angela Marie Thomas
# $Date: 2006/07/06 21:03:07 $
# 
# $Id: mkboms.sh,v 1.5 2006/07/06 21:03:07 blc Exp $
#

LANG=C
export LANG

# use /bin/sh5 on Ultrix systems to get shell functions
if [ -f /bin/sh5 ] ; then 
   if [ ! -n "${RUNNING_UNDER_BIN_SH5}" ] ; then
       RUNNING_UNDER_BIN_SH5=yes
       export RUNNING_UNDER_BIN_SH5
       exec /bin/sh5 $0 $*
   fi
fi

ProgName=`basename $0`
Version='$Revision: 1.5 $'
RHPrefix="/opt/redhat"

# Begin shell functions

#
# usage()
# Quickie "how do I use this thing?"
#
usage()
{
cat << EndOfUsage
$ProgName $Version
Usage: $ProgName [options] --release=<release>
Options:
  --host=<host>                     Host
  --target=<target>                 Target(s)
  --board=boardname                 Optional name of board for use with BSPs
  --release=<release>               release name (of the form YYqQ or
                                       <prefix>-<datestamp> for customs)
  --vault=<vault>                   Alternate release vault.  Default is
                                       /opt/redhat/release-vault/<release>/new
  --x                               set -x for shell debugging
  --version                         version
  --verbose                         verbose
  --help                            This message
EndOfUsage
}

#
# Vecho($string)
# echo string if verbose enabled
#
Vecho () {
    [ "$Verbose" = "$TRUE" ] && echo "$*"
    return 0
}

#
# Decho($string)
# echo string if verbose enabled
#
Decho () {
    [ "$Debug" = "$TRUE" ] && echo "$*"
    return 0
}

#
# dirname($dir)
# Print parent directory of $dir
#
dirname() {
    pdir="`echo $1 | sed -e 's;^\(.*\)/\([^/]*\)$;\1;'`"
    if [ "$pdir" = "" -o "$pdir" = "$1" ]; then
        pdir="."
    fi
    echo "$pdir"
}

#
# pmkdir($dir)
# mkdir -p $dir
#
pmkdir() {
    while [ $# -gt 0 ]; do
        if [ ! -d $1 ]; then
            if [ ! -d `dirname $1` ]; then
                 pmkdir `dirname $1`
                 [ $? -eq 0 ] || return $?
            fi
            mkdir $1 || return 1
        fi
        shift
    done
}

# End shell functions

# arg parsing time
while [ $# -ne 0 ]; do
    option=$1
    shift

    case $option in
    --*=*)
        optarg=`echo $option | sed -e 's/^[^=]*=//'`
        ;;
    esac

    case $option in
    --x)
        set -x
        ;;
    --host*)
        host="$optarg"
        ;;
    --targ*)
        Target="${Target} $optarg"
        ;;
    --rel*)
        release="$optarg"
        ;;
    --prod*)
        Product="$optarg"
        ;;
    --comp*)
        CompList="$optarg"
        ;;
    --pack*)
        PackList="$optarg"
        ;;
    --title*)
        title="$optarg"
        ;;
    --board*)
        boardname="$optarg"
        ;;
    --vault*)
        Vault="$optarg"
        ;;
    --vers*)
        echo "$ProgName $Version"
        exit 0
        ;;
    --verb*)
        Verbose=$TRUE
        ;;
    --debug*)
        Debug=$TRUE
        ;;
    --help)
        usage
        exit 0
        ;;
    *)
        echo "$ProgName: Invalid argument \"$option\""
        usage
        exit 1
        ;;
    esac
done

# if release not given, exit
if [ "$release" = "" ]; then
    echo "$ProgName: release is null."
    usage
    exit 1
fi
# if host not given, exit
if [ "$host" = "" ]; then
    echo "$ProgName: host is null."
    usage
    exit 1
fi
# if target not given, exit
if [ "$Target" = "" ]; then
    echo "$ProgName: target is null."
    usage
    exit 1
fi

TAR=${TAR:="tar"}
ZIP=${ZIP:="zip"}
GZIP=${GZIP:="gzip"}

Product=${Product:="gnupro"}
Vault=${Vault:="$RHPrefix/release-vault/$release/new"}
MOBDir="$Vault/templates"
ManiDir="$Vault/manifests"

# some other process creates these dirs right now
if [ ! -d $MOBDir ]; then
    echo "${ProgName}: $MOBDir doesn't exist.  Exiting."
    exit 1
fi
if [ ! -d $ManiDir ]; then
    echo "${ProgName}: $ManiDir doesn't exist.  Exiting."
    exit 1
fi

case $host in
    i[3456789]86*linux-gnulibc2.1)
        basehost=nocontrib
        ;;
    i[3456789]86*linux-gnulibc2.[23])
        basehost=i686-pc-linux-gnulibc2.2
        ;;
    i[3456789]86*-cygwin*)
        basehost=i686-pc-cygwin
        ;;
    sparc*-sun-solaris2.5*)
        basehost=nocontrib
        ;;
    sparc*-sun-solaris2.[6789]*)
        basehost=sparc-sun-solaris2.6
        ;;
    hppa*-hp-hpux1[01].*)
        basehost=hppa1.1-hp-hpux10.20
        ;;
    powerpc*-ibm-aix*)
        basehost=powerpc-ibm-aix4.3.3.0
        ;;
    *)
	basehost=$host
	;;
esac

#case "$boardname" in
#    grg)
#        boardarch=armv5b
#        ;;
#    ix*p*)
#        boardarch=armv5b
#        ;;
#    cerf*)
#        boardarch=armv5l
#        ;;
#    mizi)
#        boardarch=armv5
#        ;;
#    iq80321)
#        boardarch=armv5l
#        ;;
#    ppmc750fx)
#        boardarch=ppc
#        ;;
#    sbc82xx)
#        boardarch=ppc
#        ;;
#    cpci690)
#        boardarch=ppc
#        ;;
#    civet)
#        boardarch=ppc
#        ;;
#    *)
#	boardname="nosuchname"
#	boardarch="nosucharch"
#	;;
#esac

if [ X$boardname = X ]; then
	boardname="nosuchname"
fi


for target in $Target; do
    # set targetPrefix if not native
    if [ "$host" != "$target" ]; then
        targetPrefix="${target}-"
        targetArch="`echo $target | awk -F- '{print $1}'`"
    else
        targetPrefix=""
        targetArch="`echo $host | awk -F- '{print $1}'`"
    fi

    # set EXEEXT for cygwin
    case $host in
        *cygwin*) EXEEXT=".exe" ;;
        *) ;;
    esac
    export release host target EXEEXT targetPrefix targetArch

    if [ "X$boardname" = Xnosuchname ]; then
      echo "Creating $host-x-$target BOM for release $release of $Product"
      bomdir="$Vault/bom/$host/$target/packages"
    else
      echo "Creating $host-x-$target ($boardname) BOM for release $release of $Product"
      bomdir="$Vault/bom/$host/$target/$boardname/packages"
    fi
    compfile="$bomdir/pkgdata/${Product}.components"
    manifest="$ManiDir/$Product/${host}-manifest"

    test -d $bomdir || pmkdir $bomdir

    # copy over MOB files we need
    # is this right??????
    (cd $MOBDir; tar cf - pkgdata) | (cd $bomdir; tar xpf -)

    gccmajorver="`basename ${RHPrefix}/${release}/H-${host}/lib/gcc/${target}/* | sed 's/-.*//'`"
    if [ "$gccmajorver" = "*" ]; then
	gccmajorver=nogcc
    fi

    # create relinfo file
    rm -f $bomdir/relinfo
    echo ${title}              > $bomdir/relinfo
    echo ${host}              >> $bomdir/relinfo
    echo ${target}            >> $bomdir/relinfo
    echo "${gccmajorver}"     >> $bomdir/relinfo
    echo "${release}"         >> $bomdir/relinfo

    # create boms for each package of each component
    components=${CompList:="`egrep -v '^#|^$' $compfile`"}
    for comp in $components; do
        for ptype in native shared TARGET; do
	    if [ "$ptype" = "TARGET" ]; then 
	        insdir=$bomdir/$target/$Product/$comp
		btype=$target
	    else
	        insdir=$bomdir/$ptype/$Product/$comp
		btype=$ptype
	    fi
            # if no insdir, create
            test -d $insdir || pmkdir $insdir
    
            if [ "$PackList" != "" ]; then
                packages="$PackList"
            else
                packages=`find $MOBDir/$ptype/$Product/$comp -name "*.template" -exec basename {} .template \; 2>/dev/null`
            fi
            for pack in $packages; do
                templatefile="$MOBDir/$ptype/$Product/$comp/${pack}.template"
                bomfile="$bomdir/$btype/$Product/$comp/${pack}.bom"
                rm -f $bomfile
                egrep -v '^#|^$' $templatefile | while read type regexp when; do
                    for foo in $when; do
                        case $foo in
                            native)
                                [ "$host" = "$target" ] || continue 2 ;;
                            cygwin*)
                                if [ x"`echo $host | grep -i cygwin`" = x -a \
                                     x"`echo $target | grep -i cygwin`" = x ]; then
                                    continue 2
                                fi
                                ;;
                            nog++)
                                case $target in
                                    d10v*) continue 2 ;;
                                    *) ;;
                                esac
                                ;;
                            *) ;;
                        esac
                    done
    
                    case $type in
                        file)
                            eval grep "\"^${regexp}$\"" $manifest >> $bomfile ;;
                        dir)
                            eval grep "\"^${regexp}/\"" $manifest >> $bomfile ;;
                        # this is quite the hack.  i hate fixincludes
                        fixinc)
                            case $host in
                            #    *cygwin*|*solaris*|*linux*)
                            #        eval grep "\"^${regexp}/\"" $manifest >> $bomfile ;;
                                *)
                                    fixincexp="H-${host}/lib/gcc-lib/${host}/.*/include/"
                                    eval grep "\"^${regexp}/\"" $manifest | \
                                         grep -v "${fixincexp}" >> $bomfile ;;
                            esac
                            ;;
                        # I hate libstdc++, too
                        stdcrap)
                            stdcrapexp="H-${host}/lib/gcc-lib/"
                            eval grep "\"^${regexp}$\"" $manifest | \
                                 grep -v "${stdcrapexp}" >> $bomfile ;;
                        nosrc)
                            nosrc="usr/src/"
                            eval grep "\"^${regexp}$\"" $manifest | \
                                 grep -v "${stdcrapexp}" >> $bomfile ;;
                        *)
                            echo "${ProgName}: Unknown type \"$type\", skipping." ;;
                    esac
                done
                # makes comm easier to use
                test -f $bomfile && sort -u -o $bomfile $bomfile
            done
        done
    done
done
exit 0
