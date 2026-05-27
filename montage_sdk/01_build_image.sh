#!/bin/sh

NUM_JOBS=`cat /proc/cpuinfo | egrep '^processor\s*:' | wc -l`
cd `dirname $0`

docker build --build-arg NUM_JOBS=$NUM_JOBS  -t montage_sdk   .
