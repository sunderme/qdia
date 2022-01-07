#!/usr/bin/env sh

# Exit on errors
set -e
VERSION_NAME=0.1
echo "package build into zip for win"
# workaround for botched qt6 installation
cp /mingw64/bin/qmake-qt6.exe /mingw64/bin/qmake.exe
echo "copy dlls and qt5 plugins"
mkdir -p package-zip
cp qdia.exe package-zip/
cd package-zip
windeployqt-qt6 texstudio.exe
zip -r ./qdia-win-qt6-${VERSION_NAME}.zip *

cd ..
sha256sum ./package-zip/texstudio-win-qt6-${VERSION_NAME}.zip
cp ./package-zip/texstudio-win-qt6-${VERSION_NAME}.zip ./texstudio-${VERSION_NAME}-win-portable-qt6.zip

