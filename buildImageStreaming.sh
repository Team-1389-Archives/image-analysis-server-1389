#!/bin/sh
if [ -z $CXX ]; then
        if type clang++ >> /dev/null 2> /dev/null; then
                CXX="/usr/bin/env clang++"
        else
                CXX="/usr/bin/env g++"
        fi
fi
#if [ "$1" == 'profile' ]; then
#    PROFILING_FLAGS='-pg'
#fi
if [ -d /usr/lib/arm-linux-gnueabi/ ]; then
	EXTRA_FLAGS='-L/usr/lib/arm-linux-gnueabi/'
fi
#`pkg-config --cflags opencv` -lopencv_calib3d -lopencv_video -lopencv_contrib -lopencv_highgui -lopencv_gpu -lopencv_core -lopencv_ml -lopencv_features2d -lopencv_imgproc -lopencv_legacy -lopencv_flann -lopencv_objdetect
$CXX --std=c++0x `pkg-config --cflags opencv` -g $PROFILING_FLAGS -o streamingMain streamingMain.cpp findCircle.cpp -ljpeg -pthread $EXTRA_FLAGS -lX11 `pkg-config --libs opencv`
