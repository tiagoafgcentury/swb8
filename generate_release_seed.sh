#!/bin/bash

set -e

echo ${CMAKE_CURRENT_SOURCE_DIR}
cd ${CMAKE_CURRENT_SOURCE_DIR}

git add CMakeLists.txt

echo "Commit v${PROJECT_VERSION}"
git commit -m"Created v${PROJECT_VERSION}"

echo "tag v${PROJECT_VERSION}"
git tag v${PROJECT_VERSION}

echo "push"
git push
git push --tags

echo "${CMAKE_CURRENT_BINARY_DIR}"
cd ${CMAKE_CURRENT_BINARY_DIR}

echo "clean"
ninja clean

echo "install"
ninja install

if [ -d $ENV{HOME}/sdk-ali ]; then
    echo "ENV{HOME}/sdk-ali/03_copy_image.sh 1"
    cd $ENV{HOME}/sdk-ali/
    $ENV{HOME}/sdk-ali/03_copy_image.sh 1
    cd $ENV{HOME}/sdk-ali/updates/
    $ENV{HOME}/sdk-ali/updates/mkupdate.sh rootfs_release ${PROJECT_VERSION}-${CMAKE_BUILD_TYPE}-${NAGRA_OPERATOR}
fi
