#include <EEPROM.h>

const int MAX_PRESETS = 128;
const int MAX_MOTORS = 16;
const int ENCODER_TO_MOTOR_RATIO = 1600/20;


int presets[MAX_MOTORS][MAX_PRESETS];
int currentPositions[MAX_MOTORS];
int currentPreset;


void setup(){ 
  loadPresets();
  currentPreset = -1; // placeholder for not yet set.
}

void loop(){
}

inline void updateCurrentPosition(int motor, int newPosition){
  currentPositions[motor] = newPosition;
  EEPROM.write(currentLocationMemoryLocation(motor), newPosition);
}

inline int recallPreset(int motor, int preset){
  return presets[motor][preset];
}

void savePreset(int motor, int currentPosition){
  if(currentPreset != -1){
    presets[motor][currentPreset] = currentPosition;
    EEPROM.write(presetMemoryLocation(motor, currentPosition), currentPosition);
  }
  updateCurrentPosition(motor, currentPosition);
}

int moveToPreset(int motor, int preset){
  int pos = recallPreset(motor, preset);
  pos = pos - currentPositions[motor];
 // if(pos < 0) DriveMotor(motor, pos, CW);
 // else if(pos > 0) DriveMotor(motor, pos, ACW);
  updateCurrentPosition(motor, pos);
  currentPreset = preset;
}

void loadPresets(){
  for(int motor = 0; motor < MAX_MOTORS; motor++){
    for(int preset = 0; preset < MAX_PRESETS + 1; preset++){
      preset[motor][preset] = EEPROM.read(presetMemoryLocation(motor, preset));
    }
    currentPositions[motor] = EEPROM.read(currentLocationMemoryLocation(motor));
  }
}

void clearMemory(){
  for(int motor = 0; motor < MAX_MOTORS; motor++){
    for(int preset = 0; preset < MAX_PRESETS; preset++){
      EEPROM.write(presetMemoryLocation(motor, preset), 255);
      presets[motor][preset] = 255;
    }
  }
}

inline int presetMemoryLocation(int motor, int preset){
  return motor * (MAX_PRESETS + 1) + preset + 1;
}

inline int currentLocationMemoryLocation(int motor){
 return motor * (MAX_PRESETS + 1); 
}
