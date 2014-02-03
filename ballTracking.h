#include <vector>
#include "findCircle.h"
#include <stdint.h>

using namespace std;

extern const int widthOfFrame = 100;//width of frame taken by camera in pixels. this is necessary because circle coordinates are given with
                              //(0,0) at top left, meaning that width needs to be known to see if something is to left or right of center.

extern const float distanceRatio = 1;//distance from camera to ball can be found by: distanceFromCamera = (distanceRatio / radiusOfBallInPixels)
                                 //the distanceRatio can be found using pictures where ball is at known distance
                                 //note: distanceFromCamera represents absolute distance, not just distance coming straight out from camera

extern const float cameraAngleRatio = 1;//distance ball is horizontally from camera = (distanceFromCamera * cameraAngleRatio * distanceInPixelsFromCenterOfPicture)

extern const float maxAllowedDistance = 0.1;//maximum allowed distance from one frame to next that a ball can move to be considered same ball

struct Coords{//this represents coordinates of an object's position relative to the camera from a 2D aerial view, where x is distance
              //horizontally with positive being to camera's right and y is distance out from camera where positive is the direction
              //the camera is facing. Measured in feet.
    float x;
    float y;
    Coords(circle);//gets coords relative to camera from a circle, assuming that the circle is a ball.
    Coords(){}
    float distanceTo(Coords);
};

struct Ball:public Coords{
    Coords position;
    int certaintyOfExistence;//every time this ball is seen, certiantyOfExistance += 1. every frame not seen, certiantyOfExistance -= 1.
    Ball(Coords);

};

class BallTracker{//give this object position of balls over time, it keeps track of which one is which
    vector<Ball> ballPositions;
public:
    BallTracker();
    void nextFrame(vector<circle>);
    vector<Coords> getCurrentBalls();
};
