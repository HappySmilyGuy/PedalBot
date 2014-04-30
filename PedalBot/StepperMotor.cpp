#include "StepperMotor.h"
#include <EEPROM.h>


// --------- MACROS --------- //

#define STEPDELAY 200    // delay between each step in micro seconds
#define CW LOW           // clockwize, for DIR pin
#define ACW HIGH         // anticlockwise, for DIR pin
#define MAX_PRESETS 128  // the maximum number of presets by the MIDI standard
#define UNSET 255        // placeholding value for a unset preset
#define NOON 127         // neutral position (dial pointing straight up at noon)
#define FLASH_SPEED 300  // delay in between LED flashes

#define FIRST_DIR 53     // the first Easy driver direction pin (DIR)
#define FIRST_STEP 2     // the first Easy driver step pin (STEP)
#define FIRST_SLP 52     // the first Easy Driver sleep pin (SLP)
#define FIRST_BUTTOMN 22 // the first rotary encodor button pin
#define FIRST_LED 23     // the first limb LED pin


// -------------------- CLASS VARIABLES -------------------- //


// ----- PINS
byte dirPin;
byte stepPin;
byte sleepPin;
byte encodorPinA;
byte encodorPinB;
byte led;
byte button;

// ----- STEPPER MOTOR FACTS
int motorStepsPerRotation;
int encoderStepsPerRotation;

byte number;
// from 'dirPin' to 'number' would be consts but it causes huge amounts of errors, because the arduino can not
// specify when the constructor is called, so the compiler doesn't like using initialization lists for constants.

volotile byte currentPosition = NOON; // will always load from RAM and is set to default of NOON before loaded.
byte presets[MAX_PRESETS];


// ----- ROTRARY ENCODER VARIABLES
volatile int lastEncoded = 0;
volatile long encoderValue = 0;

long lastencoderValue = 0;

int lastMSB = 0;
int lastLSB = 0;



// -------------------- CLASS METHODS -------------------- //


// --------- PUBLIC METHODS --------- //


// ----- CONSTRUCTOR
StepperMotor::StepperMotor(const byte motorNo, const int encodorSPR, const int motorSPR) : number( motorNo ),
                                                                                           motorStepsPerRotation( motorSPR ),
                                                                                           encoderStepsPerRotation( encodorSPR ),
                                                                                           dirPin( FIRST_DIR - (2 * motorNo) ),
                                                                                           stepPin( motorNo + FIRST_STEP ),
                                                                                           sleepPin( FIRST_SLP - (2 * motorNo) ),
                                                                                           encodorPinA( calculateEncodorPinA(motorNo) ),
                                                                                           encodorPinB( calculateEncodorPinB(motorNo) ),
                                                                                           led( FIRST_LED - motorNo*2 ),
                                                                                           button( FIRST_BUTTON - motorNo*2 )
{
 
  // set up the pins  
  pinMode(dirPin, OUTPUT);
  pinMode(stepPin, OUTPUT);
  pinMode(sleepPin, OUTPUT);
  pinMode(led, OUTPUT);
  
  pinMode(encodorPinA, INPUT);
  pinMode(encodorPinB, INPUT);
  pinMode(button, INPUT);
  
  // initialise output pins
  digitalWrite(dirPin, CW);
  digitalWrite(stepPin, LOW);
  digitalWrite(sleepPin, LOW);
  digitalWrite(led, LOW);

  // Rotary Encodor interupts
  //call updateEncoder() when any high/low changed seen on interrupt [number * 2], or interrupt [(number * 2) + 1]
  attachInterrupt( number * 2, encoderMovement, CHANGE); 
  attachInterrupt((number * 2) + 1, encoderMovement, CHANGE);
  
  loadPresets(); //loads 'presets' array and 'currentPosition'
  
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
    
    //currentPosition = pos; //rotary encodor should automatically sort this out.
  }
  
}

void StepperMotor::checkButton(const byte currentPreset){
  // User presses for half a second, this saves the preset and the  flashes to say this.
  // holding on another 5 seconds clears the presets of the motor
  // holding on another 5 resets the motor as if it's in the upright possition, for when attatching to a new pedal.
 
  if(!digitalRead(button)){ // button is being pressed
  
    for(int i = 0; i < 15; i++){ 
      // 15 steps so if user can release at any point with only a 500ms wait before other things can be done
      // and it gives 2.5 seconds after clear all to let go before it saves the current possition as a preset.
      
      delay(500);
      
      if(digitalRead(button)) // button is not being pressed
        break; 
      else{
        
        switch(i){
          case 0:
            savePreset(currentPreset);
            break;
            
          case 5:
            clearPresets();
            break;
            
          case 9:
            clearAll();
            break;
        }
        
      }
    }
    
  }
  
}


void StepperMotor::savePreset(const byte currentPreset){
  
  if(currentPreset != -1){
    presets[currentPreset] = currentPosition;
    writePresetToMemory(currentPreset, currentPosition);
  }
  
  flashLED(FLASH_SPEED);
  
}


// ----- CLEARING FUNCTIONS
void StepperMotor::clearPresets(){
  
  for(int preset = 0; preset < MAX_PRESETS; preset++){
    writePresetToMemory(preset, UNSET);
    presets[preset] = UNSET;
  }
  
  flashLED(2, FLASH_SPEED, FLASH_SPEED/2);
  
}


void StepperMotor::clearAll(){ // remove all presets and reset currentPosition of this motor

  currentPosition = NOON;
  
  for(int preset = 0; preset < MAX_PRESETS; preset++){
    writePresetToMemory(preset, UNSET);
    presets[preset] = UNSET;
  }
  
  flashLED(3, FLASH_SPEED, FLASH_SPEED/2);
  
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
    
    // for debugging purposes
    //for(int preset = 0; preset < MAX_PRESETS; preset++) presets[preset] = random(0,254); //for display, remove and reset chip.
    //currentPosition = 255; // also remove  
}


int calculateEncodorPinA(const int motorNo){
  
  if(motorNo == 0){
    return 2;
  }else if(motorNo == 1){
    return 18;
  }else if(motorNo == 2){
    return 20;
  }else return -1;
  
}


int calculateEncodorPinB(const int motorNo){
  
  if(motorNo == 0){
    return 3;
  }else if(motorNo == 1){
    return 19;
  }else if(motorNo == 2){
    return 21;
  }else return -1;
  
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


void encoderMovement(){
  
  int MSB = digitalRead(encoderPin1); //MSB = most significant bit
  int LSB = digitalRead(encoderPin2); //LSB = least significant bit

  int encoded = (MSB << 1) |LSB; //converting the 2 pin value to single number
  int sum  = (lastEncoded << 2) | encoded; //adding it to the previous encoded value

  if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) currentPosition++;
  if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) currentPosition--;

  lastEncoded = encoded; //store this value for next time
  
}

void flashLED(const int time){
  
  digitalWrite(led, HIGH);
  delay(time);
  digitalWrite(led, LOW);
  
}


void flashLED(const int flashes, const int onTime, const int offTime){
  
  for(int i = 0; i < flashes - 1; i++){
    digitalWrite(led, HIGH);
    delay(onTime);
    digitalWrite(led, LOW);
    delay(offTime);
  }
  
  if(flashes > 0){
    digitalWrite(led, HIGH);
    delay(onTime);
    digitalWrite(led, LOW);
  }
  
}
