#!/bin/bash

ffmpeg -framerate 25 -i traceratops_animation_%d.png -f mp4 -pix_fmt yuv420p traceratops_video.mp4

