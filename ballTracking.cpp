#include "ballTracking.h"

//const int widthOfFrame = 100/*TBD*/;
//const int distanceRatio = 1/*TBD*/;
//const int cameraAngleRatio = 1/*TBD*/;
//const int maxAllowedDistance = widthOfFrame * 0.1f;


#include <math.h>

using namespace std;

Coords::Coords(circle c){
    float distanceToBall = (distanceRatio / c.r);
    float distanceWithoutVertical = sqrt(square(distanceToBall) - square(c.y));//distanceWithoutY represents if you made a vertical line through ball, the distance from that to camera.
    x = (distanceWithoutVertical * cameraAngleRatio *  (c.x - ((float)widthOfFrame / 2)   )    );//note: this x and the y on next line are aerial view, c.x and c.y are ball's position on screen
    y = sqrt(square(distanceWithoutVertical) - square(x)); //pythagorean theorum
}

float Coords::distanceTo(Coords other){
    return sqrt(square(x - other.x) + square(y - other.y));
}

Ball::Ball(Coords pos){
    certaintyOfExistence = 1;
    position = pos;
}

BallTracker::BallTracker():ballPositions(){

}

void BallTracker::nextFrame(vector<circle> circles){
    vector<Coords> ballCoords;//turn list of circles into list of Coords
    for (unsigned int i = 0; i < circles.size(); i++){
        ballCoords.push_back(Coords(circles[i]));
    }

    vector<Ball> accountedForBallPositions;
    float minimumDistance = 1 << 30;//set to an unrealistically big number so any comparisons will be smaller
    int bestBallMatch = -1;
    //pair each coord with coord from last frame, if are any.
    for (unsigned int i = ballCoords.size()-1; i >= 0 ; i--){///*****note: make to look for closest fit, not just first fit
        for (unsigned int j = ballPositions.size()-1; j >= 0 ; j++){
            if(ballCoords[i].distanceTo(ballPositions[j]) < minimumDistance){
                minimumDistance = ballCoords[i].distanceTo(ballPositions[j]);
                bestBallMatch = j;
            }
        }
            if (bestBallMatch != -1){
                ballPositions[bestBallMatch].certaintyOfExistence++;//add 2 to certainty, one is subtracted later so this really just adds one
                ballPositions[bestBallMatch].x = ballCoords[i].x;
                ballPositions[bestBallMatch].y = ballCoords[i].y;
                accountedForBallPositions.push_back(ballPositions[bestBallMatch]);
                ballPositions.erase(ballPositions.begin() + bestBallMatch);//now that the ball position has been accounted for, erase it from master ballPositions list. it will be put back later.
                ballCoords.erase(ballCoords.begin() + i);
            }
    }

    for (unsigned int i = 0; i < ballPositions.size(); i++){//subtract one from all certainty of existence
        ballPositions[i].certaintyOfExistence--;
    }

    for (unsigned int i = 0; i < accountedForBallPositions.size(); i++){//put the accounted for positions back in the main list now that the certaintyOfExistence has been dealt with
        ballPositions.push_back(accountedForBallPositions[i]);
    }

    for (unsigned int i = 0; i < ballCoords.size(); i++){//for all detected balls this frame that were not found to fit with any pre-known balls
        ballPositions.push_back(Ball(ballCoords[i]));
    }
}
