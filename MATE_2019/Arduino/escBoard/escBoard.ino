// Arduino Nano
// ATmega328P (Old Bootloader)

#include <Servo.h>
#include <Wire.h>

Servo M0;
Servo M1;

const int MOTOR_NEUTRAL = 1500;

int commandM0 = MOTOR_NEUTRAL;
int commandM1 = MOTOR_NEUTRAL;
  
void setup() 
{
  Wire.begin(10);
  Wire.onReceive(receiveEvent);

  pinMode(PD3, OUTPUT); //STAT0
  pinMode(PD7, OUTPUT); // Enable
  digitalWrite(PD7, 1);
  pinMode(PD6, OUTPUT); // Voltage Set
  analogWrite(PD6, 128);
  
  M0.attach(PD2);
  M1.attach(PD5);  
}

void loop() 
{
  M0.writeMicroseconds(commandM0);
  M1.writeMicroseconds(commandM1);
}

// Expecting WXYZABCD:, WXYZ goes to M0, ABCD goes to M1
void receiveEvent(int howMany)
{
  digitalWrite(PD3, 1);
  static char commandInput[9];
  char command1[4];
  char command2[4];
  int x = 0;
  
  while (Wire.available())
  {
    commandInput[x++] = Wire.read();
  }

  if (commandInput[8] == ':')
  {
    for (int i = 0; i < 4; ++i)
    {
      command1[i] = commandInput[i];
      command2[i] = commandInput[i+4];
    }
  
    for (int i = 0; i < 9; ++i)
    {
      commandInput[i] = '0';
    }
  
    commandM0 = atoi(command1);
    commandM1 = atoi(command1);
    digitalWrite(PD3, 0);
  }
  
}
