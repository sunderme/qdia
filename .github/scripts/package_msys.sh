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
zip -r ./qdia-win-qt6-${VERSION_NAME}.zip *

cd ..
sha256sum ./package-zip/qdia-win-qt6-${VERSION_NAME}.zip
cp ./package-zip/qdia-win-qt6-${VERSION_NAME}.zip ./qdia-${VERSION_NAME}-win-portable-qt6.zip

