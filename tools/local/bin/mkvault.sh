#!/bin/sh

#
# mkvault -- Vault an entire release, mark 2
#
# Angela Marie Thomas
# $Date: 2004/11/29 21:45:33 $
# 
# $Id: mkvault.sh,v 1.3 2004/11/29 21:45:33 release Exp $
#

# use /bin/sh5 on Ultrix systems to get shell functions
if [ -f /bin/sh5 ] ; then 
   if [ ! -n "${RUNNING_UNDER_BIN_SH5}" ] ; then
       RUNNING_UNDER_BIN_SH5=yes
       export RUNNING_UNDER_BIN_SH5
       exec /bin/sh5 $0 $*
   fi
fi

ProgName=`basename $0`
Version='$Revision: 1.3 $'

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
  --board=boardname                 Board name
  --release=<release>               Release name (of the form YYqQ or
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
    --board*)
        boardname="$optarg"
        ;;
    --rel*)
        Release="$optarg"
        ;;
    --prod*)
        Product="$optarg"
        ;;
    --vault*)
        Vault="$optarg"
        ;;
    --comp*)
        CompList="$optarg"
        ;;
    --pack*)
        PackList="$optarg"
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
if [ "$Release" = "" ]; then
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
Vault=${Vault:="/opt/redhat/release-vault/$Release/new"}
IHackDir=$Vault/include-hack

# some other process creates these dirs right now
if [ ! -d $Vault ]; then
    echo "${ProgName}: $Vault doesn't exist.  Exiting."
    exit 1
fi

#if [ ! -d $IHackDir ]; then
#    echo "${ProgName}: $IHackDir doesn't exist.  Can't vault some natives."
#fi

for target in $Target; do
    echo "Vaulting $host-x-$target for $Release of $Product"
    if [ X$boardname = X ]; then
        bomdir="$Vault/bom/$host/$target/packages"
        boardname=none;
    else
        bomdir="$Vault/bom/$host/$target/$boardname/packages"
    fi
    compfile="$bomdir/pkgdata/${Product}.components"

    # if no bomdir, skip
    if [ ! -d $bomdir ]; then
        echo "${ProgName}: $bomdir does not exist.  Skipping $host-x-$target (Board:$boardname)"
        continue
    fi
    # if no components file and $CompList not set, skip
    if [ ! -f $compfile -a "$CompList" = "" ]; then
        echo "${ProgName}: No components found.  Skipping $host-x-$target (Board:$boardname)"
        continue
    fi
    components=${CompList:="`egrep -v '^#|^$' $compfile`"}
    if [ "$components" = "" ]; then
        echo "${ProgName}: No components found.  Skipping $host-x-$target (Board:$boardname)"
        continue
    fi

    # bundle relative to /opt/redhat
    cd /opt/redhat

    # if gnupro and (native or vxworks), set IHACK
    # XXX does not work for zip which we use for cygwin hosted, fix later
#    if [ x"`echo $Product | grep -i gnupro`" != x -a \
#         \( "$host" = "$target" -o x"`echo $target | grep -i vxworks`" != x \) ]; then
#        case $target in
#            *solaris2.*) IHACK="" ;;
#            *linux*) IHACK="" ;;
#            *cygwin*) IHACK="" ;;
#            *) IHACK="-C $IHackDir $Release/H-$host/lib/gcc-lib/$target" ;;
#        esac
#        case $host in
#            *) IHACK="yes" ;;
#        esac
#    fi
       
    # create archives for each package of each component
    for comp in $components; do
        for ptype in native shared TARGET; do
	    if [ "$ptype" != "TARGET" ]; then 
	        insdir=$bomdir/$ptype/$Product/$comp
		btype=$ptype
	    else
	        insdir=$bomdir/$target/$Product/$comp
		btype=$target
	    fi

            # if no insdir, create
            test -d $insdir || pmkdir $insdir
    
            if [ "$PackList" != "" ]; then
                packages="$PackList"
            else
                packages=`find $bomdir/$btype/$Product/$comp -name "*.bom" -exec basename {} .bom \;`
            fi
            for pack in $packages; do
                bomfile="$bomdir/$btype/$Product/$comp/${pack}.bom"
                rm -f $insdir/${pack}.zip
                cat $bomfile | $ZIP -9 -y -u -q -@ $insdir/${pack}.zip
                if [ "$pack" = "compiler" -a "$IHACK" != "" ]; then
                    (cd $IHackDir ; \
                     find $Release/H-$host/lib/gcc-lib/$target -type f -print| \
                     $ZIP -9 -u -q -@ $insdir/${pack}.zip)
                fi
            done
        done
    done
done
