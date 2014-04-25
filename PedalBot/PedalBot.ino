#include <MIDI.h>
#include <EEPROM.h>
#include "StepperMotor.h"


// --------- Control definitions and constants --------- //

#define MAX_MOTORS 7        // the maximum number of motors by the amount of memory used per motor and maximum EEPROM memory
#define ENCODER_TO_MOTOR_RATIO 80 // 1600/20 (no. of steps per turn of motor) / (no. of steps per turn of encodor)
#define SPR 1600.0       // steps per complete rotation of the stepper motor


// --------- PINS --------- //
#define LED 13                    // Arduino LED (L) control
#define CLEAR_ALL 39              // reset button press


// --------- Global Variables --------- //
int currentPreset = -1;
StepperMotor stepperMotors[MAX_MOTORS] = { StepperMotor(0, 20, SPR),
                                           StepperMotor(1, 20, SPR),
                                           StepperMotor(2, 20, SPR),
                                           StepperMotor(3, 20, SPR),
                                           StepperMotor(4, 20, SPR),
                                           StepperMotor(5, 20, SPR),
                                           StepperMotor(6, 20, SPR) };
byte savePresetPins[MAX_MOTORS] = { A0, A1, A2, A3, A4, A5, A6 };

void setup(){
  
  // PIN initialisation
  pinMode(LED, OUTPUT);
  pinMode(CLEAR_ALL, INPUT);

  // I don't know if this will actually work in a loop
  for(int motor = 0; motor < MAX_MOTORS; motor++){
    pinMode(savePresetPins[motor], INPUT);
  }  
  
  // MIDI initialising
  MIDI.begin(MIDI_CHANNEL_OMNI);  // listen for MIDI on all channels at all frequencies
  MIDI.setHandleProgramChange(ChangePreset);
  // MIDI.turnThruOn();  

}


void loop(){
  
  MIDI.read();
  
  if(digitalRead(CLEAR_ALL) == HIGH) checkClearPresetsButton();
  
  for(int motor = 0; motor < MAX_MOTORS; motor++){
    if(digitalRead(savePresetPins[motor]) == HIGH) checkSavePresetButton(motor);
  }
  
}



// --------- MIDI FUNCTIONS --------- //
void ChangePreset(byte channel, byte number) {
  
  for(int motor; motor < MAX_MOTORS; motor++){
    //stepperMotors[motor].moveToPreset(number);
  }
  
  currentPreset = number;
  
}



// --------- Preset Funcations ---------- //
void clearAllPresets(){ // clears presets, but keeps current positions
  for(int motor = 0; motor < MAX_MOTORS; motor++){
    stepperMotors[motor].clearPresets();
  }
}



// --------- BUTTON CHECKING METHODS --------- //
void checkClearPresetsButton(){
  // User should hold the clear memory button for 12 seconds, in which they should see 
  // 1 slow flash followed by a quick flash to say memory has been cleared.
  // another 9 second wait and it will slow flash again and then quick flash, this is a complete factory reset.
  
  for(int i = 0; i < 20; i++){ // lots of delayed checks so it doesn't go off accidentily
    delay(1000);
    if(digitalRead(CLEAR_ALL) != HIGH) break;
    else{
      if(i == 8){
        digitalWrite(LED, HIGH);
        delay(1000);
        digitalWrite(LED, LOW);
      }else if(i == 9){     
        clearAllPresets();
        digitalWrite(LED, HIGH);
        delay(500);
        digitalWrite(LED,LOW);
      }else if(i == 18){
        digitalWrite(LED, HIGH);
        delay(1000);
        digitalWrite(LED, LOW);
      }else if(i == 19){
        factoryReset();
        digitalWrite(LED, HIGH);
        delay(500);
        digitalWrite(LED,LOW);
      }
    }
  }
  
}


void checkSavePresetButton(byte motor){
  // User presses for half a second, this saves the preset and LED 13 flashes to say this.
  // holding on another 5 seconds clears the presets of the motor
  // holding on another 5 resets the motor as if it's in the upright possition, for when attatching to a new pedal.
  
  for(int i = 0; i < 10; i++){ // lots of delayed checks so it doesn't go off accidentily
    delay(500);
    if(digitalRead(savePresetPins[motor]) != HIGH) break;
    else{
      if(i == 0){
        stepperMotors[motor].savePreset(currentPreset); 
        digitalWrite(LED, HIGH);
        delay(1000);
        digitalWrite(LED, LOW);
      }else if(i < 5){
        delay(500); 
      }else if(i == 5){
        stepperMotors[motor].clearPresets();
        digitalWrite(LED, HIGH);
        delay(1000);
        digitalWrite(LED, LOW);
      }else if(i < 9){        
        delay(500);
      }else if(i == 9){     
        stepperMotors[motor].clearAll();
        digitalWrite(LED, HIGH);
        delay(500);
        digitalWrite(LED,LOW);
      }
    }
  }
  
}


void factoryReset(){ //resests /ALL/ EEPROM memory

  for(int motor = 0; motor < MAX_MOTORS; motor++){
    stepperMotors[motor].clearAll();
  }
  
}
