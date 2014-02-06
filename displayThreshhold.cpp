#define cimg_use_opencv
#include <CImg.h>
#include <iostream>
#include <cstdlib>
#include "findCircle.h"
#include "cv.h"
#include <sstream>
//#define cimg_use_jpeg 1
//#include "loadCameraModded.h"

using namespace std;
using namespace cimg_library;

static void cimg_from_opencv(CImg<UINT8> &dst, const IplImage *img){
    typedef UINT8 T;
    const int step = (int)(img->widthStep - 3*img->width);
    dst.assign(img->width, img->height, 1,3);
    const unsigned char* ptrs = (unsigned char*)img->imageData;
    T *ptr_r = dst.data(0,0,0,0), *ptr_g = dst.data(0,0,0,1), *ptr_b = dst.data(0,0,0,2);
    if (step>0){
        for(int y=0;y<dst.height();y++){
            for(int x=0;x<dst.width();x++){
                *(ptr_b++) = (T)*(ptrs++); *(ptr_g++) = (T)*(ptrs++); *(ptr_r++) = (T)*(ptrs++);
            }
            ptrs+=step;
        }
    }else{
        for (unsigned long siz = (unsigned long)img->width*img->height; siz; --siz) {
            *(ptr_b++) = (T)*(ptrs++); *(ptr_g++) = (T)*(ptrs++); *(ptr_r++) = (T)*(ptrs++);
        }
    }
}

static void load_cam(CImg<UINT8> &image, CvCapture *camera){
    const IplImage *img=cvQueryFrame(camera);
    cimg_from_opencv(image, img);
}

int main(int argc, char ** argv){
    
    stringstream ss;
    
    ss << argv[1];
    
    ss >> ballHValue; 

    CvCapture *camera=cvCreateCameraCapture(3);
    //cvSetCaptureProperty(camera, CV_CAP_PROP_FRAME_WIDTH, 100);

    if(camera==NULL){
        cerr<<"Unable to open camera stream"<<endl;
        abort();
    }

    CImg<UINT8> image;
    load_cam(image, camera);


    CImgDisplay disp;

    image.display(disp);

    CImg<UINT8> modifiedImage;

    while(!disp.is_closed()){
        load_cam(image, camera);

        image = threshhold(image);


        image.display(disp);
    }

    return 0;
}
