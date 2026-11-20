#include <Wire.h>
#include <Zumo32U4.h>


// ==================================================
// HARDWARE
// ==================================================

Zumo32U4Buzzer buzzer;
Zumo32U4LineSensors lineSensors;
Zumo32U4Motors motors;
Zumo32U4ButtonA buttonA;
Zumo32U4ProximitySensors proxSensors;


// ==================================================
// SETTINGS
// ==================================================

const int MAX_SPEED = 100;
const int BASE_SPEED = 65;
const int OBJECT_THRESHOLD = 12;

#define NUM_SENSORS 5

unsigned int sensors[NUM_SENSORS];


// ==================================================
// VARIABLES
// ==================================================

int previousCorrection = 0;
int lastError = 0;

bool robotRunning = false;


// ==================================================
// MOTOR CONTROL
// ==================================================

void stopRobot()
{
  motors.setSpeeds(0, 0);

  previousCorrection = 0;
  lastError = 0;
}


// ==================================================
// OBJECT DETECTION
// ==================================================

bool objectDetected()
{
  proxSensors.read();

  int left = proxSensors.countsFrontWithLeftLeds();
  int right = proxSensors.countsFrontWithRightLeds();

  int detection = left + right;

  return detection >= OBJECT_THRESHOLD;
}


// ==================================================
// LINE FOLLOWING
// ==================================================

void followLine()
{
  // Read line sensors
  int position = lineSensors.readLine(sensors);

  // Calculate line error
  int error = position - 2000;

  // PD controller
  int P = error / 12;
  int D = (error - lastError) / 25;

  int correction = P + D;

  lastError = error;

  // Smooth steering changes
  correction = (previousCorrection * 3 + correction) / 4;

  previousCorrection = correction;

  // Calculate motor speeds
  int leftSpeed = BASE_SPEED + correction;
  int rightSpeed = BASE_SPEED - correction;

  // Limit motor speeds
  leftSpeed = constrain(leftSpeed, 0, MAX_SPEED);
  rightSpeed = constrain(rightSpeed, 0, MAX_SPEED);

  // Set motor speeds
  motors.setSpeeds(leftSpeed, rightSpeed);
}


// ==================================================
// BUTTON CONTROL
// ==================================================

void handleButton()
{
  if (buttonA.getSingleDebouncedPress())
  {
    robotRunning = !robotRunning;

    if (robotRunning)
    {
      // Reset controller when starting
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


// ==================================================
// SETUP
// ==================================================

void setup()
{
  // Welcome sound
  buzzer.play(">g32>>c32");

  // Initialize sensors
  lineSensors.initFiveSensors();
  proxSensors.initFrontSensor();

  // Wait for button A
  buttonA.waitForButton();

  // Calibration delay
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

  // Stop after calibration
  stopRobot();

  // Calibration complete sound
  buzzer.play(">g32>>c32");

  // Wait for sound to finish
  while (buzzer.isPlaying())
  {
  }

  // Start in OFF mode
  robotRunning = false;
}


// ==================================================
// MAIN LOOP
// ==================================================

void loop()
{
  // Check button
  handleButton();

  // Keep robot stopped when OFF
  if (!robotRunning)
  {
    stopRobot();
    return;
  }

  // Check for objects
  if (objectDetected())
  {
    stopRobot();
    return;
  }

  // Follow line
  followLine();
}
