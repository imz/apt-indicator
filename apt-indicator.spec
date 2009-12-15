Name: apt-indicator
Version: 0.1.13
Release: alt1

Summary: Applet for indication that newer packages are available
License: GPL
Group: System/Configuration/Packaging
Url: http://apt-indicator.sourceforge.net/
Packager: Sergey V Turchin <zerg@altlinux.org>

Requires: libqt4-core >= %{get_version libqt4-core}

Source: %name-%version.tar

Provides: egg = %version-%release, alt-update = %version-%release
Obsoletes: egg < %version-%release, alt-update < %version-%release
Requires: synaptic-usermode

BuildRequires(pre): libqt4-devel
BuildRequires: xorg-devel gcc-c++ libstdc++-devel
BuildRequires: docbook-dtds docbook-style-xsl help2man libapt-devel
BuildRequires: xml-common xsltproc
#BuildRequires: libdb4.4-devel

%description
This package contains simple applet both for Gnome and KDE which
made notifications for users that newer packages are available.


%prep
%setup -q -n %name-%version
qmake-qt4 apt-indicator.pro

%build
%add_optflags -DNDEBUG
%make 
%make -C doc
lrelease-qt4 checker/checker.pro
lrelease-qt4 agent/agent.pro
help2man --output=apt-indicator.1 --no-info apt-indicator ||:

%install
%make INSTALL_ROOT=%buildroot install

mkdir -p %buildroot/%_datadir/%name/translations/
install -m644 translations/apt_indicator_*.qm %buildroot/%_datadir/%name/translations/
mkdir -p %buildroot/%_man1dir/
[ -f %{name}.1 ] \
    && install -m644 %{name}.1 %buildroot/%_man1dir/
mkdir -p %buildroot/%_datadir/applications/
install -m644 %name.desktop %buildroot/%_datadir/applications/%name.desktop

for d in %_sysconfdir/xdg/autostart
do
mkdir -p %buildroot/$d/
install -m644 %name.desktop %buildroot/$d/%name.desktop
sed -i 's|\(^Exec=.*\)|\1 --autostart|' %buildroot/$d/%name.desktop
done

# docs
ln -sf %_docdir/%name-%version/html %buildroot/%_datadir/%name/doc
mkdir -p %buildroot/%_datadir/%name/pixmaps
install -m644 pixmaps/* %buildroot/%_datadir/%name/pixmaps


%files
%doc doc/html doc/images NEWS ChangeLog TODO README
%_bindir/*
#%_man1dir/*
%_datadir/%name
%_datadir/applications/%name.desktop
%_sysconfdir/xdg/autostart/apt-indicator.desktop


%changelog
#- allow to start upgrade program separately

* Tue Dec 15 2009 Sergey V Turchin <zerg at altlinux dot org> 0.1.13-alt1
- fix popup information window when closed
- fix reset to default settings

* Tue Nov 24 2009 Sergey V Turchin <zerg at altlinux dot org> 0.1.12-alt1
- allow restore default settings

* Mon Sep 14 2009 Sergey V Turchin <zerg at altlinux dot org> 0.1.11-alt2
- remove /usr/share/autostart entry

* Mon Aug 10 2009 Sergey V Turchin <zerg at altlinux dot org> 0.1.11-alt1
- fix detect running copy

* Mon Jun 22 2009 Sergey V Turchin <zerg at altlinux dot org> 0.1.10-alt1
- show tray icon when agent already started and icon hidden

* Tue Jun 09 2009 Sergey V Turchin <zerg at altlinux dot org> 0.1.9-alt1
- hide system tray icon when do nothing

* Fri May 08 2009 Sergey V Turchin <zerg at altlinux dot org> 0.1.8-alt1
- move agent to separate process to facilitate autostart

* Fri Apr 24 2009 Sergey V Turchin <zerg at altlinux dot org> 0.1.7-alt1
- allow to start repositories configurator
- minor usability improvements

* Tue Apr 14 2009 Sergey V Turchin <zerg at altlinux dot org> 0.1.6-alt0.M50.1
- built for M50

* Mon Apr 13 2009 Sergey V Turchin <zerg at altlinux dot org> 0.1.6-alt1
- ask for autostart at exit

* Mon Apr 13 2009 Sergey V Turchin <zerg at altlinux dot org> 0.1.5-alt1
- ignore fetch cdrom errors

* Fri Apr 10 2009 Sergey V Turchin <zerg at altlinux dot org> 0.1.4-alt0.M50.1
- built for M50

* Fri Apr 10 2009 Sergey V Turchin <zerg at altlinux dot org> 0.1.4-alt1
- allow to switch off popup messages
- fix to don't change state when sources.list-s not configured
- fix help browser behavior
- update Russian translation

* Wed Apr 08 2009 Sergey V Turchin <zerg at altlinux dot org> 0.1.3-alt0.M50.1
- built for M50

* Wed Apr 08 2009 Sergey V Turchin <zerg at altlinux dot org> 0.1.3-alt1
- update icons
- show popup message when updates available

* Wed Apr 08 2009 Sergey V Turchin <zerg at altlinux dot org> 0.1.2-alt1
- improve updating program configuration

* Tue Apr 07 2009 Sergey V Turchin <zerg at altlinux dot org> 0.1.1-alt0.M50.1
- built for M50

* Mon Apr 06 2009 Sergey V Turchin <zerg at altlinux dot org> 0.1.1-alt1
- fix menu translations
- update info window if shown
- close info window on upgrade program exit
- add xdg autostart entry

* Fri Dec 12 2008 Dmitry V. Levin <ldv@altlinux.org> 0.1.0-alt1.1
- Removed obsolete %%update_menus/%%clean_menus calls.
- Built with libapt-pkg-libc6.9-6.so.2.

* Mon Nov 10 2008 Sergey V Turchin <zerg at altlinux dot org> 0.1.0-alt1
- port to Qt4

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
