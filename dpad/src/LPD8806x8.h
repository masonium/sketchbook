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
 * 0-3: select-bits
 * 4: Clock Bit
 * 5: Clock Inhibit Bit
 */
#define SELECT_STRIP0               0xfe
#define SELECT_STRIP1               0xfd
#define SELECT_STRIP2               0xfb
#define SELECT_STRIP3               0xf7
#define SELECT_MASK                 0x0f
#define CLOCK_HIGH_BIT              0x10
#define CLOCK_INHIBIT_HIGH_BIT      0x20

#define CLOCK_HIGH() CONTROL_PORT |= CLOCK_HIGH_BIT
#define CLOCK_LOW() CONTROL_PORT &= ~CLOCK_HIGH_BIT
#define CLOCK_ENABLE() CONTROL_PORT &= ~CLOCK_INHIBIT_HIGH_BIT
#define CLOCK_DISABLE() CONTROL_PORT |= CLOCK_INHIBIT_HIGH_BIT

#define LOAD_STRIP(x) CONTROL_PORT &= 0xff - (1 << x)
#define UNSELECT_STRIPS() CONTROL_PORT |= SELECT_MASK


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
