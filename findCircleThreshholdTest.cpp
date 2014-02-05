#include "findCircle.h"

int main(int argc, char ** argv){
    CImg<UINT8> imageOrig(argv[1]);
    CImg<UINT8> image = imageOrig;
    circle c = whereBall(image);
	imageOrig.draw_circle(c.x,c.y,c.r,red);
    imageOrig.save_jpeg("thresh.jpg");
    return 0;   
}