
#include <Arduino.h>
#include <Wire.h>
#include <Zumo32U4.h>
#include <math.h>

#include "PositionTracker.h"
#include "Batteri.h"

// ==========================================
// HARDWARE
// ==========================================

Zumo32U4Buzzer buzzer;
Zumo32U4LineSensors lineSensors;
Zumo32U4Motors motors;
Zumo32U4ButtonA buttonA;
Zumo32U4ProximitySensors proxSensors;
Zumo32U4Encoders encoders;

// ==========================================
// ROBOT SYSTEMS
// ==========================================

PositionTracker positionTracker;
Returner returner;
Batteri battery;

// ==========================================
// SETTINGS
// ==========================================

const int MAX_SPEED = 100;
const int BASE_SPEED = 65;

const int OBJECT_THRESHOLD = 12;
const float LOW_BATTERY = 25.0f;

#define NUM_SENSORS 5

unsigned int sensors[NUM_SENSORS];

int previousCorrection = 0;
int lastError = 0;

bool robotRunning = false;

// ==========================================
// ROBOT STATES
// ==========================================

enum RobotState
{
    LINE_FOLLOWING,
    RETURNING_HOME,
    CHARGING
};

RobotState currentState = LINE_FOLLOWING;

// ==========================================
// CHARGING SETTINGS
// ==========================================

unsigned long lastChargeTime = 0;

const unsigned long CHARGE_INTERVAL = 100;

// ==========================================
// MOTOR CONTROL
// ==========================================

void stopRobot()
{
    motors.setSpeeds(0, 0);

    previousCorrection = 0;
    lastError = 0;
}

// ==========================================
// OBJECT DETECTION
// ==========================================

bool objectDetected()
{
    proxSensors.read();

    int left =
        proxSensors.countsFrontWithLeftLeds();

    int right =
        proxSensors.countsFrontWithRightLeds();

    return (left + right) >= OBJECT_THRESHOLD;
}

// ==========================================
// LINE FOLLOWING
// ==========================================

void followLine()
{
    int position = lineSensors.readLine(sensors);

    int error = position - 2000;

    int P = error / 12;
    int D = (error - lastError) / 25;

    int correction = P + D;

    lastError = error;

    // Smooth steering
    correction =
        (previousCorrection * 3 + correction) / 4;

    previousCorrection = correction;

    int leftSpeed = BASE_SPEED + correction;
    int rightSpeed = BASE_SPEED - correction;

    leftSpeed = constrain(leftSpeed, 0, MAX_SPEED);
    rightSpeed = constrain(rightSpeed, 0, MAX_SPEED);

    motors.setSpeeds(leftSpeed, rightSpeed);
}

// ==========================================
// RETURN HOME
// ==========================================

void returnHome()
{
    // Pause return if an object is detected
    if (objectDetected())
    {
        stopRobot();
        return;
    }

    int leftSpeed = 0;
    int rightSpeed = 0;

    bool homeReached = returner.update(
        positionTracker.getX(),
        positionTracker.getY(),
        positionTracker.getTheta(),
        leftSpeed,
        rightSpeed
    );

    if (homeReached)
    {
        stopRobot();

        currentState = CHARGING;

        lastChargeTime = millis();

        buzzer.play(">g32>>c32");

        return;
    }

    motors.setSpeeds(leftSpeed, rightSpeed);
}

// ==========================================
// CHARGING
// ==========================================

void chargeBattery()
{
    stopRobot();

    if (millis() - lastChargeTime >= CHARGE_INTERVAL)
    {
        lastChargeTime = millis();

        battery.charge(1.0f);
    }

    if (battery.getLevel() >= 100.0f)
    {
        // Reset odometry at home
        positionTracker.reset();

        // Reset return system
        returner.begin();

        // Reset line controller
        previousCorrection = 0;
        lastError = 0;

        currentState = LINE_FOLLOWING;

        buzzer.play(">c32");
    }
}

// ==========================================
// BUTTON CONTROL
// ==========================================

void handleButton()
{
    if (buttonA.getSingleDebouncedPress())
    {
        robotRunning = !robotRunning;

        if (robotRunning)
        {
            previousCorrection = 0;
            lastError = 0;

            buzzer.play(">c32");
        }
        else
        {
            stopRobot();

            buzzer.play(">g32");
        }
    }
}

// ==========================================
// SETUP
// ==========================================

void setup()
{
    buzzer.play(">g32>>c32");

    // Initialize sensors
    lineSensors.initFiveSensors();
    proxSensors.initFrontSensor();

    // Wait for calibration button press
    buttonA.waitForButton();

    delay(1000);

    // Calibrate line sensors
    for (int i = 0; i < 120; i++)
    {
        if (i > 30 && i <= 90)
        {
            motors.setSpeeds(-200, 200);
        }
        else
        {
            motors.setSpeeds(200, -200);
        }

        lineSensors.calibrate();
    }

    stopRobot();

    // Initialize position tracking
    positionTracker.begin();

    // Initialize return system
    returner.begin();

    // Initialize battery
    battery.begin();

    buzzer.play(">g32>>c32");

    while (buzzer.isPlaying())
    {
    }

    robotRunning = false;

    currentState = LINE_FOLLOWING;
}

// ==========================================
// MAIN LOOP
// ==========================================

void loop()
{
    handleButton();

    if (!robotRunning)
    {
        stopRobot();
        return;
    }

    // Update position
    float oldX = positionTracker.getX();
    float oldY = positionTracker.getY();

    positionTracker.update();

    float newX = positionTracker.getX();
    float newY = positionTracker.getY();

    // Calculate distance travelled in metres
    float dx = newX - oldX;
    float dy = newY - oldY;

    float distanceTravelled =
        sqrt(dx * dx + dy * dy) / 1000.0f;

    // Update battery
    battery.update(distanceTravelled);

    // Check battery level
    if (currentState == LINE_FOLLOWING &&
        battery.needsCharging())
    {
        stopRobot();

        currentState = RETURNING_HOME;

        buzzer.play(">g32");
    }

    // Execute robot state
    switch (currentState)
    {
        case LINE_FOLLOWING:

            if (objectDetected())
            {
                stopRobot();
            }
            else
            {
                followLine();
            }

            break;

        case RETURNING_HOME:

            returnHome();

            break;

        case CHARGING:

            chargeBattery();

            break;
    }
}
