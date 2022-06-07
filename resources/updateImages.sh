#!/bin/bash

#generate images.qrc
echo "Generate images.qrc"
rm ./images.qrc
echo "<RCC>">./images.qrc
echo "<qresource prefix=\"/\">">>./images.qrc
find images -type f -regex '.+\.\(svgz?\|png\)' -print|xargs -i echo "<file>"{}"</file>" >> ./images.qrc
echo "</qresource>">>./images.qrc
echo "</RCC>">>./images.qrc
