Name:           xinit
Version:        1.3.2
Release:        0
License:        MIT
Summary:        X Window System initializer
Url:            http://xorg.freedesktop.org/
Group:          System/X11/Utilities
Source0:        http://xorg.freedesktop.org/releases/individual/app/%{name}-%{version}.tar.bz2
Source1:        xinit.tar.bz2
Source2:        keygen.c
Source3:        keygen.1
BuildRequires:  libtool
BuildRequires:  pkg-config
BuildRequires:  pkgconfig(x11)
BuildRequires:  pkgconfig(xorg-macros) >= 1.8
Requires:       setxkbmap
Requires:       xauth
Requires:       xmodmap
Requires:       xrdb
Requires:       xsetroot
Requires:       xterm

%description
The xinit program is used to start the X Window System server and a
first client program on systems that are not using a display manager
such as xdm or in environments that use multiple window systems.
When this first client exits, xinit will kill the X server and then
terminate.

%prep
%setup -q

%build
autoreconf -fi
%configure
make %{?_smp_mflags}
gcc %{optflags} -o keygen %{SOURCE2}

%install
%make_install
install -m 0644 %{SOURCE3} %{buildroot}%{_mandir}/man1
install -m 0711 keygen %{buildroot}%{_bindir}/keygen
pushd %{buildroot}
tar xf %{SOURCE1}
popd
install -D %{buildroot}%{_sysconfdir}/X11/xinit/xinitrc %{buildroot}%{_sysconfdir}/skel/.xinitrc.template

%files
%defattr(-,root,root)
%doc ChangeLog COPYING README
%config %{_sysconfdir}/X11/xinit/
%config %{_sysconfdir}/X11/Xresources
%config %{_sysconfdir}/skel/.xinitrc.template
%{_bindir}/keygen
%{_bindir}/startx
%{_bindir}/xinit
%{_mandir}/man1/keygen.1%{?ext_man}
%{_mandir}/man1/startx.1%{?ext_man}
%{_mandir}/man1/xinit.1%{?ext_man}

%changelog
