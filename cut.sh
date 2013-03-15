#!/bin/bash

mkdir -p tiles

src=tiles.png
cut=cutdata.txt

width=`identify -format %w $src`
height=`identify -format %h $src`

cat "${cut}" | while read line; do
if test ! -z "$line"; then 
	n=`echo "${line}" | awk -F: '{print $1}'`
	x=`echo "${line}" | awk -F: '{print $2}'`
	y=`echo "${line}" | awk -F: '{print $3}'`

	time1=$(($(date +%s%N)/1000000))
	tile=tiles/${n}.png
	echo -n $tile
	w=$((x * 32))
	h=$((y * 32))
	convert "$src" -crop 32x32+$w+$h -colors 256 PNG8:"$tile"
	time2=$(($(date +%s%N)/1000000))
	dtime=$((time2-time1))
	echo " - "$dtime"ms"
fi
done

