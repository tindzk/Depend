#!/bin/sh

echo "main=Main.c
include=../Jivai/src
debug=yes
manifest=yes" | ./Depend.exe build /dev/stdin
