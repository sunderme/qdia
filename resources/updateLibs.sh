#!/bin/bash

#generate libs.qrc
echo "Generate libs.qrc"
rm ./libs.qrc
echo "<RCC>">./libs.qrc
echo "<qresource prefix=\"/\">">>./libs.qrc
find libs/ -name *.json -print|xargs -i echo "<file>"{}"</file>" >> ./libs.qrc
echo "</qresource>">>./libs.qrc
echo "</RCC>">>./libs.qrc
