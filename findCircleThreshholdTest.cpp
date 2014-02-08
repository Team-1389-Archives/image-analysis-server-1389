#include "findCircle.h"
#include <sstream>
#include <string>

int main(int argc, char ** argv){
    CImg<UINT8> image;
    stringstream ss;
    string imageNum;

    for (int i = 1; i <= 68; i++){
   	    ss << i;
   	    ss >> imageNum;
   	    ss.clear();
   	    image.load_jpeg(("../TestImages/Image-" + imageNum + ".jpg").c_str());
   	    threshhold(image).save_jpeg(("../threshholdOutputImages/ThreshedImage-" + imageNum + ".jpg").c_str());
    }
    
    return 0;   
}