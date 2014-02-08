#include "findCircle.h"
#include <vector>

int main(int argc, char ** argv){
    CImg<UINT8> image(argv[1]);
    vector<circle> cs = whereBall(image);
    for (unsigned int i = 0; i < cs.size(); i++){
    	cout << cs[i].x << ' ' << cs[i].y << ' ' << cs[i].r << endl;
    }
    return 0;   
}