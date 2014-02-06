#include "findCircle.h"

int main(int argc, char ** argv){
    CImg<UINT8> image(argv[1]);
    CImg<UINT8> backup = image;
    /*
    vector<circle> circles;
   	circles = whereBall(image);

    for (int i = 0; i < circles.size(); ++i){
    	backup.draw_circle(circles[i].x,circles[i].y,circles[i].r,red);
    }

    backup.save("output.jpg");
    */

    image = threshhold(image);
    image.save_jpeg("threshhold.jpg");
    return 0;   
}
