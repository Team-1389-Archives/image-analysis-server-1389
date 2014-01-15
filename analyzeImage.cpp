//to compile needs the following command line arguments: -ljpeg -std=gnu++0x
//also you need libjpeg and the cimg.h header in the same directory
#include <iostream>
#include <string>
#include <vector>
#include <stdlib.h>
#include <chrono>
#include <math.h>
//<stuff to do to use cimg>
#define cimg_display 0
#define cimg_use_jpeg 1

#include "cimg.h"
//</stuff to do to use cimg>

using namespace std;
using namespace std::chrono;
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
        points.push_back(newPoint);
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

rgb WHITE = {255,255,255};
rgb BLACK = {0,0,0};
hsv BALL_BLUE = {240,0.6,50};//used to compare against during threshholding
const UINT8 red[3] = {255,0,0};//this is an array instead of an rgb struct so it can be used with the cimg draw_circle(function)
int imageWidth;//used because some operations need to work proportional to image size


CImg<UINT8> threshhold(CImg<UINT8>& image, hsv color);
inline int abs(int num){return (num < 0)?-num:num;};//I think math.h already has an abs() function
CImg<unsigned char> edgeDetect(CImg<unsigned char>& image);//obsolete
inline rgb getRgb(CImg<unsigned char>& image,int x ,int y);//Why are some of these inline?
inline void setRgb(CImg<unsigned char>& image,int x ,int y, rgb color);
int difference(rgb px1, rgb px2);//obsolete
CImg<unsigned char> booleanEdgeDetect(CImg<unsigned char>& image);//Be careful about returning a CImg object, since it may copy the object
vector<outline> findOutlines(CImg<UINT8> image);//Same worry about copying here (though I'm not sure if ti does end up copying)
outline floodfill(CImg<UINT8>& image, int startX, int startY);
void dispOutlines(CImg<UINT8>& image, vector<outline> outlines);
line findPerpendicularLine(point p1, point p2);
precisePoint findIntersection(line l1, line l2);
precisePoint findEquidistant(point p1, point p2, point p3);
inline float square(float num){return num * num;}
inline float MIN3(float x,float y,float z){ return y <= z ? (x <= y ? x : y) : (x <= z ? x : z);}
inline float MAX3(float x,float y, float z)  {return y >= z ? (x >= y ? x : y) : (x >= z ? x : z);}

float precisePoint::distanceTo(point p){
    return sqrt(         square(((float) p.x ) - x)     +    square(((float) p.y ) - y)          );
}

bool possibleCenter::compare(precisePoint p, float radius){
    bool isCloseEnough  = true;
    int allowedDifference = r/4;
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

circle outline::isCircle(){//return circle with x,y, and r of -1 if not a circle
    circle ret = {-1,-1,-1};
    vector<possibleCenter> possCenters;
    if (points.size() < (unsigned int)(imageWidth/40))//if too small to do calculations on, immediately returns that not a circle
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
        if (possCenters[k].corroborations > ((int)points.size() / 2)){//if more than half of centers corroborate each other
                cout << possCenters[k].corroborations << "/" << points.size() << "=" << (float)possCenters[k].corroborations / (float)points.size() << endl;
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

int main(){
    srand(0);//seeded with 0 so same results every time
    cout << "file name:";//You may want to use command line arugments here. It makes it easier to automate.
    string file;
    getline(cin,file);
    CImg<UINT8> origonalImage(file.c_str());
    CImg<UINT8> inputImage = origonalImage;
    imageWidth = origonalImage.width();

    double totalTime = 0;

    //blur
    high_resolution_clock::time_point t1 = high_resolution_clock::now();//to time
    cout << "blurring image..."<< endl;
    inputImage.blur(imageWidth/100);
    high_resolution_clock::time_point t2 = high_resolution_clock::now();//I like the performance checking. However, it may be more advantageous, when optimizing, to use a profiler, which will tell you how much time was spent in each function.
    duration<double> time_span = duration_cast<duration<double>>(t2 - t1);//to time
    cout << "done blurring in " << time_span.count() << "s" << endl;//to time
    totalTime += time_span.count();

    //threshhold
    t1 = high_resolution_clock::now();
    cout << "threshholding image..."<< endl;
    CImg<unsigned char> save = threshhold(inputImage, BALL_BLUE);
    t2 = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(t2 - t1);
    cout << "done threshholding in " << time_span.count() << "s" << endl;
    totalTime += time_span.count();

    //save.save("threshholdOutput.jpg");

    //edge detect
    t1 = high_resolution_clock::now();
    cout << "edge detecting..."<< endl;
    save = booleanEdgeDetect(save);
    t2 = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(t2 - t1);
    cout << "done edge detecting in " << time_span.count() << "s" << endl;
    totalTime += time_span.count();

    //finding outlines
    t1 = high_resolution_clock::now();
    cout << "finding separate outlines..."<< endl;
    vector<outline> outlines = findOutlines(save);
    t2 = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(t2 - t1);
    cout<< "found " << outlines.size() << " outlines in " << time_span.count() << "s" << endl;
    totalTime += time_span.count();

    //dispOutlines(save, outlines);
    //save.save_jpeg("outputEdges.jpg");
    //cout << "image saved in output.jpg" << endl;

    //circle finding
    t1 = high_resolution_clock::now();
    cout << "checking for circles..."<< endl;
    circle c;
    for (unsigned int i = 0; i < outlines.size(); ++i){
        c = outlines[i].isCircle();
        if (c.r != -1){
            cout << "found circle: x:" << c.x << "y:" << c.y << "r:" << c.r << endl;
            origonalImage.draw_circle((int)c.x, (int)c.y, (int)c.r, red, 0.5f);
        }
    }
    time_span = duration_cast<duration<double>>(t2 - t1);
    cout << "done circle checking in " << time_span.count() << "s" << endl;
    totalTime += time_span.count();

    cout <<"Total time:" << totalTime << "s" <<  endl;

    cout << "saving in outputCircle.jpg" << endl;

    origonalImage.save("outputCircle.jpg");

    cout << "Didn't crash" << endl;
    return 0;
}

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

int difference(rgb px1, rgb px2){
    int diff = 0;
    diff += abs(px1.r - px2.r);
    diff += abs(px1.g - px2.g);
    diff += abs(px1.b - px2.b);
    return diff;
}

CImg<UINT8> threshhold(CImg<UINT8>& image, hsv color){
    rgb pixel;
    CImg<UINT8> finalImage(image.width(), image.height(),1,3,0);
    for (int x = 0; x < image.width(); ++x){
        for (int y = 0; y < image.height(); ++y){
            pixel = getRgb(image, x, y);
            if (pixel.getHsv().compare(color, 15.0f, 0.25f)){
                setRgb(finalImage,x,y,WHITE);
            }
        }
    }
    return finalImage;
}

CImg<unsigned char> edgeDetect(CImg<unsigned char>& image){
    CImg<unsigned char> edges(image.width(),image.height(),1,3,0);

    rgb currentPixel;
    rgb surroundingPixels[4];
    for (int i = 0; i < image.width() - 1; ++i){
        for (int j = 0; j < image.height() - 1; ++j){
            currentPixel = getRgb(image, i ,j);

            if (i != 0)
                surroundingPixels[0] = getRgb(image,i-1,j);
            else
                surroundingPixels[0] = currentPixel;
            if (i != image.width())
                surroundingPixels[0] = getRgb(image,i+1,j);
            else
                surroundingPixels[0] = currentPixel;
                if (j != 0)
                surroundingPixels[0] = getRgb(image,i,j-1);
            else
                surroundingPixels[0] = currentPixel;
                if (j != image.height())
                surroundingPixels[0] = getRgb(image,i,j+1);
            else
                surroundingPixels[0] = currentPixel;

            for (int elementNum = 0; elementNum < 4; elementNum++){
                if (difference(currentPixel, surroundingPixels[elementNum]) > 300){
                    setRgb(edges, i, j, WHITE);
                }
            }

        }
    }

    return edges;
}

// algorithm from http://www.cs.rit.edu/~ncs/color/t_convert.html
hsv rgb::getHsv()
{
    hsv ret;
	float min, max, delta;
	min = MIN3( r, g, b );
	max = MAX3( r, g, b );
	ret.v = max;				// v
	delta = max - min;
	if( max != 0 )
		ret.s = delta / max;		// s
	else {
		// r = g = b = 0		// s = 0, v is undefined
		ret.s = 0;
		ret.h = -1;
		return ret;
	}
	if( r == max )
		ret.h = ( g - b ) / delta;		// between yellow & magenta
	else if( g == max )
		ret.h = 2 + ( b - r ) / delta;	// between cyan & yellow
	else
		ret.h = 4 + ( r - g ) / delta;	// between magenta & cyan
	ret.h *= 60;				// degrees
	if( ret.h < 0 )
		ret.h += 360;
    return ret;
}
//Are you using variance http://en.wikipedia.org/wiki/Variance#Definition You may want to rename the variable if you're not. This is probably nit-picky, but since we will be dealing with statistics, it may end up being confusing.
bool hsv::compare(hsv other, float maxHVariance, float maxSVariance){
    if (abs(h - other.h) > maxHVariance)
        return false;
    if (abs(s - other.s) > maxSVariance)
        return false;
    return true;
}

CImg<unsigned char> booleanEdgeDetect(CImg<unsigned char>& image){
    CImg<unsigned char> edges(image.width(),image.height(),1,3,0);

    bool isEdge;
    for (int x = 0; x < image.width(); ++x){
        for (int y = 0; y < image.height(); ++y){
            if (image(x,y,0) == 255){
                    isEdge = false;
                if (x != 0){
                    if (image(x-1,y,0) == 0)//means its black pixel
                        isEdge = true;
                }
                if (x != image.width() - 1){
                    if (image(x+1,y,0) == 0)
                        isEdge = true;
                }
                if (y != 0){
                    if (image(x,y-1,0) == 0)
                        isEdge = true;
                }
                if (y != image.height() - 1){
                    if (image(x,y+1,0) == 0)
                        isEdge = true;
                }
                if (isEdge)
                    setRgb(edges, x, y, WHITE);//Do you set it to anything if it's not an edge?
            }
        }
    }
    return edges;
}

outline floodfill(CImg<UINT8>& image, int startX, int startY){
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
        setRgb(image, currentPoint.x, currentPoint.y, BLACK);
    }
    return ret;
}

vector<outline> findOutlines(CImg<UINT8> image){
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

void dispOutlines(CImg<UINT8>& image, vector<outline> outlines){
    rgb color;
    for (unsigned int i = 0; i < outlines.size(); ++i){
        color.r = rand() % 255;
        color.g = rand() % 255;
        color.b = rand() % 255;
        for (unsigned int j = 0; j < outlines[i].points.size()-1; ++j){
            setRgb(image, outlines[i].points[j].x, outlines[i].points[j].y, color);
        }
    }
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

CImg<UINT8> booleanBlur(CImg<UINT8> & image, int amount){
    int width = image.width(), height = image.height();
    float imageMatrix[width()][height()];
    float part = 1 / (float)amount;//used later
    
    //vertical part of blur
    for (int x = 0; x < width; ++x){
        for (int y = 0; y < height; ++y){
            if (image(x,y,0) == 255)
                imageMatrix[x][y] = part;
            
        }
    }
}
