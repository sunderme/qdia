#!/bin/bash

#generate libs.qrc
echo "Generate libs.qrc"
rm ./libs.qrc
echo "<RCC>">./libs.qrc
echo "<qresource prefix=\"/\">">>./libs.qrc
ls -1 libs/analog/*.json|xargs -i echo "<file>"{}"</file>" >> ./libs.qrc
ls -1 libs/gates/*.json|xargs -i echo "<file>"{}"</file>" >> ./libs.qrc
ls -1 libs/rf/*.json|xargs -i echo "<file>"{}"</file>" >> ./libs.qrc
echo "</qresource>">>./libs.qrc
echo "</RCC>">>./libs.qrc
