Name:		gplugin
Version:	0.35.1
%if "dev" == ""
Release:    1.%{?build_number}%{?!build_number:1}%{?dist}
%else
Release:    0.%{?build_number}%{?!build_number:1}%{?dist}
%endif
License:	LGPL-2.0+
Summary:	A GObject based library that implements a reusable plugin system
Url:		https://keep.imfreedom.org/rw_grim/gplugin/overview
Group:		Development/Libraries

BuildRequires:	meson >= 0.56.0
BuildRequires:	glib2-devel >= 2.44.0
BuildRequires:	gobject-introspection-devel
BuildRequires:	gettext
BuildRequires:	gtk3-devel
BuildRequires:	gtk4-devel
BuildRequires:	help2man
BuildRequires:	libxslt
BuildRequires:	lua-devel
BuildRequires:	lua-lgi
BuildRequires:	python3-devel
BuildRequires:	python3-gobject
BuildRequires:	vala
BuildRequires:	perl-Glib
BuildRequires:	perl-Glib-Object-Introspection
%if 0%{?fedora}
BuildRequires:	perl-devel
%endif
%if 0%{?opensuse}
BuildRequires:	perl-ExtUtils-Config
%endif

%package -n libgplugin0
Summary:	A GObject based library that implements a reusable plugin system
Group:		Development/Libraries

%package devel
Summary:	A GObject based library that implements a reusable plugin system
Group:		Development/Libraries
Requires:	%{name}%{?_isa} = %{version}-%{release}

%package gtk3
Summary:	A GObject based library that implements a reusable plugin system
Group:		Development/Libraries

%package gtk3-devel
Summary:	A GObject based library that implements a reusable plugin system
Group:		Development/Libraries
Requires:	%{name}-gtk3%{?_isa} = %{version}-%{release}
Requires:	%{name}-devel%{?_isa} = %{version}-%{release}

%package gtk3-vala
Summary:	A GObject based library that implements a reusable plugin system
Group:		Development/Libraries
Requires:	%{name}-gtk3%{?_isa} = %{version}-%{release}
Requires:	%{name}-devel%{?_isa} = %{version}-%{release}

%package gtk4
Summary:	A GObject based library that implements a reusable plugin system
Group:		Development/Libraries

%package gtk4-devel
Summary:	A GObject based library that implements a reusable plugin system
Group:		Development/Libraries
Requires:	%{name}-gtk4%{?_isa} = %{version}-%{release}
Requires:	%{name}-devel%{?_isa} = %{version}-%{release}

%package lua
Summary:	A GObject based library that implements a reusable plugin system
Group:		Development/Libraries

%package perl5
Summary:	A GObject based library that implements a reusable plugin system
Group:		Development/Libraries

%package python3
Summary:	A GObject based library that implements a reusable plugin system
Group:		Development/Libraries

%package vala
Summary:	A GObject based library that implements a reusable plugin system
Group:		Development/Libraries
Requires:	%{name}-devel%{?_isa} = %{version}-%{release}

%description
GPlugin is a GObject based library that implements a reusable plugin system
that supports loading plugins in other languages via loaders.  GPlugin also
implements dependencies among the plugins.

%description -n libgplugin0
GPlugin is a GObject based library that implements a reusable plugin system
that supports loading plugins in other languages via loaders.  GPlugin also
implements dependencies among the plugins.

%description devel
GPlugin is a GObject based library that implements a reusable plugin system
that supports loading plugins in other languages via loaders.  GPlugin also
implements dependencies among the plugins.

This package contains all necessary include files and libraries needed
to develop applications that require these.

%description gtk3
GPlugin is a GObject based library that implements a reusable plugin system
that supports loading plugins in other languages via loaders.  GPlugin also
implements dependencies among the plugins.

This package contains additional GTK3 dependencies for %{name} library.

%description gtk3-devel
GPlugin is a GObject based library that implements a reusable plugin system
that supports loading plugins in other languages via loaders.  GPlugin also
implements dependencies among the plugins.

This package contains all necessary include files and libraries needed
to develop GTK3 applications that require these.

%description gtk3-vala
GPlugin is a GObject based library that implements a reusable plugin system
that supports loading plugins in other languages via loaders.  GPlugin also
implements dependencies among the plugins.

This package contains the vapi bindings allowing GPluginGtk3 to be used from
vala.

%description gtk4
GPlugin is a GObject based library that implements a reusable plugin system
that supports loading plugins in other languages via loaders.  GPlugin also
implements dependencies among the plugins.

This package contains additional GTK4 dependencies for %{name} library.

%description gtk4-devel
GPlugin is a GObject based library that implements a reusable plugin system
that supports loading plugins in other languages via loaders.  GPlugin also
implements dependencies among the plugins.

This package contains all necessary include files and libraries needed
to develop GTK4 applications that require these.

%description lua
GPlugin is a GObject based library that implements a reusable plugin system
that supports loading plugins in other languages via loaders.  GPlugin also
implements dependencies among the plugins.

This package contains the Lua Loader for %{name} library.

%description perl5
GPlugin is a GObject based library that implements a reusable plugin system
that supports loading plugins in other languages via loaders.  GPlugin also
implements dependencies among the plugins.

This package contains Perl loader for %{name} library.

%description python3
GPlugin is a GObject based library that implements a reusable plugin system
that supports loading plugins in other languages via loaders.  GPlugin also
implements dependencies among the plugins.

This package contains Python3 loader for %{name} library.

%description vala
GPlugin is a GObject based library that implements a reusable plugin system
that supports loading plugins in other languages via loaders.  GPlugin also
implements dependencies among the plugins.

This package contains the vapi bindings allowing GPlugin to be used from vala.

%build
CFLAGS="%{optflags}" meson \
	--prefix=%{_prefix} \
	build

ninja -C build %{?_smp_mflags}

%install
DESTDIR=%{buildroot} ninja -C build install

# Unneeded files
rm -rf %{buildroot}%{_datadir}/doc/gplugin/

%post  -n libgplugin0 -p /sbin/ldconfig

%postun  -n libgplugin0 -p /sbin/ldconfig

%post -n gplugin-gtk3 -p /sbin/ldconfig

%postun -n gplugin-gtk3 -p /sbin/ldconfig

%files
%defattr(-,root,root)
%doc README.md ChangeLog
%license COPYING
%{_bindir}/gplugin-query
%{_libdir}/gplugin/
%{_mandir}/man1/gplugin-query.1*

%files -n libgplugin0
%defattr(-,root,root)
%doc README.md
%license COPYING
%{_libdir}/libgplugin.so.0.1.0

%files devel
%defattr(-,root,root)
%doc HACKING.md README.md
%license COPYING
%{_datadir}/gplugin/
%{_datadir}/gir-1.0/GPlugin-1.0.gir
%{_datadir}/doc/gplugin/
%{_includedir}/gplugin-1.0/
%{_libdir}/girepository-1.0/GPlugin-1.0.typelib
%{_libdir}/libgplugin.so
%{_libdir}/libgplugin.so.0
%{_libdir}/pkgconfig/gplugin.pc

%files gtk3
%defattr(-,root,root)
%doc README.md
%license COPYING
%{_bindir}/gplugin-gtk3-viewer
%{_libdir}/libgplugin-gtk3.so.0.1.0
%{_mandir}/man1/gplugin-gtk3-viewer.1*

%files gtk3-devel
%defattr(-,root,root)
%dir %{_datadir}/glade/catalogs/
%doc README.md
%license COPYING
%{_datadir}/gir-1.0/GPluginGtk3-1.0.gir
%{_datadir}/glade/catalogs/gplugin-gtk3.xml
%{_datadir}/doc/gplugin-gtk3/
%{_includedir}/gplugin-gtk3-1.0/
%{_libdir}/girepository-1.0/GPluginGtk3-1.0.typelib
%{_libdir}/libgplugin-gtk3.so
%{_libdir}/libgplugin-gtk3.so.0
%{_libdir}/pkgconfig/gplugin-gtk3.pc

%files gtk3-vala
%defattr(-,root,root)
%doc README.md
%license COPYING
%{_datadir}/vala/vapi/gplugin-gtk3.vapi
%{_datadir}/vala/vapi/gplugin-gtk3.deps

%files gtk4
%defattr(-,root,root)
%doc README.md
%license COPYING
%{_libdir}/libgplugin-gtk4.so.0.1.0

%files gtk4-devel
%defattr(-,root,root)
%doc README.md
%license COPYING
%{_datadir}/gir-1.0/GPluginGtk4-1.0.gir
%{_datadir}/doc/gplugin-gtk4/
%{_includedir}/gplugin-gtk4-1.0/
%{_libdir}/girepository-1.0/GPluginGtk4-1.0.typelib
%{_libdir}/libgplugin-gtk4.so
%{_libdir}/libgplugin-gtk4.so.0
%{_libdir}/pkgconfig/gplugin-gtk4.pc

%files lua
%defattr(-,root,root)
%doc README.md
%license COPYING
%{_libdir}/gplugin/gplugin-lua.so

%files perl5
%defattr(-,root,root)
%doc README.md
%license COPYING
%{_libdir}/gplugin/gplugin-perl5.so

%files python3
%defattr(-,root,root)
%doc README.md
%license COPYING
%{_libdir}/gplugin/gplugin-python3.so

%files vala
%defattr(-,root,root)
%doc README.md
%license COPYING
%{_datadir}/vala/vapi/gplugin.vapi
%{_datadir}/vala/vapi/gplugin.deps

%changelog
* Wed Sep 29 2021 Gary Kramlich <grim@reaperworld.com>
- Renamed gplugin-gtk to gplugin-gtk3
- Renamed gplugin-gtk-devel to gplugin-gtk3-devel
- Renamed gplugin-gtk-vala to gplugin-gtk3-vala

* Thu Sep 02 2021 Gary Kramlich <grim@reaperworld.com>
- Update the devel packages for gi-docgen

* Tue Aug 25 2020 Gary Kramlich <grim@reaperworld.com>
- Updated the perl packages for its new name.

* Tue Feb 18 2020 Gary Kramlich <grim@reaperworld.com>
- Add the perl package
- Updated the python files to reference the new name for gplugin-python3

* Thu Oct 31 2019 Gary Kramlich <grim@reaperworld.com>
- Fixed an include bug in gplugin-gtk
- Fixed building again python3.8

* Mon Aug 12 2019 Gary Kramlich <grim@reaperworld.com>
- Added vala

* Sun May 15 2016 Gary Kramlich <grim@reaperworld.com>
- Updates and pulled upstream

* Mon May 4 2015 Elliott Sales de Andrade <quantum.analyst@gmail.com> - 0.0.18-1
- Initial package release
