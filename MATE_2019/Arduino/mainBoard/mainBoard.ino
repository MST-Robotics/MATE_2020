// Teensy++ 2.0

#include <LSM9DS1_Registers.h>
#include <LSM9DS1_Types.h>
#include <Wire.h>
#include <SPI.h>
#include <SparkFunLSM9DS1.h>

#include <Servo.h>

#define LSM9DS1_M 0x1E   // Would be 0x1C if SDO_M is LOW
#define LSM9DS1_AG 0x6B  // Would be 0x6A if SDO_AG is LOW

LSM9DS1 imu;

Servo FR;
Servo BR;
Servo BL;
Servo FL;

Servo UL;
Servo UR;
Servo UB;

Servo shoulderTilt;
Servo wristTilt;
Servo wristTwist;

const int buzzer = 17;
const int water1 = 20;
const int water2 = 21;
const int water3 = 22;
const int water4 = 23;

const int COMMAND_SIZE = 52;
const int MOTOR_NEUTRAL = 1500;

int timer = 0;

String disabledCommand = ":1500;1500;1500;1500;1500;1500;1500;1500;1500;1500;0";

void setup()
{
  Wire.begin();
  Serial1.begin(115200);
  Serial1.setTimeout(80);

  imu.settings.device.commInterface = IMU_MODE_I2C;
  imu.settings.device.mAddress = LSM9DS1_M;
  imu.settings.device.agAddress = LSM9DS1_AG;

  FR.attach(27);
  BR.attach(14);
  BL.attach(15);
  FL.attach(16);

  UL.attach(25);
  UR.attach(24);
  UB.attach(26);

//  shoulderTilt.attach(9);
//  wristTilt.attach(10);
//  wristTwist.attach(11);

  pinMode(buzzer, OUTPUT);
  pinMode(water4, INPUT);

  FR.writeMicroseconds(MOTOR_NEUTRAL);
  BR.writeMicroseconds(MOTOR_NEUTRAL);
  BL.writeMicroseconds(MOTOR_NEUTRAL);
  FL.writeMicroseconds(MOTOR_NEUTRAL);

  UL.writeMicroseconds(MOTOR_NEUTRAL);
  UR.writeMicroseconds(MOTOR_NEUTRAL);
  UB.writeMicroseconds(MOTOR_NEUTRAL);

  // Scream untill IMU connected
  digitalWrite(buzzer, 1);
  if (!imu.begin())
  {
    while(1)
    {
    }
  }
  digitalWrite(buzzer, 0);
}

void loop()
{
  char driveCommands[COMMAND_SIZE];

  ++timer;

  float ax = 0.0;
  float ay = 0.0;
  float az = 0.0;

  if (imu.accelAvailable())
  {
    // Updates ax, ay, and az
    imu.readAccel();
    ax = imu.ax;
    ay = imu.ay;
    az = imu.az;
  }

  //if (imu.gyroAvailable())
  {
    // Updates gx, gy, and gz
    //imu.readGyro();
  }

  float roll = atan2(ay, az) * 180 / PI;
  float pitch = atan2(-ax, sqrt(ay * ay + az * az)) * 180 / PI;

  char cstr[16];
  itoa(timer, cstr, 10);

  // Wait untill there is at least 1 full command to read
  if (Serial1.available() >= COMMAND_SIZE - 1)
  {
    // Don't read a string that starts in the middle of a command
    if (Serial1.read() == ':')
    {
	    timer = 0;  // Reset timer if valid data received

      // Only send data back if data was received
      if (digitalRead(water4) && digitalRead(water3) && digitalRead(water2) && digitalRead(water1))
      {
        writeString("0");
      }
      else
      {
        writeString("1");
      }

      String info = Serial1.readStringUntil('\n');
      info.toCharArray(driveCommands, COMMAND_SIZE - 1);
      drive(driveCommands);

      digitalWrite(buzzer, 1);

      // Clear any backlog commands
      Serial1.flush();
    }
    else
    {
      // Clear invalid command
      Serial1.readStringUntil('\n');
    }
  }
  
  // Only run if a command has been received within ~one second
  if (timer > 250)
  {
    disabledCommand.toCharArray(driveCommands, COMMAND_SIZE - 1); 
    drive(driveCommands);
  }
  //Rough timer counting
  delay(1);
  digitalWrite(buzzer, 0);
}

// Used to serially push out a String with Serial.write()
void writeString(String stringData)
{
  for (unsigned int i = 0; i < stringData.length(); i++)
  {
    Serial1.write(stringData[i]);  // Push each char 1 by 1 on each loop pass
  }
}

void drive(char array[])
{
  char *commands[35];
  char *ptr = NULL;
  byte index = 0;
  ptr = strtok(array, ";");
  while (ptr != NULL)
  {
    commands[index] = ptr;
    index++;
    ptr = strtok(NULL, ";");
  }

  Wire.beginTransmission(10);
  Wire.write(commands[0]);
  Wire.write(commands[3]);
  Wire.write(':');
  Wire.endTransmission();

  Wire.beginTransmission(11);
  Wire.write(commands[2]);
  Wire.write(commands[1]);
  Wire.write(':');
  Wire.endTransmission();

  Wire.beginTransmission(12);
  Wire.write(commands[4]);
  Wire.write(commands[5]);
  Wire.write(':');
  Wire.endTransmission();

  Wire.beginTransmission(13);
  Wire.write(commands[6]);
  Wire.write(commands[7]);
  Wire.write(':');
  Wire.endTransmission();

  writeCommands(atoi(commands[0]), atoi(commands[3]), atoi(commands[2]),
                atoi(commands[1]), atoi(commands[4]), atoi(commands[5]),
                atoi(commands[6]), 
                atoi(commands[7]), atoi(commands[8]), atoi(commands[9]),
                atoi(commands[10]));
}

void writeCommands(int FR, int FL, int BL, int BR, int UL, int UR, int UB, int ST, int WTI, int WTW,
                   int c)
{ 
  setFR(FR);
  setFL(FL);
  setBL(BL);
  setBR(BR);

  setUL(UL);
  setUR(UR);
  setUB(UB);

  setST(ST);
  setWTI(WTI);
  setWTW(WTW);
}

void setFR(int num) { FR.writeMicroseconds(num); }

void setBR(int num) { BR.writeMicroseconds(num); }

void setBL(int num) { BL.writeMicroseconds(num); }

void setFL(int num) { FL.writeMicroseconds(num); }

void setUL(int num) { UL.writeMicroseconds(num); }

void setUR(int num) { UR.writeMicroseconds(num); }

void setUB(int num) { UB.writeMicroseconds(num); }

void setST(int num) { /*shoulderTilt.writeMicroseconds(num);*/ }

void setWTI(int num) { /*wristTilt.writeMicroseconds(num);*/ }

void setWTW(int num) { /*wristTwist.writeMicroseconds(num);*/ }
