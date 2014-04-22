#include <MIDI.h>
#include <midi_Defs.h>
#include <midi_Namespace.h>
#include <midi_Settings.h>
#include <SoftwareSerial.h>

int led = 13;



void ChangePreset(byte channel, byte number) {
  for(int i = 0; i < 4; i++){
    digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(10);               // wait for a second
    digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
    delay(10);
  }
  
  delay(1000);
  
  for(int i = 0; i < (int)number; i++){
    digitalWrite(led, HIGH);
    delay(500);
    digitalWrite(led,LOW);
    delay(500);
  }
   
  
  //pedal footswitches use channel 0
  //channel is generally unused.
  //number is the patch number being selected
  //SelectPreset((int)number);
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

void ControlChanges(byte channel, byte number, byte value){
  Serial.print("\nControl Change:\nchannel: ");
  Serial.println((int)channel, DEC);
  Serial.print("\nNote: ");
  Serial.println((int)number, DEC);
  Serial.print("value: ");
  Serial.println((int)value, DEC);
  Serial.flush();
  //0: bank select 
  //1: mod wheel 
  //2: breath controller 
  //5: portamento time 
  //6: data entry -- used with the registered parameter mechanism 
  //7: volume 
  //10: pan -- adjusts the left-right positioning of a signal in a stereo output 
  //11: expression pedal 
  //64: sustain pedal 
  //65: portamento on/off 
  //88: High resolution velocity prefix (added 2010) 
  //98 & 99: non-registered parameter number 
  //100 & 101: registered parameter number 
  //121: reset all controllers 
  //122: local control on/off 
  //123: all notes off
  
  
  
}

void setup(){
  pinMode(led, OUTPUT);
  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.setHandleControlChange(ControlChanges);
  MIDI.setHandleProgramChange(ChangePreset);

  MIDI.turnThruOn();
}

void loop(){
  MIDI.read();   
}
