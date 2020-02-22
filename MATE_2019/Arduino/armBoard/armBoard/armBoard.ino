#define HEARTBEAT_MICROS 250

void setup()
{
  pinMode(LED_BUILTIN,OUTPUT);
}

void loop()
{
  static long last_time = millis();
  if (millis()-last_time  > HEARTBEAT_MICROS)
  {
    digitalWrite(LED_BUILTIN,!digitalRead(LED_BUILTIN));
    last_time = millis();
  }
}
