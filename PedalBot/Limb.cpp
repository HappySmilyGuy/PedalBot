#include "Limb.h"
#include <EEPROM.h>


// --------- MACROS --------- //

#define STEPDELAY 200    // delay between each step in micro seconds
#define CW LOW           // clockwize, for DIR pin
#define ACW HIGH         // anticlockwise, for DIR pin
#define MAX_PRESETS 128  // the maximum number of presets by the MIDI standard
#define UNSET 255        // placeholding value for a unset preset
#define NOON 127         // neutral position (dial pointing straight up at noon)
#define FLASH_SPEED 300  // delay in between LED flashes
#define BIG_COG_TEETH 25 // the number of teeth on the limb's large cog
#define SPR 1600.0       // steps per complete rotation of the stepper limb

#define FIRST_DIR 53     // the first Easy driver direction pin (DIR)
#define FIRST_STEP 2     // the first Easy driver step pin (STEP)
#define FIRST_SLP 52     // the first Easy Driver sleep pin (SLP)
#define FIRST_BUTTON 22  // the first rotary encoder button pin
#define FIRST_LED 23     // the first limb LED pin


// -------------------- CLASS VARIABLES -------------------- //


// ----- PINS
byte dirPin;
byte stepPin;
byte sleepPin;
byte encoderPinA;
byte encoderPinB;

byte led;
byte button;

// ----- STEPPER MOTOR FACTS
int motorStepsPerRotation;

byte number;
// from 'dirPin' to 'number' would be consts but it causes huge amounts of errors, because the arduino can not
// specify when the constructor is called, so the compiler doesn't like using initialization lists for constants.

volatile byte currentPosition = NOON; // will always load from RAM and is set to default of NOON before loaded.
byte presets[MAX_PRESETS];


// ----- ROTRARY ENCODER VARIABLES
volatile int lastEncoded = 0;
volatile long encoderValue = 0;

long lastencoderValue = 0;

int lastMSB = 0;
int lastLSB = 0;



// -------------------- CLASS METHODS -------------------- //


// --------- FORWARD DECLITATIONS --------//

void encoderMovement();

    
// --------- PUBLIC METHODS --------- //


// ----- CONSTRUCTOR
Limb::Limb(const byte limbNo) : number( limbNo ),
	                        motorStepsPerRotation( SPR ),
	                        dirPin( FIRST_DIR - (2 * limbNo) ),
	                        stepPin( limbNo + FIRST_STEP ),
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
  attachInterrupt( number * 2, encoderMovement, CHANGE); 
  attachInterrupt((number * 2) + 1, encoderMovement, CHANGE);
  
  loadPresets(); //loads 'presets' array and 'currentPosition'
  
}


// ----- DESTRUCTOR
Limb::~Limb(){}


// ----- INITIALISER
/*void Limb::initialise(const byte limbNo){ 
  // this is a product of Arduino. This was as an class initialisation list, as is best,
  // but because of how an Arduino initialises stuff, it really doesn't like it.
  // In short, it just doesn't know where things initialise, so you usually have a 'Begin' method
  // as you'll see for the MIDI library. This is my equivilant.
  
  number = limbNo;
  motorStepsPerRotation = SPR;
  dirPin = FIRST_DIR - (2 * limbNo);
  stepPin = limbNo + FIRST_STEP;
  sleepPin = FIRST_SLP - (2 * limbNo);
  encoderPinA = calculateEncodorPinA(limbNo);
  encoderPinB = calculateEncodorPinB(limbNo);
  led = FIRST_LED + (limbNo * 2);
  button = FIRST_BUTTON + (limbNo * 2);*/
  



void Limb::drive(const int newPosition){ //update to include rotary encoder
  
  digitalWrite(sleepPin, HIGH);
  
  int difference = newPosition - currentPosition;
  int stepsPerNotch = motorStepsPerRotation / (360 / BIG_COG_TEETH);
  
  flashLED(1000);
  
  if(difference == 0){
    //do nothing
  }else if(difference > 0){ // if keep moving for far too long then CW and ACW are the wrong way round
  
    digitalWrite(dirPin, CW);
    
    while(currentPosition < newPosition){
      
      for(int i = 0; i < stepsPerNotch/4; i++){ // efficency: this loop is so a check isn't done every step
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(STEPDELAY);
        digitalWrite(stepPin, LOW);
        delayMicroseconds(STEPDELAY);
      }
      
    }
    
  }else if(difference < 0){
    
    digitalWrite(dirPin, ACW);
    
    while(currentPosition > newPosition){
      
      for(int i = 0; i < stepsPerNotch/4; i++){ // efficency: this loop is so a check isn't done every step
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(STEPDELAY);
        digitalWrite(stepPin, LOW);
        delayMicroseconds(STEPDELAY);
      }
      
    }
    
  }

  digitalWrite(sleepPin, LOW);
  
}


void Limb::moveToPreset(const byte preset){
  
  flashLED(1000);
  
  int newPosition = presets[preset];
  
  if(newPosition != UNSET){ // if the preset has been set
  
    //drive(newPosition);

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
  
  flashLED(FLASH_SPEED);
  flashLED(FLASH_SPEED);
  
}


// ----- CLEARING FUNCTIONS
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
    
    // for debugging purposes
    //for(int preset = 0; preset < MAX_PRESETS; preset++) presets[preset] = random(0,254); //for display, remove and reset chip.
    //currentPosition = 255; // also remove  
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


void encoderMovement(){
  
  int MSB = digitalRead(encoderPinA); //MSB = most significant bit
  int LSB = digitalRead(encoderPinB); //LSB = least significant bit

  int encoded = (MSB << 1) |LSB; //converting the 2 pin value to single number
  int sum  = (lastEncoded << 2) | encoded; //adding it to the previous encoded value

  if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) currentPosition++;
  if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) currentPosition--;

  lastEncoded = encoded; //store this value for next time
  
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
