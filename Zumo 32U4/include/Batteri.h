
#pragma once

#include <Arduino.h>

class Batteri {
public:
    void begin();

    void update(float distanceTravelled);

    float getLevel();

    bool needsCharging();

    void charge(float amount);  // Added function

private:
    float level = 100.0f;
};
