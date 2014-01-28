#ifndef LPD8806x8_H__
#define LPD8806x8_H__

#include <Arduino.h>
#include "image.h"

const uint8_t STRIP_LENGTH = 128;
const uint8_t LATCH_BYTES = (STRIP_LENGTH + 31) / 32;

/// PC0 - 37
/// PC7 - 30

// PA0 - 22
// PA7 - 29

// PL0 - 49
// PL7 - 42

#define LPD_PORT1         C
#define LPD_PORT2         A
#define LPD_CONTROL_PORT  L

/**
 * We only use 7 bits of the control port, but reserve the whole thing just in
 * case.
 */
/**
 * 0-1: PORT 1 select bits
 * 2-3: PORT 2 select bits (possibly unused)
 * 4: LOAD inhibit bit
 * 5: CLOCK Bit
 * 6: CLOCK Inihibit Bit
 */
#define LOAD_INHIBIT_HIGH       0x10
#define CLOCK_HIGH              0x20
#define CLOCK_LOW               0x00
#define CLOCK_INHIBIT_HIGH      0x40
#define CLOCK_LOAD_INHIBIT_HIGH 0x50
#define SELECT_INIHIBIT_HIGH    0x80


class LPD8806x8 {
 public:
  LPD8806x8(void); // Empty constructor; init pins & strip length later
  void show(const Image* i, const Palette* p);
  void show(const StripImage* i, const Palette* p);

 private:
  void updatePins(void);

  inline void clockOutputs();
};


#endif
