#!/bin/sh

#
# insthpux.sh -- Special front-end for HPUX
#
# Angela Marie Thomas
# angela@cygnus.com
# $Date: 2004/03/10 00:03:17 $
#
# Copyright (C) 1999 Cygnus Solutions
# 
# $Id: insthpux.sh,v 1.1.1.1 2004/03/10 00:03:17 release Exp $
#

#
# This is a hack to work around the lack of Rock Ridge support in HP-UX.
# It requires mounting the CDROM without "-o cdcase".
#

ProgName=`basename $0`
Version='$Revision: 1.1.1.1 $'

TRUE=1
FALSE=0

CP=cp
MKDIR=mkdir

# reasonable umask
umask 0002

# go away on certain signals
trap "echo \"$ProgName: Caught a sig @ \`date\`, exiting.\" ; exit 1" 1 2 3 15

# Begin shell functions

#
# usage()
# Quickie "how do I use this bloody thing?"
#
usage()
{
cat << EndOfUsage
$ProgName $Version
Usage: $ProgName

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
# Echo($string)
# echo string if not quiet enabled
#
Echo () {
    [ "$Quiet" != "$TRUE" ] && echo "$*"
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

#
# transdir($dir)
# translate a dirname from ISO to RR
#
transdir() {
    while [ $# -gt 0 ]; do
        if [ "$1" != "." ]; then
            transdir `dirname $1`
            [ $? -eq 0 ] || return $?
	    TDIR=${TDIR:="."}/`grep "^D .*\`basename $1\`" \`dirname $1\`/TRANS.TBL\;1 | awk '{print $3}'`
	else
	    TDIR="."
        fi
        shift
    done
}

#
# transfile($file)
# translate filename from ISO to RR
#
transfile() {
    odir=`dirname $file`
    ofile=`basename $file`
    TDIR=""; transdir $odir; ndir="$TDIR"
    nfile=$ndir/`grep "^F $ofile" $odir/TRANS.TBL\;1 | awk '{print $3}'`
    echo $nfile
}

mkflist() {
   trans=$1
   from=$2
   to=$3
   awk '/^F/ {print "_C= _X=/" $2 " _Y=/" $3}' $trans | \
	sed -e "s,_C=,$CP," -e "s,_X=,$from," -e "s,_Y=,$to," | \
	tr A-Z a-z >> $LinkFiles
}

mkdlist() {
   trans=$1
   from=$2
   to=$3
   awk '/^D/ {print "_C= _Y=/" $3}' $trans | \
	sed -e "s,_C=,$MKDIR," -e "s,_Y=,$to," | \
	tr A-Z a-z >> $LinkDirs
}

# End shell functions


# Root of installer program and boms
RootDir="`dirname $0`"
RootDir="`cd $RootDir ; pwd`"

# setup the arglist for the installers
InstArgs="$*"

case "$InstArgs" in
    *--help*) usage; echo ""; exit 0 ;;
    *) ;;
esac

cat << --EOF--
${ProgName} will copy files from CD to a temporary location,
rename them to use the original names as defined in the Rock Ridge
translation table TRANS.TBL;1 and execute the installation script
specified in the argument list

--EOF--

foo=""
while [ "$foo" = "" ]; do
    echo "Enter the name of a temporary directory to store the copied files"
    read foo bar
done
TmpDir=$foo/insthp.$$
LinkDirs=$TmpDir/dirlist
LinkFiles=$TmpDir/filelist

rm -rf $TmpDir
pmkdir $TmpDir || exit 1

if [ ! -f "$RootDir/trans.tbl" -a -f "$RootDir/install-gui." ]; then
    echo "${ProgName}: CD mounted with '-o cdcase'.  Please remount without this option."
    exit 1
fi

if [ ! -f "$RootDir/TRANS.TBL;1" ]; then
    echo "${ProgName}: $RootDir/TRANS.TBL;1 does not exist.  Exiting."
    exit 1
fi

echo "${ProgName}: Generating lists..."
# sort the list and remove "." so "mkdir $dir" doesn't fail on EEXIST
dirlist="`(cd $RootDir ; find . -type d -print | sort | sed -e '/^\.$/d')`"
# sort list for easy human tracking
filelist="`(cd $RootDir ; find . -type f -print | grep -v TRANS.TBL | sort)`"

if [ "$dirlist" = "" ]; then
    echo "${ProgName}: No directories in $RootDir  Exiting."
    exit 1
fi
if [ "$filelist" = "" ]; then
    echo "${ProgName}: No files in $RootDir  Exiting."
    exit 1
fi

echo "${ProgName}: Generating dir list (this may take a while)..."
for dir in $dirlist; do
    TDIR=""; transdir $dir; echo "$MKDIR $TmpDir/$TDIR" >> $LinkDirs
done
echo "${ProgName}: Generating file list (this may take a while)..."
for file in $filelist; do
    echo "$CP $RootDir/$file $TmpDir/`transfile $file`" >> $LinkFiles
done

sed -e 's/;/\\;/' $LinkFiles | sort > /tmp/lf.$$
mv /tmp/lf.$$ $LinkFiles
sort -o $LinkDirs $LinkDirs

echo "${ProgName}: Creating directories..."
cd $RootDir
sh $LinkDirs || exit 1

echo "${ProgName}: Converting files (this will take a while)..."
cd $RootDir
sh $LinkFiles || exit 1

BinFiles='install-gui utils/bin/* utils/H-*/bin/*'
cd $TmpDir
eval chmod a+x $BinFiles || exit 1

echo "${ProgName}: Running installer \"install-gui\"..."
$TmpDir/install-gui $InstArgs < /dev/null

echo "${ProgName}: Done, removing $TmpDir"
cd $RootDir
rm -rf $TmpDir
