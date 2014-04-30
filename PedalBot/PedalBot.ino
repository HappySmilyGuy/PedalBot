#include <MIDI.h>
#include <EEPROM.h>
#include "Limb.h"


// --------- Control definitions and constants --------- //

#define DEBUG true
#define MAX_LIMBS 3   // the maximum number of limbs by the amount of memory used per limb and maximum EEPROM memory


// --------- PINS --------- //
#define LED 13        // Arduino LED (L) control
#define CLEAR_ALL 39  // reset button press


// --------- Global Variables --------- //
int currentPreset = -1;
Limb limbs[MAX_LIMBS] = { Limb(),
                          Limb(),
                          Limb() };


void setup(){
  
  limbs[1].initialise(1);

/*  for(int limb = 0; limb < MAX_LIMBS; limb++){
    limbs[limb].initialise(limb);
  } */
  
    if(DEBUG){
     factoryReset();
  }

  
  // PIN initialisation
  pinMode(LED, OUTPUT);
  pinMode(CLEAR_ALL, INPUT);
  
  

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
    flashLED(number, 200, 100);
  }
  
  for(int limb = 0; limb < MAX_LIMBS; limb++){
    limbs[limb].moveToPreset(number); // ERROR: hanging
  }
  
  currentPreset = number;
  
}



// --------- BUTTON CHECKING METHODS --------- //
void checkButtons(){
  //if(digitalRead(CLEAR_ALL) == HIGH) checkClearPresetsButton();
  
  for(int limb = 0; limb < MAX_LIMBS; limb++){
    limbs[limb].checkButton(currentPreset);
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
  
  for(int limb = 0; limb < MAX_LIMBS; limb++){
    limbs[limb].clearPresets();
  }
  
  flashLED(500);
  
}


void factoryReset(){ //resests /ALL/ EEPROM memory

  for(int limb = 0; limb < MAX_LIMBS; limb++){
    limbs[limb].clearAll();
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
