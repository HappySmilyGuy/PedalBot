#include <MIDI.h>
#include <EEPROM.h>
#include <StepperMotor.h>


// --------- Control definitions and constants --------- //

#define MAX_MOTORS 16        // the maximum number of motors by the amount of memory used per motor and maximum EEPROM memory
#define ENCODER_TO_MOTOR_RATIO 80 // 1600/20 (no. of steps per turn of motor) / (no. of steps per turn of encodor)
#define SPR 1600.0       // steps per complete rotation of the stepper motor



// --------- PINS --------- //
#define LED 13                    // Arduino LED (L) control
#define CLEAR_ALL 39              // reset button press
#define FIRST_SAVE_PRESET_PIN 100 // [[[ Maybe use the ANALOGE IN pins as digital in pins ]]]


// --------- Global Variables --------- //
int currentPreset = -1;
StepperMotor stepperMotors[MAX_MOTORS];
byte savePresetPins[MAX_MOTORS];

void setup(){
  // PIN initialisation
  pinMode(LED, OUTPUT);
  pinMode(CLEAR_ALL, INPUT);
  setSavePresetPins();
  for(int motor; motor < MAX_MOTORS; motor++){
    pinMode(savePresetPins[MaxMotors], INPUT);
  }
  
  
  // MIDI initialising
  MIDI.begin(MIDI_CHANNEL_OMNI);  // listen for MIDI on all channels at all frequencies
  MIDI.setHandleProgramChange(ChangePreset);
  // MIDI.turnThruOn();
  

}

void loop(){
  MIDI.read();
  if(digitalRead(CLEAR_ALL) == HIGH) checkClearPresetsButton();
}


// --------- MIDI FUNCTIONS --------- //
void ChangePreset(byte channel, byte number) {
  //channel is generally unused.
  for(int motor; motor < MAX_MOTORS; motor++){
    //stepperMotors[motor].moveToPreset(number);
  }
  currentPreset = preset;
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
  
  for(int i = 0; i < 10; i++){ // lots of delayed checks so it doesn't go off accidentily
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
      }
      //include factory Reset after 10 more seconds
    }
  }
}

// void checkSavePresetButton
  // include holding down calls stepperMotors[motor].clearPresets();
  // holding down longer does stepperMotors[motor].clearAll();
  // maybe make function calculateMotor(byte 

void factoryReset(){ //resests /ALL/ EEPROM memory
  for(int motor = 0; motor < MAX_MOTORS; motor++){
    stepperMotors[motor].clearAll();
  }
}

// this next function is entirely to save the EEPROM as it has limited writes, so this extends it's life
void presetWriteToMemory(int motor, int preset, int newPosition){ // preset ass -1 for current location
  if(preset >= 0){
    if(EEPROM.read(presetMemoryLocation(motor, preset)) != newPosition){ //EEPROM has limited writes, so this may extend life
      EEPROM.write(presetMemoryLocation(motor, preset), newPosition);
    }
  }else if(preset == -1){ // writing new current Location
    if(EEPROM.read(currentLocationMemoryLocation(motor) != newPosition){
      EEPROM.write(currentLocationMemoryLocation(motor), newPosition);
    }
  }
}

inline void setSavePresetPins(){
  for(int motor = 0; motor < MAX_MOTORS; motor++){
    savePresetPins[motor] = FIRST_SAVE_PRESET_PIN + motor;
  } 
}
