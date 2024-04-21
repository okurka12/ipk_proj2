#!/bin/bash
#
# lets you see when you can run your server again
#
COUNT=1
while true; do
echo $COUNT
ss -a | grep --color=auto 4567
COUNT=$(expr $COUNT + 1)
sleep 1
done
