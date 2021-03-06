%bcond_with x

Name:           xinit
Version:        1.3.3
Release:        1
License:        MIT
Summary:        X Window System initializer
Url:            http://xorg.freedesktop.org/
Group:          System/X11/Utilities
Source0:        http://xorg.freedesktop.org/releases/individual/app/%{name}-%{version}.tar.bz2
Source1001: 	xinit.manifest
BuildRequires: autoconf
BuildRequires: automake
BuildRequires: dbus-devel
BuildRequires:  libtool
BuildRequires:  pkg-config
BuildRequires:  pkgconfig(x11)
BuildRequires:  pkgconfig(xorg-macros) >= 1.8
Requires:       setxkbmap
Requires:       xauth
Requires:       xmodmap
Requires:       xrdb
Requires:       xsetroot
Requires: xorg-x11-server-utils

%if !%{with x}
ExclusiveArch:
%endif

%description
The xinit program is used to start the X Window System server and a
first client program on systems that are not using a display manager
such as xdm or in environments that use multiple window systems.
When this first client exits, xinit will kill the X server and then
terminate.

%prep
%setup -q
cp %{SOURCE1001} .

%build
%autogen --with-xinitdir=%{_sysconfdir}/X11/xinit/ CFLAGS="${CFLAGS} -D_F_EXIT_AFTER_XORG_AND_XCLIENT_LAUNCHED_ "
make %{?_smp_mflags}

%install
%make_install
rm -f %{buildroot}/usr/bin/startx

%files
%manifest %{name}.manifest
%defattr(-,root,root)
%license COPYING
%config %{_sysconfdir}/X11/xinit/
%{_bindir}/xinit
%{_mandir}/man1/startx.1%{?ext_man}
%{_mandir}/man1/xinit.1%{?ext_man}

%changelog
