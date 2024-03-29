#include "Limb.h"
#include <EEPROM.h>


// --------- MACROS --------- //

#define STEPDELAY 200    // delay between each step in micro seconds
#define CW LOW           // clockwize, for DIR pin
#define ACW HIGH         // anticlockwise, for DIR pin

#define UNSET 255        // placeholding value for a unset preset
#define NOON 127           // neutral position (dial pointing straight up at noon)
#define FLASH_SPEED 300  // delay in between LED flashes
#define BIG_COG_TEETH 25 // the number of teeth on the limb's large cog
#define SPR 1600.0       // steps per complete rotation of the stepper limb

#define FIRST_DIR 53     // the first Easy driver direction pin (DIR)
#define FIRST_STEP 12     // the first Easy driver step pin (STEP)
#define FIRST_SLP 52     // the first Easy Driver sleep pin (SLP)
#define FIRST_BUTTON 22  // the first rotary encoder button pin
#define FIRST_LED 23     // the first limb LED pin


// -------------------- CLASS VARIABLES -------------------- //


// ----- PINS
byte dirPin, stepPin, sleepPin, encoderPinA, encoderPinB, led, button;


// ----- STEPPER MOTOR FACTS
byte number; //which stepper motor it is
int motorStepsPerRotation;

// all the above variables would be consts but it causes huge amounts of errors, because the arduino can not
// specify when the constructor is called, so the compiler doesn't like using initialization lists for constants.

volatile byte currentPosition = NOON; // will always load from RAM and is set to default of NOON before loaded.
byte presets[MAX_PRESETS];


// ----- ROTRARY ENCODER VARIABLES
volatile int lastEncoded = 0;

long lastencoderValue = 0;

byte lastMSB = 0;
byte lastLSB = 0;



// -------------------- CLASS METHODS -------------------- //


// --------- FORWARD DECLITATIONS --------//
void encoderMovement();
    
// --------- PUBLIC METHODS --------- //


// ----- CONSTRUCTOR
Limb::Limb(const byte limbNo) : number( limbNo ),
	                        motorStepsPerRotation( SPR ),
                                stepPin( FIRST_STEP - limbNo ),
	                        dirPin( FIRST_DIR - (2 * limbNo) ),
	                        sleepPin( FIRST_SLP - (2 * limbNo) ),
	                        encoderPinA( calculateEncodorPinA(limbNo) ),
	                        encoderPinB( calculateEncodorPinB(limbNo) ),
	                        led( FIRST_LED + (limbNo * 2) ),
	                        button( FIRST_BUTTON + (limbNo * 2) )
 {
  	 
  // set up the pins  
  pinMode(dirPin, OUTPUT);
  pinMode(stepPin, OUTPUT);
  pinMode(sleepPin, OUTPUT);
  pinMode(led, OUTPUT);
  
  pinMode(encoderPinA, INPUT);
  pinMode(encoderPinB, INPUT);
  pinMode(button, INPUT);
  
  digitalWrite(button, HIGH); //engages digital pullup
  
  // initialise output pins
  digitalWrite(dirPin, CW);
  digitalWrite(stepPin, LOW);
  digitalWrite(sleepPin, LOW);
  digitalWrite(led, LOW);

  // Rotary Encodor interupts
  //call updateEncoder() when any high/low changed seen on interrupt [number * 2], or interrupt [(number * 2) + 1]
  attachInterrupt( limbNo * 2, encoderMovement, CHANGE); 
  attachInterrupt((limbNo * 2) + 1, encoderMovement, CHANGE);
  
  loadPresets(); //loads 'presets' array and 'currentPosition'
  

}


// ----- DESTRUCTOR
Limb::~Limb(){}


void Limb::drive(const int newPosition){ //update to include rotary encoder

  int stepsPerNotch = motorStepsPerRotation / (360 / BIG_COG_TEETH);
  
  digitalWrite(sleepPin, HIGH);
  digitalWrite(led, HIGH);
  
  int difference = newPosition - currentPosition;
  
  while(difference < -5 || difference > 5){
    
    if(difference > 0){
    
      digitalWrite(dirPin, ACW);
    
      for(int i = 0; i < (difference * stepsPerNotch) / 7; i++){
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(STEPDELAY);
        digitalWrite(stepPin, LOW);
        delayMicroseconds(STEPDELAY);
      }
      
    }else if(difference < 0){
      
      digitalWrite(dirPin, CW); 
      
      for(int i = 0; i < (-difference * stepsPerNotch) / 7; i++){
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(STEPDELAY);
        digitalWrite(stepPin, LOW);
        delayMicroseconds(STEPDELAY);
      }
      
    }
    
    difference = newPosition - currentPosition;
    
  }
  
  digitalWrite(sleepPin, LOW);
  digitalWrite(led, LOW);
  
}


void Limb::moveToPreset(const byte preset){
        
  int newPosition = presets[preset];
  
  if(newPosition != UNSET){ // if the preset has been set

    drive(newPosition);

  }
  
}

void Limb::checkButton(const byte currentPreset){
  // User presses for half a second, this saves the preset and the  flashes to say this.
  // holding on another 5 seconds clears the presets of the limb
  // holding on another 5 resets the motor as if it's in the upright possition, for when attatching to a new pedal.
  
  if(!digitalRead(button)){ // button is being pressed
    
    for(int i = 0; i < 15; i++){ 
      // 15 steps so if user can release at any point with only a 500ms wait before other things can be done
      // and it gives 2.5 seconds after clear all to let go before it saves the current possition as a preset.
      
      if(i != 0) delay(500);
      
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


void Limb::savePreset(const byte currentPreset){
  
  if(currentPreset != -1){
    presets[currentPreset] = currentPosition;
    writePresetToMemory(currentPreset, currentPosition);
  }
  
  flashLED(FLASH_SPEED*2);
  
}


// ----- CLEARING METHODS
void Limb::clearPresets(){
  
  for(int preset = 0; preset < MAX_PRESETS; preset++){
    writePresetToMemory(preset, UNSET);
    presets[preset] = UNSET;
  }
  
  flashLED(2, FLASH_SPEED, FLASH_SPEED/2);
  
}


void Limb::clearAll(){ // remove all presets and reset currentPosition of this limb

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
void Limb::loadPresets(){
  
    for(int preset = 0; preset < MAX_PRESETS; preset++){
      presets[preset] = EEPROM.read(presetMemoryLocation(preset));
    }
    
    currentPosition = EEPROM.read(currentLocationMemoryLocation());

}


int Limb::calculateEncodorPinA(const int limbNo){
  
  if(limbNo == 0){
    return 2;
  }else if(limbNo == 1){
    return 18;
  }else if(limbNo == 2){
    return 20;
  }else return -1;
  
}


int Limb::calculateEncodorPinB(const int limbNo){
  
  if(limbNo == 0){
    return 3;
  }else if(limbNo == 1){
    return 19;
  }else if(limbNo == 2){
    return 21;
  }else return -1;
  
}


// ----- FUNCTIONAL
void Limb::writePresetToMemory(const byte preset, const byte newPosition){ // input '-1' for preset to write a new current location

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


void Limb::flashLED(const int time){
  
  digitalWrite(led, HIGH);
  delay(time);
  digitalWrite(led, LOW);
  
}


void Limb::flashLED(const int flashes, const int onTime, const int offTime){
  
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
