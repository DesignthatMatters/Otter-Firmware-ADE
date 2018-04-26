// This code only include the functional heating system of Otter (PID control)
// Created at MTTS on Jan. 11th 2018
// Author: Kelly Brennan

// Thermistor Parameters
#define THERMISTORNOMINAL 50000 // resistance at 25 degrees C      
#define TEMPERATURENOMINAL 25   // temp. for nominal resistance (almost always 25 C)
#define NUMSAMPLES 5            // how many samples to take and average, more takes longer, but is more "smooth"
#define BCOEFFICIENT 3950       // The beta coefficient of the thermistor (usually 3000-4000)
#define SERIESRESISTOR 10000    // the value of the 'other' resistor

// temp parameter
int setTemp = 41;

//Temperature reading setup
int THERMISTOR_PIN = A6;
uint16_t sample[NUMSAMPLES];

float currentTemp; //temperature measurement
boolean showCurrentTemp = true;
//long prevMs = 0; // set up timer

//PID Variablest
float current_error; //how far form the target temperature we are.
float old_temp; // Parameter for derivative term
int controlSignal; //Sum of Pterm and (future) Dterm

//PWM setup
int bassinetPin = 9;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(THERMISTOR_PIN, INPUT);
  pinMode(bassinetPin, OUTPUT);
  //analogReference(EXTERNAL);
  setPwmFrequency(bassinetPin, 1024); // Bassinet hums out of hearing range @ 61,250 Hz http://playground.arduino.cc/Code/PwmFrequency
}

void loop() {
  currentTemp = get_temperature();
  current_error = setTemp - currentTemp; //calculate error
  controlSignal = round(150 * current_error + 1.5 * (currentTemp - old_temp)); // P + D control. But the D control is set to 0, becuase it doesn't really do anything yet. It's based on temperature change. Need to avg set of temp values to see more change for Dterm to actually be effective.
  if (controlSignal < 0) { //When control signal becomes negative, set it to zero.
    controlSignal = 0;
  }
  if (controlSignal > 255) { //When control signal exceeds the maximum value, set it to the maximum value 255.
    controlSignal = 255;
  }
  Serial.print("Control: ");
  Serial.println(controlSignal);
  analogWrite(bassinetPin, controlSignal); //Produce PWM at specified control signal cycle.
}

float get_temperature() { //Receive temperature measurement
  uint8_t i;
  float average = 0;
  for (i = 0; i < NUMSAMPLES; i++) {      // store NUMSAMPLES of thermistor readings
    sample[i] = analogRead(THERMISTOR_PIN);
    delay(50); // 50 millisecond delay between readings
  }
  for (i = 0; i < NUMSAMPLES; i++) {
    average += sample[i];  // add all readings
  }
  average /= NUMSAMPLES; // divide to get average
  average = 1023 / average - 1;       // rest of function is copied from the last part of https://learn.adafruit.com/thermistor/using-a-thermistor
  average = SERIESRESISTOR / average;
  Serial.print("Thermistor resistance ");
  Serial.println(average);

  float steinhart;
  steinhart = average / THERMISTORNOMINAL;     // (R/Ro)
  steinhart = log(steinhart);                  // ln(R/Ro)
  steinhart /= BCOEFFICIENT;                   // 1/B * ln(R/Ro)M
  steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;                 // Invert
  steinhart -= 273.15;                         // convert to C
  Serial.print("Temp: ");
  Serial.println(steinhart);
  return steinhart;
}


// Frequency Magic --> Controls the frequency of the arduino pin, so that PWM is not audible, from Trong
void setPwmFrequency(int pin, int divisor) {
  byte mode;
  if (pin == 5 || pin == 6 || pin == 9 || pin == 10) {
    switch (divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 64: mode = 0x03; break;
      case 256: mode = 0x04; break;
      case 1024: mode = 0x05; break;
      default: return;
    }
    if (pin == 5 || pin == 6) {
      TCCR0B = TCCR0B & 0b11111000 | mode;
    } else {
      TCCR1B = TCCR1B & 0b11111000 | mode;
    }
  } else if (pin == 3 || pin == 11) {
    switch (divisor) {
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

