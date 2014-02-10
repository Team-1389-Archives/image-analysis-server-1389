//#define cimg_use_opencv  //when display
#define cimg_use_display 1;

#include "CImg.h"
#include <iostream>
#include "findCircle.h"



using namespace cimg_library;
using namespace std;

const uint8_t RED[3] = {0,255,0};

int main(){

    CImg<uint8_t> origImage;
<<<<<<< HEAD
    origImage.load_camera(3);
    CImg<uint8_t> image = origImage;
=======



    //origImage.load_camera();  //turn on when load from camera
    origImage.load_jpeg("calibrate.jpg");



    CImg<uint8_t> image = origImage;//turn on when from jpg
>>>>>>> ad7adce357ce031e98044fdc665d66b0ad2651b2

    CImgDisplay disp(image, "click when ball is in front of camera");

    /*
    while (!disp.button()) {  //turn on when camera
        //disp.wait();
        origImage.load_camera(3);
        origImage.display(disp);
    }

<<<<<<< HEAD
    while (disp.button()) {
        origImage.load_camera(3);
=======
    while (disp.button()) {   //turn on when camera
        origImage.load_camera();
>>>>>>> ad7adce357ce031e98044fdc665d66b0ad2651b2
        origImage.display(disp);
    }*/

    disp.set_title("click top left");

    while (!disp.button()) {}

    int y1 = disp.mouse_y();
    int x1 = disp.mouse_x();

    while (disp.button()) {}

    disp.set_title("click bottom right");

    while (!disp.button()) {
        image = origImage;
        image.draw_rectangle(x1,y1,disp.mouse_x(), disp.mouse_y(), RED, 0.2);
        image.display(disp);
    }

    int x2 = disp.mouse_x();
    int y2 = disp.mouse_y();

    while (disp.button()) {}

    ColorGrid colors;
    hsv pixelColor;

    for (int x = x1; x <= x2; ++x){
        for (int y = y1; y <= y2; ++y){
            pixelColor = getRgb(origImage,x,y).getHsv();
            colors.setColor(pixelColor.h, pixelColor.s, true);
        }
    }

    colors.display();

    return 0;
}
