#include "findCircle.h"
#include <stdint.h>

#define cimg_use_display
#include <CImg.h>

using namespace cimg_library;

int main(int argc, char ** argz){
	BallFinder findy;
	CImg<uint8_t> image(argz[1]), out;

	out = findy.floodThresh(image);
	
	out.save_jpeg("output.jpg");

	return 0;
}