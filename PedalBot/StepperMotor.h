#ifndef StepperMotor_H
#define StepperMotor_H

#include <Arduino.h>



class StepperMotor{
  private:
    const byte dirPin;
    const byte stepPin;
    const byte sleepPin;
    const byte encodorPinA;
    const byte encodorPinB;
    
    const int motorStepsPerRotation;
    const int encoderStepsPerRotation;
    
    const byte number;
    byte currentPosition;
    byte presets[];
    
    void loadPresets();
    void writePresetToMemory(const byte preset, const byte newPosition); 
    
  public:  
    StepperMotor(const byte motorNo, const int encodorSPR, const int motorSPR);
    ~StepperMotor();
    void drive(int angle, const byte dir);
    void moveToPreset(const byte preset);
    void savePreset(const byte currentPreset);
    void clearPresets();
    void clearAll();    
    
};

#endif
