//#define cimg_use_opencv  //when display
#ifndef cimg_use_display
#define cimg_use_display 1;
#endif

#include "CImg.h"
#include <iostream>
#include "findCircle.h"

using namespace cimg_library;
using namespace std;

ColorGrid* getColorCalibration(CImg<uint8_t>& image, int x1, int y1, int x2, int y2);

ColorGrid* askCalibration(CImg<uint8_t>& image);
