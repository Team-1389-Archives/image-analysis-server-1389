#ifndef FINDCIRCLE_H
#define FINDCIRCLE_H

#include <stdint.h>
typedef uint8_t UINT8;
#include <iostream>
#include <string>
#include <vector>
#include <stdlib.h>
#include <math.h>
#include <array>
#include "filtering.h"

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
    bool compare(hsv other, float maxHDist, float maxSDist, float maxVDist);//Consider making other a reference
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
    virtual ~BallFinder();

    vector<outline> findOutlines(CImg<UINT8> image);//Same worry about copying here (though I'm not sure if ti does end up copying)

    outline floodfill(CImg<UINT8>& image, int startX, int startY);

    vector<circle> whereBall(CImg<UINT8>& image);

    void floodFillObject(int x, int y, CImg<uint8_t>& image, CImg<uint8_t>& outputImage);

    void filteringSystem(uint8_t* data, int w, int h, uint8_t *out);

private:
    int imageWidth;//used because some operations need to work proportional to image size
    filtering_system_t m_filtering_system;
};

line findPerpendicularLine(point p1, point p2);

precisePoint findIntersection(line l1, line l2);

precisePoint findEquidistant(point p1, point p2, point p3);

#endif
