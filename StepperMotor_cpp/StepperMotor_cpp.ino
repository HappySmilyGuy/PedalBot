#include "StepperMotor.h"
#include <EEPROM.h>

#define STEPDELAY 200    // delay between each step in micro seconds
#define CW LOW           // clockwize, for DIR pin
#define ACW HIGH         // anticlockwise, for DIR pin
#define MAX_PRESETS 128  // the maximum number of presets by the MIDI standard
#define UNSET 255        // placeholding value for a unset preset
#define NOON 127         // neutral position (dial pointing straight up at noon)

#define FIRST_DIR 53     // the first Easy driver direction pin (DIR)
#define FIRST_STEP 2     // the first Easy driver step pin (STEP)
#define FIRST_SLP 52     // the first Easy Driver sleep pin (SLP)
#define FIRST_ENA 22     // the first rotary encodor encodorA pin (ENA)
#define FIRST_ENB 23     // the first rotary encodor encodorB pin (ENB)

// --------- MACROS --------- //
#define currentLocationMemoryLocation() (number * (MAX_PRESETS + 1))
#define presetMemoryLocation(preset) (number * (MAX_PRESETS + 1) + preset + 1)



// -------------------- Class variables -------------------- //


// ----- PINS
private const byte dirPin;
private const byte stepPin;
private const byte sleepPin;
private const byte encodorPinA;
private const byte encodorPinB;

// ----- STEPPER MOTOR FACTS
private const int motorStepsPerRotation;
private const int encoderStepsPerRotation;

private const byte number;
private byte currentPosition;
private byte presets[MAX_PRESETS];





// -------------------- CLASS METHODS -------------------- //


// --------- PUBLIC METHODS --------- //


// ----- CONSTRUCTOR
public StepperMotor::StepperMotor(byte motorNo, byte encodorSPR, byte motorSPR): number(motorNo),
                                                                                 motorStepsPerRotation(motorSPR),
                                                                                 encodorStepsPerRotation(encodorSPR),
                                                                                 dirPin(FIRST_DIR - 2 * number),
                                                                                 stepPin(number + FIRST_STEP),
                                                                                 sleepPin(FIRST_SLP - 2 * number),
                                                                                 encodorPinA(FIRST_ENA + 2 * number),
                                                                                 encodorPinB(FIRST_ENB + 2 * number)
{
 
  // set up the pins  
  pinMode(dirPin, OUTPUT);
  pinMode(stepPin, OUTPUT);
  pinMode(sleepPin, OUTPUT);
  pinMode(encodorPinA, INPUT);
  pinMode(encodorPinB, INPUT);
  
  // initialise output pins
  digitalWrite(dirPin, CW);
  digitalWrite(stepPin, LOW);
  digitalWrite(sleepPin, LOW);
  
  loadPresets();
  
}


// ----- DESTRUCTOR
public StepperMotor::~StepperMotor(){
  
}


public void StepperMotor::drive(int angle, byte dir){ //update to include rotary encodor

  digitalWrite(sleepPin, HIGH);
  float rotate = (float) (angle * motorStepsPerRotation) / 360.0; // to avoid rounding error
  for(int i = 0; i < rotate; i++){
      digitalWrite(stepPin, HIGH);
      delayMicroseconds(STEPDELAY);
      digitalWrite(stepPin, LOW);
      delayMicroseconds(STEPDELAY);
  }
  digitalWrite(sleepPin, LOW);

  //update currentPosition
  
}


public void StepperMotor::moveToPreset(byte preset){
  
  int pos = preset[preset];
  if(pos != UNSET){
    int angle = pos - currentPosition;
    if(angle < 0) drive(angle, CW);
    else if(angle > 0) drive(-1*angle, ACW);
    currentPosition = pos;
  }
  
}


public void StepperMotor::savePreset(byte currentPreset){
  if(currentPreset != -1){
    presets[currentPreset] = currentPosition;
    writePresetToMemory(currentPreset, currentPosition);
  }
}


// ----- CLEARING FUNCTIONS
public void StepperMotor::clearPresets(){
  
  for(int preset = 0; preset < MAX_PRESETS; preset++){
    presetWriteToMemory(preset, UNSET);
    presets[preset] = UNSET;
  }
  
}


public void StepperMotor::clearAll(){ // remove all presets and reset currentPosition of this motor

  currentPosition = NOON;  
  for(int preset = 0; preset < MAX_PRESETS; preset++){
    presetWriteToMemory(preset, UNSET);
    presets[preset] = UNSET;
  } 
  
}





// --------- PRIVATE AND INLINE METHODS --------- //


// ----- INITIALISERS
private void StepperMotor::loadPresets(){
  
    for(int preset = 0; preset < MAX_PRESETS + 1; preset++){
      presets[preset] = EEPROM.read(presetMemoryLocation(number, preset));
    }
    currentPositions = EEPROM.read(currentLocationMemoryLocation(number));
    
}



// ----- FUNCTIONAL
private void StepperMotor::writePresetToMemory(int preset, int newPosition){ // preset ass -1 for current location

  if(preset >= 0){
    if(EEPROM.read(presetMemoryLocation(motor, preset)) != newPosition){ //EEPROM has limited writes, so this may extend life
      EEPROM.write(presetMemoryLocation(motor, preset), newPosition);
    }
  }else if(preset == -1){ // writing new current Location
    if(EEPROM.read(currentLocationMemoryLocation() != newPosition){
      EEPROM.write(currentLocationMemoryLocation(), newPosition);
    }
  }
  
}



// ----- INLINES
inline int presetMemoryLocation(int preset){
  
  return number * (MAX_PRESETS + 1) + preset + 1;
  
}


inline int currentLocationMemoryLocation(){
  
  return number * (MAX_PRESETS + 1); 
  
}
