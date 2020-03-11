// Teensy++ 2.0

#include <LSM9DS1_Registers.h>
#include <LSM9DS1_Types.h>
#include <Wire.h>
#include <SPI.h>
#include <SparkFunLSM9DS1.h>
#include <SensorFusion.h>

#define LSM9DS1_M 0x1E   // Would be 0x1C if SDO_M is LOW
#define LSM9DS1_AG 0x6B  // Would be 0x6A if SDO_AG is LOW

#define SerialConnection Serial1

LSM9DS1 imu;
SF imuFusion;

const int imuReadingNum = 5;

float imuReadings[imuReadingNum*2];
int imuReadIndex = 0;

float pitchTotal = 0.0f;
float rollTotal = 0.0f;

int loops = 0;

const int buzzer = 17;
const int water1 = 20;
const int water2 = 21;
const int water3 = 22;
const int water4 = 23;

const int COMMAND_SIZE = 52;
const int MOTOR_NEUTRAL = 1500;

long lastCommandTime = 0;

String disabledCommand = ":1500;1500;1500;1500;1500;1500;1500;1500;1500;1500;0";

void setup()
{
  for (int i = 0; i < imuReadingNum * 2; ++i)
  {
    imuReadings[i] = 0.0f;
  }
  
  delay(1000); 

  Wire.begin();
  SerialConnection.begin(115200);
  SerialConnection.setTimeout(80);

  imu.settings.device.commInterface = IMU_MODE_I2C;
  imu.settings.device.mAddress = LSM9DS1_M;
  imu.settings.device.agAddress = LSM9DS1_AG;

  pinMode(buzzer, OUTPUT);
  pinMode(water4, INPUT);

  // Scream untill IMU connected
  //digitalWrite(buzzer, 1);

  while (!imu.begin())
  {
    delay(100);
  }

  digitalWrite(buzzer, 0);
}

void loop()
{
  static bool wireInit = false;
  while (!wireInit)
  {
    Wire.beginTransmission(10);
    Wire.write("1500");
    Wire.write("1500");
    Wire.write(':');
    Wire.endTransmission();
  
    Wire.beginTransmission(11);
    Wire.write("1500");
    Wire.write("1500");
    Wire.write(':');
    Wire.endTransmission();
  
    Wire.beginTransmission(12);
    Wire.write("1500");
    Wire.write("1500");
    Wire.write(':');
    Wire.endTransmission();
  
    Wire.beginTransmission(13);
    Wire.write("1500");
    Wire.write("1500");
    Wire.write(':');
    Wire.endTransmission();
    delay(90);
    ++timer;

    if (timer > 5)
    {
      wireInit = true;
    }
  }
  char driveCommands[COMMAND_SIZE];

  ++timer;

  float ax = 0.0;
  float ay = 0.0;
  float az = 0.0;

  float gx = 0.0;
  float gy = 0.0;
  float gz = 0.0;

  float mx = 0.0;
  float my = 0.0;
  float mz = 0.0;

  float deltaT;

  if (imu.accelAvailable())
  {
    // Updates Accelerometer data
    imu.readAccel();
    ax = imu.ax;//imu.calcAccel(imu.ax);
    ay = imu.ay;//imu.calcAccel(imu.ay);
    az = imu.az;//imu.calcAccel(imu.az);
  }

  if (imu.gyroAvailable())
  {
    // Updates Gyroscope data
    imu.readGyro();
    gx = imu.calcGyro(imu.gx) * DEG_TO_RAD;
    gy = imu.calcGyro(imu.gy) * DEG_TO_RAD;
    gz = imu.calcGyro(imu.gz) * DEG_TO_RAD;
  }

  if (imu.magAvailable())
  {
    // Updates Compass data
    imu.readMag();
    mx = imu.mx * 0.1;
    my = imu.my * 0.1;
    mz = imu.mz * 0.1;
  }

  deltaT = imuFusion.deltatUpdate();
  imuFusion.MahonyUpdate(gx, gy, gz, ax, ay, az, deltaT);

  pitchTotal -= imuReadings[imuReadIndex];
  imuReadings[imuReadIndex] = atan2(-ax, sqrt(ay * ay + az * az)) * RAD_TO_DEG;//imuFusion.getPitch();
  pitchTotal += imuReadings[imuReadIndex];

  rollTotal -= imuReadings[imuReadIndex+1];
  imuReadings[imuReadIndex+1] = atan2(ay, az) * RAD_TO_DEG;//imuFusion.getRoll();
  rollTotal += imuReadings[imuReadIndex+1];

  imuReadIndex += 2;
  if (imuReadIndex >= imuReadingNum*2) 
  {
    imuReadIndex = 0;
  }

  float pitch = pitchTotal / imuReadingNum;
  float roll = rollTotal / imuReadingNum;

  //float pitch = imuFusion.getPitch();
  //float roll = imuFusion.getRoll();

  //float roll = atan2(ay, az) * RAD_TO_DEG;
  //float pitch = atan2(-ax, sqrt(ay * ay + az * az)) * RAD_TO_DEG;

  ++loops;

  char cstr[16];
  itoa(timer, cstr, 10);

  // Wait untill there is at least 1 full command to read
  if (SerialConnection.available() >= COMMAND_SIZE - 1)
  {
    // Don't read a string that starts in the middle of a command
    if (SerialConnection.read() == ':')
    {
	    timer = 0;  // Reset timer if valid data received
      // Only send data back if data was received
      
      String info;
      info = disabledCommand;

      info = SerialConnection.readStringUntil('\n');
      info.remove(COMMAND_SIZE-1);
      info.toCharArray(driveCommands, COMMAND_SIZE - 1);

      // Send back Water Sensor data
      if (digitalRead(water4) && digitalRead(water3) && digitalRead(water2) && digitalRead(water1))
      {
        writeString("0;");
      }
      else
      {
        writeString("1;");
      }


      // Send back IMU data
      writeString(String(pitch,2));
      writeString(";");
      writeString(String(roll,2));

      writeString(";");
      writeString(String(loops));

      loops = 0;
      lastCommandTime = millis();

      digitalWrite(buzzer, 1);

      clearSerial();
    }
    else
    {
      // Clear invalid command
      SerialConnection.readStringUntil('\n');
    }
  }
  
  // Only run if a command has been received within 200 milliseconds
  if (millis() - lastCommandTime > 200)
  {
    disabledCommand.toCharArray(driveCommands, COMMAND_SIZE - 1); 
  }

  drive(driveCommands);
  
  //Rough timer counting
  delay(1);
  digitalWrite(buzzer, 0);
}

void clearSerial() {
  while(SerialConnection.available())
  {
    SerialConnection.read();
  }
}

// Used to serially push out a String with Serial.write()
void writeString(String stringData)
{
  for (unsigned int i = 0; i < stringData.length(); i++)
  {
    SerialConnection.write(stringData[i]);  // Push each char 1 by 1 on each loop pass
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
  
}
