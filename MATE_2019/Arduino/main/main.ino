#include <Servo.h>
#include "pins.h"

Servo FR;
Servo FL;
Servo BL;
Servo BR;

Servo UL;
Servo UR;
Servo UB;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.setTimeout(80);
  
  FR.attach(0);
  FL.attach(1);
  BL.attach(2);
  BR.attach(3);

  UL.attach(4);
  UR.attach(5);
  UB.attach(6);

  // 1500 is neutral
  FR.writeMicroseconds(1500);
  FL.writeMicroseconds(1500);
  BL.writeMicroseconds(1500);
  BR.writeMicroseconds(1500);

  UL.writeMicroseconds(1500);
  UR.writeMicroseconds(1500);
  UB.writeMicroseconds(1500);
}

void loop() {
  char driveCommands[35];
  // put your main code here, to run repeatedly:
  if (Serial.available())
    {
    String info = Serial.readStringUntil('\n');
    Serial.flush();
    writeString(info);
    info.toCharArray(driveCommands, 35);
    drive(driveCommands);
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
