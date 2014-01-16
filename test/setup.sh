#!/bin/sh
cd `dirname $0`
cd ..
if ./buildFindCircleTest.sh; then
else
	echo "Build Find Circle Test failed"
	exit 1
fi
npm install .
cd static
bower install
cd ..
node server.js