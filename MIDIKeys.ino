/*
   This examples shows how to make a simple seven keys MIDI keyboard with volume control

   Created: 4/10/2015
   Author: Arturo Guadalupi <a.guadalupi@arduino.cc>
   
   http://www.arduino.cc/en/Tutorial/MidiDevice
*/

#include "MIDIUSB.h"
#include "PitchToNote.h"
#define NUM_BUTTONS  2

template<int PIN> void ISR_key_change();

typedef struct
{
    const uint8_t pin;                                         // Digital Input pin
    const byte note;                                           // note number
    void (*volatile ISR)(void);                             // pointer to ISR to capture a change in signal
    volatile bool on_state;                                 // false for off, true for on
    volatile bool pending;                                  // false for no state change, true for new state change
    volatile unsigned long last_change_time;   // stores last time a change was made
} midikey_t;

/* 
 *  NOTE:MIDIUSB defines note 60 as C4, whereas JUCE defines note 60 as C3. 
 *  We shall centre our notes around C4 with the aim of them being pushed down an octave by JUCE
 *  So our middle C will be C4 even though we really want it to be C3
*/
volatile midikey_t keys[NUM_BUTTONS] = {
    { 2,    pitchC4,     ISR_key_change<0>,    false,  false,   0 },
    { 3,    pitchD4b,   ISR_key_change<1>,    false,  false,   0 }
};

void setup() {
    for (int i = 0; i < NUM_BUTTONS; i++)   
    {
        pinMode(keys[i].pin, INPUT_PULLUP);
        attachInterrupt(digitalPinToInterrupt(keys[i].pin), keys[i].ISR, CHANGE);
    }
}


void loop() {
    for (int i = 0; i < NUM_BUTTONS; i++)
    {
        if(keys[i].pending)
        {
            if(keys[i].on_state) // note on
            {
                noteOn(0, keys[i].note, 64); // 64 =  normal intensity
                MidiUSB.flush();
            }
            else // note off
            {
                noteOff(0, keys[i].note, 0);
                MidiUSB.flush();
            }
            keys[i].pending = false; // reset pending flag
        }
    }
}

// First parameter is the event type (0x0B = control change).
// Second parameter is the event type, combined with the channel.
// Third parameter is the control number number (0-119).
// Fourth parameter is the control value (0-127).

void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {(byte)0x0B, (byte)(0xB0 | channel), control, value};
  MidiUSB.sendMIDI(event);
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

// First parameter is the event type (0x09 = note on, 0x08 = note off).
// Second parameter is note-on/note-off, combined with the channel.
// Channel can be anything between 0-15. Typically reported to the user as 1-16.
// Third parameter is the note number (48 = middle C).
// Fourth parameter is the velocity (64 = normal, 127 = fastest).

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {(byte)0x08, (byte)(0x80 | channel), pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
}

/** ISRs **/

template<int PIN> void ISR_key_change()
{
    bool current_state = (digitalRead(keys[PIN].pin) == LOW); // get current state: LOW = on
    // check state has changed and is greater than debounce time
    if( (millis() - keys[PIN].last_change_time > 10) && (keys[PIN].on_state != current_state) )
    {
        keys[PIN].on_state = current_state; // update state
        keys[PIN].pending = true; // set flag so key is ready to be actioned
        keys[PIN].last_change_time = millis(); // update change time
    }
}


