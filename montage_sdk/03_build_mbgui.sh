#!/bin/bash

if [ -z "$1" ]; then
	CONFIG=Debug
else
	CONFIG=$1
fi

echo "Config: $CONFIG"

docker run --rm -v $HOME/Projects/mbgui:/home/century/work -w /home/century/work/build_docker -u century -it montage_sdk cmake --build . --config $CONFIG
