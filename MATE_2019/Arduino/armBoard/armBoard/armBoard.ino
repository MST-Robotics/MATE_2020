#include <Servo.h>
#include <Wire.h>

#define HEARTBEAT_MILLIS 250
#define RECV_MILLIS 100
#define I2C_ADDR 14
#define SERVO_MIN 1000
#define SERVO_MAX 2000

#define SERVOS 5 //pin 11 is not a PWM Pin! it will not work with the Servo library

#define LED_RECV 12
#define LED_ERR 8

uint8_t servo_pins[6] = {3, 5, 6, 9, 10, 11}; 
//uint16_t servo_value[6] = [1500, 1500, 1500, 1500, 1500, 1500];

Servo servos[6];

void setup()
{
  pinMode(LED_BUILTIN,OUTPUT);
  pinMode(LED_RECV,OUTPUT);
  pinMode(LED_ERR,OUTPUT);
  digitalWrite(LED_ERR,HIGH);
  digitalWrite(LED_RECV,HIGH);
  digitalWrite(LED_BUILTIN,HIGH);
  delay(1000);
  digitalWrite(LED_ERR,LOW);
  digitalWrite(LED_RECV,LOW);
  digitalWrite(LED_BUILTIN,LOW);
  
  Wire.begin(I2C_ADDR);
  Wire.onReceive(receiveEvent);
  for(int i = 0; i < SERVOS; i++)
  {
    servos[i].attach(servo_pins[i]);
  }
}


long clear_led_time = 0;

void loop()
{
  static long last_time = millis();
  if (millis()-last_time  > HEARTBEAT_MILLIS)
  {
    digitalWrite(LED_BUILTIN,!digitalRead(LED_BUILTIN));
    last_time = millis();
  }
  if ((clear_led_time) && (millis() >= clear_led_time))
  {
    digitalWrite(LED_RECV,LOW);
    clear_led_time = 0;
  }
}

void receiveEvent(int how_many)
{
  digitalWrite(LED_RECV,HIGH);
  clear_led_time = millis()+RECV_MILLIS;
  if (how_many == 3) //if the message is a valid command length
  {
    uint8_t index = Wire.read();
    if (index < SERVOS) //if the index given is valid
    {
      uint16_t value = ((uint8_t)Wire.read())*(1<<8)+((uint8_t)Wire.read()); //value is [high] [low]
      if ((value >= SERVO_MIN) && (value <= SERVO_MAX)) //if value is in a valid range
      {
        servos[index].writeMicroseconds(value); //update that motor
      }
      else
      {
        digitalWrite(LED_ERR,HIGH);
      }
    }
    else
    {
      digitalWrite(LED_ERR,HIGH);
    }
  }
  else
  {
    digitalWrite(LED_ERR,HIGH);
  }
  
  while(Wire.available()) //get rid of any extra data
  {
    Wire.read();
  }
}
