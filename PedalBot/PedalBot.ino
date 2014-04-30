#include <MIDI.h>
#include <EEPROM.h>
#include "StepperMotor.h"


// --------- Control definitions and constants --------- //

#define DEBUG 0
#define MAX_MOTORS 3        // the maximum number of motors by the amount of memory used per motor and maximum EEPROM memory
#define ENCODER_TO_MOTOR_RATIO 80 // 1600/20 (no. of steps per turn of motor) / (no. of steps per turn of encodor)
#define SPR 1600.0       // steps per complete rotation of the stepper motor


// --------- PINS --------- //
#define LED 13                    // Arduino LED (L) control
#define CLEAR_ALL 39              // reset button press


// --------- Global Variables --------- //
int currentPreset = -1;
StepperMotor stepperMotors[MAX_MOTORS] = { StepperMotor(0, 20, SPR),
                                           StepperMotor(1, 20, SPR),
                                           StepperMotor(2, 20, SPR) };

void setup(){
  
  // PIN initialisation
  pinMode(LED, OUTPUT);
  pinMode(CLEAR_ALL, INPUT);

  for(int motor = 0; motor < MAX_MOTORS; motor++){
    pinMode(savePresetPins[motor], INPUT);
  }  
  
  // MIDI initialising
  MIDI.begin(MIDI_CHANNEL_OMNI);  // listen for MIDI on all channels at all frequencies
  MIDI.setHandleProgramChange(ChangePreset);
  MIDI.turnThruOff();  // using hardware thru

}


void loop(){
  
  MIDI.read();
  
  checkButtons();
 
}



// --------- MIDI FUNCTIONS --------- //
void ChangePreset(const byte channel, const byte number) {  //if MIDI isn't working, remove 'const'
  
  if(DEBUG){
    flashLED(number, 100, 100);
  }
  
  for(int motor = 0; motor < MAX_MOTORS; motor++){
    stepperMotors[motor].moveToPreset(number);
  }
  
  currentPreset = number;
  
}



// --------- BUTTON CHECKING METHODS --------- //
void checkButtons(){
  //if(digitalRead(CLEAR_ALL) == HIGH) checkClearPresetsButton();
  
  for(int motor = 0; motor < MAX_MOTORS; motor++){
    stepperMotors[motor].checkButton(const byte currentPreset);
  }
  
}


void checkClearPresetsButton(){
  // User should hold the clear memory button for 12 seconds, in which they should see 
  // 1 slow flash followed by a quick flash to say memory has been cleared.
  // another 9 second wait and it will slow flash again and then quick flash, this is a complete factory reset.
  
  for(int i = 0; i < 20; i++){ // lots of delayed checks so it doesn't go off accidentily
  
    delay(1000);
    
    if(digitalRead(CLEAR_ALL) != HIGH)
      break;
    else{
      
      switch(i){
        case 8:
          
          break;
         
        case 9:
       
          break;
         
        case 18:
        
          break;
        case 19:
        
          break;
        
      }
      if(i == 8){
        flashLED(1000);
      }else if(i == 9){     
       // clearAllPresets();
      }else if(i == 18){
        flashLED(1000);
      }else if(i == 19){
       // factoryReset();
      }
    }
  }
  
}


void clearAllPresets(){ // clears presets, but keeps current positions
  
  for(int motor = 0; motor < MAX_MOTORS; motor++){
    stepperMotors[motor].clearPresets();
  }
  
  flashLED(500);
  
}


void factoryReset(){ //resests /ALL/ EEPROM memory

  for(int motor = 0; motor < MAX_MOTORS; motor++){
    stepperMotors[motor].clearAll();
  }
  
  flashLED(2, 500, 250);
  
}


void flashLED(const int time){
  
  digitalWrite(LED, HIGH);
  delay(time);
  digitalWrite(LED, LOW);
  
}


void flashLED(const int flashes, const int onTime, const int offTime){
  
  for(int i = 0; i < flashes - 1; i++){
    digitalWrite(LED, HIGH);
    delay(onTime);
    digitalWrite(LED, LOW);
    delay(offTime);
  }
  
  if(flashes > 0){
    digitalWrite(LED, HIGH);
    delay(onTime);
    digitalWrite(LED, LOW);
  }
  
}
