; -- authmanager.iss --
; The installation script for the Auth Manager utility

[Setup]
AppName=Auth Manager
AppVersion=1.701
DefaultDirName={pf}\authmanager
DefaultGroupName=Auth Manager
UninstallDisplayIcon={app}\Icons\Gnome-dialog-password.ico
Compression=lzma
SolidCompression=true
AllowNoIcons=true
SetupIconFile=C:\Users\morgan\Desktop\cube2AuthManager2\src\w32dist\Icons\Gnome-dialog-password.ico
ShowLanguageDialog=yes
InternalCompressLevel=max
OutputBaseFilename=AuthManager-setup
OutputDir=C:\Users\morgan\Desktop\cube2AuthManager2\src\w32dist\Output
EnableDirDoesntExistWarning=true
DirExistsWarning=yes
AppID={{72671612-1FE8-481F-8C67-B4DD832F8318}
LicenseFile=C:\Users\morgan\Desktop\cube2AuthManager2\LICENSE
RestartIfNeededByRun=false
AppCopyright=2012 - Morgan Borman
SourceDir=C:\Users\morgan\Desktop\cube2AuthManager2\src\w32dist
VersionInfoVersion=1.701
VersionInfoDescription=Setup program for the Auth Manager Utility
VersionInfoCopyright=Zllib
VersionInfoProductName=Auth Manager Setup
VersionInfoProductVersion=1.701
UninstallDisplayName=Uninstall Auth Manager

[Files]
Source: authmanager.exe; DestDir: {app}
Source: authmanager.glade; DestDir: {app}
Source: Icons\Gnome-dialog-password.ico; DestDir: {app}\Icons
Source: Icons\200px-Gnome-accessories-text-editor.png; DestDir: {app}\Icons
Source: Icons\200px-Gnome-dialog-information.png; DestDir: {app}\Icons
Source: Icons\200px-Gnome-edit-delete.png; DestDir: {app}\Icons

Source: libgobject-2.0-0.dll; DestDir: {app}
Source: libgthread-2.0-0.dll; DestDir: {app}
Source: libgtk-win32-2.0-0.dll; DestDir: {app}
Source: libpango-1.0-0.dll; DestDir: {app}
Source: libpangocairo-1.0-0.dll; DestDir: {app}
Source: libpangoft2-1.0-0.dll; DestDir: {app}
Source: libpangowin32-1.0-0.dll; DestDir: {app}
Source: libpng14-14.dll; DestDir: {app}
Source: libstdc++-6.dll; DestDir: {app}
Source: zlib1.dll; DestDir: {app}
Source: freetype6.dll; DestDir: {app}
Source: intl.dll; DestDir: {app}
Source: libatk-1.0-0.dll; DestDir: {app}
Source: libcairo-2.dll; DestDir: {app}
Source: libexpat-1.dll; DestDir: {app}
Source: libfontconfig-1.dll; DestDir: {app}
Source: libgcc_s_dw2-1.dll; DestDir: {app}
Source: libgdk_pixbuf-2.0-0.dll; DestDir: {app}
Source: libgdk-win32-2.0-0.dll; DestDir: {app}
Source: libgio-2.0-0.dll; DestDir: {app}
Source: libglib-2.0-0.dll; DestDir: {app}
Source: libgmodule-2.0-0.dll; DestDir: {app}

[Icons]
Name: {group}\Auth Manager; Filename: {app}\authmanager.exe; IconFilename: {app}\Icons\Gnome-dialog-password.ico

[UninstallDelete]
Name: {app}; Type: filesandordirs
