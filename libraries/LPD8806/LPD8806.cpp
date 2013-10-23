/*
Arduino library to control LPD8806-based RGB LED Strips
Copyright (C) Adafruit Industries
MIT license

Clearing up some misconceptions about how the LPD8806 drivers work:

The LPD8806 is not a FIFO shift register.  The first data out controls the
LED *closest* to the processor (unlike a typical shift register, where the
first data out winds up at the *furthest* LED).  Each LED driver 'fills up'
with data and then passes through all subsequent bytes until a latch
condition takes place.  This is actually pretty common among LED drivers.

All color data bytes have the high bit (128) set, with the remaining
seven bits containing a brightness value (0-127).  A byte with the high
bit clear has special meaning (explained later).

The rest gets bizarre...

The LPD8806 does not perform an in-unison latch (which would display the
newly-transmitted data all at once).  Rather, each individual byte (even
the separate G, R, B components of each LED) is latched AS IT ARRIVES...
or more accurately, as the first bit of the subsequent byte arrives and
is passed through.  So the strip actually refreshes at the speed the data
is issued, not instantaneously (this can be observed by greatly reducing
the data rate).  This has implications for POV displays and light painting
applications.  The 'subsequent' rule also means that at least one extra
byte must follow the last pixel, in order for the final blue LED to latch.

To reset the pass-through behavior and begin sending new data to the start
of the strip, a number of zero bytes must be issued (remember, all color
data bytes have the high bit set, thus are in the range 128 to 255, so the
zero is 'special').  This should be done before each full payload of color
values to the strip.  Curiously, zero bytes can only travel one meter (32
LEDs) down the line before needing backup; the next meter requires an
extra zero byte, and so forth.  Longer strips will require progressively
more zeros.  *(see note below)

In the interest of efficiency, it's possible to combine the former EOD
extra latch byte and the latter zero reset...the same data can do double
duty, latching the last blue LED while also resetting the strip for the
next payload.

So: reset byte(s) of suitable length are issued once at startup to 'prime'
the strip to a known ready state.  After each subsequent LED color payload,
these reset byte(s) are then issued at the END of each payload, both to
latch the last LED and to prep the strip for the start of the next payload
(even if that data does not arrive immediately).  This avoids a tiny bit
of latency as the new color payload can begin issuing immediately on some
signal, such as a timer or GPIO trigger.

Technically these zero byte(s) are not a latch, as the color data (save
for the last byte) is already latched.  It's a start-of-data marker, or
an indicator to clear the thing-that's-not-a-shift-register.  But for
conversational consistency with other LED drivers, we'll refer to it as
a 'latch' anyway.

* This has been validated independently with multiple customers'
  hardware.  Please do not report as a bug or issue pull requests for
  this.  Fewer zeros sometimes gives the *illusion* of working, the first
  payload will correctly load and latch, but subsequent frames will drop
  data at the end.  The data shortfall won't always be visually apparent
  depending on the color data loaded on the prior and subsequent frames.
  Tested.  Confirmed.  Fact.
*/


#include "SPI.h"
#include "LPD8806.h"

/*****************************************************************************/

// Constructor for use with arbitrary clock/data pins:
LPD8806::LPD8806(uint16_t n, uint8_t dpin, uint8_t cpin) {
  updateLength(n);
  updatePins(dpin, cpin);
}

// Activate hard/soft SPI as appropriate:
void LPD8806::begin(void) {
  startBitbang();
  begun = true;
}

// Change pin assignments post-constructor, using arbitrary pins:
void LPD8806::updatePins(uint8_t dpin, uint8_t cpin) {

  datapin     = dpin;
  clkpin      = cpin;
  clkport = dataport = 0;
  clkpinmask = datapinmask = 0;

  clkport     = portOutputRegister(digitalPinToPort(cpin));
  clkpinmask  = digitalPinToBitMask(cpin);
  dataport    = portOutputRegister(digitalPinToPort(dpin));
  datapinmask = digitalPinToBitMask(dpin);

  if(begun == true) { // If begin() was previously invoked...
    startBitbang(); // Regardless, now enable 'soft' SPI outputs
  } // Otherwise, pins are not set to outputs until begin() is called.
}

#ifndef SPI_CLOCK_DIV8
  #define SPI_CLOCK_DIV8 4
#endif

// Enable software SPI pins and issue initial latch:
void LPD8806::startBitbang() {
  pinMode(datapin, OUTPUT);
  pinMode(clkpin , OUTPUT);
  if (dataport != 0) {
    // use low level bitbanging when we can
    *dataport &= ~datapinmask; // Data is held low throughout (latch = 0)
    for(uint16_t i=((numLEDs+31)/32)*8; i>0; i--) {
      *clkport |=  clkpinmask;
      *clkport &= ~clkpinmask;
    }
  } else {
    // can't do low level bitbanging, revert to digitalWrite
    digitalWrite(datapin, LOW);
    for(uint16_t i=((numLEDs+31)/32)*8; i>0; i--) {
      digitalWrite(clkpin, HIGH);
      digitalWrite(clkpin, LOW);
    }
  }
}

// Change strip length (see notes with empty constructor, above):
void LPD8806::updateLength(uint16_t n) {
  latchBytes = (n + 31) / 32;
}

uint16_t LPD8806::numPixels(void) {
  return numLEDs;
}

void LPD8806::addPixel(uint8_t r, uint8_t g, uint8_t b)
{
  addByte(g | 0x80);
  addByte(r | 0x80);
  addByte(b | 0x80);
}

void LPD8806::addPixel(uint32_t rgb)
{
  addPixel((rgb >> 8) & 0xff);
  addPixel((rgb >> 16) & 0xff);
  addPixel((rgb >> 0) & 0xff);

}

void LPD8806::addByte(uint8_t p)
{
  uint8_t bit;
  for(bit=0x80; bit; bit >>= 1) {
      
    if (dataport != 0) {
        
      if(p & bit) *dataport |=  datapinmask;
      else        *dataport &= ~datapinmask;
      *clkport |=  clkpinmask;
      *clkport &= ~clkpinmask;
        
    }
    else {
      if (p&bit) digitalWrite(datapin, HIGH);
      else digitalWrite(datapin, LOW);
      digitalWrite(clkpin, HIGH);
      digitalWrite(clkpin, LOW);
    }
  }
}

// This is how data is pushed to the strip.  Unfortunately, the company
// that makes the chip didnt release the protocol document or you need
// to sign an NDA or something stupid like that, but we reverse engineered
// this from a strip controller and it seems to work very nicely!
void LPD8806::showBegin(void) {
}

void LPD8806::showEnd(void) {
  for (uint16_t i = 0; i < latchBytes; ++i)
    addByte(0);
}

// Convert separate R,G,B into combined 32-bit GRB color:
uint32_t LPD8806::Color(byte r, byte g, byte b) {
  return ((uint32_t)(g | 0x80) << 16) |
         ((uint32_t)(r | 0x80) <<  8) |
                     b | 0x80 ;
}
