
#pragma once
#include <Arduino.h>

class PositionTracker {
public:
    void begin();
    void update();
    void run();

    float getX();
    float getY();
    float getTheta();

    void reset();

private:
    long lastLeft = 0;
    long lastRight = 0;

    float x = 0.0f;
    float y = 0.0f;
    float theta = 0.0f;
};

class Returner {
public:
    void begin();

    bool update(
        float x,
        float y,
        float theta,
        int &leftSpeed,
        int &rightSpeed
    );
};
