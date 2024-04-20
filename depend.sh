#!/bin/bash
#
# for each .c file, output all .h files it depends on
# use gcc or tcc compiler (tcc is 3 times faster)
#
# works with tcc version
# tcc version 0.9.28rc 2024-04-16 mob@0703df1a (x86_64 Linux)
#
# or gcc 13.1.0
#

USE_TCC=true

# TCC version
################################################################################
if $USE_TCC; then
echo "-------------------------------------------------------------------------"
for FILE in *.c; do
echo "$FILE: "
DEPS=$(tcc -vv -c -o tmp.o $FILE 2> /dev/null )
DEPS=$(echo "$DEPS" | grep -F -- "->" )
DEPS=$(echo "$DEPS" | grep -F -v / )
DEPS=$(echo "$DEPS" | grep -E -o "\w+\.h" )
DEPS=$(echo "$DEPS" | tr "\n" " " )
echo "$FILE $DEPS"
echo "-------------------------------------------------------------------------"
done  # for FILE in *.c

# GCC version
################################################################################
else  # if $USE_TCC
echo "-------------------------------------------------------------------------"
for FILE in *.c; do
echo "$FILE: "
DEPS=$(gcc -H -O0 -c -o tmp.o $FILE  2>&1)
DEPS=$(echo "$DEPS" | grep -E "\.+\s*\w*\.h" )
DEPS=$(echo "$DEPS" | grep -F -v / )
DEPS=$(echo "$DEPS" | grep -o -E "\w+\.h" )
DEPS=$(echo "$DEPS" | tr "\n" " " )
echo "$FILE $DEPS"
echo "-------------------------------------------------------------------------"
done  # for FILE in *.c
fi  # if $USE_TCC
