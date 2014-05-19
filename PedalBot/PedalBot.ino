/*
This Program is made to be run on the Arduino MEGA 2560 v3
It is made to be run with the Tiny Titan Effects' PedalBot Brain v1.0 and PedalBot Limb v2.4.3
Last Eddited on 09/05/2014
Have a great day :)
*/
#include <MIDI.h>
#include <EEPROM.h>
#include "Limb.h"


// --------- Control definitions and constants --------- //

#define DEBUG false
#define MAX_LIMBS 2   // the maximum number of limbs by the amount of memory used per limb and maximum EEPROM memory


// --------- PINS --------- //
#define LED 13        // Arduino LED (L) control
#define CLEAR_ALL 39  // reset button press


// --------- Global Variables --------- //
int currentPreset = -1;
//Limb limb0 = Limb(0); //inturupt pins malfunction
Limb limb1 = Limb(1);
Limb limb2 = Limb(2);

Limb limbs[MAX_LIMBS] = { limb2, limb1 };



// ------------ SETUP ------------ //
void setup(){

  if(DEBUG){
     factoryReset();
  }

  
  // PIN initialisation
  pinMode(LED, OUTPUT);
  pinMode(CLEAR_ALL, INPUT);
  
  digitalWrite(CLEAR_ALL, HIGH); // engages digital pull-up

  // MIDI initialising
  MIDI.begin(MIDI_CHANNEL_OMNI);  // listen for MIDI on all channels at all frequencies
  MIDI.setHandleProgramChange(ChangePreset);
  MIDI.turnThruOff();  // using hardware thru

}



// ------------ MAIN LOOP ------------ //
void loop(){
  
  MIDI.read();
  
  checkButtons();
 
}



// --------- MIDI FUNCTIONS --------- //
void ChangePreset(const byte channel, const byte number) {
  
  if(DEBUG) flashLED(number, 200, 100);
  
  for(int limb = 0; limb < MAX_LIMBS; limb++){    
    limbs[limb].moveToPreset(number);    
  }
  
  currentPreset = number;
  
}



// --------- BUTTON FUNCTIONS --------- //
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
          flashLED(1000);
          break;
         
        case 9:
          clearAllPresets();
          break;
         
        case 18:
          flashLED(1000);
          break;
          
        case 19:
          factoryReset();
          break;
        
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




// --------- INTERUPT FUNCTION --------- //
void encoderMovement(){
  if(DEBUG) digitalWrite(13, HIGH);
  
  for(int limb = 0; limb < MAX_LIMBS; limb++){
   
    byte newMSB = digitalRead(limbs[limb].encoderPinA);
    byte newLSB = digitalRead(limbs[limb].encoderPinB); 
    
    // algorythm originally taken from http://bildr.org/2012/08/rotary-encoder-arduino/
    if(limbs[limb].lastMSB != newMSB || limbs[limb].lastLSB != newLSB){ // so only runs on the rotary encoder that called the interupt
      
      limbs[limb].lastMSB = newMSB; 
      limbs[limb].lastLSB = newLSB; 
    
      int encoded = (newMSB << 1) | newLSB; //converting the 2 pin value to single number
      int sum  = (limbs[limb].lastEncoded << 2) | encoded; //adding it to the previous encoded value
      
      switch(sum){
        
        case 0b1011:
        case 0b1101:
        case 0b0100:
        case 0b0010:
          limbs[limb].currentPosition++; //clockwise movement
          break;

        case 0b0001:
        case 0b1000:
        case 0b1110:
        case 0b0111:
          limbs[limb].currentPosition--; //anti-clockwise movement
          break;
          
      }
      
      limbs[limb].lastEncoded = encoded; //store this value for next time
      
    }
    
  }
  
  if(DEBUG) digitalWrite(13, LOW);
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
