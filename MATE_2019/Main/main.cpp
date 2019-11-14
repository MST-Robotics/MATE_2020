#include <iostream>

#include ".\Headers\Gamepad.h"
#include ".\Headers\PID.h"
#include ".\Headers\SerialPort.h"
#include ".\Headers\Utils.h"

using namespace std;

bool disabled = false;

char output[MAX_DATA_LENGTH];
string imu;
string prevIMU;
int pitch = 0;  // Heading up and down
int roll = 0;   // Angle side to side relative to ground

double pitchSetpoint = 0.0;
double rollSetpoint = 0.0;

int yawOffset = 0;

// Change the name of the port with the port name of your computer
// Must remember that the backslashes are essential so do not remove them
const char* port = "\\\\.\\COM3";
SerialPort arduino(port, 115200);
Gamepad gamepad1 = Gamepad(1);
Gamepad gamepad2 = Gamepad(2);

void transferData(string data)
{
  // Send motor commands to arduino
  char* charArray = new char[data.size()];
  copy(data.begin(), data.end(), charArray);
  arduino.writeSerialPort(charArray, data.size() - 1);
  delete[] charArray;

  // Wait for most of arduino message to come in through serial
  Sleep(90);

  // Expects IMU data foramatted like ":Yaw;Pitch;Roll|", all int
  arduino.readSerialPort(output, MAX_DATA_LENGTH);

  // Add received section to previously recieved section
  for (char c : output)
  {
    if (c)
    {
      imu += c;
    }
  }

  cout << "\33[2K Raw Input: " << imu << endl;

  // Make sure there is new data to process
  if (imu.size() != prevIMU.size())
  {
    // Remove any remanents of messages
    imu.erase(0, imu.find(':'));

    // Only process when there is a message ending
    if (imu.find("|"))
    {
      cout << "\33[2K" << imu << endl
           << "\33[2K" << imu.substr(1, imu.find(";")) << endl
           << "\33[2K" << imu.substr(imu.find(";") + 2, imu.find('|')) << endl;
      pitch = stoi(imu.substr(1, imu.find(";")));
      roll = stoi(imu.substr(imu.find(";") + 1, imu.find('|')));

      cout << "\33[2K >>       " << imu << endl
           << "\33[2K Pitch:   " << pitch << endl
           << "\33[2K Roll:    " << roll
           << "[F\033[F\033[F\033[F\033[F\033[F\033[F\033[F\033[F\033[F\033["
              "F\033[F\033[F\r\33[2K";

      // Erase any backlog so latest data is read next
      for (int i = 0; i < imu.size(); ++i)
      {
        output[i] = '\0';
      }
      imu.clear();
    }
    else
    {
      cout << "Not Read:" << imu << endl;
    }
  }
  else
  {
    cout << "\033[F\033[F\33[2K     No new data: \r";
  }
  prevIMU = imu;
}

// TODO split out drive and arm to their own files
void teleop(double FWD, double STR, double RCW)
{
  // : is verification character for arduino
  string data = ":";

  PID pitchPID(0.007, 0.0, 0.0);
  pitchPID.setContinuous(false);
  pitchPID.setOutputLimits(-1.0, 1.0);
  pitchPID.setSetpoint(pitchSetpoint);

  PID rollPID(0.007, 0.0, 0.0);
  rollPID.setContinuous(false);
  rollPID.setOutputLimits(-1.0, 1.0);
  rollPID.setSetpoint(rollSetpoint);

  // Let driver adjust angle of robot if necessary
  if (gamepad1.getButtonPressed(xButtons.A))
  {
    pitchSetpoint += -0.01;
  }
  else if (gamepad1.getButtonPressed(xButtons.Y))
  {
    pitchSetpoint += 0.01;
  }

  if (gamepad1.getButtonPressed(xButtons.B))
  {
    rollSetpoint += -0.01;
  }
  else if (gamepad1.getButtonPressed(xButtons.X))
  {
    rollSetpoint += 0.01;
  }

  // Will not reach full power diagonally because of controller input (depending
  // on controller)
  const double rad45 = 45.0 * 3.14159 / 180.0;

  // heading adjusts where front is
  double heading = -rad45;
  double FR = -(-STR * sin(heading) + FWD * cos(heading) - RCW);  // A
  double BR = (STR * cos(heading) + FWD * sin(heading) - RCW);    // B
  double BL = -(-STR * sin(heading) + FWD * cos(heading) + RCW);  // C
  double FL = (STR * cos(heading) + FWD * sin(heading) + RCW);    // D

  double UL = gamepad1.rightTrigger() * 0.6 - gamepad1.leftTrigger() * 1.4 -
              pitchPID.getOutput(pitch) - rollPID.getOutput(roll) + 0.45;
  double UR = gamepad1.rightTrigger() - gamepad1.leftTrigger() -
              pitchPID.getOutput(pitch) * 0.6 + rollPID.getOutput(roll) * 1.4 +
              0.45;
  double UB = (gamepad1.rightTrigger() - gamepad1.leftTrigger() +
               pitchPID.getOutput(pitch) + 0.45) *
              0.4;

  double* vals[] = {&FR, &BR, &BL, &FL, &UL, &UR, &UB};

  double max = 1.0;

  // Normalize the horizontal motor powers if calculation goes above 100%
  for (int i = 0; i < 4; ++i)
  {
    if (abs(*vals[i]) > max)
    {
      max = abs(*vals[i]);
    }
  }

  for (int i = 0; i < 4; ++i)
  {
    *vals[i] /= max;
  }

  // Normalize the vertical motor powers if calculation goes above 100%
  for (int i = 4; i < 7; ++i)
  {
    if (abs(*vals[i]) > max)
    {
      max = abs(*vals[i]);
    }
  }

  for (int i = 4; i < 7; ++i)
  {
    *vals[i] /= max;
  }

  // Don't send command if it is below a certain threshold
  // Or the robot is disabled
  for (double* num : vals)
  {
    if (abs(*num) < 0.1 || disabled)
    {
      *num = 0.0;
    }
  }

  // Convert the values to something the motors can read
  for (double* num : vals)
  {
    *num = Utils::convertRange(-1.0, 1.0, 1100.0, 1900.0, *num);
    data.append(to_string((int)*num) + ";");
  }

  double shoulder = 0.0;
  double wristTilt = 0.0;
  double wristTwist = 0.0;

  shoulder += gamepad2.leftStick_Y() * 0.05;
  wristTilt += gamepad2.rightStick_Y() * 0.05;
  wristTwist += gamepad2.rightStick_X() * 0.5;
  double* armVals[] = {&shoulder, &wristTilt, &wristTwist};

  for (double* num : armVals)
  {
    *num = Utils::convertRange(-1.0, 1.0, 1100.0, 1900.0, *num);
    data.append(to_string((int)*num) + ";");
  }

  // Claw command + end of command string character
  data.append(to_string((int)gamepad1.getButtonPressed(xButtons.A)) + "\n");

  cout << "[F\033[F\033\33[2K Sending: " << data << endl;
  transferData(data);
}

int main()
{
  if (arduino.isConnected())
  {
    cout << " Arduino connection made" << endl << endl;
  }
  else
  {
    cout << " Error in Arduino port name" << endl << endl;
  }

  if (gamepad1.connected())
  {
    cout << " Gamepad 1 connected" << endl;
  }
  else
  {
    cout << " Gamepad 1 NOT connected" << endl;
  }

  while (true)
  {
    gamepad1.update();
    gamepad2.update();
    if (gamepad1.getButtonPressed(xButtons.Back) || !gamepad1.connected())
    {
      disabled = true;
    }
    else if (gamepad1.getButtonPressed(xButtons.Start) && gamepad1.connected())
    {
      disabled = false;
    }
    teleop(-gamepad1.rightStick_X(), -gamepad1.leftStick_Y(),
           -gamepad1.leftStick_X());
    gamepad1.refresh();
    gamepad2.refresh();
  }

  cout << " Exiting" << endl;

  return 0;
}
