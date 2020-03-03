// Arduino Nano
// ATmega328P (Old Bootloader)
// (probably)

#include <Servo.h>
#include <Wire.h>

Servo M0; // Shoulder
Servo M1; // Wrist
Servo M2; // Claw

const int MOTOR_NEUTRAL = 1500;

int commandM0 = MOTOR_NEUTRAL;
int commandM1 = MOTOR_NEUTRAL;
int commandM2 = MOTOR_NEUTRAL;
  
void setup() 
{
  int i2c_addr = 10;
  Wire.begin(i2c_addr);
  Wire.onReceive(receiveEvent);

  pinMode(PD2, OUTPUT);
  pinMode(PD5, OUTPUT);
  pinMode(PD???, OUTPUT);
  pinMode(PD3, OUTPUT); //STAT0
  pinMode(PD4, OUTPUT); //STAT1
  pinMode(PD7, OUTPUT); // Enable
  digitalWrite(PD7, 1);
  pinMode(PD6, OUTPUT); // Voltage Set
  analogWrite(PD6, 100);
  
  M0.attach(PD2);
  M1.attach(PD5);
  M2.attach(PD???);

}

void loop() 
{
  static long lastTime = millis();
  if(millis()-lastTime > 500)
  {
    lastTime = millis();
    digitalWrite(PD4,!digitalRead(PD4));
  }
  M0.writeMicroseconds(commandM0);
  M1.writeMicroseconds(commandM1);
  M2.writeMicroseconds(commandM2);
}

// Expecting WXYZABCDJ:, WXYZ goes to M0, ABCD goes to M1, J converted goes to M2
void receiveEvent(int howMany)
{
  digitalWrite(PD3, 1);
  char commandInput[9] = {"150015000:"};
  char command1[4] = {"1500"};
  char command2[4] = {"1500"};
  char command3[1] = {"0"};
  int x = 0;
  
  while (Wire.available() && x<9)
  {
    commandInput[x++] = Wire.read();
  }

  if (commandInput[9] == ':')
  {
    for (int i = 0; i < 4; ++i)
    {
      command1[i] = commandInput[i];
      command2[i] = commandInput[i+4];
    }
	command3[0] = commandInput[8];
    
    for (int i = 0; i < 9; ++i)
    {
      commandInput[i] = '0';
    }
    commandM0 = 1000*(command1[0]-'0')+100*(command1[1]-'0')+10*(command1[2]-'0')+(command1[3]-'0');
    commandM1 = 1000*(command2[0]-'0')+100*(command2[1]-'0')+10*(command2[2]-'0')+(command2[3]-'0');
	commandM2 = (command3[0] == '0') ? 1100 : 1900; 
    
    digitalWrite(PD3, 0);
  }
  
}
