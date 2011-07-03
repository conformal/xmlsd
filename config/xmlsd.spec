# $xmlsd$

%define name		xmlsd
%define version		0.3.1
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
Requires: clens >= 0.0.5

%description devel
This package contains the libraries, include files, and documentation to
develop applications with xmlsd.

%files devel
%defattr(-,root,root)
/usr/include/xmlsd.h
/usr/lib/libxmlsd.so
/usr/lib/libxmlsd.a

%changelog
* Tue Jul 03 2011 - davec 0.3.1-1
- Create
