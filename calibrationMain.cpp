#define cimg_use_opencv

#include "CImg.h"
#include <iostream>



using namespace cimg_library;
using namespace std;

const uint8_t RED[3] = {0,255,0};

int main(){

    CImg<uint8_t> origImage;
    origImage.load_camera();
    CImg<uint8_t> image = origImage;

    CImgDisplay disp(image, "click top left");

    while (!disp.button()) {
        disp.wait();
    }

    int y1 = disp.mouse_y();
    int x1 = disp.mouse_x();

    while (disp.button()) {
        disp.wait();
    }

    disp.set_title("click bottom right");

    while (!disp.button()) {
        disp.wait();
        image = origImage;
        image.draw_rectangle(x1,y1,disp.mouse_x(), disp.mouse_y(), RED, 0.2);
        image.display(disp);
    }

    int x2 = disp.mouse_x();
    int y2 = disp.mouse_y();

    while (disp.button()) {
        disp.wait();
    }

    long totalR = 0, totalG = 0, totalB = 0, r = 0, g = 0, b = 0;

    for (int x = x1; x <= x2; ++x){
        for (int y = y1; y <= y2; ++y){
            totalR += origImage(x,y,0);
            totalG += origImage(x,y,1);
            totalB += origImage(x,y,2);
        }
    }

    int area = (x2 - x1 + 1) * (y2 - y1 + 1);
    r = totalR/area;
    g = totalG/area;
    b = totalB/area;

    cout << "Average rgb:" << endl;
    cout << "R:" << r << endl;
    cout << "G:" << g << endl;
    cout << "B:" << b << endl;

    return 0;
}
