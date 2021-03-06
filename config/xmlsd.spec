
%define name		xmlsd
%define version		0.10.1
%define release		1

Name: 		%{name}
Summary: 	Library that wraps expat to simplify XML use in C programs
Version: 	%{version}
Release: 	%{release}
License: 	ISC
Group: 		System Environment/Libraries
URL:		http://opensource.conformal.com/wiki/xmlsd
Source: 	%{name}-%{version}.tar.gz
Buildroot:	%{_tmppath}/%{name}-%{version}-buildroot
Prefix: 	/usr
Requires:	expat >= 2.0.1

%description
xmlsd is a library that wraps expat in order to simplify XML use in C programs
as well as adding rigid rules to an XML structure. The idea is to have XML
structures that can be passed over the network with implicit parameter
verification. This in turn enables back and forth chattering between
applications.

%prep
%setup -q

%build
make

%install
make install DESTDIR=$RPM_BUILD_ROOT LOCALBASE=/usr

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root)
/usr/lib/libxmlsd.so.*


%package devel
Summary: Libraries and header files to develop applications using xmlsd
Group: Development/Libraries
Requires: clens-devel >= 0.0.5, expat-devel >= 2.0.1

%description devel
This package contains the libraries, include files, and documentation to
develop applications with xmlsd.

%files devel
%defattr(-,root,root)
%doc /usr/share/man/man?/*
/usr/include/xmlsd.h
/usr/lib/libxmlsd.so
/usr/lib/libxmlsd.a

%changelog
* Fri May 31 2013 - davec 0.10.1-1
- Fix OpenBSD port Makefile for modern OpenBSD ports
* Fri Jan 04 2013 - davec 0.10.0-1
- Make generated XML include UTF-8 encoding param
- Add new functions to get details on validation info --
  xmlsd_validate_info() and xmlsd_get_validate_failure_string()
- Fix a couple of memory leaks in error paths
- Remove the 'version: ' prefix from xmlsd_verstring
- Other minor code cleanup
* Wed Aug 15 2012 - davec 0.9.0-1
- Properly escape reserved characters in xml attributes and values
- Change xmlsd API to model xml tree instead of a list emulating a tree
- Fix input buffer overflows where xml len was not properly checked
- Add support for Bitrig
* Tue Jul 17 2012 - davec 0.8.1-1
- Support clang builds
- Fix non-release build versioning
* Tue Apr 24 2012 - drahn 0.8.0-1
- Kill unused xml references
- Make thread safe
- Add a threaded test
- Add function prototypes
- Other minor cleanup and bug fixes
* Fri Jan 06 2012 - davec 0.7.0-1
- Add functions to set element values by type which mirror the functions
  that set attributes
* Thu Oct 27 2011 - davec 0.6.0-1
- Implement required attribute and min/max occurrence functionality
- Man page corrections and updates
- Add build versioning
- Other minor cleanup
* Fri Oct 07 2011 - davec 0.5.1-1
- Fix several memory leaks
- Add initial man page
- Other minor cleanup and bug fixes
* Tue Aug 25 2011 - davec 0.5.0-1
- Add set attribute functions for various types to XML generation API
- Modify xmlsd_generate to take two additional parameters for size of
  generated XML and a flag to determine whether an XML declaration is
  prepended
- Change prototypes of all string parameters which aren't modified to const
* Tue Jul 26 2011 - davec 0.4.0-1
- Add new XML code generation APIs
- Don't link against clens directly from library
* Tue Jul 03 2011 - davec 0.3.1-1
- Create
