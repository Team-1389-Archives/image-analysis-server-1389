#!/bin/sh
if [ -z $CC ]; then
	if type clang >> /dev/null 2> /dev/null; then
		CC="clang"
	else
		CC="gcc"
	fi
fi
exec $CC -g -o server -O3 --std=c99 -Wall -Werror main.c -lev
