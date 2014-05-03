#include <Arduino.h>
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

// This gamma correction table was taken from:
// http://rgb-123.com/ws2812-color-output/

// To save bytes, the first 35 are not included and are programatically
// generated in the 'gamma' function
PROGMEM prog_uchar GammaE[] = {3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5,
6, 6, 6, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11,
11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 18,
19, 19, 20, 21, 21, 22, 22, 23, 23, 24, 25, 25, 26, 27, 27, 28,
29, 29, 30, 31, 31, 32, 33, 34, 34, 35, 36, 37, 37, 38, 39, 40,
40, 41, 42, 43, 44, 45, 46, 46, 47, 48, 49, 50, 51, 52, 53, 54,
55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70,
71, 72, 73, 74, 76, 77, 78, 79, 80, 81, 83, 84, 85, 86, 88, 89,
90, 91, 93, 94, 95, 96, 98, 99,100,102,103,104,106,107,109,110,
111,113,114,116,117,119,120,121,123,124,126,128,129,131,132,134,
135,137,138,140,142,143,145,146,148,150,151,153,155,157,158,160,
162,163,165,167,169,170,172,174,176,178,179,181,183,185,187,189,
191,193,194,196,198,200,202,204,206,208,210,212,214,216,218,220,
222,224,227,229,231,233,235,237,239,241,244,246,248,250,252,255};

inline byte gamma(byte x) {
  if (x < 22)
    return 0;
  else if (x < 29)
    return 1;
  else if (x < 35)
    return 2;

  return pgm_read_byte(&GammaE[x-35]);
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

int count = 0;

void timer_interrupt()
{
  strip.show();
}

void setup_timer()
{
  // 16 Mhz processor
  // clock prescaler of 8 (CS = 010b)
  // 33333 for 60 Hz 
  // CTC (cleared on clock match), (WGM=0100b)
  Timer1.initialize( 1000000L / 60 );
  Timer1.attachInterrupt(timer_interrupt);
}

void setup()
{
  Serial.begin(9600);
  init_note_map();

  MIDI.begin(MIDI_CHANNEL_OMNI);

  MIDI.setHandleNoteOn(note_on);
  MIDI.setHandleNoteOff(note_off);
  MIDI.setHandleControlChange(control_change);

  strip.begin();
  setup_timer();
}

void loop()
{
  cli();
  MIDI.read();
  sei();
}
