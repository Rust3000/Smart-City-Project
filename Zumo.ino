
/*
 * Line follower for Pololu Zumo 32U4
 *
 * Follows a black line on a white background.
 * Uses proportional and derivative (PD) control.
 */

#include <Wire.h>
#include <Zumo32U4.h>

// Zumo 32U4 hardware
Zumo32U4Buzzer buzzer;
Zumo32U4LineSensors lineSensors;
Zumo32U4Motors motors;
Zumo32U4ButtonA buttonA;

// Maximum motor speed
const int MAX_SPEED = 100;

int previousCorrection = 0;

// PID control
int lastError = 0;

// Five line sensors
#define NUM_SENSORS 5
unsigned int sensors[NUM_SENSORS];


void setup()
{
  // Welcome sound
  buzzer.play(">g32>>c32");

  // Initialize the five line sensors
  lineSensors.initFiveSensors();

  // Wait for button A
  buttonA.waitForButton();

  // Calibration
  // Rotate the robot to let the sensors see
  // both the black line and white background.

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
  int position = lineSensors.readLine(sensors);

  // Calculate line error
  int error = position - 2000;

  // Gentle PD controller
  int P = error / 12;
  int D = (error - lastError) / 25;

  int correction = P + D;

  lastError = error;

  // Smooth steering changes
  correction = (previousCorrection * 3 + correction) / 4;

  previousCorrection = correction;

  // Slower base speed for thin tape
  int baseSpeed = 65;

  // Motor speeds
  int leftSpeed = baseSpeed + correction;
  int rightSpeed = baseSpeed - correction;

  // Limit speeds
  leftSpeed = constrain(leftSpeed, 0, 100);
  rightSpeed = constrain(rightSpeed, 0, 100);

  motors.setSpeeds(leftSpeed, rightSpeed);
}


