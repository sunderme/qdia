#!/usr/bin/env sh

# Exit on errors
set -e

echo "Running linuxdeployqt"

wget -c -nv "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"
chmod a+x linuxdeployqt-continuous-x86_64.AppImage
unset QTDIR; unset QT_PLUGIN_PATH ; unset LD_LIBRARY_PATH
export VERSION=linux-${VERSION_NAME}
# work-around "secret" obstacle in linuxdeployqt
mkdir -p appdir/usr/share/doc/libc6
touch appdir/usr/share/doc/libc6/copyright
./linuxdeployqt-continuous-x86_64.AppImage appdir/usr/share/applications/*.desktop -bundle-non-qt-libs -appimage -unsupported-allow-new-glibc
# ./linuxdeployqt-continuous-x86_64.AppImage appdir/usr/share/applications/*.desktop -appimage
cp QDia-${VERSION}-x86_64.AppImage ../QDia-${VERSION}-x86_64.AppImage



