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
gcc -g -c filtering.c -O3 `pkg-config --cflags ck` -D_GNU_SOURCE -D_XOPEN_SOURCE=600 -D_BSD_SOURCE -std=gnu99 -pedantic -Wall -W -Wundef -Wendif-labels -Wshadow -Wpointer-arith -Wcast-align -Wwrite-strings -Wstrict-prototypes -Wmissing-prototypes -Wnested-externs -Winline -Wdisabled-optimization -fstrict-aliasing -pipe -Wno-parentheses -Werror -Wno-unused-parameter -DCORES=4 -Wno-unused-variable $PROFILING_FLAGS
if test $? != 0 ; then
    exit 1
fi
exec $CXX --std=c++0x -g $PROFILING_FLAGS -o streamingMain streamingMain.cpp findCircle.cpp filtering.o -ljpeg -pthread $EXTRA_FLAGS -lX11 -lv4l2 `pkg-config --libs ck`
