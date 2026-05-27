#!/bin/bash

set -e

BASEDIR=$(dirname "$0")
BASEDIR=$(realpath "$BASEDIR")
cd $BASEDIR

for FILE in $(awk '/^\s*res/{ gsub(/^res\//, "", $1); print $1 }' CMakeLists.txt); do
	echo "Checking $FILE"
	DEFINE=$(awk "/$FILE/{ print \$2 }" $BASEDIR/ui/lvgl/mb_menu_resources.h)
	echo "Fond $DEFINE"
	find $BASEDIR \( -not -name 'mb_menu_resources.h' \) -and \( \( -name "*.h" -or -name "*.cpp" \) -exec grep -Hin --color=auto --regexp="\\b$DEFINE\\b" {} \; \)
done
