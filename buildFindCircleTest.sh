#!/bin/sh
if [ -z $CXX ]; then
        if type clang++ >> /dev/null 2> /dev/null; then
                CXX="/usr/bin/env clang++"
        else
                CXX="/usr/bin/env g++"
        fi
fi
exec $CXX --std=c++0x -g -o findCircleTest -Wall -Werror `pkg-config --cflags --libs opencv` streamingMain.cpp findCircle.cpp -ljpeg 
