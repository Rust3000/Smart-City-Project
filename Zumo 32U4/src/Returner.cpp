
#include <Arduino.h>
#include <math.h>

#include "PositionTracker.h"

const float RETURN_PI = 3.14159265f;

const float ARRIVAL_DISTANCE = 35.0f;
const float MAX_RETURN_SPEED = 80.0f;
const float MIN_RETURN_SPEED = 30.0f;

const float TURN_GAIN = 55.0f;
const float DISTANCE_GAIN = 1.0f;

void Returner::begin()
{
    // Home is the origin (0,0)
}

bool Returner::update(
    float x,
    float y,
    float theta,
    int &leftSpeed,
    int &rightSpeed
)
{
    // Calculate distance to home
    float distance = sqrt(x * x + y * y);

    // Check whether the robot has arrived
    if (distance <= ARRIVAL_DISTANCE)
    {
        leftSpeed = 0;
        rightSpeed = 0;

        return true;
    }

    // Calculate direction towards home
    float targetAngle = atan2(-y, -x);

    // Calculate heading error
    float angleError = targetAngle - theta;

    // Normalize angle
    while (angleError > RETURN_PI)
        angleError -= 2.0f * RETURN_PI;

    while (angleError < -RETURN_PI)
        angleError += 2.0f * RETURN_PI;

    // Calculate turning correction
    float correction = angleError * TURN_GAIN;

    // Calculate forward speed
    float speed = distance * DISTANCE_GAIN;

    speed = constrain(
        speed,
        MIN_RETURN_SPEED,
        MAX_RETURN_SPEED
    );

    // Turn in place when direction is significantly wrong
    if (fabs(angleError) > 1.2f)
    {
        speed = 0;
    }

    // Calculate motor speeds
    leftSpeed = (int)(speed - correction);
    rightSpeed = (int)(speed + correction);

    // Limit motor speeds
    leftSpeed = constrain(leftSpeed, -MAX_RETURN_SPEED,
                          MAX_RETURN_SPEED);

    rightSpeed = constrain(rightSpeed, -MAX_RETURN_SPEED,
                           MAX_RETURN_SPEED);

    return false;
}
