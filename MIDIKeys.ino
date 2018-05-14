/*
   This examples shows how to make a simple seven keys MIDI keyboard with volume control

   Created: 4/10/2015
   Author: Arturo Guadalupi <a.guadalupi@arduino.cc>
   
   http://www.arduino.cc/en/Tutorial/MidiDevice
*/

#include "MIDIUSB.h"
#include "PitchToNote.h"

#define VERSION "1.0"

#define LED_BOTTOM_CNTRL  2
#define LED_TOP_CNTRL  3

#define NUM_KEYS  10

template<int KEY> void ISR_key_change();

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
volatile midikey_t keys[NUM_KEYS] = {
    { 43,    pitchC4,       ISR_key_change<0>,    false,  false,   0 },
    { 45,    pitchD4b,     ISR_key_change<1>,    false,  false,   0 },
    { 47,    pitchD4,       ISR_key_change<2>,    false,  false,   0 },
    { 49,    pitchE4b,      ISR_key_change<3>,    false,  false,   0 },
    { 51,    pitchE4,       ISR_key_change<4>,    false,  false,   0 },
    { 53,    pitchF4,       ISR_key_change<5>,    false,  false,   0 },
    { 50,    pitchG4b,     ISR_key_change<6>,    false,  false,   0 },
    { 48,    pitchG4,       ISR_key_change<7>,    false,  false,   0 },
    { 46,    pitchA4b,     ISR_key_change<8>,    false,  false,   0 },                    
    { 44,    pitchB4b,      ISR_key_change<9>,    false,  false,   0 }
};

void setup() {
    pinMode(LED_BOTTOM_CNTRL, OUTPUT);  
    pinMode(LED_TOP_CNTRL, OUTPUT); 
    // reset led brightness
    analogWrite(LED_BOTTOM_CNTRL, 255);
    analogWrite(LED_TOP_CNTRL, 255);
    
    for (int i = 0; i < NUM_KEYS; i++)   
    {
        pinMode(keys[i].pin, INPUT_PULLUP);
        attachInterrupt(digitalPinToInterrupt(keys[i].pin), keys[i].ISR, CHANGE);
    }
    
}


void loop() {
    // fetch led brightness
    // TODO: get led brightness setting from somewhere
    byte bottom_brightness = 255; // default
    byte top_brightness = 255; // default

    // set led brightness
    analogWrite(LED_BOTTOM_CNTRL, bottom_brightness);
    analogWrite(LED_TOP_CNTRL, top_brightness);
    // process keys
    for (int i = 0; i < NUM_KEYS; i++)
    {
        if(keys[i].pending)
        {
            if(keys[i].on_state) // note on
            {
                noteOn(0, keys[i].note, 64); // 64 =  normal intensity
                MidiUSB.flush(); // flush is required to transmit note message in serial bus immediately
            }
            else // note off
            {
                noteOff(0, keys[i].note, 0);
                MidiUSB.flush(); // flush is required to transmit note message in serial bus immediately
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

template<int KEY> void ISR_key_change()
{
    bool current_state = (digitalRead(keys[KEY].pin) == LOW); // get current state: LOW = on
    // check state has changed and is greater than debounce time
    if( (millis() - keys[KEY].last_change_time > 10) && (keys[KEY].on_state != current_state) )
    {
        keys[KEY].on_state = current_state; // update state
        keys[KEY].pending = true; // set flag so key is ready to be actioned
        keys[KEY].last_change_time = millis(); // update change time
    }
}


