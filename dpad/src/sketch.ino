#include "image.h"
#include <Wire.h>
#include "clock.h"
#include "smart_card.h"
#include "LPD8806x8.h"


Image img;
Palette pal(256);

uint8_t pin = 18;

uint8_t clockPin = 42;
uint8_t clockInhibitPin = 43;
uint8_t loadInhibitPin = 44;
uint8_t indicator = 13;

DS1307_Clock clock;

uint8_t datapin = 6;
uint8_t clockpin = 7;
LPD8806x8 teststrip;

const uint8_t STRIP_IMAGE_LENGTH = 128;

StripImage stripImage(STRIP_IMAGE_LENGTH);


void setup()
{
  Serial.begin(9600);
  Serial.println("Starting...");

  //stripSpeedTest();
  //smart_card_test();

  int i = 0;
  const int mut = 4;

  const int num_colors = 8;
  for (uint8_t j = 0; j < mut; ++j)
    pal.set_color(i++, 255, 0, 0);
  for (uint8_t j = 0; j < mut; ++j)
    pal.set_color(i++, 255, 128, 0);
  for (uint8_t j = 0; j < mut; ++j)
    pal.set_color(i++, 255, 255, 0);
  for (uint8_t j = 0; j < mut; ++j)
    pal.set_color(i++, 0, 255, 0);
  for (uint8_t j = 0; j < mut; ++j)
    pal.set_color(i++, 0, 0, 255);
  for (uint8_t j = 0; j < mut; ++j)
    pal.set_color(i++, 128, 0, 255);
  for (uint8_t j = 0; j < mut; ++j)
    pal.set_color(i++, 255, 255, 255);
  for (uint8_t j = 0; j < mut; ++j)
    pal.set_color(i++, 0, 0, 0);
  
  for (uint8_t i = 0; i < 128; ++i)
    stripImage.set_color(i, 24);
}

void loop()
{
  delay(34);
  //pal.cycle_colors(32);
  
  teststrip.show(&stripImage, &pal);
  //loopImageTest();
}

void smart_card_test()
{
  uint16_t size = 5;

  SmartCard::init();
  SmartCard card;

  Serial.println(F("Writing ")); 
  
  for (uint16_t i = 0; i < size; ++i)
  {
    card.write(i);
    delay(5);
  }

  card.seek(0L);
  
  Serial.println(F("Reading..."));
  for (uint16_t i = 0; i < size; ++i)
  {
    Serial.print( card.read() );
  }
  Serial.println("");

  card.seek(2L);
  Serial.println(F("Reading..."));
  for (uint16_t i = 0; i < 3; ++i)
  {
    Serial.print( card.read() );
  }
  Serial.println("");

}
    

void setupImageTest()
{
  uint8_t s, r;
  for (uint8_t y = 0; y < 32; ++y)
  {
    img.rowcol(31, y, s, r);
    Serial.print(F("Strip "));
    Serial.print(s);
    Serial.print(F(", index "));
    Serial.println(r);
  }
}

void loopImageTest()
{
  delay(1000);
}

void rtcSetup2()
{
  clock.begin();

  ds1307_time t;
  t.seconds = 0;
  t.minutes = 36;
  t.hours = 19;
  t.day_of_week = CLOCK_WEEKDAY_SUNDAY;
  t.day = 1;
  t.month = 12;
  t.year = 13;

  clock.set_time(t, true);
}

void rtcLoop2()
{
  delay(2000);
  
  ds1307_time time;
  clock.get_time(&time);
  clock.print_time(time);
}

void setupLEDTest()
{
  DDRC = B11111111;
 
  pinMode(clockPin, OUTPUT);
  pinMode(clockInhibitPin, OUTPUT);
  pinMode(loadInhibitPin, OUTPUT);
  pinMode(indicator, OUTPUT);

  DDRL = B11100000;
  PORTL = B01100000;
  // digitalWrite(clockPin, LOW);
  // digitalWrite(clockInhibitPin, HIGH);
  // digitalWrite(loadInhibitPin, HIGH);
  
  digitalWrite(indicator, LOW);
}

void ledLoop()
{
  sendP2SByte(B11110000);
  delay(5);
  sendP2SByte(B11000011);
  delay(5);
  sendP2SByte(B11001100);
  delay(5);
  sendP2SByte(B10101010);
  delay(5);
}

// clock, CLOCK_INH, LOAD_INH
void sendP2SByte(uint8_t byte)
{
  digitalWrite(indicator, HIGH);
  PORTC = byte;
  PORTL = B01000000; //digitalWrite(loadInhibitPin, LOW);
  //delay(25);
  PORTL = B01100000; // digitalWrite(loadInhibitPin, HIGH);

  
  PORTL = B11100000; // digitalWrite(clockPin, HIGH);
  PORTL = B10100000; // digitalWrite(clockInhibitPin, LOW);
  //delay(25);
  PORTL = B00100000; // digitalWrite(clockPin, LOW);
  
  for (int i = 0; i < 6; ++i) {
  
  PORTL = B10100000; // digitalWrite(clockPin, HIGH);
  PORTL = B00100000; // digitalWrite(clockPin, LOW);
}

  //delay(25);
  PORTL = B10100000; // digitalWrite(clockPin, HIGH);
  PORTL = B11100000; // digitalWrite(clockInhibitPin, HIGH);
  //delay(25);
  PORTL = B01100000; // digitalWrite(clockPin, LOW);
  
  digitalWrite(indicator, LOW);
}

void stripSpeedTest(void)
{
  long start, elapsed;
  Serial.print("Image speed:");

  start = micros();
  for (uint8_t i = 0; i < 32; ++i)
    for (uint8_t j = 0; j < 32; ++j)
      img.set_color(i, j, i*j);
  elapsed = micros() - start;
  
  Serial.printf("%d us\n", elapsed);

  Serial.print("Palette speed:");
  start = micros();
  for (uint16_t i = 0; i < 256; ++i)
    pal.set_color(i, i+1, i*2, i * i);

  elapsed = micros() - start;
  Serial.printf("%d us\n", elapsed);

  Serial.print("LPD8806x8 show speed: ");
  start = micros();
  teststrip.show(&img, &pal);
  elapsed = micros() - start;
  Serial.printf("%d us\n", elapsed);
}
