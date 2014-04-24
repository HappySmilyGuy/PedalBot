#ifndef StepperMotor_H
#define StepperMotor_H

#include <WProgram.h>

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
    byte presets[MAX_PRESETS];
    
    void loadPresets();
    void writePresetToMemory(byte preset, byte newPosition);  
    
  public:
    StepperMotor(byte motorNo, byte encodorSPR, byte motorSPR): number(motorNo), 
                                                                motorStepsPerRotation(motorSPR),
                                                                encodorStepsPerRotation(encodorSPR),
                                                                dirPin(FIRST_DIR - 2 * number),
                                                                stepPin(number + FIRST_STEP),
                                                                sleepPin(FIRST_SLP - 2 * number),
                                                                encodorPinA(FIRST_ENA + 2 * number),
                                                                encodorPinB(FIRST_ENB + 2 * number);
    ~StepperMotor();
    void drive(int angle, byte dir);
    void moveToPreset(byte preset);
    void savePreset(byte currentPreset);
    void clearPreset();
    void clearAll();    
    
};

#endif
