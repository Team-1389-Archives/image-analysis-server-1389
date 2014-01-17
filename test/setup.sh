#!/bin/sh
cd `dirname $0`
npm install .
cd static
bower install
cd ..
node server.js