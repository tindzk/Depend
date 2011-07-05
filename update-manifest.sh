#!/bin/sh

echo "main=Depend.c
include=../Jivai/src
debug=yes
manifest=yes" | ./Depend.exe build /dev/stdin
