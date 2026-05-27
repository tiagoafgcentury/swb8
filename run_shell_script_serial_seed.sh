#!/bin/bash
echo -e " gdbserver ${IP_BOARD}:1234 /usr/bin/${TARGET}\n" >> /dev/ttyUSB0
