#include <MIDI.h>
#include <EEPROM.h>


// --------- Control definitions and constants --------- //
// General
#define DEBUG false     // false to remove all printouts and LED flashes
// Motor definitions
#define STEPDELAY 200   // delay between each step in micro seconds
#define SPR 1600.0      // steps per complete rotation of the stepper motor
#define CW LOW          // clockwize, for DIR pin
#define ACW HIGH        // anticlockwise, for DIR pin
// Preset definitions
#define MAX_PRESETS 128
#define MAX_MOTORS 16
#define ENCODER_TO_MOTOR_RATIO 80 //1600/20
#define NOON 127        // dial pointing straight up, neutral position

// --------- PINS --------- //
// General
#define LED 13          // Arduino LED (L) control
#define CLEAR_ALL 39    // reset button press
// Motors
#define DIR 53          // Easy driver direction pin
#define STEP 2          // Easy driver step pin
#define SLP 52          // Easy Driver sleep pin


// --------- Global Variables --------- //
int presets[MAX_MOTORS][MAX_PRESETS];
int currentPositions[MAX_MOTORS];
int currentPreset;



void setup(){
  pinMode(LED, OUTPUT);
  
  // MIDI initialising
  MIDI.begin(MIDI_CHANNEL_OMNI);  // listen for MIDI on all channels at all frequencies
  MIDI.setHandleProgramChange(ChangePreset);
  // MIDI.turnThruOn();
  
  // Stepper Motor Driver Initialistion
  pinMode(DIR, OUTPUT);     
  pinMode(STEP, OUTPUT);
  pinMode(SLP, OUTPUT);
  pinMode(CLEAR_ALL, INPUT);
  digitalWrite(DIR, HIGH);
  digitalWrite(STEP, LOW);
  digitalWrite(SLP, HIGH);

  // Presets initialisation
  loadPresets();
  currentPreset = -1; // placeholder for not yet set.
}

void loop(){
  MIDI.read();
  checkClearMemoryButton();
}

// --------- MIDI FUNCTIONS --------- //
void ChangePreset(byte channel, byte number) {
  //pedal footswitches use channel 0
  //channel is generally unused.
  //number is the patch number being selected
  //SelectPreset((int)number);
  
  for(int i = 0; i < number; i++){
      DriveMotor(180, CW);
  }
  
  
  
  // ----- Debugging
  if(DEBUG){    
    for(int i = 0; i < (int)number; i++){ //flashes main LED the same amount of times as the number of the preset that was just called.
      digitalWrite(LED, HIGH);
      delay(500);
      digitalWrite(LED,LOW);
      delay(500);
    }
  }
  
}

// --------- MOTOR FUNCATIONS ------- //
void DriveMotor(int motor, int angle, int dir){  
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


// --------- Preset Funcations ---------- //
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
  if(pos != 255){
    pos = pos - currentPositions[motor];
    if(pos < 0) DriveMotor(motor, pos, CW);
    else if(pos > 0) DriveMotor(motor, pos, ACW);
    updateCurrentPosition(motor, pos);
  }
  currentPreset = preset;
}

void loadPresets(){
  for(int motor = 0; motor < MAX_MOTORS; motor++){
    for(int preset = 0; preset < MAX_PRESETS + 1; preset++){
      presets[motor][preset] = EEPROM.read(presetMemoryLocation(motor, preset));
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

void clearMotor(int motor){ // remove all presets and reset currentPosition for specific motor
  resetCurrentPostion(motor);
  for(int preset = 0; preset < MAX_PRESETS; preset++){
    EEPROM.write(presetMemoryLocation(motor, preset), 255);
    presets[motor][preset] = 255;
  }
}

void resetCurrentPosition(int motor){
  currentPosition[motor] = NOON;
  EEPROM.write(currentLocationMemoryLocation(motor), NOON);
}

inline int presetMemoryLocation(int motor, int preset){
  return motor * (MAX_PRESETS + 1) + preset + 1;
}

inline int currentLocationMemoryLocation(int motor){
 return motor * (MAX_PRESETS + 1); 
}

