
#include <Arduino.h>
#include <math.h>
#include <Zumo32U4.h>

#include "PositionTracker.h"

extern Zumo32U4Encoders encoders;

// Adjust these values to match your robot
const float WHEEL_DIAMETER = 39.0f;
const float WHEEL_BASE = 85.0f;
const float COUNTS_PER_REVOLUTION = 909.7f;

const float PI_VALUE = 3.14159265f;

void PositionTracker::begin()
{
    lastLeft = encoders.getCountsLeft();
    lastRight = encoders.getCountsRight();

    x = 0.0f;
    y = 0.0f;
    theta = 0.0f;
}

void PositionTracker::update()
{
    int16_t left = encoders.getCountsLeft();
    int16_t right = encoders.getCountsRight();

    // Handle encoder counter rollover
    int16_t dLeft = (int16_t)(left - lastLeft);
    int16_t dRight = (int16_t)(right - lastRight);

    lastLeft = left;
    lastRight = right;

    // Convert encoder counts into distance
    float wheelCircumference = PI_VALUE * WHEEL_DIAMETER;

    float leftDistance =
        (dLeft / COUNTS_PER_REVOLUTION) * wheelCircumference;

    float rightDistance =
        (dRight / COUNTS_PER_REVOLUTION) * wheelCircumference;

    // Calculate movement
    float distance = (leftDistance + rightDistance) / 2.0f;

    float angleChange =
        (rightDistance - leftDistance) / WHEEL_BASE;

    // Update heading
    theta += angleChange;

    // Keep heading between -PI and PI
    while (theta > PI_VALUE)
        theta -= 2.0f * PI_VALUE;

    while (theta < -PI_VALUE)
        theta += 2.0f * PI_VALUE;

    // Update position
    float middleAngle = theta - angleChange / 2.0f;

    x += distance * cos(middleAngle);
    y += distance * sin(middleAngle);
}

void PositionTracker::run()
{
    update();
}

float PositionTracker::getX()
{
    return x;
}

float PositionTracker::getY()
{
    return y;
}

float PositionTracker::getTheta()
{
    return theta;
}

void PositionTracker::reset()
{
    x = 0.0f;
    y = 0.0f;
    theta = 0.0f;

    lastLeft = encoders.getCountsLeft();
    lastRight = encoders.getCountsRight();
}
