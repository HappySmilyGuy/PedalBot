#ifndef Limb_H
#define Limb_H

#include <Arduino.h>

#define MAX_PRESETS 128  // the maximum number of presets by the MIDI standard

class Limb{
  private:
  
    // ------ CONSTANTS
    
    // ----- PINS
    byte dirPin, stepPin, sleepPin, led, button, number;
    
    // ----- RATIOS
    int motorStepsPerRotation;
    
    // ----- VARIABLES
    byte presets[MAX_PRESETS];
    
    
    // --------- METHODS --------- //
    
    // ----- EEPROM
    void loadPresets();
    void writePresetToMemory(const byte preset, const byte newPosition);

    // ----- ROTARY ENCODOR
    int calculateEncodorPinB(const int limbNo);
    int calculateEncodorPinA(const int limbNo); 

    // ----- MISC
    void flashLED(const int time);
    void flashLED(const int flashes, const int onTime, const int offTime);
    
  public:
    // ----- VARIABLES
    byte encoderPinA; // these need to be public for the rotary encoder interupts
    byte encoderPinB; // because the function needs to be static for the interupt to work
    volatile int lastEncoded;
    byte currentPosition;
   
    
    // --------- METHODS --------- //
    Limb(const byte limbNo);
    ~Limb();
    //void initialise(const byte limbNo);
    void drive(const int newPosition);
    void moveToPreset(const byte preset);
    void checkButton(const byte currentPreset);
    void savePreset(const byte currentPreset);
    void clearPresets();
    void clearAll();    
    

    
};

#endif
