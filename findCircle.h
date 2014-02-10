#include <stdint.h>
typedef uint8_t UINT8;
#include <iostream>
#include <string>
#include <vector>
#include <stdlib.h>
#include <math.h>
#include <array>

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
    bool isBlue();
};

struct hsv{
    float h;
    float s;
    float v;
    bool compare(hsv other, float maxDistance);//Consider making other a reference
    bool compareToColor(float h, float maxHVariance, float minS);
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
    circle isCircle(int imageWidth);
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

class BallFinder{
public:
    BallFinder();

    CImg<UINT8> threshhold(CImg<UINT8>& image); //uint8_t 

    CImg<unsigned char> booleanEdgeDetect(CImg<unsigned char>& image);//Be careful about returning a CImg object, since it may copy the object

    vector<outline> findOutlines(CImg<UINT8> image);//Same worry about copying here (though I'm not sure if ti does end up copying)

    outline floodfill(CImg<UINT8>& image, int startX, int startY);

    void dispOutlines(CImg<UINT8>& image, vector<outline> outlines);

    vector<circle> whereBall(CImg<UINT8>& image);

private:
    rgb WHITE; 
    rgb BLACK;
    float ballHValue;
    hsv BALL_BLUE;
    int imageWidth;//used because some operations need to work proportional to image size
};

inline float MIN3(float x, float y, float z){return (y <= z ? (x <= y ? x : y) : (x <= z ? x : z));}

inline float MAX3(float x,float y,float z)  {return (y >= z ? (x >= y ? x : y) : (x >= z ? x : z));}

inline int abs(int num){return (num < 0)?-num:num;}//math.h only has fabs() for doubles

inline rgb getRgb(CImg<UINT8>& image,int x ,int y){//Here you use references, why not in other places? Be careful about using unsigned char. Since you want an unsigned 8-bit integer, consider using uint8_t instead.
    rgb pixel;
    pixel.r = image(x,y,0);
    pixel.g = image(x,y,1);
    pixel.b = image(x,y,2);
    return pixel;
}

inline void setRgb(CImg<UINT8>& image,int x ,int y, rgb color){
    image(x,y,0) = color.r;
    image(x,y,1) = color.g;
    image(x,y,2) = color.b;
}

inline float square(float num){return num * num;}

line findPerpendicularLine(point p1, point p2);

precisePoint findIntersection(line l1, line l2);

precisePoint findEquidistant(point p1, point p2, point p3);