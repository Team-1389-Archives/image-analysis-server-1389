#include "findCircle.h"

static const rgb WHITE={255,255,255};
static const rgb BLACK={0,0,0};

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

BallFinder::BallFinder(){
    ballHValue = 220;//remove
    float h, hVariance, sMin;
    if (!settingsReader.loadFromFile("config.txt")){
        cerr << "failed to load config.txt\nusing default values (hue min =, hue max =, saturation min =)" << endl;
        h = 200;
        hVariance = 250;
        sMin = 0.2;
    }
    else{
        cerr << "successfully loaded config.txt" << endl;
        string value;
        value = settingsReader.getString("hue");
        if (value == ""){
            cerr << "failed to load \"hue\" from config.txt, using default value of 200" << endl;
            h = 200;
        } 
        else{
            cerr << "successfully loaded \"hue\" as " << value << " from config.txt" << endl;
            h = atof(value.c_str());
        }
        value = settingsReader.getString("h_variance");
        if (value == ""){
            cerr << "failed to load \"h_variance\" from config.txt, using default value of 200" << endl;
            hVariance = 250;
        } 
        else{
            cerr << "successfully loaded \"hue_max\" as " << value << " from config.txt" << endl;
            hVariance = atof(value.c_str());
        }
        value = settingsReader.getString("saturation_min");
        if (value == ""){
            cerr << "failed to load \"saturation_min\" from config.txt, using default value of 200" << endl;
            sMin = .1;
        } 
        else{
            cerr << "successfully loaded \"saturation_min\" as " << value << " from config.txt" << endl;
            sMin = atof(value.c_str());
        }
    }
    g_hue = (int16_t)(h*10);
    g_hue_variance = (int16_t)(hVariance*10);
    g_min_s = (int16_t)(sMin*256);//LOOK AT THESE
    cerr << "Final threshholding values are: " << g_hue << "," << g_hue_variance << "," << g_min_s << endl;
    m_filtering_system=FilteringSystemNew();
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

CImg<UINT8> BallFinder::threshhold(CImg<UINT8>& image){
    rgb pixel;
    CImg<UINT8> finalImage(image.width(), image.height(),1,1,0);
    for (int x = 0; x < image.width(); ++x){
        for (int y = 0; y < image.height(); ++y){
            pixel = getRgb(image, x, y);
            if (pixel.getHsv().compareToColor(ballHValue, 70, 0.1)) //for red 17, 8, 0.4            for blue 150, 70, 0.1
                finalImage(x,y,0) = 255;
            //if (pixel.isBlue())
              //  finalImage(x,y,0) = 255;
        }
    }
    return finalImage;
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

CImg<unsigned char> BallFinder::booleanEdgeDetect(CImg<unsigned char>& image){
    CImg<unsigned char> edges(image.width(),image.height(),1,3,0);

    bool isEdge;
    for (int x = 0; x < image.width(); ++x){
        for (int y = 0; y < image.height(); ++y){
            if (image(x,y,0) > 127){///********** > 127 because after blur this is area we want;
                    isEdge = false;
                if (x != 0){
                    if (image(x-1,y,0) <= 127)//means its black pixel Why do black pixels automatically mean an edge?
                        isEdge = true;
                }
                if (x != image.width() - 1){
                    if (image(x+1,y,0) <= 127)
                        isEdge = true;
                }
                if (y != 0){
                    if (image(x,y-1,0) <= 127)
                        isEdge = true;
                }
                if (y != image.height() - 1){
                    if (image(x,y+1,0) <= 127)
                        isEdge = true;
                }
                if (isEdge)
                    setRgb(edges, x, y, WHITE);//Do you set it to anything if it's not an edge?
            }
        }
    }
    return edges;
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
        image(currentPoint.x, currentPoint.y, 0) = 0;
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

void BallFinder::dispOutlines(CImg<UINT8>& image, vector<outline> outlines){
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

void BallFinder::floodFillObject(int x, int y, CImg<uint8_t>& image, CImg<uint8_t>& outputImage){//warning: return image has some 1's in it instead of 0's. this is necessary for efficiency. treat the 1's as 0's.
    //if (outputImage.width() == 0)//means that nothing is stored in outputImage
        //outputImage = CImg<uint8_t>(image.width(), image.height(),1,1,0);

    point currentPoint;
    hsv currentHsv;
    vector<point> possiblePoints;
    vector<hsv> correspondingHsv;//represents hsv that possiblePoint needs to be compared to
    possiblePoints.push_back(point(x, y));
    correspondingHsv.push_back(getRgb(image, x, y).getHsv());
    while(!possiblePoints.empty()){
        currentPoint = possiblePoints[possiblePoints.size()-1];
        currentHsv = correspondingHsv[correspondingHsv.size()-1];//represents hsv of pixel to compareCurrentPoint to, not the hsv of currentPoint itself
        possiblePoints.pop_back();
        correspondingHsv.pop_back();
        if (outputImage(currentPoint.x, currentPoint.y, 0) == 0){//if this pixel hasn't already been tested
            if (currentHsv.compare(getRgb(image, currentPoint.x, currentPoint.y).getHsv(), 20, 0.4f, 0.6f)    ){//if color is close enough
                outputImage(currentPoint.x, currentPoint.y, 0) = 255;
                if (currentPoint.x != 0){
                    possiblePoints.push_back(point(currentPoint.x-1, currentPoint.y));
                    correspondingHsv.push_back(getRgb(image, x, y).getHsv());
                }
                if (currentPoint.x != image.width() - 1){
                    possiblePoints.push_back(point(currentPoint.x+1, currentPoint.y));
                    correspondingHsv.push_back(getRgb(image, x, y).getHsv());
                }
                if (currentPoint.y != 0){
                    possiblePoints.push_back(point(currentPoint.x, currentPoint.y-1));
                    correspondingHsv.push_back(getRgb(image, x, y).getHsv());
                }
                if (currentPoint.y != image.height()- 1){
                    possiblePoints.push_back(point(currentPoint.x, currentPoint.y+1));
                    correspondingHsv.push_back(getRgb(image, x, y).getHsv());
                }
            }
            else{//pixel wasn't right color
                //outputImage(currentPoint.x, currentPoint.y, 0) = 1;//mark as been tested but not included
            }
        }
    }
}

CImg<uint8_t> BallFinder::floodThresh(CImg<uint8_t>& image){
    CImg<uint8_t> finalImage(image.width(), image.height(),1,1,0);

    for (int x = 0; x < image.width(); x++){
        for (int y = 0; y < image.height(); y++){
            if (getRgb(image, x, y).getHsv().compareToColor(160, 40, 0.2)){
                floodFillObject(x, y, image, finalImage);
            }
        }
    }

    return finalImage;
}
