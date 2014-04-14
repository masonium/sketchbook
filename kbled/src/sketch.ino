#include <Arduino.h>
SoftwareSerial SerialSoft(1, 2);
#include <MIDI.h>
#include <Adafruit_NeoPixel.h>
#include <TimerOne.h>
#include <SoftwareSerial.h>
#include "sketch_types.h"

const int CLK_PIN = 12;
const int DATA_PIN = 11;

const int DAMPER_PEDAL = 64;
const int SOFT_PEDAL = 67;
bool is_soft_pressed = false;
bool is_damper_pressed = false;

int color_mode = CM_RAINBOW;
int decay = 16;

Adafruit_NeoPixel strip(88, DATA_PIN);

const char* NOTE_NAMES[]={"C ", "Cs", "D ", "Ds", "E ", "F ", "Fs", "G ", "Gs", "A ", "Bf", "B "};

int color_index = 0;

const uint32_t colors[] = {0x00ff00, 0x33ff00, 0xffff00, 0xff0000, 0xff00ff, 0x0000ff, 0x00ffff};

const char NUM_COLORS = sizeof(colors) / sizeof(colors[0]);

note_t NOTE_MAP[128];

// Gamma correction compensates for our eyes' nonlinear perception of
// intensity.  It's the LAST step before a pixel value is stored, and
// allows intermediate rendering/processing to occur in linear space.
// The table contains 256 elements (8 bit input), though the outputs are
// only 7 bits (0 to 127).  This is normal and intentional by design: it
// allows all the rendering code to operate in the more familiar unsigned
// 8-bit colorspace (used in a lot of existing graphics code), and better
// preserves accuracy where repeated color blending operations occur.
// Only the final end product is converted to 7 bits, the native format
// for the LPD8806 LED driver.  Gamma correction and 7-bit decimation
// thus occur in a single operation.
PROGMEM prog_uchar gammaTable[]  = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,
    2,  2,  2,  2,  2,  3,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,
    4,  4,  4,  4,  5,  5,  5,  5,  5,  6,  6,  6,  6,  6,  7,  7,
    7,  7,  7,  8,  8,  8,  8,  9,  9,  9,  9, 10, 10, 10, 10, 11,
   11, 11, 12, 12, 12, 13, 13, 13, 13, 14, 14, 14, 15, 15, 16, 16,
   16, 17, 17, 17, 18, 18, 18, 19, 19, 20, 20, 21, 21, 21, 22, 22,
   23, 23, 24, 24, 24, 25, 25, 26, 26, 27, 27, 28, 28, 29, 29, 30,
   30, 31, 32, 32, 33, 33, 34, 34, 35, 35, 36, 37, 37, 38, 38, 39,
   40, 40, 41, 41, 42, 43, 43, 44, 45, 45, 46, 47, 47, 48, 49, 50,
   50, 51, 52, 52, 53, 54, 55, 55, 56, 57, 58, 58, 59, 60, 61, 62,
   62, 63, 64, 65, 66, 67, 67, 68, 69, 70, 71, 72, 73, 74, 74, 75,
   76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91,
   92, 93, 94, 95, 96, 97, 98, 99,100,101,102,104,105,106,107,108,
  109,110,111,113,114,115,116,117,118,120,121,122,123,125,126,127
};

// This function (which actually gets 'inlined' anywhere it's called)
// exists so that gammaTable can reside out of the way down here in the
// utility code...didn't want that huge table distracting or intimidating
// folks before even getting into the real substance of the program, and
// the compiler permits forward references to functions but not data.
inline byte gamma(byte x) {
  return pgm_read_byte(&gammaTable[x]);
}

void init_note_map()
{
  for (int i = 0; i < 128; ++i)
    NOTE_MAP[i].index = -1;

  const unsigned int LOW_A = 21;
  for (unsigned int note = LOW_A; note < LOW_A + 88; ++note)
  {
    NOTE_MAP[note].index = note - LOW_A;
  }
}

int note_octave(byte pitch)
{
  return (pitch / 12) - 1;
}

const char* note_name(byte pitch)
{
  return NOTE_NAMES[pitch % 12];
}

PedalStatus debounce(bool* flag, byte value)
{
    if (!*flag && value > 40)
    {
      *flag = true;
      return PS_DOWN;
    }
    if (*flag && value < 30);
    {
      *flag = false;
      return PS_UP;
    }
    return PS_NONE;
}


void control_change(byte channel, byte number, byte value)
{
  if (number == DAMPER_PEDAL)
  {
    PedalStatus ps = debounce(&is_damper_pressed, value);
    if (ps == PS_DOWN)
    {
      decay = 2;
    }
    if (ps == PS_UP)
    {
      decay = 10;
    }
        
  }
  else if (number == SOFT_PEDAL)
  {
    if (debounce(&is_soft_pressed, value) == PS_DOWN)
    {
      color_mode += 1;
      color_mode %= CM_NUM_MODES;
    }      
  }
}

void note_off(byte channel, byte pitch, byte velocity)
{
    // Serial.print("                              Note ");
    // Serial.print(note_name(pitch));
    // Serial.print(note_octave(pitch));
    // Serial.print(" off\r\n");
    note_t si = NOTE_MAP[pitch];
    if (si.index >= 0)
    {
      strip.setPixelColor(si.index, 0, 0, 0);
      strip.show();
    }
}


void note_on(byte channel, byte pitch, byte velocity)
{
  if (velocity == 0)
  {
    note_off(channel, pitch, velocity);
  }
  else
  {
    note_t si = NOTE_MAP[pitch];
    if (si.index >= 0)
    {
      unsigned char brightness = 255;
      uint32_t color = brightness * 0x010101;
      if (color_mode == CM_RAINBOW)
      {
        color = colors[color_index];
      }
      strip.setPixelColor(si.index, color);
      strip.show();
      color_index += 1;
      color_index %= NUM_COLORS;
    }
  }
}

int currentLED = 0;

void timer_interrupt()
{
  //for (int i = 0; i < 88; ++i)
  strip.show();
}

void setup()
{
  SerialSoft.begin(9600);
  init_note_map();

  Timer1.initialize(1000000 / 60);
  Timer1.attachInterrupt(timer_interrupt);
  
  MIDI.begin(MIDI_CHANNEL_OMNI);

  MIDI.setHandleNoteOn(note_on);
  MIDI.setHandleNoteOff(note_off);
  MIDI.setHandleControlChange(control_change);

  strip.begin();

  // pinMode(7, OUTPUT);
  // pinMode(6, OUTPUT);
}

void loop()
{
    
  MIDI.read();
  //strip.show();
}
