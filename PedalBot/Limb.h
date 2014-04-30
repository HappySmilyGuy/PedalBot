#ifndef Limb_H
#define Limb_H

#include <Arduino.h>



class Limb{
  private:
  
    // ------ CONSTANTS
    
    // ----- PINS
    byte dirPin;
    byte stepPin;
    byte sleepPin;
    byte encoderPinA;
    byte encoderPinB;
    byte led;
    byte button;
    
    // ----- RATIOS
    int motorStepsPerRotation;
    
    byte number; // the number of limb
    
    // ----- VARIABLES
    byte currentPosition;
    byte presets[];
    
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
