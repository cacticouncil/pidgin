Name:		talkatu
Version:	0.1.0
%if "dev" == ""
Release:	1.%{?build_number}%{?!build_number:1}%{?dist}
%else
Release:	0.%{?build_number}%{?!build_number:1}%{?dist}
%endif
License:	GPLv2+
Summary:	A collection of GTK widgets for chat applications
Url:		https://keep.imfreedom.org/talkatu/talkatu
Group:		Development/Libraries

BuildRequires:	meson >= 0.37.0
BuildRequires:	glib2-devel >= 2.20.0
BuildRequires:	gobject-introspection-devel
BuildRequires:	gettext
BuildRequires:	gtk3-devel
BuildRequires:	help2man
BuildRequires:  glade-devel
BuildRequires:  cmark-devel
BuildRequires:  gumbo-parser-devel
BuildRequires:  gspell-devel
BuildRequires:  gtk-doc
BuildRequires:  vala

%if 0%{?suse_version} > 0
# avoid "directories not owned by a package" error
%define notownedhack 1
%else
%define notownedhack 0
%endif

%package devel
Summary:	A collection of GTK widgets for chat applications
Group:		Development/Libraries
Requires:	%{name}%{?_isa} = %{version}-%{release}

%package vala
Summary:	A collection of GTK widgets for chat applications
Group:		Development/Libraries
Requires:	%{name}%{?_isa} = %{version}-%{release}

%description
Talkatu is a collection of GTK widgets for rendering plain text, HTML and
Markdown inside of GTK.

This package contains the main library.

%description devel
Talkatu provides support for parsing and rendering plain text, HTML and
Markdown inside of GTK.

This package includes the files used for development against libtalkatu.

%description vala
Talkatu provides support for parsing and rendering plain text, HTML and
Markdown inside of GTK.

This package contains the vapi bindings allowing Talkatu to be used from vala.

%build
%meson
%meson_build

%install
%meson_install
%find_lang %{name}

# Unneeded files
rm -rf %{buildroot}%{_datadir}/doc/talkatu/
%if %{notownedhack} == 1
rm -rf %{buildroot}%{_datadir}/talkatu/
%endif

%check
%meson_test

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files -f %{name}.lang
%defattr(-,root,root)
%doc README.md ChangeLog logo.png
%if %{notownedhack} == 0
%license COPYING
%endif
%{_bindir}/talkatu-demo
%{_mandir}/man1/talkatu-demo.1*
%{_libdir}/libtalkatu.so.0.1.0

%files devel
%defattr(-,root,root)
%doc README.md HACKING logo.png
%if %{notownedhack} == 0
%license COPYING
%endif
%{_includedir}/talkatu-1.0/
%{_libdir}/libtalkatu.so
%{_libdir}/libtalkatu.so.0
%{_libdir}/pkgconfig/talkatu.pc
%{_libdir}/girepository-1.0/Talkatu-0.0.typelib
%{_datadir}/gir-1.0/Talkatu-0.0.gir
%{_datadir}/gtk-doc/html/talkatu/
%{_datadir}/glade/catalogs/talkatu.xml

%files vala
%defattr(-,root,root)
%doc README.md HACKING logo.png
%if %{notownedhack} == 0
%license COPYING
%endif
%{_datadir}/vala/vapi/talkatu.vapi
%{_datadir}/vala/vapi/talkatu.deps

%changelog
* Thu May 21 2020 Gary Kramlich <grim@reaperworld.com>
- Added Vala package

* Tue Jul 31 2018 Gary Kramlich <grim@reaperworld.com>
- Initial revision

