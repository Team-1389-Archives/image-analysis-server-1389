#include "findCircle.h"

static const rgb WHITE={255,255,255};
static const rgb BLACK={0,0,0};

inline float MIN3(float x, float y, float z){return (y <= z ? (x <= y ? x : y) : (x <= z ? x : z));}

inline float MAX3(float x,float y,float z)  {return (y >= z ? (x >= y ? x : y) : (x >= z ? x : z));}

inline int abs(int num){return (num < 0)?-num:num;}//math.h only has fabs() for doubles

inline float square(float num){return num * num;}

BallFinder::BallFinder(){
    m_filtering_system=FilteringSystemNew(//16384
        500,
	    10000,
	    150
    );
}
BallFinder::~BallFinder(){
    FilteringSystemClose(m_filtering_system);
}

float precisePoint::distanceTo(point p){
    return sqrt(         square((((float) p.x ) - x))     +    square((((float) p.y ) - y))          );
}

bool possibleCenter::compare(precisePoint p, float radius){
    bool isCloseEnough  = true;
    int allowedDifference = r/4;///*********************************************************************************************
    if (abs(p.x - x) > allowedDifference) //remember to try changing this number if circle detection not working
        isCloseEnough = false;
    else if (abs(p.y - y) > allowedDifference)
        isCloseEnough = false;
    else if (abs(r - radius) > allowedDifference)
        isCloseEnough = false;
    if (isCloseEnough){
        x = (x * corroborations + p.x) / (corroborations + 1);
        y = (y * corroborations + p.y) / (corroborations + 1);
        r = (r * corroborations + radius) / (corroborations + 1);
        corroborations++;
    }
    return isCloseEnough;
};

circle outline::isCircle(int imageWidth){//return circle with x,y, and r of -1 if not a circle
    circle ret = {-1,-1,-1};
    vector<possibleCenter> possCenters;
    if (points.size() < (unsigned int)(imageWidth/50))//if too small to do calculations on, immediately returns that not a circle
        return ret;
    point p1, p2 ,p3;
    precisePoint centerCandidate;
    possibleCenter newCenter;
    p1 = points[rand() % points.size()];
    p2 = points[rand() % points.size()];
    p3 = points[rand() % points.size()];
    centerCandidate = findEquidistant(p1, p2, p3);
    float avgDist = 0;
    avgDist = (centerCandidate.distanceTo(p1) + centerCandidate.distanceTo(p2) + centerCandidate.distanceTo(p3)) / 3;
    newCenter = {centerCandidate.x, centerCandidate.y, avgDist, 1};
    possCenters.push_back(newCenter);
    bool pairedWithPreexistingCenter;
    for (unsigned int i = 0; i < points.size(); ++i){//this for loop runs more times for bigger shapes
        p1 = points[rand() % points.size()];
        p2 = points[rand() % points.size()];
        p3 = points[rand() % points.size()];
        centerCandidate = findEquidistant(p1, p2, p3);
        if (isnan(centerCandidate.x) || isnan(centerCandidate.y) || !isfinite(centerCandidate.x) || !isfinite(centerCandidate.y))//if any of these, it means that some of p1, p2, p3 are either: colinear, the same point, or I screwed up the math. in any case, it is best to test for it
            continue;
        pairedWithPreexistingCenter = false;
        avgDist = (centerCandidate.distanceTo(p1) + centerCandidate.distanceTo(p2) + centerCandidate.distanceTo(p3)) / 3.0f;
        for (unsigned int j = 0; j < possCenters.size(); ++j){
            if (possCenters[j].compare(centerCandidate, avgDist)){
                pairedWithPreexistingCenter = true;
                break;
            }
        }
        if (!pairedWithPreexistingCenter){
            newCenter = {centerCandidate.x, centerCandidate.y, avgDist, 1};
            possCenters.push_back(newCenter);
        }
    }
    int whichCenter = -1;
    for (unsigned int k = 0; k < possCenters.size(); ++k){
        if (possCenters[k].corroborations > ((int)points.size() / 2)){//if more than half of centers corroborate each other   //original: /2
            whichCenter = k;
            break;
        }
    }
    if (whichCenter == -1)
        return ret;
    ret.r = possCenters[whichCenter].r;
    ret.x = possCenters[whichCenter].x;
    ret.y = possCenters[whichCenter].y;
    return ret;
}

bool hsv::compareToColor(float colorH, float maxHVariance, float minS){
    if (abs(h - colorH) > maxHVariance)
        return false;
    if (s < minS)
        return false;
    return true;
}

vector<circle> BallFinder::whereBall(CImg<UINT8>& image){
    imageWidth = image.width();
    //image = threshhold(image);
    //image.blur(imageWidth/100);
    //image = booleanEdgeDetect(image);
    vector<outline> outlines = findOutlines(image);
    vector<circle> circles;
    circle c;
    for (unsigned int i = 0; i < outlines.size(); ++i){
        c = outlines[i].isCircle(imageWidth);
        if (c.r != -1){
            circles.push_back(c);
        }
    }
    return circles;
}

bool rgb::isBlue(){
    if (r > b)
        return false;
    if (g > 1.5f*b)
        return false;
    return true;
}

void BallFinder::filteringSystem(uint8_t* data, int w, int h, uint8_t *out){
    FilteringSystemFilter(m_filtering_system, w, h, data, out);
}

// algorithm from http://www.cs.rit.edu/~ncs/color/t_convert.html
hsv rgb::getHsv()
{
    float r_ = ((float)r)/255.0f;
    float g_ = ((float)g)/255.0f;
    float b_ = ((float)b)/255.0f;
    hsv ret;
    float min, max, delta;
    min = MIN3( r_, g_, b_ );
    max = MAX3( r_, g_, b_ );
    ret.v = max;                // v
    delta = max - min;
    if( max != 0 )
        ret.s = delta / max;        // s
    else {
        // r = g = b = 0        // s = 0, v is undefined
        ret.s = 0;
        ret.h = -1;
        return ret;
    }
    if( r_ == max )
        ret.h = ( g_ - b_ ) / delta;        // between yellow & magenta
    else if( g_ == max )
        ret.h = 2 + (( b_ - r_ ) / delta);  // between cyan & yellow
    else
        ret.h = 4 + ( r_ - g_ ) / delta;    // between magenta & cyan
    ret.h *= 60;                // degrees
    if( ret.h < 0 )
        ret.h += 360;
    return ret;
}

bool hsv::compare(hsv other, float maxHDist, float maxSDist, float maxVDist){
    if (abs(other.h - h) > maxHDist)
        return false;
    if (abs(other.s - s) > maxSDist)
        return false;
    if (abs(other.v - v) > maxVDist)
        return false;
    return true;
}

outline BallFinder::floodfill(CImg<UINT8>& image, int startX, int startY){
    outline ret;
    point currentPoint;
    vector<point> possiblePoints;//I like that you didn't use the stack, and used a vector instead.
    possiblePoints.push_back(point(startX,startY));
    while(!possiblePoints.empty()){
        currentPoint = possiblePoints[possiblePoints.size()-1];
        possiblePoints.pop_back();
        if (image(currentPoint.x, currentPoint.y,0) == 255){
            ret.addPoint(currentPoint);
            if (currentPoint.x != 0)
                possiblePoints.push_back(point(currentPoint.x-1, currentPoint.y));
            if (currentPoint.x != image.width() - 1)
                possiblePoints.push_back(point(currentPoint.x+1, currentPoint.y));
            if (currentPoint.y != 0)
                possiblePoints.push_back(point(currentPoint.x, currentPoint.y-1));
            if (currentPoint.y != image.height()- 1)
                possiblePoints.push_back(point(currentPoint.x, currentPoint.y+1));
            if (currentPoint.x != 0 && currentPoint.y != 0)
                possiblePoints.push_back(point(currentPoint.x-1, currentPoint.y-1));
            if (currentPoint.x != 0 && currentPoint.y != image.height() - 1)
                possiblePoints.push_back(point(currentPoint.x-1, currentPoint.y+1));
            if (currentPoint.x != image.width() - 1 && currentPoint.y != 0)
                possiblePoints.push_back(point(currentPoint.x+1, currentPoint.y-1));
            if (currentPoint.x != image.height()- 1 && currentPoint.y != image.height() - 1)
                possiblePoints.push_back(point(currentPoint.x+1, currentPoint.y+1));
        }
        image(currentPoint.x, currentPoint.y,0) = 0;
    }
    return ret;
}

vector<outline> BallFinder::findOutlines(CImg<UINT8> image){
    vector<outline> outlines;
    for (int x = 0; x < image.width() - 1; ++x){
        for (int y = 0; y< image.height() - 1; y++){
            if (image(x,y,0) == 255){
                outlines.push_back(floodfill(image,x,y));
            }
        }
    }
    return outlines;
}

line findPerpendicularLine(point p1, point p2){
    float x1 = (float) p1.x;
    float x2 = (float) p2.x;
    float y1 = (float) p1.y;
    float y2 = (float) p2.y;
    line ret;
    ret.a = 2*x2 - 2*x1;
    ret.b = 2*y2 - 2*y1;
    ret.c = x1*x1 + y1*y1 - x2*x2 - y2*y2;
    return ret;
}

precisePoint findIntersection(line l1, line l2){
    if (l1.b == 0){
        line l3 = l2;
        l2 = l1;
        l1 = l3;
    }
    float x = (l1.b * l2.c - l2.b * l1.c)/(l1.a * l2.b - l1.b * l2.a);
    float y = -(l1.a * x + l1.c) / l1.b;
    precisePoint ret = {x,y};
    return ret;
}

precisePoint findEquidistant(point p1, point p2, point p3){
   line l1 = findPerpendicularLine(p1,p2);
   line l2 = findPerpendicularLine(p2,p3);
   return findIntersection(l1,l2);
}
