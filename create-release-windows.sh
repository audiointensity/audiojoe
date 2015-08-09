#!/bin/bash

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 version"
    exit
fi
ver=$1

[ -f ffmpeg ] || cp $(which ffmpeg) . || (echo Please install ffmpeg before running this script. && exit)

gcc audiojoe.c -o audiojoe.exe
7z a audiojoe-windows-$ver.zip audiojoe.bat ffmpeg.exe audiojoe.exe "Link to VideoLAN.org - Download VLC Player here.url"
