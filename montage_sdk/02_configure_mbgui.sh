#!/bin/bash

if [ ! -d $HOME/Projects/mbgui/build_docker ]; then 
	mkdir -p $HOME/Projects/mbgui/build_docker
fi

docker run -v ${HOME}/Projects/mbgui:/home/century/work -w /home/century/work/build_docker -u century -it montage_sdk cmake --toolchain ../toolchain_mips_montage.txt -G "Ninja Multi-Config" -DCMAKE_CONFIGURATION_TYPES="Debug;Release;RelWithDebInfo;MinSizeRel" -DCMAKE_DEFAULT_BUILD_TYPE="Debug" ..
