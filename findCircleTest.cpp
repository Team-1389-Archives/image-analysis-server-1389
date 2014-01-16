#include "findCircle.h"

int main(int argc, char ** argv){
    CImg<UINT8> image(argv[1]);
    circle c = whereBall(image);
    cout << c.x << ' ' << c.y << ' ' << c.r << endl;
    return 0;   
}