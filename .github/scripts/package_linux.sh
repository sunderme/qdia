#!/usr/bin/env sh

# Exit on errors
set -e

echo "Running linuxdeployqt"

#wget -c -nv "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"
#chmod a+x linuxdeployqt-continuous-x86_64.AppImage
wget -c https://github.com/$(wget -q https://github.com/probonopd/go-appimage/releases/expanded_assets/continuous -O - | grep "appimagetool-.*-x86_64.AppImage" | head -n 1 | cut -d '"' -f 2)
chmod +x appimagetool-*.AppImage
unset QTDIR; unset QT_PLUGIN_PATH ; unset LD_LIBRARY_PATH
export VERSION=linux-${VERSION_NAME}
# work-around "secret" obstacle in linuxdeployqt
mkdir -p appdir/usr/share/doc/libc6
touch appdir/usr/share/doc/libc6/copyright
#./linuxdeployqt-continuous-x86_64.AppImage appdir/usr/share/applications/*.desktop -bundle-non-qt-libs -appimage -unsupported-allow-new-glibc
# ./linuxdeployqt-continuous-x86_64.AppImage appdir/usr/share/applications/*.desktop -appimage
./appimagetool-*.AppImage -s deploy appdir/usr/share/applications/*.desktop
./appimagetool-*.AppImage ./appdir # create actual appimage
cp QDia-${VERSION}-x86_64.AppImage ../QDia-${VERSION}-x86_64.AppImage



