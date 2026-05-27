#!/bin/bash

cmake --toolchain=../toolchain_mips_montage.txt -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_VERBOSE_MAKEFILE=True `dirname $0`
