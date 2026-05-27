#!/bin/bash
xclip | awk "{ system(\"/opt/ali/buildroot-10.2.1.29/output/host/bin/mips-mti-linux-gnu-addr2line -e mbgui.nostrip \'$2\'\") }"
