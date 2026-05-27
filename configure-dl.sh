#!/bin/sh

cmake --toolchain=/home/century/mbgui/toolchain_mips_ali.txt -G "Ninja" -DCMAKE_BUILD_TYPE=MinSizeRel \
	-DMBGUI_ANIMATION=OFF \
	-DCMAKE_CXX_FLAGS_MINSIZEREL="-Os" \
	-DCMAKE_C_FLAGS_MINSIZEREL="-Os" \
	-DMBGUI_USE_NAGRA_CERT_STREAMS=OFF \
	-DMBGUI_APP_TPM=ON \
	--fresh $@
