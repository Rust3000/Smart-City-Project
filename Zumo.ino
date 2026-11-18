
/*
 * Line follower for Pololu Zumo 32U4
 *
 * Follows a black line on a white background.
 * Uses proportional and derivative (PD) control.
 *
 * Stops when an object is detected in front
 * using the built-in IR proximity sensors.
 */

#include <Wire.h>
#include <Zumo32U4.h>

// Zumo 32U4 hardware
Zumo32U4Buzzer buzzer;
Zumo32U4LineSensors lineSensors;
Zumo32U4Motors motors;
Zumo32U4ButtonA buttonA;
Zumo32U4ProximitySensors proxSensors;

// Maximum motor speed
const int MAX_SPEED = 100;

// Proximity detection threshold
const int OBJECT_THRESHOLD = 2;

int previousCorrection = 0;
int lastError = 0;

// Five line sensors
#define NUM_SENSORS 5
unsigned int sensors[NUM_SENSORS];


// Check for an object in front
bool objectDetected()
{
  proxSensors.read();

  // Read front proximity sensors only
  int left = proxSensors.countsFrontWithLeftLeds();
  int right = proxSensors.countsFrontWithRightLeds();

  int detection = left + right;

  return detection >= OBJECT_THRESHOLD;
}


void setup()
{
  // Welcome sound
  buzzer.play(">g32>>c32");

  // Initialize line sensors
  lineSensors.initFiveSensors();

  // Initialize front proximity sensors
  proxSensors.initFrontSensor();

  // Wait for button A
  buttonA.waitForButton();

  // Calibration
  delay(1000);

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
  motors.setSpeeds(0, 0);

  // Calibration complete
  buzzer.play(">g32>>c32");

  // Wait for button A again
  buttonA.waitForButton();

  // Start music
  buzzer.play("L16 cdegreg4");

  while (buzzer.isPlaying())
  {
  }
}


void loop()
{
  // Check for an object in front
  if (objectDetected())
  {
    // Stop both motors
    motors.setSpeeds(0, 0);
    return;
  }

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

  // Base speed
  int baseSpeed = 65;

  // Calculate motor speeds
  int leftSpeed = baseSpeed + correction;
  int rightSpeed = baseSpeed - correction;

  // Limit motor speeds
  leftSpeed = constrain(leftSpeed, 0, MAX_SPEED);
  rightSpeed = constrain(rightSpeed, 0, MAX_SPEED);

  // Set motor speeds
  motors.setSpeeds(leftSpeed, rightSpeed);
}
