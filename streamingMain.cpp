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

    Camera cam("/dev/video3", 640, 480);
    CImg<UINT8> image;

    CImgDisplay disp;

    CImg<UINT8> modifiedImage;
    
    BallFinder finder;

    vector<circle> cs;
    circle biggest;
    int width, height;
    uint8_t *data;
    do{
        cam.load(&data, &width, &height);
        uint8_t *out_data=new uint8_t[width*height];
        
        finder.filteringSystem(data, width, height, out_data);
        image.assign(width, height, 1, 3);
        UINT8
            *ptr_r = image.data(0,0,0,0),
            *ptr_g = image.data(0,0,0,1),
            *ptr_b = image.data(0,0,0,2);
          for(int i=0;i<(width*height);i++){
            *(ptr_r++) = data[(i*3)+1];
            *(ptr_g++) = out_data[i];
            *(ptr_b++) = 0;
          }
        //memcpy(image.data(0,0,0,0), out_data, width*height);
        modifiedImage=image;
        /*cs = finder.whereBall(modifiedImage);
        biggest.r = -1;
        for (unsigned int i = 0; i < cs.size(); i++){//find biggest circle
            if (cs[i].r > biggest.r)
                biggest = cs[i];
            image.draw_circle(cs[i].x,cs[i].y,cs[i].r,GREEN,0.5f);
        }

        if (biggest.r != -1){
            cout << biggest.x << " " << biggest.y << " " << biggest.r << '\n';
            cout.flush();
        }*/
        //image = finder.threshhold(image);
        //image.blur(image.width()/100);
        //image = finder.booleanEdgeDetect(image);
        image.display(disp);
        delete[] out_data;
    }while(!disp.is_closed());

    return 0;
}
