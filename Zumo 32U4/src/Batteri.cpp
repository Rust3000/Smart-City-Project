
#include <Arduino.h>

#include "Batteri.h"

void Batteri::begin()
{
    level = 100.0f;
}

void Batteri::update(float distanceTravelled)
{
    // distanceTravelled is measured in metres
    const float FORBRUK = 25.0f;

    level -= distanceTravelled * FORBRUK;

    if (level < 0.0f)
    {
        level = 0.0f;
    }
}

float Batteri::getLevel()
{
    return level;
}

bool Batteri::needsCharging()
{
    return level <= 50.0f;
}

void Batteri::charge(float amount)
{
    level += amount;

    if (level > 100.0f)
    {
        level = 100.0f;
    }
}
