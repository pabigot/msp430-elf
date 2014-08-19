#!/bin/sh
# \
if [ -f $INSTALLDIR/tcldist/$ARCH/bin/bltwish24 ]; then
# \
exec $INSTALLDIR/tcldist/$ARCH/bin/wish8.3 "$0" "$@"
# \
else
# \
exec wish8.3 "$0" "$@"
# \
fi

# add http and uri libraries to search path
lappend auto_path [file join [pwd] utils lib http2.3]
lappend auto_path [file join [pwd] utils lib uri]

if {![catch {set f [open http.fwp]}]} {
  foreach x [split [read $f] \n] {
    set dir [file dirname $x]
    lappend auto_path $dir
  }
  close $f
}

if {![catch {set f [open http.fwp]}]} {
  set dir [file dirname [file dirname [lindex [split [read $f] \n] 0]]]
  close $f
  lappend auto_path $dir/http2.3
  lappend auto_path $dir/uri
}
#package require http

if {[string compare $tcl_platform(platform) "windows"] == 0} {
  set install(packageprefix) win
  package require registry
} else {
  set install(packageprefix) ""
}

set install(location) [file dirname $argv0]
if {"." == $install(location)} {
    set install(location) [pwd]
} elseif {[file pathtype $install(location)] != "absolute"} {
    set install(location) [file join [pwd] $install(location)]
}

# web install is our fallback position if we can't find any packages locally
set install(url) "http://tclish.sourceforge.net/latest"
set install(style) web
if {![catch {set f [open install.url]}]} {
  set install(url) [string trim [read $f]]
  close $f
}
set install(currpackage) ""

# if packages.fwp exists, then we must be a onefile install
if {![catch {set f [open $install(packageprefix)packages.fwp]}]} {
  set install(style) onefile
  close $f
# otherwise, if a packages subdirectory exists, we must be a CD install
} elseif {[file isdirectory [file join $install(location) packages]]} {
  set install(style) cd
}
set install(webtimeout) 6000
set install(blocksize) 10240

proc usage {} {
  global argv0
  puts "Usage: $argv0 ?\[-web | -url url | -cd\]? ?-username uname? ?-password pass?"
}

set install(uname) ""
set install(password) ""
set install(authtok) ""

set arglist $argv
while {[llength $arglist] > 0} {
  switch -- [lindex $arglist 0] {
    "-cd" { set install(style) cd }
    "-web" { set install(style) web }
    "-url" { 
      set install(style) web; 
      set install(url) [lindex $arglist 1] 
      set arglist [lrange $arglist 1 end]
    }
    "-help" { usage; exit 0 }
    "-username" { 
      set install(uname) [lindex $arglist 1] 
      set arglist [lrange $arglist 1 end]
    }
    "-password" { 
      set install(password) [lindex $arglist 1] 
      set arglist [lrange $arglist 1 end]
    }
  }
  set arglist [lrange $arglist 1 end]
}


# -------------------------------------------------------------------
# Routines for encoding and decoding base64
# encoding from Time Janes, 
# decoding from Pascual Alonso,
# namespace'ing and bugs from Parand Tony Darugar 
# (tdarugar@binevolve.com)
# 
# $Id: install.tcl,v 1.2 2005/10/22 00:51:28 blc Exp $
# -------------------------------------------------------------------

namespace eval base64 {
  set charset "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
  
  # ----------------------------------------
  # encode the given text
  proc encode {text} {
    set encoded ""
    set y 0
    for {set i 0} {$i < [string length $text] } {incr i} {
      binary scan [string index $text $i] c x
      if { $x < 0 } {
        set x [expr $x + 256 ]
      }
      set y [expr ( $y << 8 ) + $x]
      if { [expr $i % 3 ] == 2}  {
        append  encoded [string index $base64::charset [expr ( $y & 0xfc0000 ) >> 18 ]]
	append  encoded [string index $base64::charset [expr ( $y & 0x03f000 ) >> 12 ]]
	append  encoded [string index $base64::charset [expr ( $y & 0x000fc0 ) >> 6 ]]
	append  encoded [string index $base64::charset [expr ( $y & 0x00003f ) ]]
	set y 0
      }
    }
    if { [expr $i % 3 ] == 1 } {
      set y [ expr $y << 4 ]
      append encoded [string index $base64::charset [ expr ( $y & 0x000fc0 ) >> 6]]
      append encoded [string index $base64::charset [ expr ( $y & 0x00003f ) ]]
      append encoded "=="
    }
    if { [expr $i % 3 ] == 2 } {
      set y [ expr $y << 2 ] 
      append  encoded [string index $base64::charset [expr ( $y & 0x03f000 ) >> 12 ]]
      append  encoded [string index $base64::charset [expr ( $y & 0x000fc0 ) >> 6 ]]
      append  encoded [string index $base64::charset [expr ( $y & 0x00003f ) ]]
      append encoded "="
    }
    return $encoded
  }

  # ----------------------------------------
  # decode the given text
  # Generously contributed by Pascual Alonso
  proc decode {text} {
    set decoded ""
    set y 0
    if {[string first = $text] == -1} {
      set lenx [string length $text]
    } else {
      set lenx [string first = $text]
    }
    for {set i 0} {$i < $lenx } {incr i} {
      set x [string first [string index $text $i] $base64::charset]
      set y [expr ( $y << 6 ) + $x]
      if { [expr $i % 4 ] == 3}  {
        append decoded \
	  [binary format c [expr $y >> 16 ]]
	append decoded \
	  [binary format c [expr ( $y & 0x00ff00 ) >> 8 ]]
	append decoded \
	  [binary format c [expr ( $y & 0x0000ff ) ]]
	set y 0
      }
    }
    if { [expr $i % 4 ] == 3 } {
      set y [ expr $y >> 2 ] 
	append decoded \
	  [binary format c [expr ( $y & 0x00ff00 ) >> 8 ]]
	append decoded \
	  [binary format c [expr ( $y & 0x0000ff ) ]]
    }
    if { [expr $i % 4 ] == 2 } {
      set y [ expr $y >> 4 ] 
	append decoded \
	  [binary format c [expr ( $y & 0x0000ff ) ]]
    }
    return $decoded
  }
}

if {[string compare $install(uname) ""] != 0} {
# create auth token for web fetches
  set install(authtok) \
          [list Authorization \
                "Basic [base64::encode $install(uname):$install(password)]"]
}

##-----------------------------------------------------------------------
##                                                                      
##        This file is part of the Tcl Installer Project
##                     (c) Copyright 2001
##           Michael McLennan (mmc@cadence.com),
##           Mark Harrison (markh@usai.asiainfo.com),
##           Kris Raney (kris@kraney.com)
##           and others.
## 
##    This software is distributed under the same terms and conditions
##       as Tcl, see http://www.tcltk.com/ for details or contact the
##                              author.
##-----------------------------------------------------------------------

package provide installer 0.1

namespace eval installer {;
  namespace export \
          gauge_create \
          gauge_value
}; # end of nameespace installer

if {![catch {package present Tk}]} {

# ----------------------------------------------------------------------
# GAUGE LIBRARY
# This is the progress gauge from the "Effective Tcl/Tk Programming"
# book, with some small modifications.  We use this to report progess
# of the installation.
# ----------------------------------------------------------------------
  option add *Gauge.borderWidth 2 widgetDefault
  option add *Gauge.relief sunken widgetDefault
  option add *Gauge.length 400 widgetDefault
  option add *Gauge.color gray widgetDefault


  proc installer::gauge_create {win {type "percent"} {color ""}} {
      global gaugeInfo

      if {$type != "percent" && $type != "counter"} {
          error "bad type \"$type\": should be percent or counter"
      }
      set gaugeInfo($win-type) $type
      set gaugeInfo($win-max) 100

      frame $win -class Gauge
      set len [option get $win length Length]

      canvas $win.display -borderwidth 0 -background white \
          -highlightthickness 0 -width $len -height 20
      pack $win.display -expand yes

      if {$color == ""} {
          set color [option get $win color Color]
      }
      $win.display create rectangle 0 0 0 20 \
          -outline "" -fill $color -tags bar
      $win.display create text [expr 0.5*$len] 10 \
          -anchor c -tags value
      if {$type == "percentage"} {
        $win.display itemconfigure value -text "0%"
      }

      return $win
  }

  proc installer::gauge_value {win val {max ""}} {
      global gaugeInfo

      if {$max != ""} {
          set gaugeInfo($win-max) $max
      }
      if {$val < 0} {
          set val 0
      } elseif {$val > $gaugeInfo($win-max)} {
          set val $gaugeInfo($win-max)
      }

      switch -- $gaugeInfo($win-type) {
          percent {
              if {$gaugeInfo($win-max) > 0} {
                set percent [expr 100.0*$val/$gaugeInfo($win-max)]
              } else {
                set percent 100.0
              }
              set msg [format "%3.0f%%" $percent]
              $win.display itemconfigure value -text $msg
          }
          counter {
              set msg [format "%d of %d" $val $gaugeInfo($win-max)]
              $win.display itemconfigure value -text $msg
          }
      }

      if {$gaugeInfo($win-max) > 0} {
        set w [expr double($val)/$gaugeInfo($win-max)*[winfo width $win.display]]
      } else {
        set w [expr double($val)*[winfo width $win.display]]
      }
      set h [winfo height $win.display]
      $win.display coords bar 0 0 $w $h

      update idletasks
  }

}

##-----------------------------------------------------------------------
## 									
##        This file is part of the Tcl Installer Project
##		       (c) Copyright 2001
##           Kris Raney (kris@kraney.com)
##           and others.
## 
##    This software is distributed under the same terms and conditions
##       as Tcl, see http://www.tcltk.com/ for details or contact the
##				author.
##-----------------------------------------------------------------------

package provide installer 0.1

package require uri

namespace eval installer {;
  namespace export \
        bg_download \
        text_download \
        gui_download \
        reset
}; # end of nameespace installer

# installer::bg_download --
#
#   This method provides a high-level interface for doing potentially
#   long-running downloads.
#   (The "constructor" that creates a download instance.)
#
# Arguments:
#    -progress {}       -- a callback to be called as progress is made.
#                          when called, the callback string has the additional
#                          arguments token, url, url_index, total_num_urls,
#                          current_size, and total_size appended.
#    -done {}           -- a callback to be called when the entire batch of
#                          downloads is complete. When called, the callback
#                          string has the additional argument token appended.
#    -error {}          -- a callback to be called in the case of an error. When
#                          called, the callback has the additional arguments
#                          token and error appended.
#
#    -base {}           -- a base url for all of the urls to download
#    -urls {}           -- a list of urls to be downloaded. These may be
#                          absolute, or relative to the base url.
#    -destination /tmp  -- a destination directory where the files will be
#                          placed. The files will keep their name as specified
#                          in the url
#    -blocksize 8192    -- this is simply passed to the http::geturl command for
#                          each download.
#    -timeout 0         -- this is passed on to http::geturl
#    -headers {}        -- this is passed on to http::geturl
#    -query {}          -- this is passed on to http::geturl
#    -headerlist {}     -- if specified, this overrides the -headers option.
#                          This specifies a list of values, one per url, which
#                          will be passed to http::geturl as -headers for the
#                          corresponding url
#    -querylist {}      -- if specified, this overrides the -query option.
#                          This specifies a list of values, one per url, which
#                          will be passed to http::geturl as -query for the
#                          corresponding url
#    -block 0           -- a boolean flag indicating whether this call should
#                          block until all files are downloaded
#
# Results:
#   token indicating instance
#
proc installer::bg_download {args} {
  variable bgd
  if {![info exists bgd(sessionno)]} {
    set bgd(sessionno) 0
  }
  set token [namespace current]::[incr bgd(sessionno)]
  variable $token
  upvar 0 $token session

  array set session {
    -progress {}
    -done {}
    -error {}

    -base {}
    -urls {}
    -destination /tmp
    -blocksize 8192
    -timeout 0
    -headers {}
    -query {}
    -headerlist {}
    -querylist {}
    -block 0
    currfile 0
    currfilesize 0
    downloaded 0
  }
  set options {
    progress 
    done 
    error 
    base 
    urls 
    destination 
    blocksize 
    timeout 
    headers 
    query 
    headerlist 
    querylist
    block
  }
  set pat ^-([join $options |])$
  foreach {flag value} $args {
    if {[regexp $pat $flag]} {
      if {[info exists session($flag)] && \
                  [string is integer -strict $session($flag)] && \
                  ![string is integer -strict $value]} {
        return -code error "Bad value for $flag ($value), must be integer"
      }
      set session($flag) $value
    } else {
      return -code error "Unknown option $flag, can be: [join -$options ", -"]"
    }
  }
  set session(totalfiles) [llength $session(-urls)]

  # verify the urls all make sense
  if {"" == $session(-urls)} {
    return -code error "No urls specified"
  }
  if {"" == $session(-base)} {
    foreach url $session(-urls) {
      if {[uri::isrelative $url]} {
        return -code error "Unable to download non-absolute url $url"
      }
    }
  } else {
    if {[uri::isrelative $session(-base)]} {
      return -code error "Base URL must be an absolute URL"
    }
  }

  set session(afterevent) [after idle [namespace code "_next_download $token"]]
  if {$session(-block)} {
    vwait $token\(complete)
  }
  return $token
}

# installer::reset --
#
#   Used to cancel a download
#
# Arguments:
#   token        -- the token specifying the particular download instance
#   why          -- the reason for the cancellation
#
# Results:
#   void
#
proc installer::reset {token why} {
  variable $token
  upvar 0 $token session

  if {![info exists session]} return

  if {[info exists session(http_tok)]} {
    http::reset $session(http_tok) $why
  }
  return
}

# installer::cleanup --
#
#   The "destructor" for bg_download. This will be called automatically
#   when the download completes.
#
# Arguments:
#   token        -- the token specifying the particular download instance
#
# Results:
#   void
#
proc cleanup {token} {
  variable $token
  upvar 0 $token session

  if {![info exists session]} return

  if {[info exists session(http_tok)]} {
    http::cleanup $session(http_tok)
    unset session(http_tok)
  }
  if {[info exists session(fd)]} {
    close $session(fd)
  }
  catch {after cancel $session(afterevent)}
  unset session
}

# installer::_next_download --
#
#   A "protected method" used by bgdownload as a callback for when a single
#   download has completed
#
# Arguments:
#   token        -- the token specifying the particular download instance
#   httpgarbage  -- ignored, but must be allowed since it's put there by the
#                   http package 
#
# Results:
#   void
#
proc installer::_next_download {token {httpgarbage {}}} {
  variable $token
  upvar 0 $token session

  # check to see if we've been interrupted with a "cleanup"
  if {![info exists session]} return

  if {[info exists session(http_tok)]} {
    if {"ok" != [http::status $session(http_tok)]
        || ( [lindex [http::code $session(http_tok)] 1] != "200"
             && [lindex [http::code $session(http_tok)] 1] != "206" )} {
      # we had a problem with the download
      set tok $session(http_tok)
      unset session(http_tok)
      _try_callback $token $session(-error) [list $token [http::code $tok]]
      set session(complete) 1
      http::cleanup $tok
      return
    }
    after idle "http::cleanup $session(http_tok)"
    unset session(http_tok)
    if {[info exists session(skippedpart)]} {
      unset session(skippedpart)
    }
  }

  incr session(currfile)

  if {$session(currfile) > $session(totalfiles)} {
    _try_callback $token $session(-done) [list $token] $session(-error) 
    set session(complete) 1
    cleanup $token
    return
  }

  set url [lindex $session(-urls) [expr $session(currfile) -1]]
  set filepath [file join $session(-destination) [file tail $url]]

# figure out what query needs to be done on the geturl
  if {[llength $session(-querylist)] == 0} {
    if {"" == $session(-query)} {
      set queryflags ""
    } else {
      set queryflags "-query $session(-query)"
    }
  } else {
    if {[set q [lindex $session(-querylist) [expr $session(currfile) -1]]] == ""} {
      set queryflags ""
    } else {
      set queryflags "-query $q"
    }
  }

# figure out what headers need to be sent on the geturl
  if {[llength $session(-headerlist)] == 0} {
    if {"" == $session(-headers)} {
      set headerflags ""
    } else {
      set headerflags "-headers $session(-headers)"
    }
  } else {
    if {[set h [lindex $session(-headerlist) [expr $session(currfile) -1]]] == ""} {
      set headerflags ""
    } else {
      set headerflags "-headers $h"
    }
  }

  if {[info exists session(fd)]} {
    after idle "close $session(fd)"
    unset session(fd)
  }
  if {[file exists $filepath]} {
    set tok [eval http::geturl [uri::resolve $session(-base) $url] \
                               $headerflags $queryflags -validate 1]
    upvar 0 $tok state
    if {[file size $filepath] == $state(totalsize)} {
      set ret [_try_callback $token $session(-progress) \
         [list $token \
               [lindex $session(-urls) [expr $session(currfile)-1]] \
               $session(currfile) \
               $session(totalfiles) \
               $state(totalsize) \
               $state(totalsize) ] \
         $session(-error)]
      http::cleanup $tok
      if {!$ret} {
        set session(afterevent) [after idle [namespace code "_next_download $token"]]
      }
      return
    }
    if {[file size $filepath] < $state(totalsize)
        && [set i [lsearch -exact $state(meta) Accept-Ranges]] != -1
        && [lsearch -exact [lindex $state(meta) [expr $i +1]] bytes] != -1} {
      # resume download
      set headers [lindex $headerflags 1]
      lappend headers Range bytes=[file size $filepath]-
      set headerflags [list -headers $headers]
      http::cleanup $tok
      set session(fd) [open $filepath "a"]
      set session(skippedpart) [file size $filepath]
    } else {
      # server doesn't support partial downloads
      set session(fd) [open $filepath "w"]
    }
  } else {
    set session(fd) [open $filepath "w"]
  }

  fconfigure $session(fd) -translation binary

  set session(http_tok) [eval \
     http::geturl [uri::resolve $session(-base) $url] $queryflags $headerflags \
                  -blocksize $session(-blocksize) \
                  -timeout $session(-timeout) \
                  -channel $session(fd) \
                  -progress [list [namespace code "_download_progress $token"]] \
                  -command [list [namespace code "_next_download $token"]] ]
}

# installer::_download_progress --
#
#   A "protected method" used by bgdownload as a callback for when a block
#   has been downloaded by the http package
#
# Arguments:
#   token        -- the token specifying the particular download instance
#   httptoken    -- the token for the http::geturl
#   totalsize    -- the total size of the file being downloaded
#   current      -- the current size of the file on disk
#
# Results:
#   void
#
proc installer::_download_progress {token httptoken totalsize current}  {
  variable $token
  upvar 0 $token session

# this kludge attempts to force binary translation mode, but misses the first
# block

  upvar 0 $httptoken state
  fconfigure $state(sock) -translation binary

  if {[info exists session(skippedpart)]} {
    incr totalsize $session(skippedpart)
    incr current $session(skippedpart)
  }
  _try_callback $token $session(-progress) \
         [list $token \
               [lindex $session(-urls) [expr $session(currfile)-1]] \
               $session(currfile) \
               $session(totalfiles) \
               $current \
               $totalsize ] \
         $session(-error)
}

# installer::_try_callback --
#
#   A "protected method" used by bgdownload to call the user specified
#   callbacks.
#
# Arguments:
#   token        -- the token specifying the particular download instance
#   cb           -- the callback to be called
#   cbargs       -- a list of arguments to be passed to the callback
#   (optional) errcb -- a callback to be used in the case of an error
#
# Results:
#   void
#
proc installer::_try_callback {token cb cbargs {errcb {}}} {
  set errcondition 0
  if {"" != $cb} {
    if {[catch {uplevel #0 $cb $cbargs} err]} {
      if {"" != $errcb} {
        catch {uplevel #0 $errcb [list $token $err]}
      }
      set session(complete) 1
      set errcondition 1
      cleanup $token
    }
  }
  return $errcondition
}

# installer::_steal_opt --
#
#   A "protected method" used by "subclasses" of bgdownload to take arguments
#   out of the args list they've been passed. This essentially "overrides"
#   behavior in the "parent".
#
# Arguments:
#   optname      -- The name of the option to be "stolen" if it's present. Also
#                   set as a return variable to hold the value of that option.
#   arglist      -- $args
#
# Results:
#   a new arglist, minus the "stolen" argument.
#
proc installer::_steal_opt {optname default arglist} {
  upvar $optname opt
  set i [lsearch -exact $arglist "-$optname"]
  if {$i != -1} {
    set opt [lindex $arglist [expr $i +1]]
    set arglist [lreplace $arglist $i [expr $i +1]]
  } else {
    set opt $default
  }
  return $arglist
}

# installer::text_download --
#
#   This method provides a high-level interface for doing potentially
#   long-running downloads. It "inherits from" bg_download, but provides the
#   additional behavior that a text-based progress report is output.
#
# Arguments:
#    (same as bg_download)
#
# Results:
#   void
#
proc installer::text_download {args} {
  set args [_steal_opt progress "" $args]
  set args [_steal_opt done "" $args]
  set args [_steal_opt error "" $args]
  set args [_steal_opt block 0 $args]

  if {[catch {set token [eval bg_download $args [list \
                   -progress [namespace code _text_progress] \
                   -done [namespace code _text_done] \
                   -error [namespace code _text_error] ] ]} err]} {
    return -code error $err
  }

  variable $token
  upvar 0 $token session

  set session(textprogress) $progress
  set session(textdone) $done
  set session(texterror) $error
  set session(-block) $block

  set session(lastfile) 0

  if {$session(-block) } {
    vwait $token\(complete)
  }
  return $token
}

# installer::_text_progress --
#
#   A "protected method" used by text_download as a callback for when a block
#   has been downloaded by the http package. Outputs status on stdout.
#
# Arguments:
#  (as specified for bg_download progress callbacks)
#
# Results:
#   void
#
proc installer::_text_progress {token file filenum totalfiles current totalsize} {
  variable $token
  upvar 0 $token session

  if {$session(lastfile) != $filenum} {
    set session(lastfile) $filenum
    if {$filenum > 1} {
      puts "\r    |=========================================| 100 %"
    }
    puts "downloading $filenum of $totalfiles: \"$file\""
    puts -nonewline "    |                                         |   0 %"
    flush stdout
    set session(progress) -1
  }

  variable _progress
  if {[incr session(progress)] >= [llength $_progress]} {
    set session(progress) 0
  }
  puts -nonewline "\r [lindex $_progress $session(progress)]  |"

  if {$totalsize == 0} {
    set totalsize 1
  }
  set portion [expr 1.0 * $current / $totalsize * 40]
  for {set x 0} {$x <= $portion} {incr x} {
    puts -nonewline "="
  }
  for {} {$x <= 40} {incr x} {
    puts -nonewline " "
  }
  puts -nonewline "| [format "%3d" [expr int(100.0 * $current/$totalsize)]] %"
  flush stdout

  _try_callback $token $session(textprogress) [list $token \
                                              $file \
                                              $filenum \
                                              $totalfiles \
                                              $current \
                                              $totalsize] _text_error
}

# installer::_text_done --
#
#   A "protected method" used by text_download as a callback for when the entire
#   batch of downloads is complete.
#
# Arguments:
#  token -- the download instance
#
# Results:
#   void
#
proc installer::_text_done {token} {
  variable $token
  upvar 0 $token session

  puts "\r    |=========================================| 100 %"
  puts done

  _try_callback $token $session(textdone) $token _text_error
}

# installer::_text_error --
#
#   A "protected method" used by text_download as a callback for error
#   conditions
#
# Arguments:
#  token -- the download instance
#  err -- the error message
#
# Results:
#   void
#
proc installer::_text_error {token err} {
  variable $token
  upvar 0 $token session

  puts ""
  puts "Error downloading file [lindex $session(-urls) [expr $session(currfile) - 1]]:\n$err"

  _try_callback $token $session(texterror) [list $token $err]
}

# installer::gui_download --
#
#   This method provides a high-level interface for doing potentially
#   long-running downloads. It "inherits from" bg_download, but provides the
#   additional behavior that a gui-based progress report is output.
#
# Arguments:
#    (same as bg_download)
#    (optional)-parent {}        -- a widget in which to build the megawidget.
#                                   If not specified, a toplevel is created to
#                                   be the parent.
#
# Results:
#   void
#
proc installer::gui_download {args} {
  set args [_steal_opt parent "" $args]
  set args [_steal_opt progress "" $args]
  set args [_steal_opt done "" $args]
  set args [_steal_opt error "" $args]
  set args [_steal_opt block 0 $args]

  if {[catch {set token [eval bg_download $args [list \
                   -progress [namespace code _gui_progress] \
                   -done [namespace code _gui_done] \
                   -error [namespace code _gui_error] ] ]} err]} {
    return -code error $err
  }

  variable $token
  upvar 0 $token session

  set session(guiprogress) $progress
  set session(guidone) $done
  set session(guierror) $error
  set session(parent) $parent
  set session(display) 1
  set session(-block) $block

  set session(closeonexit) 0

  if {"" == $session(parent)} {
    regsub -all :: $token _ safename
    set session(parent) [toplevel .download$safename]
    wm title $session(parent) "Downloading..."
    button $session(parent).closebutton -text "Cancel" \
                -command [namespace code "_close_win $session(parent) $token"]
    pack $session(parent).closebutton -side bottom -padx 4 -pady 4
    wm protocol $session(parent) WM_DELETE_WINDOW {
      invoke $session(parent).closebutton
    }
    checkbutton $session(parent).closeonexit \
                            -text "Close this window when finished" \
                            -variable $token\(closeonexit) \
                            -command {}
    pack $session(parent).closeonexit -side bottom -padx 4 -pady 4
  } else {
    if {"." == $session(parent)} {
      set session(parent) ""
    }
  }

  gauge_create $session(parent).file_gauge
  pack $session(parent).file_gauge -side bottom -padx 4 -pady 4
  gauge_create $session(parent).all_gauge counter
  pack $session(parent).all_gauge -side bottom -padx 4 -pady 4
  label $session(parent).filename -text [lindex $session(-urls) 0]
  pack $session(parent).filename -side left -padx 4 -pady 4 -fill x

  set session(lastfile) 0

# do this last because it calls "update"
  gauge_value $session(parent).all_gauge 1 $session(totalfiles)

  # the session variable may have been deleted during the update called by
  # gauge_value
  if {[info exists session(-block)] && $session(-block) } {
    vwait $token\(complete)
  }
  return $token
}

# installer::_gui_progress --
#
#   A "protected method" used by gui_download as a callback for when a block
#   has been downloaded by the http package. Outputs status on the download
#   "megawidget".
#
# Arguments:
#  (as specified for bg_download progress callbacks)
#
# Results:
#   void
#
proc installer::_gui_progress {token file filenum totalfiles current totalsize} {
  variable $token
  upvar 0 $token session

  if {!$session(display)} {return}

  if {$session(lastfile) != $filenum} {
    $session(parent).filename configure -text "Downloading: $file"
    gauge_value $session(parent).all_gauge $filenum $totalfiles
    set session(lastfile) $filenum
  }

  if {$totalsize == 0} {
    set totalsize 1
  }
  gauge_value $session(parent).file_gauge \
                                      [expr 100.0 * $current/$totalsize]
  _try_callback $token $session(guiprogress) [list $token \
                                              $file \
                                              $filenum \
                                              $totalfiles \
                                              $current \
                                              $totalsize] _gui_error
}

# installer::_gui_done --
#
#   A "protected method" used by text_download as a callback for when the entire
#   batch of downloads is complete.
#
# Arguments:
#  token -- the download instance
#
# Results:
#   void
#
proc installer::_gui_done {token} {
  variable $token
  upvar 0 $token session

  if {$session(display)} {
    gauge_value $session(parent).all_gauge $session(totalfiles) $session(totalfiles)
    gauge_value $session(parent).file_gauge 100
    $session(parent).filename configure -text "Downloading: Complete"
    if {$session(closeonexit)} {
      $session(parent).closebutton invoke
    } else {
      if {[info command $session(parent).closeonexit] != ""} {
        $session(parent).closeonexit configure -state disabled
      }
      if {[info command $session(parent).closebutton] != ""} {
        $session(parent).closebutton configure -text "Close"
      }
    }
  }
  _try_callback $token $session(guidone) $token _gui_error
}

# installer::_gui_error --
#
#   A "protected method" used by gui_download as a callback for error conditions
#
# Arguments:
#  token -- the download instance
#  err -- the error message
#
# Results:
#   void
#
proc installer::_gui_error {token err} {
  variable $token
  upvar 0 $token session

  if {[info command $session(parent).closeonexit] != ""} {
    $session(parent).closeonexit configure -state disabled
  }
  if {[info command $session(parent).closebutton] != ""} {
    $session(parent).closebutton configure -text "Close"
  }
  tk_messageBox -title "Download Error" \
                -message "Error downloading file [lindex $session(-urls) [expr $session(currfile) - 1]]:\n$err"
  _try_callback $token $session(guierror) [list $token $err]
}

# installer::_close_win --
#
#   A "protected method" used by gui_download as a callback for the cancel/close
#   button on the toplevel (if no parent was specified)
#
# Arguments:
#  token -- the download instance
#  err -- the error message
#
# Results:
#   void
#
proc installer::_close_win {win token} {
  variable $token
  upvar 0 $token session

  if {[info exists session]} {
    set session(display) 0
    reset $token "User cancelled download"
  }
  destroy $win
}


namespace eval installer {
  set _progress [list / - \\ |]
}

# white is optimal for Windows due to the radiobutton style
option add *Radiobutton.selectColor white startupFile
option add *Checkbutton.selectColor white startupFile

option add *Entry.background white startupFile
option add *Listbox.background white startupFile
option add *Text.background white startupFile
option add *Text.font explainFont startupFile
option add *Panedwindow.grip.cursor sb_h_double_arrow startupFile
option add *Divlist.selectBackground NavyBlue startupFile
option add *Divlist.selectForeground white startupFile
option add *About.tclish.info*background #696969 startupFile
option add *About.tclish.info*foreground white startupFile
option add *About.tclish.info*padY 0 startupFile
option add *About.tclish*Button.background black startupFile
option add *About.tclish*Button.foreground white startupFile
option add *About.explain*background #696969 startupFile
option add *About.explain*foreground white startupFile

switch $install(style) {
  web {
    if {[string compare $tcl_platform(platform) "windows"] == 0} {
      if {[info exists env(TEMP)]} {
        set parameter(tclish-package-dir) $env(TEMP)
      } elseif {[info exists env(TMP)]} {
        set parameter(tclish-package-dir) $env(TMP)
      } else {
        set parameter(tclish-package-dir) C:/temp
      }
    } else {
      set parameter(tclish-package-dir) /tmp
    }
  }
  onefile {
    set parameter(tclish-package-dir) packages
  }
  default {
    set parameter(tclish-package-dir) [file join $install(location) packages]
  }
}

wm withdraw .

proc fatalerror {message} {
  foreach x [after info] {
    after cancel $x
  }
  destroy .placard
  update
  tk_messageBox -title "Tclish: Error" -icon error \
      -message $message
  exit 1
}

proc geturldata {url} {
  global install

  if {[catch {
         set tok [http::geturl $url -headers $install(authtok) \
                                    -timeout $install(webtimeout)]
       } err]} {
    fatalerror "Error connecting to web location \"$url\"\n$err"
  }
  if {[string compare [lindex [http::code $tok] end] "OK"] != 0} {
    fatalerror "Error retrieving information from \"$url\"\n[http::code $tok]"
  }
  set data [http::data $tok]
  http::cleanup $tok
  return $data
}

proc create_photo {imagename} {
  global install

  switch $install(style) {
    web {
      set data [geturldata $install(url)/img/$imagename]
      return [image create photo -data $data]
      unset data
    }
    onefile {
      return [image create photo -file [file join img $imagename]]
    }
    default {
      return [image create photo -file [file join $install(location) img $imagename]]
    }
  }
}

proc install_platform {} {
    global tcl_platform

    if {[string compare $tcl_platform(platform) "windows"] == 0} {
      return "win"
    }
    switch -- $tcl_platform(os) {
        SunOS {
            if {[package vcompare $tcl_platform(osVersion) 5.0] >= 0} {
              if {[package vcompare $tcl_platform(osVersion) 5.6] >= 0} {
                return "sol26"
              } else {
                return "sol"
              }
            }
            return "sunos"
        }
        AIX {
          return "aix4"
        }
        HP-UX {
          return "hp10"
        }
        Linux {
          return "linux"
        }
    }
    return "unknown"
}

# dummy settings for custom variables in img/custom.cfg
array set customize {
  title ""
  creator ""
  copyright ""
  credits ""
  patchString ""
  host ""
  target ""
  gnuprovn ""
  gccvn ""
  buttonBackground ""
  buttons ""
  splashScreen ""
  mainScreen ""
  installScreen ""
  winopts ""
  installdir ""
  uinstalldir ""
  winstalldir ""
}

proc custom_set {index value} {
  global customize
  set customize($index) $value
}

set customizeInterp [interp create -safe]
foreach x [array names customize] {
  $customizeInterp alias $x custom_set $x
}
$customizeInterp alias create_photo create_photo

switch $install(style) {
  web {
    set customtxt [geturldata $install(url)/img/custom.cfg]
  }
  onefile {
    set f [open [file join img custom.cfg]]
    set customtxt [read $f]
    close $f
    unset f
  }
  default {
    set f [open [file join $install(location) img custom.cfg]]
    set customtxt [read $f]
    close $f
    unset f
  }
}
if {[catch {$customizeInterp eval $customtxt} err]} {
  error "Error reading custom.cfg: $err"
}
interp delete $customizeInterp
unset customtxt
unset customizeInterp

# ----------------------------------------------------------------------
font create headingFont -family helvetica -size 18 -weight bold
font create explainFont -family helvetica -size 12 -weight normal

image create photo eff-sm -data {
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
CugvxR1+B0jdAQEAOw==
}

image create photo powered  -data {
R0lGODlhLwBLAPf/AP/////37//3AP/n5//n1v/e1v/eAP/Wxv/WAP/Gtf/G
AP+ce/+Uc/9rQv85CPf///fGvfe9rfe1APetnPetlPetAPeljPechPeUe/eU
CPeMc/eEY/eECPdzWvdaMfdSKfdCGPc5CO+9te+Ue++Ec+97Y+9zUu9zCO9r
CO9jQu9aOe9SMe9SCO9KIe9CGO85EO8xCO8pAOfe5+fGxuetpeetnOecjOeU
hOeEc+dKIedCIedCEOdCCOdCAOc5GOc5EOcxEOcxCOcpEOcpAOchEOchAOcY
AN7v997n9969vd61td6clN5zY95aOd5SMd5KKd5KId5CId45EN4xEN4pCNbe
59bW59bGzta9xtalrdaMhNZzY9ZrWtZrUtZjStY5GM7Gxs7Gvc6lpc6clM6U
hM6Ec857a84xGMbG3sbGvca1xsa1rcatrcatpcalpcalnMaUjMaEe8ZjUsZK
OcYxIb3Ozr3Gzr3Gxr29vb21tb2tvb1ja705IbXG1rXGzrW9vbW1vbWlrbVr
a7VSSrVCMbUxKa29zq29xq2txq2tta2cnK1aWqW9xqWtxqWttaWUnKVrc6U5
MaUxMaUxKZylxpylvZylrZyUrZyEjJxKUpxCQpStxpSlxpR7lJR7hJRre5Q5
QpQxMYyt1oyltYycxoycrYyUvYyUrYxSa4xSWoxCSoScxoRre4QpOXuUtXuM
pXtje3tSa3tCUnsxQnOEpXN7rXNrjHMxSnMxQnMpOWuMrWuErWuEnGtCWmsp
SmN7nFpzpVpzlFprnFpjlFpSc1pCa1JrlFJjnFI5Y1IxUkprrUpjjEpalEpK
e0oxUkohSkJjnEJajEIxWkIpUjlrrTlalDlSjDkhSjFKhDE5czExYylalClK
hCkxYykpWikhWikYSiFSlCFCeyExaxhKjBhChBgxcxghWhBCjBA5exA5cxAh
YxAhWgghYwAxewAxcwApewApcwApawAhawAhYwAYYwAYWgAAAGlpaQAAAAAA
AAAAAAAAAAAAAAAAAAAAACH5BAEAAPgALAAAAAAvAEsAQAj/APEJHEiwoMGD
CBMabBLkh8OHHHYQOaFgCpCHP4IEsRDgwYMIMDCKdBgEykEzMRxKUQlkR4Yd
DodsIDXvnU142mCEaNFkBxAhJwzs8CEExQkgMJLBE2dwR8ORDoGEmGMu3jt3
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
}

image create photo tclish-title-img -data {
R0lGODlhWgGMAOcAAAAAACkYGCEhIRAQECkpKWNjY0JCQgAACDExMVJSWkpK
ShAQIRAQMRAQOQgIOQgIQjk5OVpaWmtrawgIKVJSUikpUiEhQkI5WjEhSnNz
c4yMjLW1tefn7////3t7hM7Ozvf394SEhJSUlDk5czExY87O1q2trd7e53Nz
52tr3mtr1mNjxkJCjO/v76WlpcbGxmNjzkpKlFpatXt7e1JSpUpKnFJSc9bW
1lpanFoxQmMhMbVCUr0pQq0pOcY5QsYYKZw5QpQxOVo5SkopMUIYGHsxMaUp
Ma0YKd4xOd4YKd4QGN4IENYAEM4AEK0xOZycnEIhKbUhKfcIEP8IGL0AEEop
SlpKUntCSow5UqUhKVJSlHMxUr0QGP8QIRgYSv8YKf8hMYQpMaUACN6MjN5C
Ur29vf+EjIQACNatrffe3v9CSuettYwACP9aWv+9xpQhGMa9vf+tra1ja4wx
Me+UnK0AEL2cnGsxOfdrc6WlvYyMxv+UlDEx1gAA3iEh787O3ns5WntahBAQ
9wAA/zE5/7VKlGs5Wq0AY0IpUtYhMcZSUv8xQkJC3lIArdZaY3NCe95aY1oA
pd5rc6UAKWMhayEA3rUAIdZrayEAtVoAe961vYR7cyEAzmtjY3MAStYxMb1z
c85CSrWtraWMjGtr/8YYSnNjY2Naxq2cnHt7/ykpa62l/8aMjGtSUkIAjJSE
e5xze0pK95ycrYyM/3tza5RSWlophJRra4SEpUoIrWNjhJxja8acvXtKUmtr
jEIAvcbG/4QAhBgY/62tvXt7nFpahKWltZSUrZyctWNj3mMhnGtrtTk5xq0x
CK1zxqU5CN69AO9zCJyUQqUIa7VaCOfnAPdaEP//AP9KEIwQCOdCEO+MCL2U
APe9AM57ANaEAM5CCPetAPfeAP9CGDExpYx7tRAQziEhrQgIpQAAvRAQtXt7
vQAArQAA1jEpvWNj5yEhvYSEazk5nCEhlBAAxikphBgYnFJS1hgYlAgInIyM
3nNz/xAQORAQORAQORAQOSwAAAAAWgGMAAAI/gABCBxIsKDBgwgTKhQYQIDD
hwICBBjQMACBAgYoFsBYUcCAAQcOgDxAAEGAkChJJlCwQMCCli9jynzJYEFN
Bjhz4mzAoIFPBz+BPhBKtMFQo0WPKk3KFKnTpU+bQp0qtWrUq1SxWs3KdatX
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
daRyvy4aU6Fx8Hae5y3kFgYEADs=
}

image create photo tclish-icon -data {
R0lGODlhyAA4APf/AP///xAQECEhISkpKTExMTk5OUJCQkpKSlJSUlpaWmNj
Y2tra3Nzc3t7e4yMjKWlpbW1tb29vcbGxs7Ozvf397WtraWcnIyEhHNra2tj
Y5xzc++trd6cnHNKStZ7e95zc3s5OZRCQow5OdZCQv8YGIwAAKUAAJwQCN4Y
CIRSSoRKQr0hCPc5GO8xCIRrY/dKEPdaEN5SCPdrEP+UOfdzAKVSCHtrWrVa
ANaECP+9AO+1AGNzcwAQ/yEp9+fn787O1tbW3t7e55SUnHNze2trc8bG1q2t
vbW1xqWltUpKUq2txjk5QpyctZSUrYyMpYSEnCkpMYSEpVpac3NzlGtrjGNj
hDExQlJSc3NzpUpKazk5WnNztZSU70JCa3Nzxjk5YykpSjExWlpapWtrznNz
3ikpUmNjxmtr1lJSpXNz5zExYxgYMRAQIWNjzlpavUJCjDk5eyEhSikpYzk5
jCEhUkpKvWNj/1pa7zk5nCkpc0pKzikpezExlDk5rRAQMTExnEJC5ykplCkp
nDExvSEhnDEx5ykpzggIKTEx/yEhrSEhtRgYlCEh5wgIOQgIQhAQrRAQtRAQ
vRAQ7wgItQAACAAAEAAApQAArQAAvQAAzgAA9wAA/yEQvXNrlCkYQkoAvVox
jFohpda978at3loApXtKnCkQMYxrlJRCpWMQc2sAe3sAhEI5QhgQGEIhQjEQ
MVIpSoQAa5wphLUpjKUAa3MhUmMpSs5jnDkpMbUAWqUhWjEYId5rlMY5a3Mp
Qr0AQmNKUpwAMe+tvXMxQqVaa4Q5Sv9rjFIpMd5ac/9SczkQGJQhObUpQt4Y
Od5zhKU5SntCSrUxQpwhMcZze7VaY3spMedCUr0pOb0QIfeUnJw5QpwxOf85
StYhMc4QId4QIecQIf8QIc4IGL0AEM4AEPdze9Zja/dja95SWv9aY+9SWsY5
Qq0xOdY5QqUpMbUhKcYYIa0QGP8IGJwACO8AEP8AEP9SWs4hKfcYIcYQGN4I
ELUACN4ACOcACAAAAMDAwCH5BAEAAP8ALAAAAADIADgAQAj/AP0JHEiwoMGC
rQQoFJBQQIIBrSi1ClCpUiuLbNZo1MjGT5yOfkKKbESykSM/jkyiVJnyZMuV
LlnKjEkTps2XOGfe1JmzZs+dPnkKrWlQYkMBA5IuVDiAgIECDP0FWLo0QABK
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
PDW0SDAgADs=
}

image create photo checkBoxIcon-blank -data {
R0lGODdhEAAQAIAAAPj8+AAAACwAAAAAEAAQAAACDoSPqcvtD6OctNqLsz4FADs=
}

image create photo checkBoxIcon-0 -data {
R0lGODlhEAAQAJH/AP///9nZ2QAAAMDAwCwAAAAAEAAQAEACJISPeRLtLZSYtMr3otqcYXh9
gcaV5oka3kcu4hiyMdaq1Z1uBQA7
}

image create photo checkBoxIcon-1 -data {
R0lGODdhEAAQAIQAAPj8+Pi0sOAYGPjEwPjg4OgQEOAEAAAAAEAwMOAkIOgkIPj08NjY2NjA
wNiMiPjQ0Ni4uOBcWNigoPCUkNisqOAUEOBISDg0MOBgYAAAAAAAAAAAAAAAAAAAAAAAAAAA
ACwAAAAAEAAQAAAFUiAgjmRpnugZCEM6EoVhBONh3weSyMpSM4wGkOGQGR6kAwMiiDAkxknp
QKnILFZDwnTYGWWVC7fxlVEO3OXXiZ4CITIMsJ0cNhxDeg3Hd/n/IyEAOw==
}

image create photo installIcon -data {
R0lGODlhrwDSAPf/AP///wgICBAQEBgYGCEhISkpKTk5OUJCQkpKSlJSUlpaWmtra3Nzc3t7
e4SEhIyMjJycnKWlpbW1tb29vcbGxs7OztbW1t7e3ufn5+/v797W1r21tYR7e1JKShAICOfe
1sa9td7WztbOxr21ra2Ue7WUc+/OpefGnM6thMale72cc7WUa62MY+/exq2chOfOrd7Gpc61
lKWMa5yEY5R7WoxzUoRrSnNaOefWvca1nOfGlN69jNa1hLWljJyMc+fOpbWcc62Ua6WMY4Rr
Qsa9rb2le969hJyEWt7WxufWtd7OrZSEY97GlHtrSnNjQmtaOWNSMdbOvc7Gta2llJSMe4x7
UoRzSntrQnNjOVpKIYR7Y9bOtbWtlK2ljJyUe1JKMXNrSlpSMVJKKUpCGM7Oxt7e1q2tpZSU
jJyclL29rbW1pZycjKWllIyMe4SEc3t7a0JCOXNzY62tlGtrWjExKYyMcykpIVJSQnNzWlpa
Qjk5KUpKMUpSOWNrWpScjFJaSjE5KXuEc1pjUjlCMefv55ylnGtza3uEe1JaUkpSSmt7azlC
OWNzYykxKVJjUnOUc2uMc1JrWnuUhGuUe2Nza0paUnuMhGuEe621tZScnFpjY4SUlDlCQmNz
cylCSoSUnGt7hClCUlpja0JjjFpznFJrlDFSjGNzlJSctYSMpUJShFpjpUpS1tbW3sbG1kpK
Upycrb291qWlvZSUrb293qWlxq2t1qWl1qWl3oyMvYSEtZSUznNzpYSExmNjnHt7xmtrrYSE
3nNzzmtrxlJSnGtrzoSE/3t7/1pavUJClGtr71pazmtr/1JSxmNj71JSzlJS1lpa70pKzlpa
/0JCvUpK1jk5zjk51kJC/zk53ikp3ikp/yEh9yEh/xAQ7xAQ/wgI3ggI9wAA/0pC3ntz52tj
3nNr75yU55SM73tztbWt3sa93qWcvb21zq2lvca91pSMnIx7nLWlva2ctc7Gzr2tvWNaY5SE
lIx7jMatxrWctaWMpSEYIYRrewAAANnZ2SH5BAEAAP8ALAAAAACvANIAQAj/AP8JHEiwoMGD
CBMqXMiwocOHECNKlHjBgkULFSpQmCDBjJkIER5oQcOmZKEpheSklBNBpUszLF3KTFmyps01
XdboRKOzp0+dXUii2XDRwoWJSJMqXVhUY0cIDxgoOGCAgaQRFdrZwoVMnNevYMGGG/utrNmz
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
}

image create photo installedIcon -data {
R0lGODlhwADSAPf/AP///wgICBAQEBgYGCEhISkpKTExMTk5OUpKSlJSUlpaWmNjY3Nzc3t7
e4SEhJSUlJycnKWlpa2trbW1tc7OztbW1t7e3rWtrca9vXtzc2NaWox7e5yEe+fe1s69rd7W
zr21raWMc72Ua87GvYyEe+fGnN69lNa1jM6thMale6WEWq2chO/WtefOraWMa5yEY5R7WqWU
e+fGlNa1hGNSOaWEUufOpa2Ua4RrQufezsa9raWcjN7GnJyEWt7Wxr21pZyUhM69nHNjQmta
OWNSMVJCIdbOvc7GtbWtnK2llKWMUox7UoRzSntrQnNjOca9paWchGNaQtbOta2ljHtzWnNr
SlJKKVpSKe/v5/f379bWzufn3q2tpdbWxr29rc7OvVJSSq2tnKWllIyMe2trWlpaSoyMcykp
IVJSQnt7Y3NzWkpKOWNjSkJCMVpaQiEhGFJSOUpKMXN7Y2NrUnuEazE5KVpjUnN7c1JaUlpj
WjE5MXOEe3N7e0JKSilCQilCSkJSWjFKWoyUnClShFJrjDlajFJrlDFSjCFChGNrnFJanEpS
lEJKjEpSrUpSxs7O1pSUnJycpcbG1r293rW11rW13oyMra2t3nNzlDExQnt7paWl3oSEtaWl
51pahJyc54SEzpSU73NzvYyM52NjpXNzzmtrxnNz3nt772NjzlpavTk5e2Nj1lpaxmtr72Nj
51pa1lJSzkpKvTk5lFJS1kpKxkpK3lJS/0JC50JC/zk59zk5/zEx3jEx/ykp3ggIMSkp/wgI
ORAQeyEh/xgYvRgY7wgIcxAQ9xAQ/wgIlAgItQgIvQgIxggI5wAAGAAAKQAAQgAAUgAAWgAA
awAAewAAjAAAnAAAtQAAvQAAzgAA3gAA5wAA9wAA/1pS71JK7zEpvTEp7ykh5yEY52tj52Na
3kpCrRgQjJSM53NrxmtjvRgQWkpCe7Wt3q2l1ox71hgQOb211rWtzq2lxhAIKb21zmtje1JK
Y62czrWlzpSEpaWMtXtrhIx7jAAAANnZ2SH5BAEAAP8ALAAAAADAANIAQAj/AP8JHEiwoMGD
CBMqXMiwocOHECNKnEjxoYUKGDNq3Mixo8ePIEOKHEkSpIWKKFMWvIiRwhcMGCRAetDgDpkF
DBpIIINBSxeXFIJKGoohnqR6SOPVk6cUKVKmSaPGm0rVqFVJU7FqHbqVq1ejXCc9kjRpaNmv
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
eoUx+od+GqiDWqh/enwopyUCAgA7
}

# ----------------------------------------------------------------------
proc percent_subst {percent string subst} {
    if {![string match %* $percent]} {
        error "bad pattern \"$percent\": should be %something"
    }
    regsub -all {\\|&} $subst {\\\0} subst
    regsub -all $percent $string $subst string
    return $string
}

# ----------------------------------------------------------------------
# BUGHANDLER
# These procedures trap unexpected errors and give the user a chance
# to send a bug report via e-mail.
# ----------------------------------------------------------------------
option add *Bughandler*Text.background white startupFile
option add *Bughandler*Text.width 50 startupFile
option add *Bughandler*Text.height 10 startupFile
option add *Bughandler*Entry.background white startupFile
option add *Bughandler*remarks.text.font "helvetica 12 normal" startupFile
option add *Bughandler*Label.font "helvetica 10 normal" startupFile
option add *Bughandler.title.font "helvetica 12 bold" startupFile

image create bitmap bugHandlerIcon -data {
#define bug_width 31
#define bug_height 28
static unsigned char bug_bits[] = {
   0x00, 0xe0, 0x03, 0x00, 0x00, 0xf0, 0x07, 0x00, 0x00, 0xf8, 0x0f, 0x00,
   0x10, 0xfc, 0x1f, 0x04, 0x10, 0xfc, 0x1f, 0x04, 0x10, 0xfc, 0x1f, 0x04,
   0x10, 0xaa, 0x2a, 0x04, 0x60, 0xd5, 0x55, 0x03, 0xc0, 0xaa, 0xaa, 0x01,
   0x40, 0xd5, 0x55, 0x01, 0xa0, 0xaa, 0xaa, 0x02, 0x60, 0xd5, 0x55, 0x03,
   0xa1, 0xaa, 0xaa, 0x42, 0x62, 0xd5, 0x55, 0x23, 0xbc, 0xaa, 0xaa, 0x1e,
   0x60, 0xd5, 0x55, 0x03, 0xa0, 0xaa, 0xaa, 0x02, 0x60, 0xd5, 0x55, 0x03,
   0xa0, 0xaa, 0xaa, 0x02, 0x60, 0xd5, 0x55, 0x03, 0xc0, 0xaa, 0xaa, 0x01,
   0x80, 0xd5, 0xd5, 0x00, 0x80, 0xab, 0xea, 0x00, 0xe0, 0xd6, 0xb5, 0x03,
   0x10, 0xfc, 0x1f, 0x04, 0x10, 0x30, 0x06, 0x04, 0x10, 0x00, 0x00, 0x04,
   0x10, 0x00, 0x00, 0x04};
}

toplevel .bughandler -class Bughandler
wm title .bughandler "Unexpected Error"
wm group .bughandler .
wm transient .bughandler .
wm protocol .bughandler WM_DELETE_WINDOW {
    .bughandler.cntls.cancel invoke
}
wm withdraw .bughandler

frame .bughandler.cntls
pack .bughandler.cntls -side bottom -fill x -padx 4 -pady 4
button .bughandler.cntls.send -text "Send E-mail" -command {
    set bughandler(result) "send"
}
pack .bughandler.cntls.send -side left -expand yes
button .bughandler.cntls.review -text "Review Message" -command {
    set bughandler(result) "review"
}
pack .bughandler.cntls.review -side left -expand yes
button .bughandler.cntls.cancel -text "Ignore" -command {
    set bughandler(result) "ignore"
}
pack .bughandler.cntls.cancel -side left -expand yes

frame .bughandler.sep -borderwidth 1 -height 2 -relief sunken
pack .bughandler.sep -side bottom -fill x -pady 8

frame .bughandler.info
pack .bughandler.info -expand yes -fill both -padx 4 -pady 4
frame .bughandler.info.opts -class Bughandler
pack .bughandler.info.opts -expand yes -fill both
.bughandler configure -background [.bughandler.info.opts cget -background]

frame .bughandler.info.opts.user
grid columnconfigure .bughandler.info.opts.user 1 -weight 1

label .bughandler.info.opts.user.title -wraplength 6i -anchor w -justify left -text "Enter information about yourself, so we can follow up if we need additional information:"
grid .bughandler.info.opts.user.title -row 0 -column 0 -columnspan 2 -sticky ew

label .bughandler.info.opts.user.unamel -text "Name:"
grid .bughandler.info.opts.user.unamel -row 1 -column 0 -sticky e
entry .bughandler.info.opts.user.uname
grid .bughandler.info.opts.user.uname -row 1 -column 1 -sticky ew
label .bughandler.info.opts.user.uphonel -text "Phone:"
grid .bughandler.info.opts.user.uphonel -row 2 -column 0 -sticky e
entry .bughandler.info.opts.user.uphone
grid .bughandler.info.opts.user.uphone -row 2 -column 1 -sticky ew
label .bughandler.info.opts.user.uemaill -text "E-mail:"
grid .bughandler.info.opts.user.uemaill -row 3 -column 0 -sticky e
entry .bughandler.info.opts.user.uemail
grid .bughandler.info.opts.user.uemail -row 3 -column 1 -sticky ew

frame .bughandler.info.opts.remarks
label .bughandler.info.opts.remarks.title \
    -text "Describe what you were doing when the error occurred:"
pack .bughandler.info.opts.remarks.title -anchor w
text .bughandler.info.opts.remarks.text -wrap word \
    -yscrollcommand ".bughandler.info.opts.remarks.sbar set"
pack .bughandler.info.opts.remarks.text -side left -expand yes -fill both
scrollbar .bughandler.info.opts.remarks.sbar -orient vertical \
    -command ".bughandler.info.opts.remarks.text yview"
pack .bughandler.info.opts.remarks.sbar -side right -fill y

label .bughandler.info.opts.explain -wraplength 5i -text "Click on the \"Send E-mail\" button if you want to send a report of this incident to our development team."

label .bughandler.info.opts.icon -image bugHandlerIcon
label .bughandler.info.opts.title -wraplength 5i -justify center \
    -text "This application has encountered an expected error:"

label .bughandler.info.opts.mesg -wraplength 5i -justify center

pack .bughandler.info.opts.remarks -side bottom -expand yes -fill both -pady 4
pack .bughandler.info.opts.user -side bottom -fill x -pady 4
pack .bughandler.info.opts.icon -side left
pack .bughandler.info.opts.explain -side bottom -fill x -padx 8 -pady 4
pack .bughandler.info.opts.title -fill x
pack .bughandler.info.opts.mesg -fill x


toplevel .bughandlershow -class Bughandler
wm title .bughandlershow "Review E-mail Bug Report"
wm group .bughandlershow .
wm transient .bughandlershow .
wm protocol .bughandlershow WM_DELETE_WINDOW {
    .bughandlershow.cntls.cancel invoke
}
wm withdraw .bughandlershow

frame .bughandlershow.cntls
pack .bughandlershow.cntls -side bottom -fill x -padx 4 -pady 4
button .bughandlershow.cntls.send -text "Send E-mail" -command {
    set bughandler(review) "send"
}
pack .bughandlershow.cntls.send -side left -expand yes
button .bughandlershow.cntls.cancel -text "Cancel" -command {
    set bughandler(review) "ignore"
}
pack .bughandlershow.cntls.cancel -side left -expand yes

frame .bughandlershow.sep -borderwidth 1 -height 2 -relief sunken
pack .bughandlershow.sep -side bottom -fill x -pady 8

frame .bughandlershow.info
pack .bughandlershow.info -expand yes -fill both -padx 4 -pady 4
frame .bughandlershow.info.mesg -class Bughandler
pack .bughandlershow.info.mesg -expand yes -fill both
text .bughandlershow.info.mesg.text -width 50 -height 15 -wrap none \
    -state disabled -yscrollcommand ".bughandlershow.info.mesg.sbar set"
pack .bughandlershow.info.mesg.text -side left -expand yes -fill both
scrollbar .bughandlershow.info.mesg.sbar -orient vertical \
    -command ".bughandlershow.info.mesg.text yview"
pack .bughandlershow.info.mesg.sbar -side right -fill y

# ----------------------------------------------------------------------
#  USAGE:  bughandler_install <program> <email>
#
#  Installs a bgerror handler for unexpected errors.  When an error
#  occurs, the bughandler_activate procedure is called to notify
#  the user and let them send a bug report.
# ----------------------------------------------------------------------
proc bughandler_install {program email} {
    global bughandler
    set bughandler(program) $program
    set bughandler(email) $email
    proc bgerror {err} {bughandler_activate $err}
}

# ----------------------------------------------------------------------
#  USAGE:  bughandler_activate <err>
#
#  Invoked by bgerror whenever an unexpected error occurs.  Pops up
#  an error dialog displaying the <err> message, and gives the user
#  an opportunity to send a bug report.
# ----------------------------------------------------------------------
proc bughandler_activate {err} {
    global bughandler env errorInfo
    set stackTrace $errorInfo

    set opts .bughandler.info.opts

    if {[llength [split $err \n]] > 5} {
        set err [lrange [split $err \n] 0 4]
        lappend err "..."
        set err [join $err \n]
    }

    $opts.mesg configure -text $err
    $opts.remarks.text delete 1.0 end

    if {[info exists env(NAME)]} {
        $opts.user.uname delete 0 end
        $opts.user.uname insert 0 $env(NAME)
    }
    if {[info exists env(REPLYTO)]} {
        $opts.user.uemail delete 0 end
        $opts.user.uemail insert 0 $env(REPLYTO)
    }

    set x [expr ([winfo screenwidth .bughandler] \
        - [winfo reqwidth .bughandler])/2]
    set y [expr ([winfo screenheight .bughandler] \
        - [winfo reqheight .bughandler])/2]
    wm geometry .bughandler "+$x+$y"

    while {1} {
        wm deiconify .bughandler
        raise .bughandler
        vwait bughandler(result)
        wm withdraw .bughandler
        
        switch -- $bughandler(result) {
            send {
                set rem [string trim [$opts.remarks.text get 1.0 end]]
                set uname [string trim [$opts.user.uname get]]
                set uphone [string trim [$opts.user.uphone get]]
                set uemail [string trim [$opts.user.uemail get]]
                bughandler_send [bughandler_format \
                    $uname $uphone $uemail $rem $err $stackTrace]
                break;  # no errors--continue on
            }
            review {
                set remarks [string trim [$opts.remarks.text get 1.0 end]]
                set uname [string trim [$opts.user.uname get]]
                set uphone [string trim [$opts.user.uphone get]]
                set uemail [string trim [$opts.user.uemail get]]
                bughandler_show [bughandler_format \
                    $uname $uphone $uemail $remarks $err $stackTrace]
                break;  # no errors--continue on
            }
            ignore {
                break;  # do nothing (continue on)
            }
        }
    }
}

# ----------------------------------------------------------------------
#  USAGE:  bughandler_format <name> <phone> <email> <moreinfo> \
#                          <error> <stack>
#
#  Formats an electronic mail message with address <from>, with
#  carbon copies to <cc>.  The message has <moreinfo> from the
#  user, an <error> message, and a <stack> trace.
# ----------------------------------------------------------------------
proc bughandler_format {uname uphone uemail moreinfo error stack} {
    global bughandler env

    set message "To: $bughandler(email)"
    if {[string length $uemail] > 0} {
        append message "\nFrom: $uemail"
    }
    append message "\nSubject: BUG REPORT ($bughandler(program))"
    append message "\nDate: [clock format [clock seconds]]"
    append message "\n"  ;# sendmail terminates header with blank line
    append message "\n--------------------------------------------------"
    append message "\n   USER: $uname"
    append message "\n  PHONE: $uphone"
    append message "\n E-MAIL: $uemail"
    append message "\n--------------------------------------------------"

    if {[string length $moreinfo] > 0} {
        append message "\n\nCOMMENTS:\n$moreinfo"
    }
    append message "\n\nERROR:\n$error"
    append message "\n\nSTACK:\n$stack"

    return $message
}

# ----------------------------------------------------------------------
#  USAGE:  bughandler_show <message>
#
#  Pops up a dialog letting the user review an electronic mail
#  <message>.  If the user clicks on "Send E-mail", then the message
#  is sent by calling bughandler_send.
# ----------------------------------------------------------------------
proc bughandler_show {message} {
    global bughandler

    .bughandlershow.info.mesg.text configure -state normal
    .bughandlershow.info.mesg.text delete 1.0 end
    .bughandlershow.info.mesg.text insert 1.0 $message
    .bughandlershow.info.mesg.text configure -state disabled

    set x [expr ([winfo screenwidth .bughandlershow] \
        - [winfo reqwidth .bughandlershow])/2]
    set y [expr ([winfo screenheight .bughandlershow] \
        - [winfo reqheight .bughandlershow])/2]
    wm geometry .bughandlershow "+$x+$y"

    wm deiconify .bughandlershow
    raise .bughandlershow
    vwait bughandler(review)
    wm withdraw .bughandlershow

    if {$bughandler(review) == "send"} {
        bughandler_send $message
    }
}

# ----------------------------------------------------------------------
#  USAGE:  bughandler_send <message>
#
#  Sends an electronic mail <message>.  This message should be
#  generated by bughandler_format, or it should be some other message
#  properly formatted for sendmail.
# ----------------------------------------------------------------------
proc bughandler_send {message} {
    catch {
        set fid [open "| /usr/lib/sendmail -oi -t" "w"]
        puts -nonewline $fid $message
        close $fid
    }
}

# Put up an introductory placard while the program is loading...
# ----------------------------------------------------------------------
wm withdraw .

toplevel .placard -background black -borderwidth 2 -relief flat
wm overrideredirect .placard yes
wm withdraw .placard

frame .placard.info -borderwidth 2 -relief raised
pack .placard.info

# put up splash screen
label .placard.info.logo -image $customize(splashScreen)
pack .placard.info.logo
label .placard.info.copyright -font "helvetica 8 normal" \
    -text $customize(copyright)
pack .placard.info.copyright -pady 4

# print loading message for user
label .placard.info.splashfooter -font "helvetica 8 normal" \
    -text "Loading, please wait..."
pack .placard.info.splashfooter -pady 4

update idletasks
set x [expr ([winfo screenwidth .]-[winfo reqwidth .placard]) / 2]
set y [expr ([winfo screenheight .]-[winfo reqheight .placard]) / 2]
wm geometry .placard "+$x+$y"

update
wm deiconify .placard
update

after 3000 {
    update
    destroy .placard
    set x [expr ([winfo screenwidth .]-[winfo reqwidth .]) / 2]
    set y [expr ([winfo screenheight .]-[winfo reqheight .]) / 2]
    wm geometry . "+$x+$y"
    wm deiconify .
}

# ----------------------------------------------------------------------
# TOPIC VIEWER
# This viewer displays overview information about each package.
# ----------------------------------------------------------------------
option add *Topicviewer.heading.borderWidth 2 widgetDefault
option add *Topicviewer.heading.relief raised widgetDefault
option add *Topicviewer.heading*background DimGray widgetDefault
option add *Topicviewer.heading*highlightBackground DimGray widgetDefault
option add *Topicviewer.heading*Label.foreground white widgetDefault

proc topicviewer_create {win} {
    frame $win -class Topicviewer

    frame $win.heading -borderwidth 2 -relief raised
    pack $win.heading -fill x

    label $win.heading.icon -width 74 -height 74 -image [image create photo]
    pack $win.heading.icon -side left -padx 4 -pady 4

    frame $win.heading.info
    pack $win.heading.info -side left -expand yes -fill x
    label $win.heading.info.title -anchor w -justify left -font headingFont
    pack $win.heading.info.title -anchor w

    label $win.heading.info.subt -justify left \
        -font "helvetica 12 normal"
    pack $win.heading.info.subt -anchor w

    bind $win.heading.info <Configure> {topicviewer_resize %W}

    frame $win.info
    pack $win.info -expand yes -fill both
    text $win.info.text -width 10 -height 5 -wrap word -spacing1 6 \
        -state disabled -yscrollcommand "$win.info.sbar set"
    pack $win.info.text -side left -expand yes -fill both
    $win.info.text tag configure "normal" -lmargin1 4 -lmargin2 4 -rmargin 4

    scrollbar $win.info.sbar -orient vertical -command "$win.info.text yview"
    pack $win.info.sbar -side right -fill y
}

proc topicviewer_show {win title subtitle message {imagedata ""}} {
    set imh [$win.heading.icon cget -image]
    $imh blank
    $imh configure -data {}
    if {[string length $imagedata] > 0} {
        $win.heading.icon configure -width 74
        if {[lsearch [image names] $imagedata] == -1} {
          $imh configure -data $imagedata
        } else {
          $imh copy $imagedata
        }
    } else {
        $win.heading.icon configure -width 1
    }

    $win.info.text configure -state normal
    $win.info.text delete 1.0 end
    $win.info.text insert 1.0 $message "normal"
    $win.info.text see 1.0
    $win.info.text configure -state disabled

    $win.heading.info.title configure -text $title
    $win.heading.info.subt configure -text $subtitle
}

proc topicviewer_resize {win} {
    $win.subt configure -wraplength [expr [winfo width $win]-10]
    update idletasks
    if {[winfo reqheight $win] > [winfo height $win]} {
        pack $win -anchor n
    } else {
        pack $win -anchor c
    }
}

# ----------------------------------------------------------------------
# DIVIDED LIST
# This divlist component is like a listbox, but it adds headings for
# groups of elements within it.
# ----------------------------------------------------------------------
option add *Divlist.lbox.background white widgetDefault
option add *Divlist.lbox.width 40 widgetDefault
option add *Divlist.lbox.height 10 widgetDefault
option add *Divlist.lbox.cursor left_ptr widgetDefault
option add *Divlist.lbox.font "helvetica 12 normal" widgetDefault

proc divlist_create {win {cmd ""}} {
    global divInfo

    set divInfo($win-pending) ""
    set divInfo($win-command) $cmd

    set counter 0
    while {1} {
        set var "divData#[incr counter]"
        upvar #0 $var data
        if {![info exists data]} {
            break
        }
    }
    set divInfo($win-data) $var
    set data(categories) ""

    frame $win -class Divlist
    scrollbar $win.sbar -command "$win.lbox yview"
    pack $win.sbar -side right -fill y
    text $win.lbox -wrap none -takefocus 0 \
        -yscrollcommand "$win.sbar set"
    pack $win.lbox -side left -expand yes -fill both

    set btags [bindtags $win.lbox]
    set i [lsearch $btags Text]
    if {$i >= 0} {
        set btags [lreplace $btags $i $i]
    }
    bindtags $win.lbox $btags

    $win.lbox tag configure "title" -spacing1 6 -lmargin1 2 \
        -font "helvetica 12 bold"
    $win.lbox tag configure "short" \
        -font "helvetica 4"
    $win.lbox tag configure "entry" -lmargin1 4 \
        -font "helvetica 10 normal"

    set selectbg [option get $win selectBackground Foreground]
    set selectfg [option get $win selectForeground Background]
    $win.lbox tag configure "selected" \
        -background $selectbg -foreground $selectfg

    bind $win.lbox <Configure> "divlist_resize $win %w"
    bind $win <Destroy> "divlist_destroy $win"
    return $win
}

proc divlist_destroy {win} {
    global divInfo

    if {[info exists divInfo($win-data)]} {
        set var $divInfo($win-data)
        upvar #0 $var data
        unset data
        unset divInfo($win-data)
        unset divInfo($win-command)
        unset divInfo($win-pending)
    }
}

proc divlist_add {win category {title ""} {clientData ""} {checkbox 1}} {
    global divInfo

    if {![info exists divInfo($win-data)]} {
        error "not a divlist component: $win"
    }
    upvar #0 $divInfo($win-data) data

    if {[lsearch $data(categories) $category] < 0} {
        lappend data(categories) $category
        set data(cat-$category) ""
        divlist_redraw $win eventually
    }

    if {"" != $title} {
        if {[lsearch $data(cat-$category) $title] >= 0} {
            error "entry \"$category/$title\" already exists in divlist"
        }
        lappend data(cat-$category) $title
#        set data(cat-$category) [lsort -command divlist_cmp $data(cat-$category)]
        set data(data-$category-$title) $clientData
        set data(checkbox-$category-$title) $checkbox
        set data(checked-$category-$title) 0
        divlist_redraw $win eventually
    }
}

proc divlist_redraw {win {when "now"}} {
    global divInfo

    if {![info exists divInfo($win-data)]} {
        error "not a divlist component: $win"
    }
    upvar #0 $divInfo($win-data) data

    if {$when == "eventually"} {
        if {$divInfo($win-pending) == ""} {
            set divInfo($win-pending) [after idle [list divlist_redraw $win]]
        }
        return
    }
    set divInfo($win-pending) ""

    $win.lbox delete 1.0 end

    set counter 0
    foreach category $data(categories) {
        if {[llength $data(cat-$category)] > 0} {
            $win.lbox insert end "$category\n" "title"
            set lwin $win.lbox.line[incr counter]
            frame $lwin -height 1 -background black
            $win.lbox window create end -window $lwin
            $win.lbox tag add "short" end-1char end
            $win.lbox insert end "\n" "short"

            foreach title $data(cat-$category) {
                set tag "item[incr counter]"
                set data(tag-$category-$title) $tag

                if {$data(checkbox-$category-$title)} {
                    if {$data(checked-$category-$title)} {
                        $win.lbox image create end -image checkBoxIcon-1
                    } else {
                        $win.lbox image create end -image checkBoxIcon-0
                    }
                    $win.lbox tag add "check-$tag" end-2char end-1char
                    $win.lbox tag bind "check-$tag" <ButtonPress> \
                        [list divlist_toggle $win $category $title check-$tag]
                } else {
                    $win.lbox image create end -image checkBoxIcon-blank
                }

                $win.lbox insert end " $title\n" [list "entry" $tag]
                $win.lbox tag bind $tag <ButtonPress> \
                    [list divlist_select $win $category $title]
            }
        }
    }
}

proc divlist_resize {win w} {
    global divInfo

    foreach lwin [$win.lbox window names] {
        $lwin configure -width [expr $w-2]
    }
}

proc divlist_select {win args} {

    global divInfo

    if {![info exists divInfo($win-data)]} {
        error "not a divlist component: $win"
    }
    upvar #0 $divInfo($win-data) data

    ## added by steve, probably wrong but...
    set category ""
    set title ""

    switch -- [llength $args] {
        1 {
            set num [lindex $args 0]
            set count 0
            set found 0
            foreach category $data(categories) {
                foreach title $data(cat-$category) {
                    if {$count == $num} {
                        set found 1
                        break
                    }
                    incr count
                }
                if {$found} break
            }
        }
        2 {
            set category [lindex $args 0]
            set title [lindex $args 1]
        }
        default {
            error "bad index \"$args\": should be category/title or integer"
        }
    }

    if {![info exists data(tag-$category-$title)]} {
        error "can't find \"$category / $title\""
    }
    set node $data(tag-$category-$title)

    $win.lbox tag remove "selected" 1.0 end
    $win.lbox tag add "selected" $node.first $node.last

    if {$divInfo($win-command) != ""} {
        set cmd $divInfo($win-command)
        set cmd [percent_subst "%c" $cmd [list $category]]
        set cmd [percent_subst "%t" $cmd [list $title]]
        set cmd [percent_subst "%d" $cmd [list $data(data-$category-$title)]]
        set cmd [percent_subst "%n" $cmd $node]
        uplevel #0 $cmd
    }
}

proc divlist_get_checked {win} {
    global divInfo

    if {![info exists divInfo($win-data)]} {
        error "not a divlist component: $win"
    }
    upvar #0 $divInfo($win-data) data

    set all ""

    foreach category $data(categories) {
        foreach title $data(cat-$category) {
            if {$data(checked-$category-$title)} {
                lappend all $data(data-$category-$title)
            }
        }
    }
    return $all
}

proc divlist_check_all {win {state 1}} {
    global divInfo

    if {![info exists divInfo($win-data)]} {
        error "not a divlist component: $win"
    }
    upvar #0 $divInfo($win-data) data

    foreach category $data(categories) {
        foreach title $data(cat-$category) {
            if {$data(checkbox-$category-$title)} {
                set data(checked-$category-$title) $state
            }
        }
    }
    divlist_redraw $win eventually
}

proc divlist_toggle {win category title tag} {
    global divInfo

    if {![info exists divInfo($win-data)]} {
        error "not a divlist component: $win"
    }
    upvar #0 $divInfo($win-data) data

    if {$data(checked-$category-$title)} {
        set data(checked-$category-$title) 0
        $win.lbox image configure $tag.first -image checkBoxIcon-0
    } else {
        set data(checked-$category-$title) 1
        $win.lbox image configure $tag.first -image checkBoxIcon-1
    }
}

proc divlist_cmp {name1 name2} {
    regsub -all "~|`|!|@|#|\\\$|%|\\^|&|\\*|\\(|\\)|_|-|\\+|=|\\\{|\\\}|\\\[|\\\]|:|;|\"|'|<|>|,|\\.|/|\\?|\\|" $name1 "" name1
    regsub -all "~|`|!|@|#|\\\$|%|\\^|&|\\*|\\(|\\)|_|-|\\+|=|\\\{|\\\}|\\\[|\\\]|:|;|\"|'|<|>|,|\\.|/|\\?|\\|" $name2 "" name2
    return [string compare [string tolower $name1] [string tolower $name2]]
}

# ----------------------------------------------------------------------
# NOTEBOOK LIBRARY
# This is the basic notebook from the "Effective Tcl/Tk Programming"
# book.  We use "Next" and "Back" buttons to traverse through the
# panels.
# ----------------------------------------------------------------------
option add *Notebook.page.borderWidth 2 widgetDefault
option add *Notebook.page.relief sunken widgetDefault

proc notebook_create {win} {
    global nbInfo

    frame $win -class Notebook
    pack propagate $win 0

    set nbInfo($win-count) 0
    set nbInfo($win-pages) ""
    set nbInfo($win-current) ""
    return $win
}

proc notebook_page {win name} {
    global nbInfo

    set page "$win.page[incr nbInfo($win-count)]"
    lappend nbInfo($win-pages) $page
    set nbInfo($win-page-$name) $page
    set nbInfo($win-name-$page) $name

    frame $page

    if {$nbInfo($win-count) == 1} {
        after idle [list notebook_display $win $name]
    }
    return $page
}

proc notebook_display {win name} {
    global nbInfo

    set page ""
    if {[info exists nbInfo($win-page-$name)]} {
        set page $nbInfo($win-page-$name)
    } elseif {[winfo exists $name]} {
        set page $name
    } elseif {[winfo exists $win.page$name]} {
        set page $win.page$name
    }
    if {$page == ""} {
        error "bad notebook page \"$name\""
    }

    notebook_fix_size $win

    if {$nbInfo($win-current) != ""} {
        pack forget $nbInfo($win-current)
    }
    pack $page -expand yes -fill both
    set nbInfo($win-current) $page
}

proc notebook_fix_size {win} {
    global nbInfo

    update idletasks

    set maxw 0
    set maxh 0
    foreach page $nbInfo($win-pages) {
        set w [winfo reqwidth $page]
        if {$w > $maxw} {
            set maxw $w
        }
        set h [winfo reqheight $page]
        if {$h > $maxh} {
            set maxh $h
        }
    }
    set bd [$win cget -borderwidth]
    set maxw [expr $maxw+2*$bd]
    set maxh [expr $maxh+2*$bd]
    $win configure -width $maxw -height $maxh
}

proc notebook_current {win} {
    global nbInfo
    set page $nbInfo($win-current)
    return $nbInfo($win-name-$page)
}

# ----------------------------------------------------------------------
# PANEDWINDOW LIBRARY
# Use this customized version instead of the one in the Efftcl
# library.  This version supports a vertical sash.
# ----------------------------------------------------------------------
proc panedwindow_create {win width height} {
    global pwInfo

    frame $win -class Panedwindow -width $width -height $height
    frame $win.pane1
    place $win.pane1 -relx 0 -rely 0.5 -anchor w \
        -relwidth 0.5 -width -5 -relheight 1.0
    frame $win.pane2
    place $win.pane2 -relx 1.0 -rely 0.5 -anchor e \
        -relwidth 0.5 -width -5 -relheight 1.0

    frame $win.sash -width 2 -borderwidth 1 -relief sunken
    place $win.sash -relx 0.5 -rely 0.5 -relheight 1.0 -anchor c

    frame $win.grip -width 10 -height 10 \
        -borderwidth 1 -relief raised
    place $win.grip -relx 0.5 -rely 0.95 -anchor c

    bind $win.grip <ButtonPress-1>   "panedwindow_grab $win"
    bind $win.grip <B1-Motion>       "panedwindow_drag $win %X"
    bind $win.grip <ButtonRelease-1> "panedwindow_drop $win %X"

    return $win
}

proc panedwindow_grab {win} {
    $win.grip configure -relief sunken
}

proc panedwindow_drag {win x} {
    set realX [expr $x-[winfo rootx $win]]
    set Xmax  [winfo width $win]
    set frac [expr double($realX)/$Xmax]
    if {$frac < 0.10} {
        set frac 0.10
    }
    if {$frac > 0.90} {
        set frac 0.90
    }
    place $win.sash -relx $frac
    place $win.grip -relx $frac

    return $frac
}

proc panedwindow_drop {win x} {
    set frac [panedwindow_drag $win $x]
    panedwindow_divide $win $frac
    $win.grip configure -relief raised
}

proc panedwindow_divide {win frac} {
    place $win.sash -relx $frac
    place $win.grip -relx $frac

    place $win.pane1 -relwidth $frac
    place $win.pane2 -relwidth [expr 1-$frac]
}

proc panedwindow_pane {win num} {
    return $win.pane$num
}

# ----------------------------------------------------------------------
# DIRSELECT LIBRARY
# This library allows the user to select a destination directory
# either by entering the name or browsing via the standard dialogue
# ----------------------------------------------------------------------
option add *Dirselect*Entry.background white widgetDefault

proc dirselect_create {win {label "Directory:"}} {
    frame $win -class Dirselect

    frame $win.entry  
    label $win.entry.label -text $label
    entry $win.entry.value
    button $win.entry.browse -text "Browse..." -command "dirselect_browse $win" 
    grid $win.entry.label -row 0 -column 0 
    grid $win.entry.value -row 0 -column 1 -sticky ew
    grid $win.entry.browse -row 0 -column 2

    grid columnconfigure $win.entry 0 -weight 0
    grid columnconfigure $win.entry 1 -weight 2
    grid columnconfigure $win.entry 2 -weight 0

    pack $win.entry -side bottom -fill x -pady 4

    bind $win.entry.value <KeyPress-Return> "dirselect_type_finalize $win"

    return $win
}

proc dirselect_browse {win} {
    
    set initial [dirselect_get $win]
    if {$initial == ""} { 
	set initial [pwd]
    }

    set dir [tk_chooseDirectory -parent $win \
		 -title "Choose Install Directory"\
		 -initialdir $initial]

    # put the dir name in the entry box
    $win.entry.value delete 0 end
    $win.entry.value insert 0 $dir    
}


proc dirselect_type_finalize {win} {
    $win.entry.value selection clear
    $win.entry.value icursor end
}

proc dirselect_get {win} {
    return [$win.entry.value get]
}
    
proc dirselect_set {win dir} {

    ## what should this do with the new directory chooser?
    ## set the default location in the dirselect widget?
    $win.entry.value delete 0 end
    $win.entry.value insert 0 $dir
  return 1
}

## what's the difference between this and dirselect_set?
proc dirselect_dir {win dir} {
    dirselect_set $win $dir
}



# ----------------------------------------------------------------------
# BROWSE WINDOW
# ----------------------------------------------------------------------
toplevel .browse
switch $install(style) {
  web {
    wm title .browse "$customize(title): Web Install"
  }
  cd {
    wm title .browse "$customize(title): CD Install"
  }
  default {
    wm title .browse $customize(title)
  }
}
wm group .browse .
wm protocol .browse WM_DELETE_WINDOW {
    .browse.cntls.back invoke
}
wm withdraw .browse
frame .browse.cntls
pack .browse.cntls -side bottom -fill x -padx 4 -pady 4

button .browse.cntls.back -text "< Back" -width 10 -command {
    wm withdraw .browse
    set x [expr ([winfo screenwidth .]-[winfo reqwidth .]) / 2]
    set y [expr ([winfo screenheight .]-[winfo reqheight .]) / 2]
    wm geometry . "+$x+$y"
    wm deiconify .
    raise .
}
pack .browse.cntls.back -side left -expand yes

button .browse.cntls.install -text "Install..." -width 10 -command {
    .logo.install invoke
}
pack .browse.cntls.install -side left -expand yes

#button .browse.cntls.manage -text "Manage..." -width 10 -command {
#    .logo.install invoke
#}
#pack .browse.cntls.manage -side left -expand yes

panedwindow_create .browse.info [expr "600 + ( [winfo screenwidth .] -600 ) / 3 "] [expr "400 + ( [winfo screenheight .] -400 ) /8"]
#panedwindow_create .browse.info 600 400
pack .browse.info -expand yes -fill both
panedwindow_divide .browse.info 0.35

set win [panedwindow_pane .browse.info 2]
topicviewer_create $win.topic
pack $win.topic -expand yes -fill both -padx 6 -pady 6
set components(browseTopic) $win.topic

set win [panedwindow_pane .browse.info 1]
frame $win.pad
pack $win.pad -expand yes -fill both -padx 6 -pady 6

label $win.pad.title -text "Packages available:"
pack $win.pad.title -anchor w

divlist_create $win.pad.packages [list pkg_show %d $components(browseTopic)]
pack $win.pad.packages -expand yes -fill both
set components(browseList) $win.pad.packages

# ----------------------------------------------------------------------
# INSTALL WINDOW
# ----------------------------------------------------------------------
toplevel .install
wm title .install $customize(title)
wm group .install .
wm protocol .install WM_DELETE_WINDOW {
    .install.cntls.cancel invoke
}
wm withdraw .install
frame .install.cntls
pack .install.cntls -side bottom -fill x -padx 4 -pady 4

button .install.cntls.back -text "< Back" -width 10 -command install_back
pack .install.cntls.back -side left -expand yes
set components(installBack) .install.cntls.back

button .install.cntls.next -text "Next >" -width 10 -command install_next
pack .install.cntls.next -side left -expand yes
set components(installNext) .install.cntls.next

button .install.cntls.cancel -text "Cancel" -width 10 -command {
    wm withdraw .install
    set x [expr ([winfo screenwidth .]-[winfo reqwidth .]) / 2]
    set y [expr ([winfo screenheight .]-[winfo reqheight .]) / 2]
    wm geometry . "+$x+$y"
    wm deiconify .
    raise .
}
pack .install.cntls.cancel -side left -expand yes
set components(installCancel) .install.cntls.cancel

notebook_create .install.nb
pack .install.nb -expand yes -fill both
set components(installNotebook) .install.nb

# ----------------------------------------------------------------------
# INSTALL OPTIONS
# ----------------------------------------------------------------------
set page [notebook_page $components(installNotebook) "options"]
set components(optionpage) $page
frame $page.float
pack $page.float -expand yes -padx 10 -pady 10

frame $page.float.optionframe
pack $page.float.optionframe -side right -expand y -fill both

label $page.float.icon -image $customize(installScreen)
pack $page.float.icon -side left -padx 10

label $page.float.optionframe.title -text "Installation Options" -font headingFont
pack $page.float.optionframe.title -anchor w -pady 8

# radiobuttons will be made after we know all the choices needed...

# ----------------------------------------------------------------------
# PACKAGE LIST
# ----------------------------------------------------------------------
set page [notebook_page $components(installNotebook) "packages"]
#panedwindow_create $page.info 600 400
panedwindow_create $page.info [expr "600 + ( [winfo screenwidth .] -600 ) / 3 "] [expr "400 + ( [winfo screenheight .] -400 ) /8"]
pack $page.info -expand yes -fill both
panedwindow_divide $page.info 0.35

set win [panedwindow_pane $page.info 2]
topicviewer_create $win.topic
pack $win.topic -expand yes -fill both -padx 6 -pady 6
set components(packagesTopic) $win.topic

set win [panedwindow_pane $page.info 1]
frame $win.pad
pack $win.pad -expand yes -fill both -padx 6 -pady 6

label $win.pad.title -text "Select packages to install:"
pack $win.pad.title -anchor w

divlist_create $win.pad.packages [list pkg_show %d $components(packagesTopic)]
pack $win.pad.packages -expand yes -fill both
set components(packagesList) $win.pad.packages

# ----------------------------------------------------------------------
# DEPENDENCIES
# ----------------------------------------------------------------------
set page [notebook_page $components(installNotebook) "dependencies"]
frame $page.float
pack $page.float -expand yes -padx 10 -pady 10

label $page.float.title -text "Package Dependencies" -font headingFont
pack $page.float.title -anchor w -pady 8

label $page.float.explain -font explainFont -justify left -wraplength 400
pack $page.float.explain -anchor w
set components(dependencyExplain) $page.float.explain

frame $page.float.list
pack $page.float.list -expand yes -fill both
listbox $page.float.list.box -height 5 -selectmode single \
    -yscrollcommand "$page.float.list.sbar set"
pack $page.float.list.box -side left -expand yes -fill both
scrollbar $page.float.list.sbar -orient vertical \
    -command "$page.float.list.box yview"
pack $page.float.list.sbar -side right -fill y
set components(dependencyList) $page.float.list.box

radiobutton $page.float.include -font explainFont -justify left \
    -text "Install these packages as well." \
    -variable install(dependencies) -value "include"
pack $page.float.include -anchor w -pady 4
set components(dependencyDefault) $page.float.include

radiobutton $page.float.exclude -font explainFont -justify left \
    -text "Skip these packages.  I know what I'm doing." \
    -variable install(dependencies) -value "exclude"
pack $page.float.exclude -anchor w -pady 4

# ----------------------------------------------------------------------
# INSTALLATION DIRECTORY(s)
# ----------------------------------------------------------------------
set page [notebook_page $components(installNotebook) "target"]
frame $page.float
pack $page.float -expand yes -fill both -padx 50 -pady 50

label $page.float.title -text "Installation Directory" -font headingFont
pack $page.float.title -pady 8
set components(targetTitle) $page.float.title

label $page.float.explain -font explainFont -justify left
pack $page.float.explain -anchor w
set components(targetExplain) $page.float.explain

dirselect_create $page.float.dirs "Install Here:"
dirselect_dir $page.float.dirs "/"
pack $page.float.dirs -expand yes -fill both
set components(targetLocation) $page.float.dirs

# ----------------------------------------------------------------------
# Generic Form
# ----------------------------------------------------------------------

set page [notebook_page $components(installNotebook) "form"]
frame $page.float
pack $page.float -expand yes -fill both -padx 50 -pady 50

label $page.float.title -text "Install Data" -font headingFont
pack $page.float.title -pady 8
set components(formTitle) $page.float.title

label $page.float.explain -font explainFont -justify left
pack $page.float.explain -anchor w
set components(formExplain) $page.float.explain

frame $page.float.formframe
pack $page.float.formframe -expand y -fill y
set components(form-frame) $page.float.formframe

for {set x 0} {$x < 10} {incr x} {
  label $components(form-frame).label-$x -justify left -text "foo"
  entry $components(form-frame).entry-$x -width 40
  button $components(form-frame).browse-$x -text "Browse..." \
           -command [list form_browse $x]
  set components(form-label-$x) $components(form-frame).label-$x
  set components(form-entry-$x) $components(form-frame).entry-$x
  set components(form-browse-$x) $components(form-frame).browse-$x
}
set form(numshown) 0

proc form_browse {index} {
  global components
  if {"" != \
      [set file [tk_getOpenFile -parent $components(installNotebook) \
                         -title [$components(form-label-$index) cget -text]]]} {
    $components(form-entry-$index) delete 0 end
    $components(form-entry-$index) insert 0 $file
  }
}

# ----------------------------------------------------------------------
# LOCATE TAR FILES
# ----------------------------------------------------------------------
set page [notebook_page $components(installNotebook) "tarfiles"]
frame $page.float
pack $page.float -expand yes -fill both -padx 50 -pady 50

label $page.float.explain -font explainFont -wraplength 500 -text "Enter the location of the \"packages\" directory on the CD-ROM."
pack $page.float.explain -anchor w

dirselect_create $page.float.dirs
dirselect_dir $page.float.dirs "/"
pack $page.float.dirs -expand yes -fill both
set components(tarfilesLocation) $page.float.dirs

# ----------------------------------------------------------------------
# INSTALL SCREEN
# ----------------------------------------------------------------------
set page [notebook_page $components(installNotebook) "install"]
label $page.title -text "Ready to Install" -font headingFont
grid $page.title -row 0 -column 0 -pady 4

label $page.icon -image installIcon
grid $page.icon -row 1 -column 0

label $page.explain -font explainFont \
    -text "Click on \"Install\" to finish the installation"
grid $page.explain -row 2 -column 0 -pady 4

# ----------------------------------------------------------------------
# PROGRESS SCREEN
# ----------------------------------------------------------------------
set page [notebook_page $components(installNotebook) "progress"]
label $page.title -text "Installing..." -font headingFont
grid $page.title -row 0 -column 0 -pady 4
set components(progressTitle) $page.title

frame $page.package
grid $page.package -row 1 -column 0
label $page.package.title -text "Package:"
pack $page.package.title -side left
label $page.package.name -font explainFont
pack $page.package.name -side left
set components(progressPackage) $page.package

installer::gauge_create $page.pgauge "counter"
grid $page.pgauge -row 2 -column 0
set components(progressPackageGauge) $page.pgauge

grid rowconfigure $page 3 -minsize 12

frame $page.task
grid $page.task -row 4 -column 0 -sticky ew
label $page.task.name
pack $page.task.name -side left
set components(progressTaskName) $page.task.name
label $page.task.mesg -font explainFont -width 10 -anchor w
pack $page.task.mesg -side left -expand yes -fill both
set components(progressTaskMessage) $page.task.mesg

installer::gauge_create $page.gauge "percent"
grid $page.gauge -row 5 -column 0
set components(progressInstallGauge) $page.gauge

# ----------------------------------------------------------------------
# FAILED SCREEN
# ----------------------------------------------------------------------
set page [notebook_page $components(installNotebook) "failed"]
label $page.finished -text "Installation Failed" -font headingFont
grid $page.finished -row 0 -column 0 -pady 4

label $page.explain -font explainFont
grid $page.explain -row 1 -column 0 -pady 4
set components(failedExplain) $page.explain

# ----------------------------------------------------------------------
# FINAL SCREEN
# ----------------------------------------------------------------------
set page [notebook_page $components(installNotebook) "final"]
label $page.finished -text "Installation Complete" -font headingFont
grid $page.finished -row 0 -column 0 -pady 4

label $page.icon -image installedIcon
grid $page.icon -row 1 -column 0

label $page.explain -font explainFont
grid $page.explain -row 2 -column 0 -pady 4
set components(finishedExplain) $page.explain

# ----------------------------------------------------------------------
# ABOUT dialog
# ----------------------------------------------------------------------
toplevel .about -class About
wm title .about "About $customize(title)"
wm group .about .
wm transient .about .
wm protocol .about WM_DELETE_WINDOW {
    .about.ok invoke
}
wm withdraw .about
bind .about <KeyPress-Return> {.about.ok invoke}

frame .about.tclish
pack .about.tclish -padx 8 -pady 8

frame .about.tclish.info -borderwidth 20 -relief flat
place .about.tclish.info -relx 0.5 -rely 0 -anchor n

label .about.tclish.info.custom \
    -text "This Tclish installer was customized for:" \
    -font "helvetica 10 normal"
pack .about.tclish.info.custom

label .about.tclish.info.app -text $customize(title) -font "helvetica 18 bold"
pack .about.tclish.info.app

label .about.tclish.info.brought -text "brought to you by" \
    -font "helvetica 8 normal"
pack .about.tclish.info.brought

label .about.tclish.info.org -text $customize(creator) \
    -font "helvetica 12 italic"
pack .about.tclish.info.org

label .about.tclish.info.credits -text $customize(credits) \
    -font "helvetica 8 normal"
pack .about.tclish.info.credits -pady 4

label .about.tclish.icon -image tclish-icon
place .about.tclish.icon -relx 1 -rely 1 -anchor se

button .about.tclish.show -text "What is Tclish?" -command {
    wm withdraw .about
    set x [expr ([winfo screenwidth .tclish]-[winfo reqwidth .tclish]) / 2]
    set y [expr ([winfo screenheight .tclish]-[winfo reqheight .tclish]) / 2]
    wm geometry .tclish "+$x+$y"
    wm deiconify .tclish
} -font "helvetica 10 normal"
place .about.tclish.show -relx 0 -rely 1 -anchor w

update idletasks
set imh [.about.tclish.icon cget -image]
set h [expr [image height $imh]-10+[winfo reqheight .about.tclish.info]]
set w1 [expr [image width $imh]-20+[winfo reqwidth .about.tclish.show]]
set w2 [winfo reqwidth .about.tclish.info]
set w [expr {($w1 > $w2) ? $w1 : $w2}]

.about.tclish configure -width $w -height $h
place .about.tclish.show -x [expr ($w-$w1)/2] -y [expr -[image height $imh]/2]
place .about.tclish.icon -x [expr -($w-$w1)/2]

button .about.ok -text "OK" -default "active" -command {wm withdraw .about}
pack .about.ok -side bottom -padx 4 -pady 8

# ----------------------------------------------------------------------
# ABOUT TCLISH dialog
# ----------------------------------------------------------------------
toplevel .tclish -class About
wm title .tclish "About Tclish"
wm group .tclish .
wm transient .tclish .
wm protocol .tclish WM_DELETE_WINDOW {
    .tclish.ok invoke
}
wm withdraw .tclish
bind .tclish <KeyPress-Return> {.tclish.ok invoke}

label .tclish.title -image tclish-title-img
pack .tclish.title -padx 4 -pady 4

label .tclish.authors -font "helvetica 12 normal" \
    -text "by Michael McLennan (mmc@cadence.com),
Mark Harrison (markh@usai.asiainfo.com),
and Kris Raney (kris@kraney.com)"
pack .tclish.authors -pady 4

label .tclish.copyright -font "helvetica 8 normal" \
    -text "Copyright \251 1998  The Tcl/Tk Consortium"
pack .tclish.copyright

frame .tclish.explain -borderwidth 10 -relief flat
pack .tclish.explain -padx 8 -pady 8
label .tclish.explain.text -wraplength 250 \
    -font "helvetica 10 normal" \
    -text "Tclish is a general-purpose installation tool for UNIX-based systems.  It was built with Tcl/Tk, borrowing heavily from the material in the book \"Effective Tcl/Tk Programming\" written by Mark Harrison and Michael McLennan, Addison-Wesley, 1997.\n\nYou can customize Tclish and bundle it with your own CD-ROM projects.  For more information, see the Tclish web site on SourceForge:\n\nhttp://tclish.sourceforge.net/"
pack .tclish.explain.text -side right -expand yes -fill both -padx 2 -pady 2

label .tclish.explain.icon -image powered
pack .tclish.explain.icon -expand yes -padx 2 -pady 2

label .tclish.explain.icon2 -image eff-sm
pack .tclish.explain.icon2 -expand yes -padx 2 -pady 2

button .tclish.ok -text "OK" -default "active" -command {wm withdraw .tclish}
pack .tclish.ok -side bottom -padx 4 -pady 8

# ----------------------------------------------------------------------
set pagelist ""
proc pagelist_enqueue {index page} {
  global pagelist

  set pagelist [lrange $pagelist 0 $index]
  lappend pagelist $page
}
proc install_reset {} {
  global install components pageindex

  set install(buttonStack) ""
  set pageindex 0
  pagelist_enqueue $pageindex options
  $components(installNext) configure -state normal
  $components(installBack) configure -state normal
  $components(installCancel) configure -state normal
  install_display
}

proc install_next {} {
  global install components pageindex pagelist

  set page [notebook_current $components(installNotebook)]
  set button [$components(installNext) cget -text]

  focus $components(installNext)

  set cmd "install_next_$page"
  if {[info commands $cmd] != ""} {
    if {"fail" != [$cmd $pageindex]} {
        incr pageindex
        while {[info exists install(parameter-$pageindex-default)] \
               && "skip" == [lindex $install(parameter-$pageindex-default) 0]} {
          incr pageindex
        }
        set install(buttonStack) [linsert $install(buttonStack) 0 $button]
        install_display
    }
  }
}

proc install_back {} {
  global install components pagelist pageindex

  incr pageindex -1

  set button [lindex $install(buttonStack) 0]
  set install(buttonStack) [lrange $install(buttonStack) 1 end]

  if {-1 == $pageindex} {
      $components(installCancel) invoke
      return
  }

  while {"skip" == [lindex $pagelist $pageindex]} {
    incr pageindex -1
  }
  install_display
}

proc install_display {} {
  global pageindex pagelist components

  set page [lindex $pagelist $pageindex]
  while {[string compare [set button [install_display_$page $pageindex]] \
                         "skip"] == 0} {
    incr pageindex
    set page [lindex $pagelist $pageindex]
  }

  if {[string compare $button ""] == 0} {
    $components(installNext) configure -text "Next >"
  } else {
    $components(installNext) configure -text $button
  }
}

proc install_display_options {index} {
  global components

  notebook_display $components(installNotebook) "options"
  return
}

proc install_display_skip {index} {
  return skip
}

proc install_display_options {index} {
  global components

  notebook_display $components(installNotebook) "options"
  return
}

proc install_next_options {index} {
  global install packages components pagelist installoptions installoption divInfo

  # if custom install and "normal" install is specified, check only those
  if {$install(option) == "custom"} {
    divlist_check_all $components(packagesList)
    pagelist_enqueue $index packages
    return
  }
  if {$install(option) == "full"} {
    divlist_check_all $components(packagesList)
  } else {
    divlist_check_all $components(packagesList) 0
    foreach x $installoption($install(option)-packages) {
      foreach {category pkg} $x {}
      upvar #0 $packages(pkg-$pkg) pkgInfo
      upvar #0 $divInfo($components(packagesList)-data) data
      divlist_toggle $components(packagesList) \
          $category $pkgInfo(title) check-$data(tag-$category-$pkgInfo(title))
    }
  }

  set install(packages) [divlist_get_checked $components(packagesList)]
  set num [llength $install(packages)]
  if {$num == 1} {
      set install(packageNoun) "package"
      set install(packageVerb) "is"
  } else {
      set install(packageNoun) "packages"
      set install(packageVerb) "are"
  }

  install_check_dependencies $index
}

proc install_display_packages {index} {
  global components

  notebook_display $components(installNotebook) "packages"
  return
}

proc install_next_packages {index} {
  global install components

  set install(packages) [divlist_get_checked $components(packagesList)]
  set num [llength $install(packages)]
  if {$num == 0} {
      tk_messageBox -title "Tclish: Error" -icon error \
          -message "Select one or more packages to install by clicking on the checkbox next to the package name."
      return "fail"
  } elseif {$num == 1} {
      set install(packageNoun) "package"
      set install(packageVerb) "is"
  } else {
      set install(packageNoun) "packages"
      set install(packageVerb) "are"
  }

  install_check_dependencies $index
}

proc install_check_dependencies {index} {
  global install components

  foreach name $install(packages) {
      foreach base [pkg_requires $name] {
          if {[lsearch $install(packages) $base] < 0} {
              set needed($base) 1
          }
      }
  }

  if {[array size needed] > 0} {
      $components(dependencyList) delete 0 end
      eval $components(dependencyList) insert 0 [lsort [array names needed]]
      pagelist_enqueue $index dependencies
  } else {
    install_setup_parameter_queries $index
  }
}

proc install_display_dependencies {index} {
  global install components

  notebook_display $components(installNotebook) "dependencies"
  $components(dependencyExplain) configure -text "To use the $install(packageNoun) that you've selected, the following packages must also be installed:"
  $components(dependencyDefault) invoke
  return
}

proc install_next_dependencies {index} {
  install_setup_parameter_queries $index
}

proc install_setup_parameter_queries {index} {
  global packages install components packageParser tcl_platform parameter

  # reset package list, in case we've backed up
  set install(packages) [divlist_get_checked $components(packagesList)]

  if {$install(dependencies) == "include"} {
    set install(packages) "[$components(dependencyList) get 0 end] $install(packages)"
  }

  set params {}
  set currindex [expr $index + 1]
  foreach name $install(packages) {
    upvar #0 $packages(pkg-$name) pkgInfo
    foreach {type title message paramname default} $pkgInfo(parameters) {
      set install(parameter-$currindex-type) $type
      set install(parameter-$currindex-title) $title
      set install(parameter-$currindex-message) $message
      set install(parameter-$currindex-name) $paramname
      set install(parameter-$currindex-default) $default
      if {[set i [lsearch -exact $params $paramname]] != -1} {
        set previndex [lindex $params [expr $i + 1]]
        # promote sourcedirs to targets if any occurences need to be writable
        if {[string compare directory $type] == 0 \
            && [string compare $install(parameter-$previndex-type) "sourcedir"] == 0} {
          set install(parameter-$previndex-type) directory
        }
        continue
      }
      lappend params $paramname $currindex
      set parameter($install(parameter-$currindex-name)) \
                          [parameter_check_default $currindex]
      switch $type {
        directory {
          pagelist_enqueue [expr $currindex -1] target
        }
        sourcedir {
          pagelist_enqueue [expr $currindex -1] target
        }
        default {
          pagelist_enqueue [expr $currindex -1] form
        }
      }
      incr currindex
    }
  }

  if {$install(style) == "web"} {
    set install(parameter-$currindex-type) directory
    set install(parameter-$currindex-title) {Temporary Directory}
    set install(parameter-$currindex-message) \
        {Select a temporary location for downloaded files}
    set install(parameter-$currindex-name) tclish-package-dir
    lappend params tclish-package-dir $currindex
    set install(parameter-$currindex-default) $parameter(tclish-package-dir)
    pagelist_enqueue [expr $currindex -1] target
    incr currindex
  }
  install_check_tarfiles [expr $currindex - 1]
}

proc parameter_check_default {currindex} {
  global packageParser install parameter

  if {[string compare "" \
       [$packageParser eval \
            [list info commands $install(parameter-$currindex-default)]]] \
      != 0} {
    $packageParser eval {array unset parameter}
    $packageParser eval [list array set parameter [array get parameter]]
    return [$packageParser eval $install(parameter-$currindex-default)]
  }
  return $install(parameter-$currindex-default)
}

proc install_display_form {index} {
  global install components parameter pagelist form

  notebook_display $components(installNotebook) "form"
  $components(formTitle) configure -text $install(parameter-$index-title)
  $components(formExplain) configure -text $install(parameter-$index-message)
  for {set x 0} {$x < 10} {incr x} {
    $components(form-entry-$x) delete 0 end
  }

  set default $parameter($install(parameter-$index-name))
  if {[string compare [lindex $default 0] "-force"] == 0} {
    set default [lrange $default 1 end]
    set i 0
    foreach x $install(parameter-$index-name) {
      if {"-file" == [lindex $x 0] \
          || "-passwd" == [lindex $x 0]} {
        set parameter([lrange $x 1 end]) [lindex $default $i]
      } elseif {"-option" == [lindex $x 0]} {
        set parameter([lindex $x 1]) [lindex $default $i]
      } else {
        set parameter($x) [lindex $default $i]
      }
      incr i
    }
    set pagelist [lreplace $pagelist $index $index skip]
    return skip
  }
  set i 0
  foreach x $install(parameter-$index-name) {
    if {"-file" == [lindex $x 0] \
        || "-passwd" == [lindex $x 0]} {
      set x [lrange $x 1 end]
    } elseif {"-option" == [lindex $x 0]} {
      set choices [lrange $x 2 end]
      set x [lindex $x 1]
    }
    if {[info exists parameter($x)]} {
      set default [lreplace $default $i $i $parameter($x)]
    }
    incr i
  }
  set numtoshow [llength $install(parameter-$index-name)]

# set up the blanks on the form that are already showing, that still need to be
# shown
  for {set x 0} {$x < $form(numshown) && $x < $numtoshow} {incr x} {
    if {[info exists components(form-option-$x)]} {
      destroy $components(form-option-$x)
      unset components(form-option-$x)
    }
    grid forget $components(form-entry-$x)
    grid forget $components(form-browse-$x)
    set title [lindex $install(parameter-$index-name) $x]
    if {"-file" == [lindex $title 0]} {
      $components(form-label-$x) configure -text [lrange $title 1 end]:
      $components(form-entry-$x) configure -show ""
      grid configure $components(form-entry-$x) -column 1 -row $x
      grid configure $components(form-browse-$x) -column 3 -row $x
    } elseif {"-passwd" == [lindex $title 0]} {
      $components(form-label-$x) configure -text [lrange $title 1 end]:
      $components(form-entry-$x) configure -show "*"
      grid configure $components(form-entry-$x) -column 1 -row $x
    } elseif {"-option" == [lindex $title 0]} {
      $components(form-label-$x) configure -text [lindex $title 1]:
      eval tk_optionMenu $components(form-frame).menu-$x \
          install(option-select-$x) $choices
      set components(form-option-$x) $components(form-frame).menu-$x
      grid configure $components(form-option-$x) -column 1 -row $x
    } else {
      $components(form-label-$x) configure -text $title:
      $components(form-entry-$x) configure -show ""
      grid configure $components(form-entry-$x) -column 1 -row $x
    }
    $components(form-entry-$x) insert 0 [lindex $default $x]
  }

# set up the blanks on the form that are not showing, that need to be
# shown
  for {set x $form(numshown)} {$x < $numtoshow} {incr x} {
    set title [lindex $install(parameter-$index-name) $x]
    if {[info exists components(form-option-$x)]} {
      destroy $components(form-option-$x)
      unset components(form-option-$x)
    }
    grid forget $components(form-entry-$x)
    grid forget $components(form-browse-$x)
    if {"-file" == [lindex $title 0]} {
      $components(form-label-$x) configure -text [lrange $title 1 end]:
      $components(form-entry-$x) configure -show ""
      grid configure $components(form-browse-$x) -column 3 -row $x
    } elseif {"-passwd" == [lindex $title 0]} {
      $components(form-label-$x) configure -text [lrange $title 1 end]:
      $components(form-entry-$x) configure -show "*"
      grid configure $components(form-entry-$x) -column 1 -row $x
    } elseif {"-option" == [lindex $title 0]} {
      $components(form-label-$x) configure -text [lindex $title 1]:
      eval tk_optionMenu $components(form-frame).menu-$x \
          install(option-select-$x) $choices
      set components(form-option-$x) $components(form-frame).menu-$x
      grid configure $components(form-option-$x) -column 1 -row $x
    } else {
      $components(form-label-$x) configure -text $title:
      $components(form-entry-$x) configure -show ""
      grid configure $components(form-entry-$x) -column 1 -row $x
    }
    $components(form-entry-$x) insert 0 [lindex $default $x]
    grid configure $components(form-label-$x) -column 0 -row $x
  }

# set up the blanks on the form that are showing, that don't need to be
# shown
  for {set x $numtoshow} {$x < $form(numshown)} {incr x} {
    grid forget $components(form-label-$x) \
                $components(form-entry-$x) \
                $components(form-browse-$x)
    if {[info exists components(form-option-$x)]} {
      destroy $components(form-option-$x)
      unset components(form-option-$x)
    }
  }

  set form(numshown) $numtoshow

  return
}

proc install_next_form {index} {
  global parameter install components form packageParser

  set allparams {}
  for {set x 0} {$x < $form(numshown)} {incr x} {
    set paramname [lindex $install(parameter-$index-name) $x]
    if {"-file" == [lindex $paramname 0] \
        || "-passwd" == [lindex $paramname 0]} {
      set paramname [lrange $paramname 1 end]
      set parameter($paramname) [$components(form-entry-$x) get]
    } elseif {"-option" == [lindex $paramname 0]} {
      set paramname [lindex $paramname 1]
      set parameter($paramname) $install(option-select-$x)
    } else {
      set parameter($paramname) [$components(form-entry-$x) get]
    }
    lappend allparams $parameter($paramname)
  }
  if {$install(parameter-$index-type) != "form"} {
    catch {set ret [$packageParser eval $install(parameter-$index-type) $allparams]} ret
    if {"" != $ret} {
      tk_messageBox -title "Tclish: Error" -icon error \
          -message "Error in form data: $ret"
      return "fail"
    }
  }
  return
}

proc install_display_target {index} {
  global install components parameter pagelist

  notebook_display $components(installNotebook) "target"
  set default $parameter($install(parameter-$index-name))
  if {[string compare [lindex $default 0] "-force"] == 0} {
    set parameter($install(parameter-$index-name)) [lrange $default 1 end]
    set pagelist [lreplace $pagelist $index $index skip]
    return skip
  }
  dirselect_set $components(targetLocation) $default
  $components(targetTitle) configure -text $install(parameter-$index-title)
  $components(targetExplain) configure -text $install(parameter-$index-message)

  return
}

proc install_next_target {index} {
  global install components parameter

  set parameter($install(parameter-$index-name)) \
             [dirselect_get $components(targetLocation)]
  if {"" == $parameter($install(parameter-$index-name))} {
    tk_messageBox -title "Tclish: Error" -icon error -message "Select the directory."
    return "fail"
  }
  if {"sourcedir" == $install(parameter-$index-type)} {
    if {![file isdirectory $parameter($install(parameter-$index-name))]} {
        tk_messageBox -title "Tclish: Error" -icon error -message "Directory \"$parameter($install(parameter-$index-name))\" does not exist."
        return "fail"
    }
  } else {
    if {![file isdirectory $parameter($install(parameter-$index-name))]} {
      set result [tk_messageBox -title "Tclish: Query" -icon question \
             -type yesno \
             -message "Directory \"$parameter($install(parameter-$index-name))\" does not exist.  Create directory?"]
      if {$result == "no"} {
        return "fail"
      }
      if {[catch {file mkdir $parameter($install(parameter-$index-name))} err]} {
        tk_messageBox -title "Tclish: Error" -icon error -message "Unable to create directory $parameter($install(parameter-$index-name))."
        return "fail"
      }
    } elseif {![file writable $parameter($install(parameter-$index-name))]} {
      tk_messageBox -title "Tclish: Error" -icon error -message "Can't modify directory \"$parameter($install(parameter-$index-name))\""
      return "fail"
    }
  }
}

proc install_display_tarfiles {index} {
  global install components parameter

  notebook_display $components(installNotebook) "tarfiles"
  dirselect_set $components(tarfilesLocation) $parameter(tclish-package-dir)
  return
}

proc install_next_tarfiles {index} {
  global install components parameter

  set file [dirselect_get $components(tarfilesLocation)]
  if {![file exists $file]} {
    tk_messageBox -title "Tclish: Error" -icon error \
        -message "Can't find the $install(packageNoun) on the CD-ROM.  Enter the location of the \"packages\" directory on the CD-ROM."
    return "fail"
  }
  set parameter(tclish-package-dir) [file dirname $file]
  install_check_tarfiles
}

proc install_check_tarfiles {index} {
  global install parameter

  if {$install(style) != "cd" \
      || [file isdirectory $parameter(tclish-package-dir)]} {
    pagelist_enqueue $index install
  } else {
    pagelist_enqueue $index tarfiles
  }
}

proc install_display_install {index} {
  global components

  notebook_display $components(installNotebook) "install"
  return Install
}

proc install_next_install {index} {
    global install components

    if {$install(style) == "web"} {
      if {[set ret [install_do_download]] == "fail"} {
        return "fail"
      }
    }
    if {[set ret [install_do_install]] == "fail"} {
      return "fail"
    }

    pagelist_enqueue $index final
}

proc install_do_download {} {
  global install components

  $components(progressTitle) configure -text "Downloading..."
  return [install_do_something download_package]
}

proc install_do_install {} {
  global install components

  $components(progressTitle) configure -text "Installing..."
  return [install_do_something install_package]
}

proc install_do_something {what} {
  global install components

    notebook_display $components(installNotebook) "progress"
    $components(installNext) configure -state disabled
    $components(installBack) configure -state disabled
    $components(installCancel) configure -state disabled

    set count 1
    set max [llength $install(packages)]
    if {$max > 1} {
        grid $components(progressPackage) -sticky w
        grid $components(progressPackageGauge) -row 2 -column 0
    } else {
        grid $components(progressPackage) -sticky ""
        grid forget $components(progressPackageGauge)
    }
    update

    foreach name $install(packages) {
        $components(progressPackage).name configure -text [pkg_title $name]
        installer::gauge_value $components(progressPackageGauge) $count $max

        if {[catch {eval $what [list $name]} result] != 0} {
            if {[llength [split $result "\n"]] > 5} {
                set result [lrange [split $result "\n"] 0 4]
                lappend result "..."
                set result [join $result "\n"]
            }
            notebook_display $components(installNotebook) "failed"
            $components(failedExplain) configure -text "The installation of package \"$name\" failed:\n$result"
            $components(installNext) configure -state normal -text "Exit"
            $components(installBack) configure -state normal
            $components(installCancel) configure -state normal
            return "fail"
        }
        installer::gauge_value $components(progressPackageGauge) [incr count] $max
    }
    $components(installNext) configure -state normal
    $components(installBack) configure -state normal
    $components(installCancel) configure -state normal

    $components(finishedExplain) configure -text "Your $install(packageNoun) $install(packageVerb) now installed and ready to use.\nClick on \"Finish\" to exit this program."
}

proc install_display_final {index} {
  global components

  notebook_display $components(installNotebook) "final"
  $components(installBack) configure -state disabled
  return Finish
}

proc install_next_failed {index} {
    exit 1
}

proc install_next_final {index} {
    exit
}

# ----------------------------------------------------------------------
proc install_untar {dir tarfile {fudge 0}} {
  global install customize tcl_platform env

  install_progress 0 100 "Extracting manifest..." ""

  #
  # Find the tar file for this package and get a list of contents.
  #
  set tarfile [install_packagefile $tarfile $install(currpackage)]
  if {![file exists $tarfile]} {
      error "Distribution file \"$tarfile\" not found"
  }

  set wd [pwd]
  if {[string compare $tcl_platform(platform) "windows"] != 0} {
    if {![file exists $tarfile.manifest]} {
      set out [open "| uncompress | tar tf - > $tarfile.manifest" a]
      fconfigure $out -translation binary
      set in [open $tarfile]
      fconfigure $in -translation binary
      fcopy $in $out
      close $in
      close $out
    }
    set f [open $tarfile.manifest]
    set output [read $f]
    close $f
    set outlist [split $output "\n"]

    set filelist ""
    foreach name $outlist {
        if {![string match */ $name]} {
              set path [string trimleft $name "./"]
              set path [string trimright $path "/"]
              lappend filelist $path
        }
    }
    if {[llength $filelist] == 0} {
        error "can't extract manifest $tarfile.manifest"
    }
    set max [expr [llength $filelist] + [file size $tarfile]/$install(blocksize) + $fudge]

    cd $dir

    set install(progress) 0
    foreach file $filelist {
        install_progress [incr install(progress)] $max \
            "Looking for an existing installation..." ""

        if {[file exists $file] && ![file isdirectory $file]} {
            set dir [file dirname $file]
            set perm [file attributes $dir -permissions]
            file attributes $dir -permissions [expr 0$perm|0200]
            file delete -force $file
        }
    }

    #
    # Unpack the tar file.
    #
    install_progress $install(progress) $max \
        "Uncompressing:" [file tail $tarfile]

    set in [open $tarfile]
    fconfigure $in -translation binary
    set out [open "| uncompress | tar xf -" "a+"]
    fconfigure $out -translation binary
    fcopy $in $out -size $install(blocksize) \
                   -command [list install_package_fcopy $in $out $install(blocksize)]
    vwait install(tarfile)
  } else {
    set install(progress) 1
# windows install, just run the self-extracting exe
    if {$install(style) == "onefile"} {
      install_progress $install(progress) 3 \
          "Writing temp file:" [file tail $tarfile]
      set in [open $tarfile]
      fconfigure $in -translation binary
      if {[info exists env(TEMP)]} {
        set tempfile [file join $env(TEMP) [file tail $tarfile]]
      } else {
        set tempfile [file join C:\ [file tail $tarfile]]
      }
      set out [open $tempfile w]
      fconfigure $out -translation binary
      fcopy $in $out
      close $in
      close $out
    } else {
      set tempfile $tarfile
    }
    incr install(progress)
    install_progress $install(progress) 3 \
        "Extracting temp file:" [file tail $tarfile]
    cd $dir
    eval exec "$tempfile" $customize(winopts)
    incr install(progress)
    if {$install(style) == "onefile"} {
      install_progress $install(progress) 3 \
          "Cleaning up temp file:" [file tail $tarfile]
      file delete $tempfile
    }
  }
  cd $wd
}

proc install_packagefile {filename {pkgname {}}} {
  global install parameter tcl_platform packages packageParser
  if {[string compare $pkgname ""] == 0} {
    set platform [install_platform]
  } else {
    upvar #0 $packages(pkg-$pkgname) pkgInfo

    $packageParser eval {array unset parameter}
    $packageParser eval [list array set parameter [array get parameter]]
    set platform [$packageParser eval $pkgInfo(platformnamecmd)]
  }
  regsub "\\*" $filename $platform filename
  if {[string compare $tcl_platform(platform) "windows"] == 0 } {
    regsub "&" $filename exe filename
  } else {
    regsub "&" $filename tar.Z filename
  }

  return [file join $parameter(tclish-package-dir) $filename]
}

proc install_package {name} {
    global install packages components customize packageParser parameter

    if {![info exists packages(pkg-$name)]} {
        error "package \"$name\" not defined"
    }

    upvar #0 $packages(pkg-$name) pkgInfo

    if {[string compare "" $pkgInfo(installcmd)] != 0} {
      $packageParser eval {array unset parameter}
      $packageParser eval [list array set parameter [array get parameter]]
      set install(currpackage) $name
      $packageParser eval $pkgInfo(installcmd) [list $name]
    }
}

proc install_package_output {fid} {
    global install

    if {[gets $fid line] < 0} {
        catch {close $fid}
        set install(tarfile) "done"
    } else {
        install_progress [incr install(progress)]
    }
}

proc install_package_fcopy {in out chunk bytes {err ""}} {
  global install
  if {([string length $err] != 0) || [eof $in]} {
    set install(tarfile) all_read
    close $in
    close $out
  } else {
    install_progress [incr install(progress)]
    fcopy $in $out -command [list install_package_fcopy $in $out $chunk] \
                   -size $chunk 
  }
}

proc download_package {name} {
  global install packages

  if {![info exists packages(pkg-$name)]} {
      error "package \"$name\" not defined"
  }
  upvar #0 $packages(pkg-$name) pkgInfo

  foreach file $pkgInfo(files) {
    #
    # Find the tar file for this package and get a list of contents.
    #
    set filepath [install_packagefile $file $name]
    installer::bg_download -urls $install(url)/packages/[file tail $filepath] \
                -headers $install(authtok) \
                -destination [file dirname $filepath] \
                -blocksize $install(blocksize) \
                -progress [namespace code download_progress] \
                -block 1
  }
}

#proc download_progress {filename token totalsize current}
proc download_progress {token file filenum totalfiles current totalsize} {
  global install
  install_progress $current $totalsize "Saving file..." [file tail $file]
}

proc install_patch_files {filelist dir} {
  global customize install

  # Scan through all files in the distribution and patch
  # as needed.
  #
  if {[string length $dir] >
      [string length $customize(patchString)]} {
      error "directory name is longer than space allocated in binary files"
  }

#  install_progress $install(progress) $max "Scanning installation..." ""

  foreach file $filelist {
      install_progress [incr install(progress)]

      if {![file isdirectory $file]} {
          install_patch_file $file $dir
      }
  }
}

proc install_patch_file {file dir} {
    global install customize

    if {[catch {set perm [file attributes $file -permissions]} err]} {
        return
    }
    file attributes $file -permissions [expr 0$perm|0200]

    set fid [open $file "r+"]
    fconfigure $fid -translation binary
    set contents [read $fid]

    set binary "auto"
    set patch $customize(patchString)
    set patchLen [string length $patch]
    set replaceLen [string length $dir]

    set start 0
    while {1} {
        set offset [string first $patch [string range $contents $start end]]

        if {$offset < 0} {
            break
        }

        if {$binary == "auto"} {
            install_progress "" "" "Patching:" $file

            set binary 0
            if {[string first "\000" $contents] >= 0 ||
                [regexp "\[^\001-\176\]" $contents]} {
                set binary 1
            }
            if {!$binary} {
                close $fid
                set fid [open $file "w"]
            }
        }

        #
        # If this is a binary file, then seek the appropriate position
        # and write out the patch string.  If this is an ASCII file,
        # write out the information leading up to the patch string,
        # then the replacement string.  Remember, in an ASCII file, we
        # can't just pad with null bytes.
        #
        if {$binary} {
            set endpos [expr $start+$offset+$patchLen]
            if {[string index $contents $endpos] == "\000"} {
                set tail ""
            } else {
                set tail [string range $contents $endpos end]
                set endpos [expr [string first "\000" $tail]-1]
                set tail [string range $tail 0 $endpos]
            }

            seek $fid [expr $start+$offset]
            puts -nonewline $fid $dir
            puts -nonewline $fid $tail

            set pad [expr $patchLen - $replaceLen]

            if {$pad > 0} {
                puts -nonewline $fid [binary format a$pad ""]
            }
        } else {
            set endpos [expr $start+$offset-1]
            puts -nonewline $fid [string range $contents $start $endpos]
            puts -nonewline $fid $dir
        }
        set start [expr $start+$offset+$patchLen]
    }

    if {$binary == "0"} {
        puts -nonewline $fid [string range $contents $start end]
    }
    close $fid
    file attributes $file -permissions $perm
}

proc install_progress {num {max ""} {task "!@#"} {message "!@#"}} {
    global install components

    if {"!@#" != $task} {
        $components(progressTaskName) configure -text $task
        update idletasks
    }
    if {"!@#" != $message} {
        $components(progressTaskMessage) configure -text $message
        update idletasks
    }
    if {"" != $num} {
        installer::gauge_value $components(progressInstallGauge) $num $max
    }
    return $num
}

# ----------------------------------------------------------------------
# Procedures to handle package descriptions...
# ----------------------------------------------------------------------
set packages(count) 0
set packages(current) ""

set packageParser [interp create]
$packageParser alias untar install_untar
$packageParser alias packagefile install_packagefile
$packageParser alias progress install_progress
$packageParser alias patch_files install_patch_files

$packageParser eval {rename package tcl_package}
$packageParser alias package pkg_cmd_package
$packageParser alias title pkg_cmd_title
$packageParser alias version pkg_cmd_version
$packageParser alias requires pkg_cmd_requires
$packageParser alias category pkg_cmd_category
$packageParser alias installcmd pkg_cmd_installcmd
$packageParser alias platformnamecmd pkg_cmd_platformnamecmd
$packageParser alias part pkg_cmd_part
$packageParser alias description pkg_cmd_description
$packageParser alias parameters pkg_cmd_parameters
$packageParser alias files pkg_cmd_files
$packageParser alias exampleTarfile list
$packageParser alias icon pkg_cmd_imagedata
$packageParser alias create_photo create_photo
$packageParser alias platform install_platform
$packageParser alias tk_messageBox tk_messageBox

$packageParser alias installoption installoption_cmd_installoption
$packageParser alias installoption_add installoption_cmd_installoption_add

if {[string compare $tcl_platform(platform) "windows"] == 0 } {
  $packageParser alias registry registry
  $packageParser alias shortcut freewrap::shortcut
  $packageParser alias getSpecialDir freewrap::getSpecialDir
}

set installoptions ""
proc installoption_cmd_installoption {name title} {
  global installoption installoptions

  if {[info exists installoption($name-title)]} {
    error "install option \"$name\" already defined"
  }
  lappend installoptions $name
  set installoption($name-title) $title
  set installoption($name-packages) ""
}

proc installoption_cmd_installoption_add {name category pkg} {
  global installoption packages

  if {![info exists installoption($name-title)]} {
    error "install option \"$name\" not found"
  }
  if {![info exists packages(pkg-$pkg)]} {
    error "package \"$pkg\" not found while defining install option \"$name\"."
  }
  lappend installoption($name-packages) [list $category $pkg]
}

proc pkg_cmd_package {name body} {
    global packageParser packages components

    if {[info exists packages(pkg-$name)]} {
        error "package \"$name\" already defined"
    }
    set vname "package[incr packages(count)]"
    set packages(pkg-$name) $vname
    upvar #0 $vname pkgInfo
    set pkgInfo(name) $name
    set pkgInfo(title) ""
    set pkgInfo(version) ""
    set pkgInfo(requires) ""
    set pkgInfo(category) "Misc"
    set pkgInfo(installcmd) ""
    set pkgInfo(platformnamecmd) "platform"
    set pkgInfo(description) ""
    set pkgInfo(imagedata) ""
    set pkgInfo(parameters) ""
    set pkgInfo(files) ""

    set packages(current) $name
    $packageParser eval $body

    set checkbox [expr {"" != $pkgInfo(installcmd)}]

    divlist_add $components(packagesList) \
        $pkgInfo(category) $pkgInfo(title) $name $checkbox

    divlist_add $components(browseList) \
        $pkgInfo(category) $pkgInfo(title) $name 0
}

proc pkg_show {name win} {
    global packages

    if {![info exists packages(pkg-$name)]} {
        error "package \"$name\" not defined"
    }
    upvar #0 $packages(pkg-$name) pkgInfo

    topicviewer_show $win \
        $pkgInfo(category) $pkgInfo(title) $pkgInfo(description) \
        $pkgInfo(imagedata)
}

proc pkg_title {name} {
    global packages

    if {![info exists packages(pkg-$name)]} {
        error "package \"$name\" not defined"
    }
    upvar #0 $packages(pkg-$name) pkgInfo

    return "$name $pkgInfo(version)"
}

proc pkg_requires {name} {
    global packages

    if {![info exists packages(pkg-$name)]} {
        error "package \"$name\" not defined"
    }
    upvar #0 $packages(pkg-$name) pkgInfo

    return $pkgInfo(requires)
}

foreach field {
    title version requires category installcmd description imagedata
    parameters platformnamecmd
} {
    proc pkg_cmd_$field {val} [format {
        global packages
        set name $packages(current)
        upvar #0 $packages(pkg-$name) pkgInfo
        set pkgInfo(%s) $val
    } $field]
}

proc pkg_cmd_files {file} {
    global packages
    set name $packages(current)
    upvar #0 $packages(pkg-$name) pkgInfo
    lappend pkgInfo(files) $file
}

# ----------------------------------------------------------------------
# MAIN WINDOW
# ----------------------------------------------------------------------
wm title . $customize(title)
wm resizable . 0 0

frame .logo
pack .logo -fill both -expand y

label .logo.icon -background $customize(buttonBackground) \
                 -image $customize(mainScreen) -borderwidth 2 -relief raised
pack .logo.icon -side left -fill both -expand y

frame .logo.bg -background $customize(buttonBackground)
pack .logo.bg -side left -fill y

button .logo.browse -text "Browse..." -width 10 -command {
    wm withdraw .
    set x [expr ([winfo screenwidth .browse]-[winfo reqwidth .browse]) / 2]
    set y [expr ([winfo screenheight .browse]-[winfo reqheight .browse]) / 2]
    wm geometry .browse "+$x+$y"
    wm deiconify .browse
}
place .logo.browse -relx 1 -x -10 -rely 0.2 -anchor e
.logo.browse configure -highlightbackground $customize(buttonBackground)
eval .logo.browse configure  $customize(buttons)

button .logo.install -text "Install..." -width 10 -command {
    install_reset
    wm withdraw .
    wm withdraw .browse
    set x [expr ([winfo screenwidth .install]-[winfo reqwidth .install]) / 2]
    set y [expr ([winfo screenheight .install]-[winfo reqheight .install]) / 2]
    wm geometry .install "+$x+$y"
    wm deiconify .install
    raise .install
}
place .logo.install -relx 1 -x -10 -rely 0.4 -anchor e
.logo.install configure -highlightbackground $customize(buttonBackground)
eval .logo.install configure $customize(buttons)

#button .logo.manage -text "Manage..." -width 10 -command {
#}
#place .logo.manage -relx 1 -x -10 -rely 0.6 -anchor e
#.logo.manage configure -highlightbackground $customize(buttonBackground)
#eval .logo.manage configure $customize(buttons)

button .logo.exit -text "Exit" -width 10 -command {
    exit
}
place .logo.exit -relx 1 -x -10 -rely 0.8 -anchor e
.logo.exit configure -highlightbackground $customize(buttonBackground)
eval .logo.exit configure $customize(buttons)

button .logo.about -text "About..." -width 10 -command {
    set x [expr ([winfo screenwidth .about]-[winfo reqwidth .about]) / 2]
    set y [expr ([winfo screenheight .about]-[winfo reqheight .about]) / 2]
    wm geometry .about "+$x+$y"
    wm deiconify .about
    raise .about
}
place .logo.about -relx 1 -x -10 -rely 1 -y -10 -anchor se
.logo.about configure -highlightbackground $customize(buttonBackground)
eval .logo.about configure $customize(buttons)

after idle {
    update idletasks
    .logo.bg configure -width [winfo reqwidth .logo.about]
    set width 300
    set height 300
    if {[set newwidth [expr [winfo reqwidth .logo.bg] + \
                            [winfo reqwidth .logo.icon]]] > $width} {
      set width $newwidth
    }
    if {[set newheight [winfo reqheight .logo]] > $height} {
      set height $newheight
    }
    wm geometry . [set width]x[set height]
}

# ----------------------------------------------------------------------
# LOAD PACKAGE DESCRIPTIONS
# ----------------------------------------------------------------------
set pkgDescFiles ""
if {$install(style) != "web"} {
# FILESYSTEM-BASED install
  if {$install(style) == "onefile"} {
    set f [open $install(packageprefix)packages.fwp]
    set wrappedfilelist [read $f]
    close $f
    set f [open $install(packageprefix)binpackages.fwp]
    append wrappedfilelist [read $f]
    close $f
    set filelist ""
    foreach file $wrappedfilelist {
      if {[string match *.pkg $file]} {
        lappend filelist $file
      }
    }
  } else {
    set filelist [glob -nocomplain [file join $parameter(tclish-package-dir) *.pkg]]
  }
  foreach file [lsort $filelist] {
      set status [catch {
          set fid [open $file "r"]
          append pkgDescFiles [read $fid] "\n"
          close $fid
      } result]

      if {$status != 0} {
          fatalerror "Can't load package description \"$file\": $result"
      }
  }

} else {
# WEB-BASED INSTALL
  set index [geturldata $install(url)/packages/]
  while {[regexp {<IMG[^>]*> <[^>]*>([^<]*\.pkg)</A>[^\n]*\n(.*)$} $index dummy file index]} {
    append pkgDescFiles [geturldata $install(url)/packages/$file] "\n"
  }
}

if {[catch {$packageParser eval $pkgDescFiles} result] != 0} {
  fatalerror "Error in package \"$packages(current)\":\n$result"
}


# ----------------------------------------------------------------------
# INSTALL OPTIONS CONTINUED
# ----------------------------------------------------------------------

if {[llength $installoptions] == 0} {
  set installoptions full
  set installoption(full-title) {Full installation.
Install everything on the cd-rom.}
}
set i 0
foreach x $installoptions {
  if {"full" == $x} {
    radiobutton $components(optionpage).float.optionframe.option-$i \
      -font explainFont -justify left \
      -text $installoption(full-title) \
      -variable install(option) -value $x
  } else {
    radiobutton $components(optionpage).float.optionframe.option-$i \
      -font explainFont -justify left \
      -text $installoption($x-title) \
      -variable install(option) -value $x
  }
  pack $components(optionpage).float.optionframe.option-$i -anchor w -pady 4
  incr i
}
$components(optionpage).float.optionframe.option-0 invoke

radiobutton $components(optionpage).float.optionframe.custom \
    -font explainFont -justify left \
    -text "Customized installation.\nSelect specific packages to install." \
    -variable install(option) -value "custom"
pack $components(optionpage).float.optionframe.custom -anchor w -pady 4

divlist_check_all $components(packagesList)
update
divlist_select $components(packagesList) 0
divlist_select $components(browseList) 0

bughandler_install "Tclish: install.tcl" bugs@tcltk.com
