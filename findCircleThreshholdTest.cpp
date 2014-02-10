#include "findCircle.h"
#include <sstream>
#include <string>

int main(int argc, char ** argv){
<<<<<<< HEAD
    CImg<UINT8> image(argv[1]);
    CImg<UINT8> backup = image;
    /*
=======
    BallFinder finder;
    unsigned char green[3] = {0,255,0};

    CImg<UINT8> image;
    CImg<UINT8> orig;
    stringstream ss;
    string imageNum;

>>>>>>> bd88a13e3efc152fb9f182adf3c055b1614cdc84
    vector<circle> circles;
    vector<outline> outlines;
    circle c;

    for (int i = 1; i <= 68; i++){
   	    ss << i;
   	    ss >> imageNum;
   	    ss.clear();
   	    image.load_jpeg(("../TestImages/Image-" + imageNum + ".jpg").c_str());
        orig = image;

   	    image = finder.threshhold(image);
        image.save_jpeg(("../threshholdOutputImages/ThreshedImage-" + imageNum + ".jpg").c_str());

        image.blur(image.width()/100);  //minimal blurring needed for red, width/200 for blue
        image.save_jpeg(("../blurredOutputImages/BlurredImage-" + imageNum + ".jpg").c_str());

        image = finder.booleanEdgeDetect(image);
        image.save_jpeg(("../edgedOutputImages/EdgedImage-" + imageNum + ".jpg").c_str());

        outlines = finder.findOutlines(image);

        circles.clear();

        for (unsigned int i = 0; i < outlines.size(); ++i){
          c = outlines[i].isCircle(image.width());
          if (c.r != -1){
            circles.push_back(c);
          }
        }

        for (unsigned int i = 0; i < circles.size(); i++){
          orig.draw_circle((int)circles[i].x,(int)circles[i].y,(int)circles[i].r, green, 0.5);
        }

<<<<<<< HEAD
    backup.save("output.jpg");
    */

    image = threshhold(image);
    image.save_jpeg("threshhold.jpg");
=======
        orig.save_jpeg(("../circleOutputImages/CircleImage-" + imageNum + ".jpg").c_str());

      }
    
>>>>>>> bd88a13e3efc152fb9f182adf3c055b1614cdc84
    return 0;   
}
