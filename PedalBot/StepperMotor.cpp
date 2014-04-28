#include "StepperMotor.h"
#include <EEPROM.h>


// --------- MACROS --------- //

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


// -------------------- Class variables -------------------- //


// ----- PINS
byte dirPin;
byte stepPin;
byte sleepPin;
byte encodorPinA;
byte encodorPinB; 

// ----- STEPPER MOTOR FACTS
int motorStepsPerRotation;
int encoderStepsPerRotation;

byte number;
// from dirPin to number would be consts but it causes huge amounts of errors, because the arduino can not
// specify when the constructor is called, so the compiler doesn't like using initialization lists for constants.

byte currentPosition;
byte presets[MAX_PRESETS];





// -------------------- CLASS METHODS -------------------- //


// --------- PUBLIC METHODS --------- //


// ----- CONSTRUCTOR
StepperMotor::StepperMotor(const byte motorNo, const int encodorSPR, const int motorSPR) : number( motorNo ),
                                                                                           motorStepsPerRotation( motorSPR ),
                                                                                           encoderStepsPerRotation( encodorSPR ),
                                                                                           dirPin( FIRST_DIR - (2 * motorNo) ),
                                                                                           stepPin( motorNo + FIRST_STEP ),
                                                                                           sleepPin( FIRST_SLP - (2 * motorNo) ),
                                                                                           encodorPinA( FIRST_ENA + (2 * motorNo) ),
                                                                                           encodorPinB( FIRST_ENB + (2 * motorNo) )
{
 
  // set up the pins  
  pinMode(dirPin, OUTPUT);
  pinMode(stepPin, OUTPUT);
  pinMode(sleepPin, OUTPUT);
  pinMode(encodorPinA, INPUT);
  pinMode(encodorPinB, INPUT);
  pinMode(13, OUTPUT);
  
  // initialise output pins
  digitalWrite(dirPin, CW);
  digitalWrite(stepPin, LOW);
  digitalWrite(sleepPin, LOW);
  
  currentPosition = NOON;
  
  loadPresets();
  
}


// ----- DESTRUCTOR
StepperMotor::~StepperMotor(){
  
}


void StepperMotor::drive(int angle, const byte dir){ //update to include rotary encodor
  
  digitalWrite(sleepPin, HIGH);
  digitalWrite(dirPin, dir); //set the direction to rotate
  
  float rotate = ((float) (angle)) * (motorStepsPerRotation / 360.0); // to avoid rounding error
  
  for(int i = 0; i < rotate*4; i++){ //* 2 here to turn far enough <<<WARNING
      digitalWrite(stepPin, HIGH);
      delayMicroseconds(STEPDELAY);
      digitalWrite(stepPin, LOW);
      delayMicroseconds(STEPDELAY);
  }
  
  digitalWrite(sleepPin, LOW);
  
}


void StepperMotor::moveToPreset(const byte preset){
  
  int pos = presets[preset];
  
  if(pos != UNSET){ // if the preset has been set

    int angle = pos - currentPosition;
    if(angle < 0){
      drive(-1*angle, CW);
    }else if(angle > 0){
      drive(angle, ACW); 
    }else if(angle == 0){

    }
    
    currentPosition = pos;
  }
  
}


void StepperMotor::savePreset(const byte currentPreset){
  
  if(currentPreset != -1){
    presets[currentPreset] = currentPosition;
    writePresetToMemory(currentPreset, currentPosition);
  }
  
}


// ----- CLEARING FUNCTIONS
void StepperMotor::clearPresets(){
  
  for(int preset = 0; preset < MAX_PRESETS; preset++){
    writePresetToMemory(preset, UNSET);
    presets[preset] = UNSET;
  }
  
}


void StepperMotor::clearAll(){ // remove all presets and reset currentPosition of this motor

  currentPosition = NOON;
  
  for(int preset = 0; preset < MAX_PRESETS; preset++){
    writePresetToMemory(preset, UNSET);
    presets[preset] = UNSET;
  } 
  
}





// --------- PRIVATE AND INLINE METHODS --------- //


// ----- INLINES
inline int presetMemoryLocation(const byte preset){
  
  return number * (MAX_PRESETS + 1) + preset + 1;
  
}


inline int currentLocationMemoryLocation(){
  
  return number * (MAX_PRESETS + 1); 
  
} 


// ----- INITIALISERS
void StepperMotor::loadPresets(){
  
    for(int preset = 0; preset < MAX_PRESETS; preset++){
      presets[preset] = EEPROM.read(presetMemoryLocation(preset));
    }
    
    currentPosition = EEPROM.read(currentLocationMemoryLocation());
    
    for(int preset = 0; preset < MAX_PRESETS; preset++) presets[preset] = random(0,254); //for display, remove and reset chip.
    currentPosition = 255; // also remove  
}


// ----- FUNCTIONAL
void StepperMotor::writePresetToMemory(const byte preset, const byte newPosition){ // preset ass -1 for current location

  if(preset >= 0){
    
    if(EEPROM.read(presetMemoryLocation(preset)) != newPosition){ //EEPROM has limited writes, so this may extend life
      EEPROM.write(presetMemoryLocation(preset), newPosition);
    }
    
  }else if(preset == -1){ // writing new current Location
  
    if(EEPROM.read(currentLocationMemoryLocation()) != newPosition){
      EEPROM.write(currentLocationMemoryLocation(), newPosition);
    }
    
  }
  
}
