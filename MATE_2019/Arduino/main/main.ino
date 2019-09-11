#include <LSM9DS1_Registers.h>
#include <LSM9DS1_Types.h>
#include <SparkFunLSM9DS1.h>

#include <Servo.h>
#include "pins.h"

Servo FR;
Servo FL;
Servo BL;
Servo BR;

Servo UL;
Servo UR;
Servo UB;

const int COMMAND_SIZE = 36;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.setTimeout(80);
  
  FR.attach(2);
  FL.attach(3);
  BL.attach(4);
  BR.attach(5);

  UL.attach(6);
  UR.attach(7);
  UB.attach(8);

  // 1500 is neutral
  FR.writeMicroseconds(1500);
  FL.writeMicroseconds(1500);
  BL.writeMicroseconds(1500);
  BR.writeMicroseconds(1500);

  UL.writeMicroseconds(1500);
  UR.writeMicroseconds(1500);
  UB.writeMicroseconds(1500);

  randomSeed(analogRead(0));
}

void loop() {
  char driveCommands[COMMAND_SIZE];

  // Replace random with actual IMU data
  String IMUString = ":";
  IMUString = IMUString + (int) random(0, 360) + ";";
  IMUString = IMUString + (int) random(-180, 180) + ";";
  IMUString = IMUString + (int) random(-180, 180) + "|";

  // Wait untill there is at least 1 full command to read
  if (Serial.available() >= COMMAND_SIZE-1)
  {
    // Don't read a string that starts in the middle of a command
    if (Serial.read() == ':')
    {
      // Only send data back if data was received
      writeString(IMUString);
      
      String info = Serial.readStringUntil('\n');
      info.toCharArray(driveCommands, COMMAND_SIZE-1);
      drive(driveCommands);

      // Clear any backlog commands
      Serial.flush();
    }
    else
    {
      // Clear invalid command
      Serial.readStringUntil('\n');
    }
  }
}

// Used to serially push out a String with Serial.write()
void writeString(String stringData) {
  for (int i = 0; i < stringData.length(); i++)
  {
    Serial.write(stringData[i]);   // Push each char 1 by 1 on each loop pass
  }
}

void drive(char array[])
{
  char *commands[35];
  char *ptr = NULL;
  byte index = 0;
  ptr = strtok(array, ";");
  while(ptr != NULL)
  {
      commands[index] = ptr;
      index++;
      ptr = strtok(NULL, ";");
  }

  setFR(atoi(commands[0]));
  setFL(atoi(commands[3]));
  setBL(atoi(commands[2]));
  setBR(atoi(commands[1]));
  
  setUL(atoi(commands[4]));
  setUR(atoi(commands[5]));
  setUB(atoi(commands[6]));

}

void setFR(int num)
{
  FR.writeMicroseconds(num);
}

void setFL(int num)
{
  FL.writeMicroseconds(num);
}

void setBL(int num)
{
  BL.writeMicroseconds(num);
}

void setBR(int num)
{
  BR.writeMicroseconds(num);
}

void setUL(int num)
{
  UL.writeMicroseconds(num);
}

void setUR(int num)
{
  UR.writeMicroseconds(num);
}

void setUB(int num)
{
  UB.writeMicroseconds(num);
}
