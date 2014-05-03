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


#include <SPI.h>
#include "LPD8806x8.h"

#define PORT1_DATAf( x ) PORT ## x
#define fPORT1_DATAf( x ) PORT1_DATAf( x )
#define PORT1_DATA  fPORT1_DATAf( LPD_PORT1 )

#define PORT2_DATAf( x ) PORT ## x
#define fPORT2_DATAf( x ) PORT2_DATAf( x )
#define PORT2_DATA  fPORT2_DATAf( LPD_PORT2 )

#define CONTROL_DATAf( x ) PORT ## x
#define fCONTROL_DATAf( x ) CONTROL_DATAf( x )
#define CONTROL_PORT  fCONTROL_DATAf( LPD_CONTROL_PORT )


#define PORT1_DDRf( x ) DDR ## x
#define fPORT1_DDRf( x ) PORT1_DDRf( x )
#define PORT1_DDR  fPORT1_DDRf( LPD_PORT1 )

#define PORT2_DDRf( x ) DDR ## x
#define fPORT2_DDRf( x ) PORT2_DDRf( x )
#define PORT2_DDR  fPORT2_DDRf( LPD_PORT2 )

#define CONTROL_DDRf( x ) DDR ## x
#define fCONTROL_DDRf( x ) CONTROL_DDRf( x )
#define CONTROL_DDR  fCONTROL_DDRf( LPD_CONTROL_PORT )

/*****************************************************************************/

// via Michael Vogt/neophob: empty constructor is used when strip length
// isn't known at compile-time; situations where program config might be
// read from internal flash memory or an SD card, or arrive via serial
// command.  If using this constructor, MUST follow up with updateLength()
// and updatePins() to establish the strip length and output pins!
LPD8806x8::LPD8806x8(void) {
  updatePins(); // Must assume hardware SPI until pins are set
}

// update the directions of the ports
void LPD8806x8::updatePins() {
  PORT1_DDR = 0xff;
  PORT2_DDR = 0xff;

  CONTROL_DDR = 0xff;
  CLOCK_DISABLE();
  UNSELECT_STRIPS();
}

void LPD8806x8::show(const StripImage* img, const Palette* pal) {

  for (uint8_t n = 0; n < 128; ++n) {
    for (uint8_t ix = 0; ix < 3; ++ix) {
      // Load the data into all of the shift registers.
      PORT1_DATA = pal->_c[img->_c[n]].grb[ix];
      PORT2_DATA = pal->_c[img->_c[n]].grb[ix];
      LOAD_STRIP(0);
      UNSELECT_STRIPS();
      
      clockOutputs();
    }
  }

  for (int i = 0; i < LATCH_BYTES; ++i) {
    PORT1_DATA = 0;
    PORT2_DATA = 0;
    LOAD_STRIP(0);
    UNSELECT_STRIPS();

    clockOutputs();
  }
}

void LPD8806x8::show(const Image* img, const Palette* pal) {

  CLOCK_DISABLE();
  for (uint8_t n = 0; n < STRIP_LENGTH; ++n) {
    for (uint8_t ix = 0; ix < 3; ++ix) {

      // Load the data into all of the shift registers.
      PORT1_DATA = pal->_c[img->_c[0][n]].grb[ix];
      PORT2_DATA = pal->_c[img->_c[4][n]].grb[ix];
      LOAD_STRIP(0);
      UNSELECT_STRIPS();

      PORT1_DATA = pal->_c[img->_c[1][n]].grb[ix];
      PORT2_DATA = pal->_c[img->_c[5][n]].grb[ix];
      LOAD_STRIP(1);
      UNSELECT_STRIPS();

      PORT1_DATA = pal->_c[img->_c[2][n]].grb[ix];
      PORT2_DATA = pal->_c[img->_c[6][n]].grb[ix];
      LOAD_STRIP(2);
      UNSELECT_STRIPS();

      PORT1_DATA = pal->_c[img->_c[3][n]].grb[ix];
      PORT2_DATA = pal->_c[img->_c[7][n]].grb[ix];
      LOAD_STRIP(3);
      UNSELECT_STRIPS();

      clockOutputs();
    }
  }
    
  for (int i = 0; i < LATCH_BYTES; ++i) {
    for (int j = 0; j < 4; ++j) {
      PORT1_DATA = 0;
      PORT2_DATA = 0;
      LOAD_STRIP(j);
      UNSELECT_STRIPS();
    }

    clockOutputs();
  }
}

inline void LPD8806x8::clockOutputs()
{
  // Here, we need to keep the load inhibit pin high, but the select bits
  // don't matter.
  CLOCK_ENABLE();
  CLOCK_HIGH();
  
  for (uint8_t i = 0; i < 7; ++i) {
    CLOCK_LOW();
    CLOCK_HIGH();
  }

  CLOCK_LOW();
  CLOCK_DISABLE();
}
