#include <SPI.h>

//#include "pitches.h" // buzzer reference

//7-Seg. Libraries
#include <Wire.h>
#include "Adafruit_GFX.h"
#include "Adafruit_LEDBackpack.h"

// Thermistor Parameters
#define THERMISTORNOMINAL 50000 // resistance at 25 degrees C - FOR ADE PTF PROTOTYPE
//#define THERMISTORNOMINAL 10000 // resistance at 25 degrees C - FOR DTM WIRE PROTOTYPE      
#define TEMPERATURENOMINAL 25   // temp. for nominal resistance (almost always 25 C)
#define NUMREGIONS 5            // how many separately controlled heating regions we have
#define NUMSAMPLES 6            // how many samples to take and average, more takes longer, but is more "smooth"
#define BCOEFFICIENT 3950       // The beta coefficient of the thermistor (usually 3000-4000)
#define SERIESRESISTOR 10000    // the value of the 'other' resistor

// Pin numbers
const int POWER_BUTTON_PIN = 3;     // on/off button pin
const int ALARM_BUTTON_PIN = 2;     // button to 
const int UP_BUTTON_PIN = 1;
const int DOWN_BUTTON_PIN = 0;

const int TCO_BAD = 12; // LED pin for "too hot" alarm
const int COLD_LED = 11; // LED pin for "too cold" alarm
const int buzzPin1 = 10; 
const int buzzPin2 = 13;

int THERMISTOR_PINS[] = {A0, A1, A2, A3, A4};
int CONTROL_PINS[] = {5, 6, 7, 8, 9};

int power_previous = 0; // last read on power button (analog 0 - 1023)
int alarm_previous = 0; //last read on alarm button (analog 0 - 1023)

//7-Seg. Display Variables
Adafruit_7segment matrix = Adafruit_7segment();
boolean sevseg_on = false;

// temp parameters
int setTemp = 34; 
int upTemp = 0;
int downTemp = 0;
int minTemp = 28; 
int maxTemp = 38;

//Temperature reading setup
uint16_t samples[NUMREGIONS][NUMSAMPLES];

float currentTemp[NUMREGIONS]; //temperature measurement
boolean showCurrentTemp = true;
long prevMs = 0; // set up timer
long prevMsSet = 0; // set up timer
long newMs = 0; // set up timer

int dispDelayDefault = 500;
int dispDelaySet = 1000; // milliseconds to allow set temp interactions without displaying current temperature (simulated multi-threading)
int debouncer = 400; // milliseconds to delay code for debouncing

boolean justSet = false;

// Alarm Parameters
boolean soundAlarm = false;
/* // OLD CODE USING PITCHES.H TO CONTROL NOTES
int melody[] = { NOTE_C4, NOTE_C4, NOTE_C4 };
int noteDurations[] = { 4, 4, 4};  // note durations: 4 = quarter note, 8 = eighth note, etc.
*/
//PID Variables
float current_error; //how far form the target temperature we are.
float old_temp; // Parameter for derivative term
int controlSignal; //Sum of Pterm and (future) Dterm

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600); 
  pinMode(TCO_BAD, OUTPUT); //sets LED pins to output
  pinMode(COLD_LED, OUTPUT); 
  pinMode(buzzPin1, OUTPUT); 
  pinMode(buzzPin2, OUTPUT);
  for (int i = 0; i < NUMREGIONS; i ++) {
    pinMode(THERMISTOR_PINS[i], INPUT); 
    pinMode(CONTROL_PINS[i], OUTPUT);
    setPwmFrequency(CONTROL_PINS[i],1024); // Bassinet hums out of hearing range @ 61,250 Hz http://playground.arduino.cc/Code/PwmFrequency
  }
  analogReference(EXTERNAL); 

  //7-Seg. Display
  
  Serial.println("7 Segment Backpack Test");
  
  matrix.begin(0x70); // begins I2C communication with seven segment display
  matrix.setBrightness(3); // sets brightness, on scale of 0 (dim) to 15 (bright)
}

void loop() {
// Power button code
  int power_current = digitalRead(POWER_BUTTON_PIN); // read power button state
  if (power_current == HIGH && power_previous == LOW) { // if power button is pressed
    if (sevseg_on){
      turnOffDisp(); // turn off display if it is on
      sevseg_on = false;
    }
    else{
      displayVersion();
      delay(1000);//Display version number for 1 sec
      currentTempUpdate(); // turn on display if it was off
      sevseg_on = true;
    }
  }
  power_previous = power_current; // refresh button state in memory
  if (sevseg_on) {
    if (showCurrentTemp){
      currentTempUpdate(); // only show current temperature if the display is on and the user is not changing the set temp
      showCurrentTemp = false;
      prevMs = millis();
    }
 
      // 7-Seg. Display Code
    upTemp = digitalRead(UP_BUTTON_PIN); // get signal fom membrane switch (up arrow)
    downTemp = digitalRead(DOWN_BUTTON_PIN); //get signal from membrane switch (down arrow)

    if (upTemp == HIGH && setTemp < maxTemp) {  // condition that membrane switch is high (pressed) and that the desired set temp is less than max
      setTemp = setTemp+1; // increment set temperature by 1
    }
    if (downTemp == HIGH && setTemp > minTemp){ //condition that membrane switch is high (pressed) and that desired set temp is greater than min
      setTemp = setTemp-1; // increment set temperature by -1
    }
    if (upTemp == HIGH || downTemp == HIGH){
      setTempUpdate(); // show temp even when already at min/ max
      showCurrentTemp = false; //pause currentTemp updates
      prevMsSet = millis(); //get time of latest setTemp button press
    }
    newMs = millis();
    if(newMs - prevMsSet > dispDelaySet && justSet){ // check if enough time has passed since latest setTemp button press
      showCurrentTemp = true; // if so, allow current temp display again
      justSet == false;
    }
    if(newMs - prevMs > dispDelayDefault && !justSet) { // this is to manually add delay on default temp display
     showCurrentTemp = true; 
    }

//    PID_loop(); // update heating control system
    bangbang_loop();
    Serial.print("Target: "); Serial.print(setTemp);
    Serial.print("  Current: "); for (int i = 0; i < 5; i ++) Serial.print(currentTemp[i]);
    Serial.print("  Error: "); Serial.print(current_error);
    Serial.print("  Control Sig:  "); Serial.println(controlSignal);
  }
  else {
    null_loop();
  }
  


// Alarm button code -- WILL ALARM EVEN WHEN DISPLAY IS NOT ON
  int alarm_current = digitalRead(ALARM_BUTTON_PIN);  // read alarm button state
  if (alarm_current == HIGH && alarm_previous == LOW) { //if alarm button is pressed anew, min threshold 50 for debouncing (despite pull down) 
    digitalWrite(COLD_LED, !digitalRead(COLD_LED));       //toggled the LED state
    soundAlarm = !soundAlarm; 
  }
  alarm_previous = alarm_current; // refresh button state in memory
  if(soundAlarm){
      diffDriveAlarm(3,5); //NOTE: HOLD DOWN PWR/ ALARM BUTTON TO TURN OFF ALARM WHEN STARTED
      delay(100); // NOTE: Timing is finicky with this buzzer-- do not remove this delay, or the buzzer will stop getting 
  }
}

void setTempUpdate() {
  int setTemp_tens = setTemp/10; //get tens place of setTemp, separate setTemp into two digits to write to seven segment display
  int setTemp_ones = setTemp%10; //get ones place of setTemp
  matrix.writeDigitNum(0, setTemp_tens); // write tens digit
  matrix.writeDigitNum(1, setTemp_ones, false); // write ones digit, clear decimal place
  matrix.writeDigitRaw(3,0); // clear third digit
  matrix.writeDisplay(); // refreshes display with new content
  delay(debouncer);
  return;
}

void currentTempUpdate() {
  get_temperatures();
  float temp = 0; // average the current temperature readings
  for (int i = 0; i < NUMREGIONS; i ++)
    temp += currentTemp[i]/NUMREGIONS;
  int temp_whole = (int)temp;
  int currentTemp_tens = temp_whole/10; // separate currentTemp into three digits, tens place
  int currentTemp_ones = temp_whole%10; // ones place
  int currentTemp_dec = (temp-temp_whole)*10; // tenths place
  matrix.writeDigitNum(0, currentTemp_tens); // write tens place
  matrix.writeDigitNum(1, currentTemp_ones, true); // write ones place, with a decimal following
  matrix.writeDigitNum(3, currentTemp_dec); // write tenths place
  matrix.writeDisplay(); // update display
  return;
}

void get_temperatures() { //Receive temperature measurement
  float sample[NUMREGIONS][NUMSAMPLES];
  for (int i = 0; i < NUMSAMPLES; i ++) {         // store NUMSAMPLES of thermistor readings per region
    for (int j = 0; j < NUMREGIONS; j ++)
      sample[j][i] = analogRead(THERMISTOR_PINS[j]);
    delay(50); // 50 millisecond delay between readings
  }

  for (int j = 0; j < NUMREGIONS; j ++) {
    float average = 0;
    for (int i = 0; i < NUMSAMPLES; i ++)
       average += sample[j][i];  // add all readings
    average /= NUMSAMPLES; // divide to get average
    average = 1023 / average - 1;       // rest of function is copied from the last part of https://learn.adafruit.com/thermistor/using-a-thermistor
    average = SERIESRESISTOR / average;
    Serial.print("Thermistor resistance "); 
    Serial.println(average);
   
    currentTemp[j] = average / THERMISTORNOMINAL;     // (R/Ro)
    currentTemp[j] = log(currentTemp[j]);               // ln(R/Ro)
    currentTemp[j] /= BCOEFFICIENT;                   // 1/B * ln(R/Ro)
    currentTemp[j] += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
    currentTemp[j] = 1.0 / currentTemp[j];              // Invert
    currentTemp[j] -= 273.15;                         // convert to C
    Serial.print("Temp: ");
    Serial.println(currentTemp[j]); 
  }
}


void turnOffDisp(){
    for (int i=0; i<=4; i++){
      matrix.writeDigitRaw(i,0);  // clear each digit
    }
    matrix.writeDisplay(); // update display
    return;
}

//Display firmware version by printing version digits to the LCD panel
void displayVersion(){
  matrix.writeDigitNum(0,0);
  matrix.writeDigitNum(1,0,true);
  matrix.writeDigitNum(3,1);
  matrix.writeDisplay();
}

/*
 * // OLD ALARM CODE WITH MORE CONTROL OF NOTES, IN CASE IT'S NECESSARY
void medAlarm(){ 
    for (int thisNote = 0; thisNote < 3; thisNote++) {
      int noteDuration = 1000 / noteDurations[thisNote];
      tone(BUZZER, melody[thisNote], noteDuration);
      // to distinguish the notes, set a minimum time between them.
      // the note's duration + 30% seems to work well:
      int pauseBetweenNotes = noteDuration * 1.30;
      delay(pauseBetweenNotes);
      // stop the tone playing:
      noTone(BUZZER);
    }
    return;
  }
*/ 
void diffDriveAlarm (int repeats, int duration){
  for (int r=0; r<repeats; r++){
    for (int i=0; i<duration; i++){
      diffDriveTone(2270,1000000); // 440 Hz A for 1 sec
    }
    delay(300);
  }
  return;
}
void diffDriveTone (int period, int duration) {
//   NOTE: Buzzer stops making sounds at durations > 1000000 microseconds. 
//  Pulled from https://forums.adafruit.com/viewtopic.php?f=25&t=29920  
    char phase = 0;
    int i;
    
    for (i=0 ; i < duration ; i+=period) {
        digitalWrite( buzzPin1, phase & 1 ); // bit bang to double volume -- "differential drive"
        phase++;
        digitalWrite( buzzPin2, phase & 1 );
        delayMicroseconds( period );
    }
    return;
}

void bangbang_loop() {
  for (int j = 0; j < NUMREGIONS; j ++) {
    current_error = setTemp - currentTemp[j];
    if (current_error > 0.1) {
     controlSignal = 255;
    } else if (current_error > -0.1) {
     controlSignal = 128;
    } else {
      controlSignal = 0;
    }
    analogWrite(CONTROL_PINS[j], controlSignal); 
  }
}

void PID_loop() {
  for (int j = 0; j < NUMREGIONS; j ++) {
    current_error = setTemp - currentTemp[j]; //calculate error
    controlSignal = round(300*current_error+0*(currentTemp[j]-old_temp)); // P + D control. But the D control is set to 0, becuase it doesn't really do anything yet. It's based on temperature change. Need to avg set of temp values to see more change for Dterm to actually be effective.
    if (controlSignal < 0){ //When control signal becomes negative, set it to zero.
      controlSignal = 0; 
    }
    if (controlSignal > 255) { //When control signal exceeds the maximum value, set it to the maximum value 255.
      controlSignal = 255;
    }
  //  Serial.print("Control: ");
  //  Serial.println(controlSignal);
    analogWrite(CONTROL_PINS[j], controlSignal); //Produce PWM at specified control signal cycle.
  }
}

void null_loop() {
  for (int j = 0; j < NUMREGIONS; j ++) {
    analogWrite(CONTROL_PINS[j], 0);
  }
}

// Frequency Magic --> Controls the frequency of the arduino pin, so that PWM is not audible, from Trong
void setPwmFrequency(int pin, int divisor) {
  byte mode;
  if(pin == 5 || pin == 6 || pin == 9 || pin == 10) {
    switch(divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 64: mode = 0x03; break;
      case 256: mode = 0x04; break;
      case 1024: mode = 0x05; break;
      default: return;
    }
    if(pin == 5 || pin == 6) {
      TCCR0B = TCCR0B & 0b11111000 | mode;
    } else {
      TCCR1B = TCCR1B & 0b11111000 | mode;
    }
  } else if(pin == 3 || pin == 11) {
    switch(divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 32: mode = 0x03; break;
      case 64: mode = 0x04; break;
      case 128: mode = 0x05; break;
      case 256: mode = 0x06; break;
      case 1024: mode = 0x07; break;
      default: return;
    }
    TCCR1B = TCCR1B & 0b11111000 | mode;
  }
}
