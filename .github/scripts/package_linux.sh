#!/usr/bin/env sh

# Exit on errors
set -e

echo "Running linuxdeployqt"

wget -c -nv "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"
chmod a+x linuxdeployqt-continuous-x86_64.AppImage
unset QTDIR; unset QT_PLUGIN_PATH ; unset LD_LIBRARY_PATH
export VERSION=linux-${VERSION_NAME}
./linuxdeployqt-continuous-x86_64.AppImage appdir/usr/share/applications/*.desktop -bundle-non-qt-libs -appimage
# ./linuxdeployqt-continuous-x86_64.AppImage appdir/usr/share/applications/*.desktop -appimage
cp QDia-${VERSION}-x86_64.AppImage ../QDia-${VERSION}-x86_64.AppImage



