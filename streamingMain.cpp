#include "camera.h"
#include <CImg.h>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <signal.h>
#include "findCircle.h"
//#define cimg_use_jpeg 1
//#include "loadCameraModded.h"

using namespace std;
using namespace cimg_library;

static bool running=true;

template<typename T>
class cpp11_owning_ptr{//Shim for the cpp11 owning_ptr
public:
    cpp11_owning_ptr(T* ptr)
        : m_ptr(ptr)
    {}
    virtual ~cpp11_owning_ptr(){
        if(m_ptr!=NULL){
            delete m_ptr;
        }
    }
    T* get() const{return m_ptr;}
private:
    T* m_ptr;
};

static void stop_running(int sig){
    running=false;
}

int main(int argc, char** argv){
    signal(SIGINT, stop_running);
    signal(SIGTERM, stop_running);
    
    bool should_display=(argc>1 && strcmp(argv[1], "display")==0);
    unsigned char GREEN[] = {0,255,0};

    //CvCapture *camera=cvCreateCameraCapture(3);
    //cvSetCaptureProperty(camera, CV_CAP_PROP_FRAME_WIDTH, 100);

    /*
    if(camera==NULL){
        cerr<<"Unable to open camera stream"<<endl;
        abort();
    }*/
    ConfigReader config("config.txt");
    int exposure=config.getIntDefault("exposure", 25);
    int whitepoint=config.getIntDefault("whitepoint", 3000);
    cerr<<"Exposure is "<<exposure<<endl;
    cerr<<"Whitepoint is "<<whitepoint<<endl;
    Camera cam("/dev/video3", 640, 480,
        exposure, whitepoint);
    CImg<UINT8> image;
    cpp11_owning_ptr<CImgDisplay> disp(should_display?new CImgDisplay():NULL);

    CImg<UINT8> modifiedImage;
    
    BallFinder finder(config);

    vector<circle> cs;
    circle biggest;
    int width, height;
    uint8_t *data;
    uint8_t *out_data=NULL;
    cerr.flush();
    do{
        cam.load(&data, &width, &height);
        if(out_data==NULL){
            out_data=new uint8_t[width*height];
        }
        
        finder.filteringSystem(data, width, height, out_data);
        image.assign(out_data, width, height, 1, 1, true);
        //memcpy(image.data(0,0,0,0), out_data, width*height);
        modifiedImage=image;
        cs = finder.whereBall(modifiedImage);
        biggest.r = -1;
        for (unsigned int i = 0; i < cs.size(); i++){//find biggest circle
            if (cs[i].r > biggest.r)
                biggest = cs[i];
            image.draw_circle(cs[i].x,cs[i].y,cs[i].r,GREEN);
        }

        if (biggest.r != -1){
            cout << biggest.x << " " << biggest.y << " " << biggest.r << '\n';
        }else{
            cout << "0 0 -1\n";
        }
        cout.flush();
        //image = finder.threshhold(image);
        //image.blur(image.width()/100);
        //image = finder.booleanEdgeDetect(image);
        if(should_display){
            image.display(*disp.get());
        }
        if (should_display && disp.get()->is_closed()){
            running = false;
        }
    }while(running);

    return 0;
}
