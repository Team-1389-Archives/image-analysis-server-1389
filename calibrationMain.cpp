//#define cimg_use_opencv  //when display
#include "calibration.h"

int main(){
    CImg<uint8_t> image("calibration.jpg");
    
    ColorGrid* colors = askCalibration(image);
    
    colors->display();
    
    BallFinder findy;
    findy.setColorGrid(colors);
    
    image = findy.threshhold(image);
    
    image.save_jpeg("threshed.jpg");
    
    delete[] colors;
}
