#include "kbled.h"
#include <math.h>
#include <string.h>

namespace KB
{
  ////////////////////////////////////////////////////////////////////////////////
  //// KeyState state machine specializations
  
  typedef void (*note_off_func)(KeyData& kd, byte velocity);
  typedef void (*note_on_func)(KeyData& kd, byte velocity);
  typedef void (*update_func)(KeyData& kd, const KBLed*, uint16_t);
  
  struct KeyStateUpdater
  {
    note_off_func note_off;
    note_on_func note_on;
    update_func update;
  };


  /*
   * data[0:1] = brightness
   * data[2] = decay;
   */

  struct DefaultKeyData
  {
    int16_t brightness;
    uint8_t decay;
  };

  class DefaultKeyState
  {
  public:
    static void note_off(KeyData& kd, byte velocity)
    {
      DefaultKeyData* dkd = reinterpret_cast<DefaultKeyData*>(kd.data);
      dkd->decay = 80;
    }
    
    static void note_on(KeyData& kd, byte velocity)
    {
      DefaultKeyData* dkd = reinterpret_cast<DefaultKeyData*>(kd.data);
      dkd->decay = 30;
      dkd->brightness = 2 << 15 -1;
    }
    
    static void update(KeyData& kd, const KBLed* kb, uint16_t millis)
    {
      DefaultKeyData* dkd = reinterpret_cast<DefaultKeyData*>(kd.data);
      dkd->brightness = dkd->brightness - millis * dkd->decay * 256 / 1000;
      dkd->brightness = dkd->brightness > 0 ? dkd->brightness : 0;
    }
  };

  /*
   * data[0:1] = brightness
   * data[2:4] = color
   * data[5] = decay
   */
 
  class RepeaterKeyState
  {
  public:
    static void note_off(KeyData& kd, byte velocity)
    {
    }
    
    static void note_on(KeyData& kd, byte velocity)
    {
    }
    
    static void update(KeyData& kd, const KBLed* kb, uint16_t micros)
    {

    }
  };
  
  
  KeyStateUpdater states[1]  = {{ DefaultKeyState::note_off,
                                  DefaultKeyState::note_on,
                                  DefaultKeyState::update }};
  
  //////////////////////////////////////////////////////////////////////////////
  // Utility functiosn

  enum DebounceState
  {
    DS_CLOSED, DS_OPEN, DS_NONE
  };
  
  DebounceState debounce(bool* flag, byte value)
  {
    if (!*flag && value > 40)
    {
      *flag = true;
      return DS_CLOSED;
    }
    if (*flag && value < 30);
    {
      *flag = false;
      return DS_OPEN;
    }
    return DS_NONE;
  }

  
  KBLed::KBLed() : is_damper_pressed(false)
  {
    memset(keys, 0, sizeof(keys));
    memset(key_colors, 0, sizeof(key_colors));
  }

  
  void KBLed::note_on(byte channel, byte pitch, byte velocity)
  {
    KeyData& kd = keys[pitch_to_key(pitch)];
    states[kd.state].note_on(kd, velocity); 
  }
  void KBLed::note_off(byte channel, byte pitch, byte velocity)
  {
    KeyData& kd = keys[pitch_to_key(pitch)];
    states[kd.state].note_off(kd, velocity);
  }

  void KBLed::update(uint16_t millis)
  {
    for (uint8_t kidx = 0; kidx < 88; ++kidx) {
      KeyData& kd = keys[kidx];
      states[kd.state].update(kd, this, millis);
    }
  }
  
  void KBLed::control_change(byte channel, byte number, byte value)
  {
    debounce(&is_damper_pressed, value);
  }
}
