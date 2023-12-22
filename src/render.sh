#!/bin/sh

OUTPUT="../output/"

ffmpeg -framerate $(eval "cat ${OUTPUT}framerate.var") -i ../output/%d.ppm -c:v libx264 -crf 15 -pix_fmt yuv420p ../output/output.mp4 -y

# open the rendered mp4
if command -v cvlc &> /dev/null
then
	cvlc ../output/output.mp4 --loop
else
	open ../output/output.mp4
fi

