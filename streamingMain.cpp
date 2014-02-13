#include "camera.h"
#include <CImg.h>
#include <iostream>
#include <cstdlib>
#include "findCircle.h"
//#define cimg_use_jpeg 1
//#include "loadCameraModded.h"

using namespace std;
using namespace cimg_library;

int main(){

    unsigned char GREEN[] = {0,255,0};

    //CvCapture *camera=cvCreateCameraCapture(3);
    //cvSetCaptureProperty(camera, CV_CAP_PROP_FRAME_WIDTH, 100);

    /*
    if(camera==NULL){
        cerr<<"Unable to open camera stream"<<endl;
        abort();
    }*/
    Camera cam("/dev/video0", 640, 480);
    CImg<UINT8> image;
    
    cam.load(image);
    image.resize_halfXY();

    CImgDisplay disp;

    image.display(disp);

    CImg<UINT8> modifiedImage;
    
    BallFinder finder;

    vector<circle> cs;
    circle biggest;

    while(!disp.is_closed()){
        cam.load(image);
        image.resize_halfXY();
        
        modifiedImage = image;
        cs = finder.whereBall(modifiedImage);
        biggest.r = -1;
        for (unsigned int i = 0; i < cs.size(); i++){//find biggest circle
            if (cs[i].r > biggest.r)
                biggest = cs[i];
        }
        

        //image = threshhold(image, BALL_BLUE);
        //image = image.blur(image.width()/300);
        //cout << "width:" << image.width() /80 << endl;
        //image = booleanEdgeDetect(image);

        if (biggest.r != -1){
            cout << biggest.x << " " << biggest.y << " " << biggest.r << '\n';
            cout.flush();
        }
        image.draw_circle(biggest.x,biggest.y,biggest.r,GREEN);
        image = finder.threshhold(image);
        image.display(disp);
    }

    return 0;
}
