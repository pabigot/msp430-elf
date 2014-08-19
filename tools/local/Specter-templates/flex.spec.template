Summary: A tool for creating scanners (text pattern recognizers).
Name: flex
Version: 2.5.4a
Release: 29
License: BSD
Group: Development/Tools
Prefix: %{_prefix}
BuildRoot: %{_tmppath}/%{name}-root

Source: ftp://ftp.gnu.org/non-gnu/flex/flex-2.5.4a.tar.gz

Patch0: flex-2.5.4a-skel.patch
Patch1: flex-2.5.4-glibc22.patch
Patch2: flex-2.5.4a-gcc3.patch
Patch3: flex-2.5.4a-gcc31.patch
Patch4: flex-2.5.4a2.patch

BuildPrereq: autoconf

%description
The flex program generates scanners.  Scanners are programs which can
recognize lexical patterns in text.  Flex takes pairs of regular
expressions and C code as input and generates a C source file as
output.  The output file is compiled and linked with a library to
produce an executable.  The executable searches through its input for
occurrences of the regular expressions.  When a match is found, it
executes the corresponding C code.  Flex was designed to work with
both Yacc and Bison, and is used by many programs as part of their
build process.

You should install flex if you are going to use your system for
application development.

%prep
%setup -q -n %{name}-2.5.4
%patch0 -p1
%patch1 -p1 -b .glibc22
%patch2 -p1 -b .glib3
%patch3 -p1 -b .gcc31
%patch4 -p1 -b .yynoinput

%build
autoconf
%configure
make

%install
rm -rf $RPM_BUILD_ROOT

%makeinstall mandir=$RPM_BUILD_ROOT/%{_mandir}/man1

( cd ${RPM_BUILD_ROOT}
  ln -sf flex .%{_bindir}/lex
  ln -s flex.1 .%{_mandir}/man1/lex.1
  ln -s flex.1 .%{_mandir}/man1/flex++.1
  ln -s libfl.a .%{_libdir}/libl.a
)

%clean
rm -rf ${RPM_BUILD_ROOT}

%files
%defattr(-,root,root)
%doc COPYING NEWS README
%{_bindir}/*
%{_mandir}/man1/*
%{_libdir}/*.a
%{_includedir}/FlexLexer.h

%changelog
* Wed Jan 22 2003 Tim Powers <timp@redhat.com>
- rebuilt

* Tue Jan  7 2003 Jeff Johnson <jbj@redhat.com> 2.5.4a-28
- don't include -debuginfo files in package.

* Mon Nov  4 2002 Than Ngo <than@redhat.com> 2.5.4a-27
- YY_NO_INPUT patch from Jean Marie

* Fri Jun 21 2002 Tim Powers <timp@redhat.com>
- automated rebuild

* Tue Jun 18 2002 Than Ngo <than@redhat.com> 2.5.4a-25
- don't forcibly strip binaries

* Thu May 23 2002 Tim Powers <timp@redhat.com>
- automated rebuild

* Tue Apr  2 2002 Than Ngo <than@redhat.com> 2.5.4a-23
- More ISO C++ 98 fixes (#59670)

* Tue Feb 26 2002 Than Ngo <than@redhat.com> 2.5.4a-22
- rebuild in new enviroment

* Wed Feb 20 2002 Bernhard Rosenkraenzer <bero@redhat.com> 2.5.4a-21
- More ISO C++ 98 fixes (#59670)

* Tue Feb 19 2002 Bernhard Rosenkraenzer <bero@redhat.com> 2.5.4a-20
- Fix ISO C++ 98 compliance (#59670)

* Wed Jan 23 2002 Than Ngo <than@redhat.com> 2.5.4a-19
- fixed #58643

* Wed Jan 09 2002 Tim Powers <timp@redhat.com>
- automated rebuild

* Tue Nov  6 2001 Than Ngo <than@redhat.com> 2.5.4a-17
- fixed for working with gcc 3 (bug #55778)

* Sat Oct 13 2001 Than Ngo <than@redhat.com> 2.5.4a-16
- fix wrong License (bug #54574)

* Sun Jun 24 2001 Elliot Lee <sopwith@redhat.com>
- Bump release + rebuild.

* Sat Sep 30 2000 Bernhard Rosenkraenzer <bero@redhat.com>
- Fix generation of broken code (conflicting isatty() prototype w/ glibc 2.2)
  This broke, among other things, the kdelibs 2.0 build
- Fix source URL

* Thu Sep  7 2000 Jeff Johnson <jbj@redhat.com>
- FHS packaging (64bit systems need to use libdir).

* Wed Jul 12 2000 Prospector <bugzilla@redhat.com>
- automatic rebuild

* Tue Jun  6 2000 Bill Nottingham <notting@redhat.com>
- rebuild, FHS stuff.

* Thu Feb  3 2000 Bill Nottingham <notting@redhat.com>
- handle compressed man pages

* Fri Jan 28 2000 Bill Nottingham <notting@redhat.com>
- add a libl.a link to libfl.a

* Wed Aug 25 1999 Jeff Johnson <jbj@redhat.com>
- avoid uninitialized variable warning (Erez Zadok).

* Sun Mar 21 1999 Cristian Gafton <gafton@redhat.com> 
- auto rebuild in the new build environment (release 6)

* Fri Dec 18 1998 Bill Nottingham <notting@redhat.com>
- build for 6.0 tree

* Mon Aug 10 1998 Jeff Johnson <jbj@redhat.com>
- build root

* Mon Apr 27 1998 Prospector System <bugs@redhat.com>
- translations modified for de, fr, tr

* Thu Oct 23 1997 Donnie Barnes <djb@redhat.com>
- updated from 2.5.4 to 2.5.4a

* Mon Jun 02 1997 Erik Troan <ewt@redhat.com>
- built against glibc

* Thu Mar 20 1997 Michael Fulbright <msf@redhat.com>
- Updated to v. 2.5.4
