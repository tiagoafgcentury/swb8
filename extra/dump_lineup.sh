#!/bin/bash

FILE=/tmp/lineup-`date +%Y%m%d-%H%M%S`.json

/usr/bin/psql -h 10.0.11.4 -p 5440 -U postgres portal -tc "select lineup from dvb.lineup order by id_lineup desc limit 1" | json_reformat > $FILE
echo $FILE
xdg-open $FILE
