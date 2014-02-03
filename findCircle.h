#include <stdint.h>
typedef uint8_t UINT8;
#include <iostream>
#include <string>
#include <vector>
#include <stdlib.h>
#include <math.h>

//#define cimg_use_jpeg 1


#ifndef cimg_display
#define cimg_display 0
#endif // cimg_display

#include "CImg.h"

using namespace std;
using namespace cimg_library;

struct hsv;
struct rgb;
struct point;
struct outline;
struct possibleCenter;
struct circle;
struct line;
struct precisePoint;


struct precisePoint{
    float x;
    float y;
    float distanceTo(point);
};

struct circle{
    float x;
    float y;
    float r;
};

struct rgb{
    UINT8 r;
    UINT8 g;
    UINT8 b;
    hsv getHsv();//gets equivilent hsv
};

struct hsv{
    float h;
    float s;
    float v;
    bool compare(hsv other, float maxHVariance, float maxSVariance);//Consider making other a reference
};


struct point{
    
    point(int a,int b){
        x = a;
        y = b;
    }
    point(){}
    int x;
    int y;
};

struct outline{                      
    vector<point> points;
    void addPoint(point newPoint){
        points.push_back(newPoint);    // may want to move to findCircle.cpp, or even a file for small functions
    }
    circle isCircle();
};

struct possibleCenter{
    float x;
    float y;
    float r;
    int corroborations;
    bool compare(precisePoint p, float radius);
};

struct line{//ax+by+c=0  
    float a;
    float b;
    float c;
};

// All 3 defined in findCircle.cpp
extern rgb WHITE; 
extern rgb BLACK;
extern hsv BALL_BLUE;//used to compare against during threshholding


extern const UINT8 red[3];//this is an array instead of an rgb struct so it can be used with the cimg draw_circle(function)

extern int imageWidth;//used because some operations need to work proportional to image size

CImg<UINT8> threshhold(CImg<UINT8>& image, hsv color); //uint8_t 

inline int abs(int num){return (num < 0)?-num:num;};//math.h only has fabs() for doubles

inline rgb getRgb(CImg<unsigned char>& image,int x ,int y);//Why are some of these inline?

inline void setRgb(CImg<unsigned char>& image,int x ,int y, rgb color); //shouldn't it be consistently UINT8?

CImg<unsigned char> booleanEdgeDetect(CImg<unsigned char>& image);//Be careful about returning a CImg object, since it may copy the object

vector<outline> findOutlines(CImg<UINT8> image);//Same worry about copying here (though I'm not sure if ti does end up copying)

outline floodfill(CImg<UINT8>& image, int startX, int startY);

void dispOutlines(CImg<UINT8>& image, vector<outline> outlines);

line findPerpendicularLine(point p1, point p2);

precisePoint findIntersection(line l1, line l2);

precisePoint findEquidistant(point p1, point p2, point p3);

inline float square(float num){return num * num;}

circle whereBall(CImg<UINT8>& image);

inline int MIN3(int x, int y, int z){return (y <= z ? (x <= y ? x : y) : (x <= z ? x : z));}

inline int MAX3(int x,int y,int z)  {return (y >= z ? (x >= y ? x : y) : (x >= z ? x : z));}