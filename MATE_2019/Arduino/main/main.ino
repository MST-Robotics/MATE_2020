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
  // put your main code here, to run repeatedly:
  if (Serial.available()){
    String info = Serial.readStringUntil('\n');
    Serial.flush();
    writeString(info);
  }
}

// Used to serially push out a String with Serial.write()
void writeString(String stringData) {
  for (int i = 0; i < stringData.length(); i++)
  {
    Serial.write(stringData[i]);   // Push each char 1 by 1 on each loop pass
  }
}
