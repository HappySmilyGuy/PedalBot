#ifndef Limb_H
#define Limb_H

#include <Arduino.h>



class Limb{
  private:
  
    // ------ CONSTANTS
    
    // ----- PINS
    const byte dirPin;
    const byte stepPin;
    const byte sleepPin;
    const byte encoderPinA;
    const byte encoderPinB;
    const byte led;
    const byte button;
    
    // ----- RATIOS
    const int motorStepsPerRotation;
    const int encoderStepsPerRotation;
    
    const byte number; // the number of limb
    
    // ----- VARIABLES
    byte currentPosition;
    byte presets[];
    
    // ----- EEPROM
    void loadPresets();
    void writePresetToMemory(const byte preset, const byte newPosition);

    // ----- ROTARY ENCODOR
    int calculateEncodorPinB(const int limbNo);
    int calculateEncodorPinA(const int limbNo);
    void encoderMovement();

    // ----- MISC
    void flashLED(const int time);
    void flashLED(const int flashes, const int onTime, const int offTime);
    
  public:  
    Limb(const byte limbNo, const int encoderSPR, const int motorSPR);
    ~Limb();
    void drive(const int newPosition);
    void moveToPreset(const byte preset);
    void checkButton(const byte currentPreset);
    void savePreset(const byte currentPreset);
    void clearPresets();
    void clearAll();    
    

    
};

#endif
