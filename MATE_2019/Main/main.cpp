#include <iostream>

#include ".\Headers\Gamepad.h"
#include ".\Headers\PID.h"
#include ".\Headers\SerialPort.h"
#include ".\Headers\Utils.h"

using namespace std;

char output[MAX_DATA_LENGTH];
string imu;
string prevIMU;
int yaw = 0;    // Heading left to right
int pitch = 0;  // Heading up and down
int roll = 0;   // Angle side to side relative to ground

int yawOffset = 0;

// Change the name of the port with the port name of your computer
// Must remember that the backslashes are essential so do not remove them
const char* port = "\\\\.\\COM7";
SerialPort arduino(port, 115200);
Gamepad gamepad = Gamepad(1);

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

  // Make sure there is new data to process
  if (imu.size() != prevIMU.size())
  {
    // Remove any remanents of messages
    imu.erase(0, imu.find(':'));

    // Only process when there is a message ending
    if (imu.find("|"))
    {
      yaw = stoi(imu.substr(1, Utils::findNth(imu, ";", 1) - 1));
      pitch = stoi(imu.substr(Utils::findNth(imu, ";", 1) + 1,
                              Utils::findNth(imu, ";", 2) - 1));
      roll = stoi(imu.substr(Utils::findNth(imu, ";", 2) + 1, imu.find('|')));

      cout << "\33[2K >>       " << imu << endl
           << "\33[2K Yaw:     " << yaw << endl
           << "\33[2K Pitch:   " << pitch << endl
           << "\33[2K Roll:    " << roll << "\033[F\033[F\033[F\033[F\033[F\r";

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
    cout << "No new data" << endl;
  }
  prevIMU = imu;
}

void drive()
{
  // : is verification character for arduino
  string data = ":";

  double FWD = gamepad.leftStick_Y();
  double STR = gamepad.leftStick_X();
  double RCCW = gamepad.rightStick_X();

  PID pitchPID(0.001, 0.0, 0.0);
  pitchPID.setContinuous(false);
  pitchPID.setOutputLimits(-1.0, 1.0);
  pitchPID.setSetpoint(0.0);

  PID rollPID(0.001, 0.0, 0.0);
  rollPID.setContinuous(false);
  rollPID.setOutputLimits(-1.0, 1.0);
  rollPID.setSetpoint(0.0);

  // Will not reach full power diagonally because of controller input (depending
  // on controller)
  // rad45 adjusts where front is
  const double rad45 = 45.0 * 3.14159 / 180.0;
  double heading = rad45 + yaw + yawOffset;
  double FR = (-STR * sin(heading) + FWD * cos(heading) + RCCW);
  double BR = (STR * cos(heading) + FWD * sin(heading) + RCCW);
  double BL = (-STR * sin(heading) + FWD * cos(heading) - RCCW);
  double FL = (STR * cos(heading) + FWD * sin(heading) - RCCW);

  // PID outputs may be backwards. Needs testing
  double UL = gamepad.rightTrigger() - gamepad.leftTrigger() +
              pitchPID.getOutput(pitch) + rollPID.getOutput(roll);
  double UR = gamepad.rightTrigger() - gamepad.leftTrigger() +
              pitchPID.getOutput(pitch) - rollPID.getOutput(roll);
  double UB = gamepad.rightTrigger() - gamepad.leftTrigger() -
              pitchPID.getOutput(pitch);

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

  // Convert the values to something the motors can read
  for (double* num : vals)
  {
    *num = Utils::convertRange(-1.0, 1.0, 1100.0, 1900.0, *num);
    data.append(to_string((int)*num) + ";");
  }

  data.pop_back();
  data.append("\n");

  cout << "Sending: " << data << endl;
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

  if (gamepad.connected())
  {
    cout << " Gamepad 1 connected" << endl;
  }
  else
  {
    cout << " Gamepad 1 NOT connected" << endl;
  }

  while (!gamepad.getButtonPressed(xButtons.Back))
  {
    gamepad.update();
    if (gamepad.getButtonPressed(xButtons.A))
    {
      yawOffset = -yaw;
	}
    drive();
    gamepad.refresh();
  }

  cout << " Exiting" << endl;

  return 0;
}
