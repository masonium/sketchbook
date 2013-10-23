#include "LPD8806.h"
#include "SPI.h"

// Example to control LPD8806-based RGB LED Modules in a strip

/*****************************************************************************/

// Number of RGB LEDs in strand:
int nLEDs = 120;

// Chose 2 pins for output; can be any valid output pins
int dataPin  = 11;
int clockPin = 12;

// First parameter is the number of LEDs in the strand.  The LED strips
// are 32 LEDs per meter but you can extend or cut the strip.  Next two
// parameters are SPI data and clock pins:
LPD8806 strip = LPD8806(nLEDs, dataPin, clockPin);

// You can optionally use hardware SPI for faster writes, just leave out
// the data and clock pin parameters.  But this does limit use to very
// specific pins on the Arduino.  For "classic" Arduinos (Uno, Duemilanove,
// etc.), data = pin 11, clock = pin 13.  For Arduino Mega, data = pin 51,
// clock = pin 52.  For 32u4 Breakout Board+ and Teensy, data = pin B2,
// clock = pin B1.  For Leonardo, this can ONLY be done on the ICSP pins.
//LPD8806 strip = LPD8806(nLEDs);
unsigned long elapsed = 0;
unsigned long last = 0;
uint8_t offset = 0;

unsigned long interval = 250;
void setup() {
  uint8_t i = 0;
  // Start up the LED strip
  strip.begin();
    
  strip.showBegin();
  for (i = 0; i < nLEDs; ++i)
    strip.addPixel(0, 0, 0);
  strip.showEnd();
}

class Pattern
{
public:  
  int intensity;
  int rm, gm, bm;
  bool half;
};


void solid_color(struct Pattern p)
{
  strip.showBegin();
  for (int i = 0; i < nLEDs; ++i) 
  {
    int m = !p.half || (i % 2 == 0) ? 1 : 0;
    strip.addPixel((p.intensity * m * p.rm) >> 1,
                   (p.intensity * m * p.gm) >> 1,
                   (p.intensity * m * p.bm) >> 1);
  }
  strip.showEnd();
}

void loop() {
  uint8_t i;
  unsigned long curr = millis();
  elapsed += curr - last;
  last = curr;
  
  while (elapsed > interval) {
    offset = (offset + 1) % 3;
    elapsed -= interval;
  }
  
  int transitionTime = 3000;
  
  // hues(0.5f);
  // delay(transitionTime);
  // hues(0.25f);
  // delay(transitionTime);
  // red();
  // delay(transitionTime);
  // redgreen();
  // delay(transitionTime);
  // green();
  // delay(transitionTime);
  // greenblue();
  // delay(transitionTime);
  // blue();
  // delay(transitionTime);
  // bluered();
  // delay(transitionTime);
  // hues(1.0);
  // delay(transitionTime);
  Pattern x[] = {{255, 1, 1, 1, false},
                 {223, 1, 1, 1, false},
                 {191, 1, 1, 1, false},
                 {127, 1, 1, 1, false},
                 {63, 1, 1, 1, false},
                 {31, 1, 1, 1, false},
                 {255, 1, 0, 0, false},
                 {127, 1, 0, 0, false},
                 {63,  1, 0, 0, false},
                 {255, 0, 1, 0, false},
                 {127, 0, 1, 0, false},
                 {63,  0, 1, 0, false},
                 {255, 0, 0, 1, false},
                 {127, 0, 0, 1, false},
                 {63,  0, 0, 1, false},
                 {255, 1, 1, 0, false},
                 {127, 1, 1, 0, false},
                 {63,  1, 1, 0, false},
                 {255, 1, 0, 1, false},
                 {127, 1, 0, 1, false},
                 {63,  1, 0, 1, false},
                 {255, 0, 1, 1, false},
                 {127, 0, 1, 1, false},
                 {63,  0, 1, 1, false},
                 {255, 1, 1, 1, true},
                 {127, 1, 1, 1, true},
                 {255, 1, 0, 0, true},
                 {255, 0, 1, 0, true},
                 {255, 0, 0, 1, true},
                 {63, 1, 0, 0, true},
                 {63, 0, 1, 0, true},
                 {63, 0, 0, 1, true}};
  
  hues(1.0);
  delay(5000);
  
  for (int i = 0; i < sizeof(x) / sizeof(Pattern); ++i)
  {
    solid_color(x[i]);
    delay(5000);
  }
}

void address()
{
  strip.showBegin();
  strip.addPixel(0, 0, 255);
  for (int i = 0; i < nLEDs-1; ++i)
    strip.addPixel(0, 0, 0);
  strip.showEnd();
}
void white(int x)
{
  strip.showBegin();
  for (int i = nLEDs - 1; i >= 0; --i) {
    strip.addPixel(x, x, x);
  }
  strip.showEnd();
}


void red()
{
  strip.showBegin();
  for (int i = nLEDs - 1; i >= 0; --i) {
    unsigned char brightness = 127 * i / (nLEDs - 1);
    strip.addPixel(brightness, 0, 0);
  }
  strip.showEnd();
}


void green()
{
  strip.showBegin();
  for (int i = (nLEDs - 1); i >= 0; --i) {  
    unsigned char brightness = 127 * i / (nLEDs - 1);
    strip.addPixel(0, brightness, 0);
  }
  strip.showEnd();
}
void blue()
{
  strip.showBegin();
  for (int i = (nLEDs - 1); i >= 0; --i) {  
    unsigned char brightness = 127 * i / (nLEDs - 1);
    strip.addPixel(0, 0, brightness);
  }
  strip.showEnd();
}

void redgreen()
{
  strip.showBegin();
  for (int i = 0; i < nLEDs; ++i) {
    int x = i * 127 / (nLEDs - 1);
    strip.addPixel( x, 127 - x, 0 );
  }
  strip.showEnd();
}

void greenblue()
{
  strip.showBegin();
  for (int i = 0; i < nLEDs; ++i) {
    int x = i * 127 / (nLEDs - 1);
    strip.addPixel( 0, x, 127 - x );
  }
  strip.showEnd();
}

void bluered()
{
  strip.showBegin();
  for (int i = 0; i < nLEDs; ++i) {
    int x = i * 127 / (nLEDs - 1);
    strip.addPixel( 127 - x, 0, x );
  }
  strip.showEnd();
}

void HSVtoRGB( float *r, float *g, float *b, float h, float s, float v )
{
  int i;
  float f, p, q, t;

  if( s == 0 ) {
    // achromatic (grey)
    *r = *g = *b = v;
    return;
  }

  h /= 60;			// sector 0 to 5
  i = floor( h );
  f = h - i;			// factorial part of h
  p = v * ( 1 - s );
  q = v * ( 1 - s * f );
  t = v * ( 1 - s * ( 1 - f ) );

  switch( i ) {
  case 0:
    *r = v;
    *g = t;
    *b = p;
    break;
  case 1:
    *r = q;
    *g = v;
    *b = p;
    break;
  case 2:
    *r = p;
    *g = v;
    *b = t;
    break;
  case 3:
    *r = p;
    *g = q;
    *b = v;
    break;
  case 4:
    *r = t;
    *g = p;
    *b = v;
    break;
  default:		// case 5:
    *r = v;
    *g = p;
    *b = q;
    break;
  }

}

void hues(float value)
{
  strip.showBegin();
  float r, g, b;
  for (int i = 0; i < nLEDs; ++i) {
    float hue_p = (i * 360) / (nLEDs * 1.0f);
    r = g = b = 0.0f;
    HSVtoRGB(&r, &g, &b, hue_p, value, value);
    strip.addPixel(r * 127, g * 127, b * 127);
  }
  strip.showEnd();
}
