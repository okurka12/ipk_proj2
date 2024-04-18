#!/bin/bash

for FILE in *.c; do

echo "-------------------------------------------------------------------------"
echo "$FILE: "
gcc -H -c $FILE -o tmp.o 2>&1 | grep -E "\.+.*\.h" | grep -F -v /
echo ""

done  # for FILE
