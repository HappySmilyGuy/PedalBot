#include <Servo.h>
#include <EEPROM.h>

const int MAX_PRESETS = 128;
const int MAX_MOTORS = 16;
const int SERVO_IN_MIN = 5;
const int SERVO_IN_MAX = 178;
const int ENCODER_TO_MOTOR_RATIO = 1600/20;

int potpin = 0;  // analog pin used to connect the potentiometer
int val;    // variable to read the value from the analog pin
int presets[NO_OF_MOTORS][MAX_PRESETS];
int num;
char Char;
int maxi;
int mini;


void setup(){ 
  readAllPresets();
}

void loop(){
  if(Serial.available() > 0){
    char typed = Serial.read();
    Serial.println(typed);
     if(typed == 's'){  //saves
       while(Serial.available() <= 0){
         delay(1); 
       }
       int AR = analogRead(potpin);
       val = map(analogRead(potpin), mini, maxi, SERVO_IN_MIN, SERVO_IN_MAX);
       Char = Serial.read();
       num = ((int)Char)%128;
       presets[num] = val;
     }else if(typed == 'c'){
       clearMemory();
     }else{
       num = (int)typed;
       if(num >= 33){
         num = num%128;
         if(presets[num] > SERVO_IN_MIN && presets[num] < SERVO_IN_MAX){
           myservo.write(presets[num]);
         }else{
           Serial.println(" has not been set, or has been set incorrectly.");
         }           
       }
     }
  }
}



inline int recallPreset(int motor, int preset){
  return presets[motor][preset];
}

void savePreset(int motor, int preset, int currentPosition){
  presets[motor][preset] = currentPosition;
  EEPROM.write(motor*(MAX_PRESETS + 1) + preset + 1, currentPosition);
}

int moveToPreset(int motor, int preset){
  int pos = recallPreset(motor, preset);
  pos = pos - currentPosition;
  if(pos < 0) DriveMotor(motor, pos, CW);
  else if(pos > 0) DriveMotor(motor, pos, ACW);
}

void loadPresets(){
  for(int motor = 0; motor < MAX_MOTORS; motor++){
    for(int preset = 0; preset < MAX_PRESETS + 1; preset++){
      preset[motor][preset] = EEPROM.read(motor*(MAX_PRESETS+1) + preset + 1);
    }
  }
}

void clearMemory(){
  for(int i = 0; i < MAX_PRESETS; i++){
    EEPROM.write(i, 255);
    presets[i] = 255;
  }
}

int presetMemoryLocation(int motor, int preset){
  return motor * (MAX_PRESETS + 1) + preset + 1;
}

int currentLocationMemoryLocation(int motor){
 return motor * (MAX_PRESETS + 1); 
}
  


