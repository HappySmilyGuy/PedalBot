#include <MIDI.h>

// --------- Control constants --------- //
const boolean DEBUG = false; // false to remove all printouts and LED flashes
const int STEPDELAY = 200;   // delay between each step in micro seconds
const float SPR = 1600.0;    // steps per complete rotation of the stepper motor
const int CW = LOW;          // clockwize, for DIR pin
const int ACW = HIGH;        // anticlockwise, for DIR pin

// --------- PINS --------- //
const int DIR = 53; // Easy driver direction pin
const int STEP = 2; // Easy driver step pin
const int SLP = 52; // Easy Driver sleep pin
const int LED = 13; // Arduino LED (L) control
const int CLEAR_ALL = 39; // reset button press

// --------- Global Variables --------- //




void setup(){
  pinMode(LED, OUTPUT);
  
  // MIDI initialising
  MIDI.begin(MIDI_CHANNEL_OMNI);  // listen for MIDI on all channels at all frequencies
  MIDI.setHandleControlChange(ControlChanges);
  MIDI.setHandleProgramChange(ChangePreset);
  MIDI.turnThruOn();
  
  // Stepper Motor Driver Initialistion
  pinMode(DIR, OUTPUT);     
  pinMode(STEP, OUTPUT);
  pinMode(SLP, OUTPUT);
  pinMode(CLEAR_ALL, INPUT);
  digitalWrite(DIR, HIGH);
  digitalWrite(STEP, LOW);
  digitalWrite(SLP, HIGH); 
}

void loop(){
  MIDI.read();
  if(digitalRead(CLEAR_ALL) == HIGH){ // lots of delayed checks so it doesn't go off accidentily
    delay(2000);
    if(digitalRead(CLEAR_ALL) == HIGH){
      delay(2000);
      if(digitalRead(CLEAR_ALL) == HIGH){
        delay(2000);
        if(digitalRead(CLEAR_ALL) == HIGH){
          digitalWrite(LED, HIGH);
          delay(1000);
          digitalWrite(LED,LOW);
        }
      }
    }
  }
}


void ChangePreset(byte channel, byte number) {
  //pedal footswitches use channel 0
  //channel is generally unused.
  //number is the patch number being selected
  //SelectPreset((int)number);
  
  for(int i = 0; i < number; i++){
      DriveMotor(180, CW);
  }
  
  
  
  // ----- Debugging ----- //
  if(DEBUG){    
    for(int i = 0; i < (int)number; i++){ //flashes main LED the same amount of times as the number of the preset that was just called.
      digitalWrite(LED, HIGH);
      delay(500);
      digitalWrite(LED,LOW);
      delay(500);
    }
    Serial.print("\nCHANNEL\ndec: ");
    Serial.println((int)channel, DEC);
    Serial.print("hex: ");
    Serial.println(channel, HEX);
    Serial.print("bin: ");
    Serial.println(channel, BIN);
    Serial.flush();
    Serial.print("\nNUMBER\ndec: ");
    Serial.println((int)number, DEC);
    Serial.print("hex: ");
    Serial.println(number, HEX);
    Serial.print("bin: ");
    Serial.println(number, BIN);
    Serial.flush();
  }
  
}

void ControlChanges(byte channel, byte number, byte value){
  if(DEBUG){
    Serial.print("\nControl Change:\nchannel: ");
    Serial.println((int)channel, DEC);
    Serial.print("\nNote: ");
    Serial.println((int)number, DEC);
    Serial.print("value: ");
    Serial.println((int)value, DEC);
    Serial.flush();
  }
  // 0: bank select 
  // 1: mod wheel 
  // 2: breath controller 
  // 5: portamento time 
  // 6: data entry -- used with the registered parameter mechanism 
  // 7: volume 
  // 10: pan -- adjusts the left-right positioning of a signal in a stereo output 
  // 11: expression pedal 
  // 64: sustain pedal 
  // 65: portamento on/off 
  // 88: High resolution velocity prefix (added 2010) 
  // 98 & 99: non-registered parameter number 
  // 100 & 101: registered parameter number 
  // 121: reset all controllers 
  // 122: local control on/off 
  // 123: all notes off 
}

void DriveMotor(int angle, int dir){  
  digitalWrite(SLP, HIGH);
  float rotate = (float) (angle * SPR) / 360.0; // to avoid rounding error
  for(int i = 0; i < rotate; i++){
      digitalWrite(STEP, HIGH);
      delayMicroseconds(STEPDELAY);          
      digitalWrite(STEP, LOW); 
      delayMicroseconds(STEPDELAY);
  }
  digitalWrite(SLP, LOW); 
}

