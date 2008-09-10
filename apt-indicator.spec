Name: apt-indicator
Version: 0.1.0
Release: alt0.0.2

Summary: Applet for indication that newer packages are available
License: GPL
Group: System/Configuration/Packaging

Url: http://apt-indicator.sourceforge.net/
Packager: Sergey V Turchin <zerg@altlinux.org>

Source: %name-%version.tar

Provides: egg = %version-%release, alt-update = %version-%release
Obsoletes: egg < %version-%release, alt-update < %version-%release
Requires: synaptic-usermode

BuildRequires: xorg-devel gcc-c++ libqt4-devel libstdc++-devel
BuildRequires: docbook-dtds docbook-style-xsl help2man libapt-devel
BuildRequires: xml-common xsltproc
BuildRequires: libdb4.4-devel

%description
This package contains simple applet both for Gnome and KDE which
made notifications for users that newer packages are available.


%prep
%setup -q -n %name-%version
qmake-qt4

%build
%add_optflags -DNDEBUG
%make 
%make -C doc
lrelease-qt4 %name.pro
help2man --output=apt-indicator.1 --no-info apt-indicator

%install
%make INSTALL_ROOT=%buildroot install

mkdir -p %buildroot/%_datadir/%name/translations/
install -m644 translations/apt_indicator_*.qm %buildroot/%_datadir/%name/translations/
mkdir -p %buildroot/%_man1dir/
install -m644 %{name}.1 %buildroot/%_man1dir/
mkdir -p %buildroot/%_datadir/autostart/
install -m644 %name.desktop %buildroot/%_datadir/autostart/%name.desktop
mkdir -p %buildroot/%_datadir/applications/
install -m644 %name.desktop %buildroot/%_datadir/applications/%name.desktop

#hack for RC
#ln -sf ../doc/%name-%version/html $RPM_BUILD_ROOT%_datadir/%name/doc 

%post
%update_menus
%postun
%clean_menus

%files
%doc doc/html doc/images NEWS ChangeLog TODO README
%_bindir/*
%_man1dir/*
%_datadir/%name
%_datadir/applications/%name.desktop
%_datadir/autostart/%name.desktop

%changelog
* Wed Apr 28 2004 Stanislav Ievlev <inger@altlinux.org> 0.0.4-alt1
- 0.0.4 rc1

* Wed Mar 10 2004 Stanislav Ievlev <inger@altlinux.org> 0.0.3-alt2
- update russian translation

* Tue Mar 09 2004 Stanislav Ievlev <inger@altlinux.org> 0.0.3-alt1
- 0.0.3

* Thu Jan 15 2004 Dmitry V. Levin <ldv@altlinux.org> 0.0.2-alt5.1
- Rebuilt with apt-0.5.15cnc5.

* Tue Sep 23 2003 Sergey V Turchin <zerg at altlinux dot org> 0.0.2-alt5
- add requires to synaptic-usermode

* Tue Sep 02 2003 Sergey V Turchin <zerg at altlinux dot org> 0.0.2-alt4
- update code from cvs

* Fri Jun 20 2003 Sergey V Turchin <zerg at altlinux dot org> 0.0.2-alt3
- update from cvs

* Thu Jun 05 2003 Sergey V Turchin <zerg at altlinux dot ru> 0.0.2-alt2
- fix Cancel button in YesNoDialog

* Wed Jun 04 2003 Sergey V Turchin <zerg at altlinux dot ru> 0.0.2-alt1
- new version

* Thu May 29 2003 Stanislav Ievlev <inger@altlinux.ru> 0.0.1-alt2
- added documentation

* Tue May 27 2003 Sergey V Turchin <zerg at altlinux dot ru> 0.0.1-alt1
- update from cvs
- new version
- fix BuildRequires
- first build for Sisyphus

* Tue May 27 2003 Sergey V Turchin <zerg at altlinux dot ru> 0.0.0-alt6
- update from cvs (fixed autostart.desktop)

* Tue May 27 2003 Sergey V Turchin <zerg at altlinux dot ru> 0.0.0-alt5
- update from cvs
- add to %_datadir/autostart/*.desktop to %%files

* Mon May 26 2003 Sergey V Turchin <zerg at altlinux dot ru> 0.0.0-alt4
- update from cvs
- add to %_datadir/autostart for KDE

* Mon May 26 2003 Sergey V Turchin <zerg at altlinux dot ru> 0.0.0-alt3

* Mon May 26 2003 Sergey V Turchin <zerg at altlinux dot ru> 0.0.0-alt2
- update from cvs
- add menu

* Mon May 26 2003 Stanislav Ievlev <inger@altlinux.ru> 0.0.0-alt1
- Initial release for Daedalus
