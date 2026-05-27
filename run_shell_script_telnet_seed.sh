#!/bin/bash
(sleep 0.4; echo "${DEBUG_USER}"; sleep 0.2; echo -e "\r"; sleep 0.2; echo -e "killall -9 gdbserver ${TARGET}"; sleep 1; echo -e "nice -n -10 gdbserver ${IP_BOARD}:1234 /usr/bin/${TARGET}\r"; sleep 10000000) | telnet "${IP_BOARD}"
