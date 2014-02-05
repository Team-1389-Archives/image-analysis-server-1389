#define cimg_use_display 0

#include <CImg.h>
#include "findCircle.h"
#include <stdint.h>

int main(int args, char ** argz){
	CImg<uint8_t> image;
	image.load_jpeg(argz[1]);
	image = threshhold(image, BALL_BLUE);
	image.save_jpeg("threshhold.jpg");
	for (int i = 0 ; i < 10; i++){
		image = dialate(image);
	}
	image.save_jpeg("dialate.jpg");
	for (int i = 0 ; i < 10; i++){
		image = erode(image);
	}
	image.save_jpeg("erode.jpg");

}