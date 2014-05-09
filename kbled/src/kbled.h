#pragma once

#include <stdint.h>

/**
 * class KBLed
 * Input-ouptut agnostic class for managing colored keyboard state. Assumes that
 * each key of a standard 88-key keyboard is modified by MIDI data and
 * represented in output by an RGB LED.
 *
 * KBLed does not internally manage the input data (MIDI messages) or output
 * data (light strips or driver ICs).
 */

#define PEDAL_DEBOUNCE_DOWN_LIMIT 40
#define PEDAL_DEBOUNCE_UP_LIMIT   30

#define LOW_A                     21

#define DAMPER_PEDAL              64
#define SOFT_PEDAL                67

typedef uint8_t byte;

namespace KB 
{
  enum KeyState
  {
    KS_DEFAULT
  };
  
  struct KeyData
  {
    uint8_t data[7];
    KeyState state;
  };

  enum PedalStatus
  {
    PS_DOWN,
    PS_UP,
    PS_NONE
  };
  

  class KBLed
  {
  public:
    KBLed();

    void note_on(byte channel, byte pitch, byte velocity);
    void note_off(byte channel, byte pitch, byte velocity);

    void control_change(byte channel, byte number, byte value);

    void set_key_state(byte key, KeyState state);
    

    void set_key_brightness(byte key_index, byte value);
    void set_key_color(byte key_index, byte r, byte g, byte b);

    uint8_t* key_color(byte key_index)
    {
      return &key_colors[key_index][0];
    }
    /**
     * Update all of the keys based on the time elapsed.
     **/
    void update(uint16_t micros);
    
  private:
    byte pitch_to_key(byte pitch) {
      return pitch - LOW_A;
    }
    
    KeyData keys[88];
    bool is_damper_pressed;

    byte global_decay;
    
    uint8_t key_colors[88][3];
  };

};

