#define cimg_use_opencv
#include <CImg.h>
#include <iostream>
#include "findCircle.h"
#define cimg_use_jpeg 1

using namespace std;
using namespace cimg_library;

int main(){

    CImg<UINT8> image;
    image.load_camera(0);

    CImgDisplay disp;

    image.display(disp);

    CImg<UINT8> modifiedImage;

    circle c;

    while(!disp.is_closed()){
        image.load_camera();
        modifiedImage = image;
        c = whereBall(modifiedImage);
        if (c.x != -1)
            cout << c.x << " " << c.y << " " << c.r << endl;
        image.draw_circle(c.x,c.y,c.r,red);
        image.display(disp);
    }

    return 0;
}
