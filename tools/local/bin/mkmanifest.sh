#!/bin/sh

#
# make-manifest
# cheesy manifest generator for GNUPro releases
#

ProgName=`basename $0`
Version='$Revision: 1.3 $'

LANG=C; export LANG

usage()
{
cat << EndOfUsage
$ProgName $Version
Usage: $ProgName [options] <release_name>
Options:
  release_name                      release name (of the form YYqQ or
                                       <prefix>-<datestamp> for customs)
  --vault=<vault>                   Alternate release vault.  Default is
                                       /opt/redhat/release-vault/<release_name>/new
  --product=<product>               Product name being manifested.  Default is
				       gnupro
  --x                               set -x for shell debugging
  --help                            This message
EndOfUsage
}



while [ $# -gt 1 ]; do
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
    --vault*)
	VAULTDIR="$optarg"
	;;
    --prod*)
	PRODUCT="$optarg"
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

if [ $# -ne 1 ]; then
    usage
    exit 1
fi

Release="$1"

VAULTDIR=${VAULTDIR:="/opt/redhat/release-vault/$Release/new"}
PRODUCT=${PRODUCT:="gnupro"}
MANIDIR=${MANIDIR:="$VAULTDIR/manifests/$PRODUCT"}

if [ ! -d /opt/redhat/$Release ]; then
    echo "release name \"$Release\" doesn't exist in /opt/redhat"
    exit 1
fi

test -d $MANIDIR || mkdir -p $MANIDIR

cd /opt/redhat/$Release
# make sure we only get the directory, not any wonky files or symlinks
# should generate a list of all hosts
THosts=`echo H-*/. | sed -e 's/\/\.//g' -e 's/H-//g'`
Hosts=""
for thost in $THosts; do
    if [ ! -h /opt/redhat/$Release/H-$thost ]; then
        Hosts="$Hosts $thost"
    fi
done
unset THosts thost

# for each host found, generate full manifest
for host in $Hosts; do

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

    case $host in
	# don't include man pages in cygwin manifests
#        *cygwin*)
#            cp /dev/null  $MANIDIR/$host-manifest
#            (cd /opt/redhat ; \
#             echo "$Release/COPYING" ; \
#             echo "$Release/COPYING.LIB" ; \
#             echo "$Release/COPYING.NEWLIB" ; \
#             echo "$Release/REDHAT" ; \
#             find $Release/H-$host $Release/[a-z]* \! -type d -print | sort) \
#                > $MANIDIR/$host-manifest
#            ;;
        *)
            cp /dev/null  $MANIDIR/$host-manifest
            (cd /opt/redhat ; \
             echo "$Release/COPYING" ; \
             echo "$Release/COPYING.LIB" ; \
             echo "$Release/COPYING.NEWLIB" ; \
             echo "$Release/REDHAT" ; \
#             find $Release/H-$host $Release/contrib/H-$basehost $Release/[abd-z]* \! -type d -print \
             find $Release/H-$host $Release/[a-z]* $Release/[A-GI-Z]* \! -type d -print \
	     | grep -v local/Specter | grep -v local/patches | grep -v local/installer \
	     | grep -v local/bin/mkboms.sh | grep -v local/bin/mkvault.sh | grep -v local/bin/mkmanifest.sh \
	     | grep -v local/bin/Specter \
	     | grep -v %redact \
             | sort) \
             | uniq \
	     | grep -v $Release/usertools-src \
	     | grep -v CVS \
                > $MANIDIR/$host-manifest
            ;;
    esac
done
