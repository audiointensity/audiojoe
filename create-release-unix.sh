#!/bin/bash

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 version-name"
    exit
fi
ver=$1

[ -f ffmpeg ] || cp $(which ffmpeg) . || (echo Please install ffmpeg before running this script. && exit)

gcc audiojoe.c -lrt -o audiojoe
7z a audiojoe-debian-$ver.zip audiojoe.sh ffmpeg audiojoe "Please install VLC if you do not already have it.txt"
