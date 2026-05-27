#!/bin/bash

docker run -v ${HOME}/Projects/mbgui:/home/century/work -w /home/century/work/build_docker --read-only montage_sdk make $@
