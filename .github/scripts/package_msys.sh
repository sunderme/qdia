#!/usr/bin/env sh

# Exit on errors
set -e
VERSION_NAME=0.1
echo "package build into zip for win"
# workaround for botched qt6 installation
mkdir -p package-zip
cp qdia.exe package-zip/
cd package-zip
windeployqt-qt6 qdia.exe
ldd qdia.exe | awk '{print $3}'| grep ming | xargs -I{} cp -u {} .
cd ..
echo "make installer"
cp ../resources/qdia-msys.nsi .
cp ../resources/FileAssociation.nsh .
makensis qdia-msys.nsi

cd package-zip
zip -r ./qdia-win-qt6-${VERSION_NAME}.zip *

cd ..
sha256sum ./package-zip/qdia-win-qt6-${VERSION_NAME}.zip
cp ./package-zip/qdia-win-qt6-${VERSION_NAME}.zip ./qdia-${VERSION_NAME}-win-portable-qt6.zip
cp ./qdia_installer.exe ./qdia-win-qt6-${VERSION_NAME}.exe


