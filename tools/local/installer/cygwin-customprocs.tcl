#
# Custom Installer Routines for Red Hat GES Releases
#
# This file contains custom installer routines for use with Red Hat GES
# package files.  This file tries to be  host and package independent,
# but does include Cygwin-specific information currently.
#

# This procedure takes the package name as a parameter
# and looks up the action required to install that package.
proc redhat_install_package {package} {
  global tcl_platform parameter
  redhat_install_archive $parameter(installdir) $parameter(files-$package)
}

# The procedure returns default installation location
# for binary files and documentation on the current platform.

proc redhat_get_default_installdir {} {
  global tcl_platform

  if {[string compare $tcl_platform(platform) "windows"] == 0} {
    # Windows default installation directory.
    return [file nativename [file join C:/ RedHat]]
  }

  # Unix default installation directory.
  return "/opt/redhat"
}

# Extraction routine
proc redhat_install_archive {dir archivefile} {
    global install components tcl_platform winpath relinfo parameter

    redhat_get_relinfo

    #
    # Find the tar file for this package and get a list of contents.
    #

    set archivefile [packagefile $archivefile]
    if {![file exists $archivefile]} {
        error "Distribution file \"$archivefile\" not found"
    }

    #
    # Set the "archive type" dependent information.
    #

    if {[string match {*.zip} $archivefile]} {
        # For zip'd file.
        set archivesuffix ".zip"
        set listcommand "unzip -l $archivefile"
	set extractcommand "unzip -o $archivefile"
        set scanpattern "%*d %*s %*d:%*d %s"
        set skiptail 5
    } elseif {[string match {*.tar.gz} $archivefile]} {
        # For gzip'd tar file.
        set archivesuffix ".tar.gz"
	set listcommand {gzip -d -c $archivefile | tar tvf -}
	set extractcommand {gzip -d -c $archivefile | tar xvf -}
        set scanpattern "%*s %*s %*d %*s %*d:%*d %s"
        set skiptail 1
    } elseif {[string match {*.tgz} $archivefile]} {
        # For gzip'd tar file.
        set archivesuffix ".tgz"
	set listcommand {gzip -d -c $archivefile | tar tvf -}
	set extractcommand {gzip -d -c $archivefile | tar xvf -}
        set scanpattern "%*s %*s %*d %*s %*d:%*d %s"
        set skiptail 1
    } elseif {[string match {*.tar.bz2} $archivefile]} {
        # For bzip2'd tar file.
        set archivesuffix ".tar.bz2"
	set listcommand {bzip2 -d -c $archivefile | tar tvf -}
	set extractcommand {bzip2 -d -c $archivefile | tar xvf -}
        set scanpattern "%*s %*s %*d %*s %*d:%*d %s"
        set skiptail 1
    } elseif {[string match {*.tz2} $archivefile]} {
        # For bzip2'd tar file.
        set archivesuffix ".tz2"
	set listcommand {bzip2 -d -c $archivefile | tar tvf -}
	set extractcommand {bzip2 -d -c $archivefile | tar xvf -}
        set scanpattern "%*s %*s %*d %*s %*d:%*d %s"
        set skiptail 1
    } elseif {[string match {*.tar.Z} $archivefile]} {
        # For compressed tar file.
        set archivesuffix ".tar.Z"
	set listcommand {compress -d -c $archivefile | tar tvf -}
	set extractcommand {gzip -d -c $archivefile | tar xvf -}
        set scanpattern "%*s %*s %*d %*s %*d:%*d %s"
        set skiptail 1
    } elseif {[string match {*.tar} $archivefile]} {
        # For tar file.
        set archivesuffix ".tar"
	set listcommand {tar tvf $archivefile}
	set extractcommand {tar xvf $archivefile}
        set scanpattern "%*s %*s %*d %*s %*d:%*d %s"
        set skiptail 1
    } else {
        error "Archive format for \"$archivefile\" unknown."
    }

    #
    # Check for bomfile or create one in memory.
    #

#    regsub $archivesuffix $archivefile ".bom" bomfile
#    if {[file exists $bomfile]} {
#        set mfid [open $bomfile "r"]
#        set output [read -nonewline $mfid]
#        set listcommand "from bomfile"
#        set scanpattern "%s"
#        set skiptail 0
#        close $mfid
#    } else {
   
        # TODO: If there is no manifesto file, create one.
        # TODO: If we can't open the manifesto file for creation
        #       we will just store the manifesto in memory.

        # The following ways of setting and executing $archivecommand seem
        # odd, but work with exec so please don't change unless you know
        # what you are doing.
        progress 0 100 "Extracting manifest..." ""

        catch {eval [concat exec $listcommand]} output
#    }

    set outlist [split $output "\n"]
    lappend outlist $listcommand
    set max [llength $outlist]

#    set filelist ""
#    foreach line $outlist {
#        if {[scan $line $scanpattern name] == 1} {
#            if {![string match */ $name]} {
#                set path [string trimleft $name "./"]
#                set path [string trimright $path "/"]
#                lappend filelist $path
#		incr max
#            }
#        }
#    }

    set old_pwd [pwd]
    cd $dir

    set install(progress) 0
#    foreach file $filelist {
#        progress [incr install(progress)] $max \
#            "Looking for an existing installation..." ""
#
#        if {[file exists $file] && ![file isdirectory $file]} {
#            file delete -force $file
#        }
#    }

    #
    # Unpack the archive file.
    #
    progress $install(progress) $max \
        "Unpacking:" [file tail $archivefile]

    set errlog [file join [pwd] errlog.txt]
    set fid [open "| $extractcommand 2>$errlog" "r"]
    fileevent $fid readable [list redhat_install_archive_output $fid]
    vwait install(tarfile)

    # Check that we weren't interrupted while installing
    # the archive.  This might occur if there is a bug in
    # the untar/unzip version we use causing it to crash,
    # if we ran out of disk space OR if there was disk errors
    # while extracting the archive contents.
    incr install(progress) $skiptail
#    puts "$install(progress) :: $max"
    # We use <= here because fixincludes dictates occasionally going over
    if {$install(progress) < $max} {
        error "Archive file\n\t$archivefile\nwas not installed correctly.\nOnly unpacked $install(progress) of $max files.\nErrors in $errlog.\nIf the log file is empty or truncated, your disk may be full."
    }
    cd $old_pwd

#    if {$tcl_platform(platform) == "windows"} {
#        catch {file mkdir $parameter(installdir)/$relinfo(gnupro)/H-i686-pc-cygwin/tmp}
#    }

    return
}

proc redhat_install_archive_output {fid} {
    global install

    if {[gets $fid line] < 0} {
        catch {close $fid}
        # Finished installing tarfile.
        set install(tarfile) "done"
    } else {
        # Update progress meter.
        progress [incr install(progress)]
    }
}

#
# Windows specific routines
#

# This procedure uses mount to set mount points to what we support.
# Old mount points, if any, are saved and can be restored by running
# restoreoldmounts.bat

proc muck_with_mounts {package} {

  global parameter relinfo errorInfo
  redhat_get_relinfo
  progress 0 100 "About to update mount table"

  # Set variables of paths for later use

  # First the Native Windows Path types
  set installdir $parameter(installdir)
  set errlog [file join $installdir errlog.txt]
  set restoreoldmount [file join $installdir restoreoldmounts.bat ]
  set restorenewmount [file join $installdir restorenewmounts.bat ]
  set optredhatmount [file join $installdir]
  set slashdir [file join $installdir $relinfo(gnupro) ]
  set slashbindir [file join $slashdir bin ]
  set slashlibdir [file join $slashdir lib ]
  set slashoptredhatdir [file join $slashdir opt redhat ]
  set mountcommand [file join $slashdir bin mount ]
  set umountcommand [file join $slashdir bin umount ]
  set mkpasswdcommand [file join $slashdir bin mkpasswd ]
  set mkgroupcommand [file join $slashdir bin mkgroup ]
  # Next POSIX Path types
  set releasemountpoint [file join /opt redhat $relinfo(gnupro) ]
  progress 5

  # Verify some miscellanious files/directories necessary for smooth operation
  catch {file attributes $releasehostdir/bin/cygwin1.dll -permissions 0775}
  catch {file attributes $releasehostdir/bin/mount.exe -permissions 0775}
  catch {file attributes $releasehostdir/bin/umount.exe -permissions 0775}
  catch {file attributes $releasehostdir/bin/mkgroup.exe -permissions 0775}
  catch {file attributes $releasehostdir/bin/mkpasswd.exe -permissions 0775}

  progress 10

  # Make a backup copy of the mount table prior to making any changes to it
  set outid [open "$restoreoldmount" "w" ]
  if { $outid < 0 } {
    global errorInfo
    error "Failed to create (outid is $outid) $restoreoldmount: $errorInfo"
  }
  set inid [open "| $mountcommand -m" "r" ]
  if { $inid < 0 } {
    error "Failed to run $mountcommand -m: $errorInfo"
  }
  while { ! [eof $inid ] } {
    puts $outid [ gets $inid ]
  }
  close $outid
  close $inid

  progress 20

  # Unmount user-mode cygdrive mount
  catch {eval exec {$umountcommand -c -u}}

  # Remount cygdrive mount in binary mode as a system-mount
  set fid [open "| $mountcommand -f -c -s -b /cygdrive 2>$errlog"]
  if { $fid < 0 } {
    error "Failed to run $mountcommand -f -c -s -b /cygdrive: $errorInfo"
  }
  close $fid

  progress 30

  # Replace the (perhaps) existing mounts
  catch {eval exec {$umountcommand -f -u /}}
  catch {eval exec {$umountcommand -f -s /}}
  catch {eval exec {$umountcommand -f -u /usr/lib}}
  catch {eval exec {$umountcommand -f -s /usr/lib}}
  catch {eval exec {$umountcommand -f -u /usr/bin}}
  catch {eval exec {$umountcommand -f -s /usr/bin}}
  catch {eval exec {$umountcommand -f -u /usr/include}}
  catch {eval exec {$umountcommand -f -s /usr/include}}

  set fid [open "| $mountcommand -s -f -b $slashdir / 2>$errlog"]
  if { $fid < 0 } {
    error "Failed to run $mountcommand -s -f b $slashdir /: $errorInfo"
  }
  gets $fid mountoutput
  close $fid

  # Make sure some directories exist before we mount them
    catch {file mkdir $slashbindir}
    catch {file mkdir $slashlibdir}
    set fid [open "| $mountcommand -s -f -b $slashbindir /usr/bin 2>$errlog"]
    if { $fid < 0 } {
      error "Failed to run $mountcommand -s -f -b $slashbindir /usr/bin: $errorInfo"
    }
    gets $fid mountoutput
    close $fid
    set fid [open "| $mountcommand -s -f -b $slashlibdir /usr/lib 2>$errlog"]
    if { $fid < 0 } {
      error "Failed to run $mountcommand -s -f -b $slashlibdir /usr/lib: $errorInfo"
    }
    gets $fid mountoutput
    close $fid

  progress 40

  # Unmount and remount /opt/redhat
#  catch {eval exec {$umountcommand -f -u /opt/redhat 2>$errlog}}
#  catch {eval exec {$umountcommand -f -s /opt/redhat 2>$errlog}}
#  catch {file mkdir $slashoptredhatdir}
#  set fid [open "| $mountcommand -s -f -b $optredhatmount /opt/redhat 2>$errlog"]
#  if { $fid < 0 } {
#    error "Failed to run $mountcommand -s -f -b $optredhatmount /opt/redhat: $errorInfo"
##  }
#  gets $fid mountoutput
#  close $fid
#
#  progress 50

  # Unmount and remount /opt/redhat/releasename
  catch {eval exec {$umountcommand -f -u $releasemountpoint}}
  catch {eval exec {$umountcommand -f -s $releasemountpoint}}
  set fid [open "| $mountcommand -s -f -b $slashdir $releasemountpoint 2>$errlog"]
  if { $fid < 0 } {
    error "Failed to run $mountcommand -s -f -b $slashdir $releasemountpoint: $errorInfo"
  }
  gets $fid mountoutput
  close $fid

  progress 60

  # Ensure the creation of passwd and group files
  if { [ file exists $mkpasswdcommand ] && [ file exists $mkgroupcommand ] } {
    catch { file delete /etc/passwd.preinst /etc/group.preinst }
    catch { file rename /etc/passwd /etc/passwd.preinst }
    catch { file rename /etc/group /etc/group.preinst }
    catch { eval exec {$mkpasswdcommand -l >> /etc/passwd } }
    catch { eval exec {$mkpasswdcommand -d >> /etc/passwd } }
    catch { eval exec {$mkgroupcommand -l >> /etc/group } }
    catch { eval exec {$mkgroupcommand -d >> /etc/group } }
  }

  progress 70

  # Make sure permissions are valid
  catch { eval exec {$slashdir/bin/find $slashdir -name \*.exe -o -name \*.dll -exec chmod a+x \{\} \;} }
  catch { eval exec {$slashdir/bin/find $slashdir exec chmod a+r \{\} \;} }

  progress 80

  set outid [open "$restorenewmount" "w" ]
  if { $outid < 0 } {
    global errorInfo
    error "Failed to create (outid is $outid) $restorenewmount: $errorInfo"
  }
  set inid [open "| $mountcommand -m" "r" ]
  if { $inid < 0 } {
    error "Failed to run $mountcommand -m: $errorInfo"
  }
  while { ! [eof $inid ] } {
    puts $outid [ gets $inid ]
  }
  close $outid
  close $inid

  progress 100
}


# This procedure will set the PATH variable on
# a Windows95, Windows98 or Windows NT.

proc redhat_set_PATH {package} {
  global tcl_platform parameter install winpath

  progress 0 100 "About to set PATH"

  # If winpath doesn't exist, initialize it.
  if {![info exists winpath]} {
    set winpath(needpath) "no"
    set winpath(didpath) "no"
  }

  # We only want to set PATH if binaries were installed.
  if { [info exists parameter(setpath)] } {
    set winpath(needpath) "yes"
  }

  # Get release information
  redhat_get_relinfo

  if { $winpath(needpath) == "yes" && $winpath(didpath) == "no" } {
    switch -- $tcl_platform(os) {
      "Windows NT" {
        progress 50 100 "Setting PATH" ""
        install_nt_path $parameter(installdir)
      }
      "Windows 95" {
        progress 50 100 "Setting PATH" ""
        install_w95_path $parameter(installdir)
      }
      "Windows 98" {
        progress 50 100 "Setting PATH" ""
        install_w95_path $parameter(installdir)
      }
      "default" {
         error "An attempt has been made to set the PATH variable on\
         a non-Window operating system.  This is currently not supported."
      }
    }
  }
  progress 100
}


proc install_nt_path {installdir} {
    global relinfo winpath

    set key "HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment"
    set value "Path"
    set type "expand_sz"

    set newpath "$installdir\\$relinfo(gnupro)\\bin"
    if { [ file exists "$newpath" ] } {
      set oldpath [ registry get "$key" "$value" ]

      if { [ string first $newpath $oldpath ] < 0 } {
        set newpath "$newpath;$oldpath"
        registry set "$key" "$value" "$newpath" "$type"
      }
    }

    set winpath(didpath) "yes"
}

proc install_w95_path {installdir} {
    global env relinfo winpath

    set windrv [string range $env(windir) 0 1]
    set autoexec_bat "$windrv\\autoexec.bat"
    set autoexec_sav "$windrv\\autoexec.sav"

    set gppath "$installdir\\$relinfo(gnupro)\\bin"

  set inid [open "$autoexec_bat" "r" ]
  if { $inid < 0 } {
    error "Failed to open $autoexec_bat: $errorInfo"
  }
  set skipbat 0
  while { ! [eof $inid ] } {
    if { [ string first $gppath [ gets $inid ] ] > 0 } {
      set skipbat 1
    }
  }
  close $inid

  if { $skipbat == 0 } {
    file copy -force -- "$autoexec_bat" "$autoexec_sav"
    set fid [open "$autoexec_bat" "a"]
    puts $fid "REM begin $relinfo(title) PATH additions"
    puts $fid "set PATH=$gppath;%PATH%"
    puts $fid "REM end   $relinfo(title) PATH additions"
    close $fid
  }

    set winpath(didpath) "yes"
}

# Read release information from relinfo file.
proc redhat_get_relinfo {} {
    global relinfo

    set fid [open "packages/relinfo" "r"]
    gets $fid relinfo(title)
    gets $fid relinfo(host)
    gets $fid relinfo(target)
    gets $fid relinfo(gccvn)
    gets $fid relinfo(gnupro)
    gets $fid relinfo(contributed)
    close $fid
}

proc redhat_fixincludes { booga } {
    global env relinfo parameter

    progress 0 100 "Running fixincludes                    "

    # Get release information
    redhat_get_relinfo

    # figure out whether the install host and CD match
    set old_pwd [pwd]
    cd /tmp
    set guessHost [eval exec $old_pwd/utils/bin/config.guess]
    cd $old_pwd
    set sysrootlink "$parameter(installdir)/$relinfo(gnupro)/sys-roots/$relinfo(target)"
    if { ! [file exists "$sysrootlink"]} {
	set sysrootlink "/"
    }
    set dosysroot 0
    switch -glob -- $guessHost {
        "i[3456789]86*linux-gnu*" {
    	    set dosysroot 1
# we're using a sysroot here, so it's valid to have a different host run
# the same tools
	    set instHost "$relinfo(host)"
        }
        "sparc-sun-solaris2.5*" {
    	    set dosysroot 1
            set instHost "sparc-sun-solaris2.5"
        }
        "sparc-sun-solaris2.6*" {
    	    set dosysroot 1
            set instHost "sparc-sun-solaris2.6"
        }
        "sparc-sun-solaris2.7*" {
    	    set dosysroot 1
            set instHost "sparc-sun-solaris2.7"
        }
        "sparc-sun-solaris2.8*" {
    	    set dosysroot 1
            set instHost "sparc-sun-solaris2.8"
        }
        "hppa*-hp-hpux10.*" {
            set instHost "hppa1.1-hp-hpux10.20"
        }
        "mips*-sgi-irix6.*" {
            set instHost "mips-sgi-irix6.5"
        }
        "powerpc-ibm-aix4*" {
            set instHost "powerpc-ibm-aix4.3.3.0"
        }
        "powerpc-ibm-aix5*" {
            set instHost "powerpc-ibm-aix5.1.0.0"
        }
        "hppa*-hp-hpux11.*" {
            set instHost "hppa1.1-hp-hpux11.00"
        }
        "default" {
            set instHost "$guessHost"
        }
    }

    if [string compare $dosysroot 0] {
        set sysroottarget "$parameter(installdir)/$relinfo(gnupro)/H-$relinfo(host)/$relinfo(target)/sys-root"
	if {! [ file exists "$sysroottarget" ] } {
            exec ln -s "$sysrootlink" "$sysroottarget"
	}
        set FixIncDir "$sysroottarget/usr/include"
    } else {
        set FixIncDir "/usr/include"
    }
  
    set iToolsDir $parameter(installdir)/$relinfo(gnupro)/H-$relinfo(host)/$relinfo(target)/install-tools

    if { ! [file isdirectory $iToolsDir] } {
	error "Directory \"$iToolsDir\" does not exist.  Can not run fixincludes."
    }

    regsub {sparc64(.*)} $relinfo(host) {sparc\1} tmp_host
    if { "$instHost" != "$tmp_host" } {
       error "This host does not appear to be $relinfo(host).  Can not run fixincludes."
    }

    set FIXLOG $parameter(installdir)/$relinfo(gnupro)/H-$relinfo(host)/fixlog.[pid]
    set ERRLOG $parameter(installdir)/$relinfo(gnupro)/H-$relinfo(host)/errlog.[pid]
    set Ginc $parameter(installdir)/$relinfo(gnupro)/H-$relinfo(host)/lib/gcc-lib/$relinfo(target)/$relinfo(gccvn)/include
    if { ! [file isdirectory $Ginc] } {
        error "Directory \"$Ginc\" does not exist.  Can not run fixincludes."
    }

    set env(CPP) "$parameter(installdir)/$relinfo(gnupro)/H-$relinfo(host)/bin/gcc -E"
    set env(GCC_EXEC_PREFIX) $parameter(installdir)/$relinfo(gnupro)/H-$relinfo(host)/lib/gcc-lib/

    if { [file exists $iToolsDir/fix-header] } {
        set env(FIX_HEADER) $iToolsDir/fix-header
    }
    if { [file exists $iToolsDir/fixproto-defines] } {
        set env(FIXPROTO_DEFINES) $iToolsDir/fixproto-defines
    }

    set unfixDir $Ginc/../.unfixed
    catch {eval exec {rm -rf $unfixDir}}
    catch {file mkdir $unfixDir}
    catch {
	foreach f [glob $Ginc/*.h] {
	    file copy $f $unfixDir
	}
    }

    set env(srcdir) ""
    set env(INSTALL_ASSERT_H) ""
    set env(TARGET_MACHINE) $relinfo(target)
    set env(SHELL) /bin/sh

    # cd to $iToolsDir so fixincludes can find fixincl
    # also should be writeable directory just in case fixincludes
    # creates files in $CWD
    if { [file exists $iToolsDir/fixinc.sh] } {
        cd $iToolsDir
	catch {eval exec $iToolsDir/fixinc.sh $Ginc $FixIncDir > $FIXLOG 2>$ERRLOG} ret
        if { $ret != "" } {
            error "Running fixinc.sh failed.  Verbose output in \"$FIXLOG\".  Error output in \"$ERRLOG\"."
	}
	# System header files now ANSI compatible.
    }
    if { [file exists $iToolsDir/fixproto] } {
        cd $iToolsDir
	catch {eval exec $iToolsDir/fixproto $Ginc $Ginc $FixIncDir >> $FIXLOG 2>>$ERRLOG} ret
        if { $ret != "" } {
            error "Running fixproto failed.  Verbose output in \"$FIXLOG\".  Error output in \"$ERRLOG\"."
	}
	# ANSI/POSIX prototypes added to include files.
    }

    if { [file exists $Ginc/limits.h] } {
	    file rename -force $Ginc/limits.h $Ginc/syslimits.h
    }

    catch {
	foreach f [glob $unfixDir/*.h] {
	    file copy -force $f $Ginc
	}
    }
    cd $old_pwd
  
    progress 100
}

#deprecated via sysroot
proc redhat_whichglibc {} {
    global env

    set tmpc /tmp/try[pid]
    set fid [open "$tmpc.c" "w"]
    puts $fid "#include <features.h>"
    puts $fid "int main (int argc, char **argv)"
    puts $fid "{"
    puts $fid "printf(\"%d\\n\", __GLIBC_MINOR__);"
    puts $fid "exit(0);"
    puts $fid "}"
    puts $fid ""
    close $fid

    set old_pwd [pwd]
    cd /tmp
    catch {eval exec gcc $tmpc.c -o $tmpc} ret
    if { $ret != "" } {
            error "Can't determine glibc version.\n$ret"
    }
    catch {eval exec $tmpc} ret
    catch {file delete $tmpc $tmpc.c}
    cd $old_pwd
    return "$ret"
}
