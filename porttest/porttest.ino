#include <Arduino.h>

void setup()
{
  DDRA = B11111111;
}

unsigned char SET_X = 0;

void loop()
{
  SET_X += 1;
  PORTA = SET_X;
  delay(1000);
}
