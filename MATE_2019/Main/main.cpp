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

int yawOffset = 0;

// Change the name of the port with the port name of your computer
// Must remember that the backslashes are essential so do not remove them
const char* port = "\\\\.\\COM8";
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
      cout << imu << endl
		  << imu.substr(1, imu.find(";")) << endl
           << imu.substr(imu.find(";") + 2, imu.find('|')) << endl;
      pitch = stoi(imu.substr(1, imu.find(";")));
      roll = stoi(imu.substr(imu.find(";") + 1, imu.find('|')));

      cout << "\33[2K >>       " << imu << endl
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
    cout << "No new data: " << endl;
  }
  prevIMU = imu;
}

void teleop()
{
  // : is verification character for arduino
  string data = ":";

  double FWD = gamepad.leftStick_Y();
  double RCW = gamepad.leftStick_X();
  double STR = gamepad.rightStick_X();

  PID pitchPID(0.02, 0.0, 0.0);
  pitchPID.setContinuous(false);
  pitchPID.setOutputLimits(-1.0, 1.0);
  pitchPID.setSetpoint(0.0);

  PID rollPID(0.01, 0.0, 0.0);
  rollPID.setContinuous(false);
  rollPID.setOutputLimits(-1.0, 1.0);
  rollPID.setSetpoint(0.0);

  // Will not reach full power diagonally because of controller input (depending
  // on controller)
  // rad45 adjusts where front is
  const double rad45 = 45.0 * 3.14159 / 180.0;

  double heading = -rad45;
  double FR = 0.0;  //(-STR * sin(heading) + FWD * cos(heading) - RCW);  // A // BL
  double BR = 0.0;  //(STR * cos(heading) + FWD * sin(heading) - RCW); // B   // FL
  double BL = 0.0;  //(-STR * sin(heading) + FWD * cos(heading) + RCW); // C  // FR
  double FL = 0.0;  //(STR * cos(heading) + FWD * sin(heading) + RCW);  // D  // BR

  BL = (-STR * sin(heading) + FWD * cos(heading) + RCW);
  FL = (STR * cos(heading) + FWD * sin(heading) - RCW);
  FR = (-STR * sin(heading) + FWD * cos(heading) + RCW);
  BR = (STR * cos(heading) + FWD * sin(heading) - RCW);

  double UL = gamepad.leftTrigger() - gamepad.rightTrigger() -
              pitchPID.getOutput(pitch) - rollPID.getOutput(roll);
  double UR = gamepad.leftTrigger() - gamepad.rightTrigger() -
              pitchPID.getOutput(pitch) + rollPID.getOutput(roll);
  double UB = 0.4 * (gamepad.leftTrigger() - gamepad.rightTrigger() +
              pitchPID.getOutput(pitch));

  double* vals[] = {&FR, &BR, &BL, &FL, &UL, &UR, &UB};

  double max = 1.0;

  // Stop motors if disabled
  if (disabled)
  {
    for (double* num : vals)
    {
      *num = 0.0;
    }
  }

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
  for (double* num : vals)
  {
    if (abs(*num) < 0.1)
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

  // Claw command + end of command string character
  data.append(to_string((int)gamepad.getButtonPressed(xButtons.A)) + "\n");

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

  while (true)
  {
    gamepad.update();
    if (gamepad.getButtonPressed(xButtons.Back))
    {
      disabled = !disabled;
    }
    teleop();
    gamepad.refresh();
  }

  cout << " Exiting" << endl;

  return 0;
}
