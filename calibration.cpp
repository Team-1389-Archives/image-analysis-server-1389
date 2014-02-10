#include "calibration.h"

ColorGrid* getColorCalibration(CImg<uint8_t>& image, int x1, int y1, int x2, int y2){
    ColorGrid* colors = new ColorGrid;
    hsv pixelColor;

    for (int x = x1; x <= x2; ++x){
        for (int y = y1; y <= y2; ++y){
            pixelColor = getRgb(image,x,y).getHsv();
            colors->setColor(pixelColor.h, pixelColor.s, true);
        }
    }

    return colors;
}

ColorGrid* askCalibration(CImg<uint8_t>& origImage){
    char RED[] = {255,0,0};

    CImg<uint8_t> image = origImage;

    CImgDisplay disp(image);

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

    ColorGrid* colors = getColorCalibration(origImage,x1,y1,x2,y2);

    return colors;
}
