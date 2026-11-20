
#include <Wire.h>
#include <Zumo32U4.h>

Zumo32U4Buzzer buzzer;
Zumo32U4LineSensors lineSensors;
Zumo32U4Motors motors;
Zumo32U4ButtonA buttonA;
Zumo32U4ProximitySensors proxSensors;

const int MAX_SPEED = 100;
const int OBJECT_THRESHOLD = 12;

#define NUM_SENSORS 5
unsigned int sensors[NUM_SENSORS];

int previousCorrection = 0;
int lastError = 0;

bool robotRunning = false;

// Check for an object in front
bool objectDetected()
{
  proxSensors.read();

  int left = proxSensors.countsFrontWithLeftLeds();
  int right = proxSensors.countsFrontWithRightLeds();

  return (left + right) >= OBJECT_THRESHOLD;
}

void stopRobot()
{
  motors.setSpeeds(0, 0);
  previousCorrection = 0;
  lastError = 0;
}

void setup()
{
  buzzer.play(">g32>>c32");

  lineSensors.initFiveSensors();
  proxSensors.initFrontSensor();

  // Wait for button A before calibration
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

  buzzer.play(">g32>>c32");

  // Wait for calibration sound to finish
  while (buzzer.isPlaying())
  {
  }

  // Robot starts in OFF mode
  robotRunning = false;
}

void loop()
{
  // Check if button A has been pressed
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

  // If robot is OFF, keep motors stopped
  if (!robotRunning)
  {
    stopRobot();
    return;
  }

  // Check for an object in front
  if (objectDetected())
  {
    stopRobot();
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

  motors.setSpeeds(leftSpeed, rightSpeed);
}
