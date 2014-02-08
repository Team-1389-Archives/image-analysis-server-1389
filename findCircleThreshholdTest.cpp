#include "findCircle.h"
#include <sstream>
#include <string>

int main(int argc, char ** argv){
    CImg<UINT8> image;
    CImg<UINT8> orig;
    stringstream ss;
    string imageNum;

    vector<circle> circles;
    vector<outline> outlines;
    circle c;

    for (int i = 1; i <= 68; i++){
   	    ss << i;
   	    ss >> imageNum;
   	    ss.clear();
   	    image.load_jpeg(("../TestImages/Image-" + imageNum + ".jpg").c_str());
        orig = image;

   	    image = threshhold(image);
        image.save_jpeg(("../threshholdOutputImages/ThreshedImage-" + imageNum + ".jpg").c_str());

        image.blur(image.width()/100);
        image.save_jpeg(("../blurredOutputImages/BlurredImage-" + imageNum + ".jpg").c_str());

        image = booleanEdgeDetect(image);
        image.save_jpeg(("../edgedOutputImages/EdgedImage-" + imageNum + ".jpg").c_str());

        outlines = findOutlines(image);

        circles.clear();

        for (unsigned int i = 0; i < outlines.size(); ++i){
          c = outlines[i].isCircle();
          if (c.r != -1){
            circles.push_back(c);
          }
        }

        for (unsigned int i = 0; i < circles.size(); i++){
          orig.draw_circle((int)circles[i].x,(int)circles[i].y,(int)circles[i].r, red, 0.5);
        }

        orig.save_jpeg(("../circleOutputImages/CircleImage-" + imageNum + ".jpg").c_str());

      }
    
    return 0;   
}
