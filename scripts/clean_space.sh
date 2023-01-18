#!/bin/sh

all=$1
chmod -R 777 /data/media/0/realdata
if [ "$all" = "all" ];then
  rm -rf /sdcard/dashcam /data/media/0/realdata
  exit 0
fi
size_g=$(df /sdcard|tail -n1|awk '{print int($4/(1024*1024))}')
if [ $size_g -le 4 ];then
  ls -1t /sdcard/dashcam|sed -n '5,$p'|tac|head -n5|while read f;do
    f=/sdcard/dashcam/$f
    fuser $f || rm -f $f
  done

  ls -1t /data/media/0/realdata|sed -n '2,$p'|tac|head -n2|while read f;do
    f=/data/media/0/realdata/$f; rm -rf $f
  done
fi
