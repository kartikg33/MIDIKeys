/*
   This examples shows how to make a simple seven keys MIDI keyboard with volume control

   Created: 4/10/2015
   Author: Arturo Guadalupi <a.guadalupi@arduino.cc>
   
   http://www.arduino.cc/en/Tutorial/MidiDevice
*/

#include "MIDIUSB.h"
#include "PitchToNote.h"
#define NUM_BUTTONS  2

typedef struct
{
    uint8_t pin;
    byte note;
} midikey_t;

/* NOTE: to musicians, middle C is C4 (note 60), not C3 (note 48) */
const midikey_t keys[NUM_BUTTONS] = {
  { 2,  pitchC4 },
  { 3,  pitchD4b }
};

const int intensityPot = 0;  //A0 input

uint8_t pressedButtons = 0x00;
uint8_t previousButtons = 0x00;
uint8_t intensity;

void setup() {
  for (int i = 0; i < NUM_BUTTONS; i++)
    pinMode(keys[i].pin, INPUT_PULLUP);
}


void loop() {
  readButtons();
  readIntensity();
  playNotes();
}

// First parameter is the event type (0x0B = control change).
// Second parameter is the event type, combined with the channel.
// Third parameter is the control number number (0-119).
// Fourth parameter is the control value (0-127).

void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {(byte)0x0B, (byte)(0xB0 | channel), control, value};
  MidiUSB.sendMIDI(event);
}

void readButtons()
{
  for (int i = 0; i < NUM_BUTTONS; i++)
  {
    if (digitalRead(keys[i].pin) == LOW)
    {
      bitWrite(pressedButtons, i, 1);
      delay(50);
    }
    else
      bitWrite(pressedButtons, i, 0);
  }
}

void readIntensity()
{
  int val = analogRead(intensityPot);
  intensity = (uint8_t) (map(val, 0, 1023, 0, 127));
}

void playNotes()
{
  for (int i = 0; i < NUM_BUTTONS; i++)
  {
    if (bitRead(pressedButtons, i) != bitRead(previousButtons, i))
    {
      if (bitRead(pressedButtons, i))
      {
        bitWrite(previousButtons, i , 1);
        noteOn(0, keys[i].note, intensity);
        MidiUSB.flush();
      }
      else
      {
        bitWrite(previousButtons, i , 0);
        noteOff(0, keys[i].note, 0);
        MidiUSB.flush();
      }
    }
  }
}

// First parameter is the event type (0x09 = note on, 0x08 = note off).
// Second parameter is note-on/note-off, combined with the channel.
// Channel can be anything between 0-15. Typically reported to the user as 1-16.
// Third parameter is the note number (48 = middle C).
// Fourth parameter is the velocity (64 = normal, 127 = fastest).

void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {(byte)0x09, (byte)(0x90 | channel), pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {(byte)0x08, (byte)(0x80 | channel), pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
}
