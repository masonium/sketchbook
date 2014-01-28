#include <Arduino.h>
#include <MIDI.h>
#include <LPD8806.h>

const int LED_PIN = 11;

const int stripLength = 52;

const int CLK_PIN = 6;
const int DATA_PIN =7;
LPD8806 strip(stripLength, DATA_PIN, CLK_PIN);

const char* NOTE_NAMES[]={"C", "Cs", "D", "Ds", "E", "F", "Fs", "G", "Gs", "A", "Bf", "B"};

int note_octave(byte pitch)
{
  return (pitch / 12) - 1;
}

const char* note_name(byte pitch)
{
  return NOTE_NAMES[pitch % 12];
}

const int DAMPER_PEDAL = 64;

void control_change(byte channel, byte number, byte value)
{
  if (number == DAMPER_PEDAL)
  {
    Serial.print("              Pedal at ");
    Serial.print(value);
    Serial.print("\r\n");
  }
}

void note_off(byte channel, byte pitch, byte velocity)
{
    Serial.print("                              Note ");
    Serial.print(note_name(pitch));
    Serial.print(note_octave(pitch));
    Serial.print(" off\r\n");
}

void note_on(byte channel, byte pitch, byte velocity)
{
  if (velocity == 0)
  {
    note_off(channel, pitch, velocity);
  }
  else
  {
    Serial.print("Note ");
    Serial.print(note_name(pitch));
    Serial.print(note_octave(pitch));
    Serial.print(" on at velocity ");
    Serial.print(velocity);
    Serial.print("\r\n");
  }
}

int currentLED = 0;

void setup()
{
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  // MIDI.begin(MIDI_CHANNEL_OMNI);

  // MIDI.setHandleNoteOn(note_on);
  // MIDI.setHandleNoteOff(note_off);
  // MIDI.setHandleControlChange(control_change);

  strip.begin();

  pinMode(7, OUTPUT);
  pinMode(6, OUTPUT);
  
  Serial.begin(9600);
  Serial.print("hello");
}

void loop()
{
  
  strip.setPixelColor(currentLED, 0, 0, 0);
  currentLED = (currentLED + 1) % stripLength;
  strip.setPixelColor(currentLED, 255, 255, 255);
  
  strip.show();
  delay(100);
  // digitalWrite(7, HIGH);
  // digitalWrite(6, HIGH);
  // delay(500);
  // digitalWrite(6, LOW);
  // delay(500);
  // digitalWrite(6, HIGH);
  // digitalWrite(7, LOW);
  // delay(500);
  // digitalWrite(6, LOW);
  // delay(500);
    
  //MIDI.read();
}
