//#include <SD.h> //Load SD card library

//SD Info
const int chipSelect = 4; //chipSelect pin for the SD card Reader
//File SensorData;

// resistance at 25 degrees C of each thermistor
const int THERMISTORNOMINAL[] {10240, 10180, 9960, 9850, 9890};      
// temp. for nominal resistance (almost always 25 C)
#define TEMPERATURENOMINAL 25   
// how many samples to take and average, more takes longer
// The beta coefficient of the thermistor (usually 3000-4000)
#define BCOEFFICIENT 3950

// for calculating elapsedTime
long startTime = millis();

// which analog pin to connect
const int THERMISTOR_PIN[] {A0, A1, A2, A3, A4};         
// how many samples to take and average, more takes longer
// but is more 'smooth'
#define NUMSAMPLES 5
// the value of the 'other' resistor
#define SERIESRESISTOR 15000    
 
int sample[NUMSAMPLES];

void setup() {
  Serial.begin(9600);
  // connect AREF to 3.3V and use that as VCC, less noisy!
  analogReference(EXTERNAL);

  Serial.begin(9600);//sets the baud rate
 // if (!SD.begin(chipSelect)) {        //Checking the SD Card
  //Serial.println("Error, card failed to begin"); 
//}
}

//print temperature values
void loop() {
  long currentTime = millis();
  String elapsedTime = String((currentTime - startTime));
  
  String current_temperature_1 = String(get_temperature(0)); //Get current temperature
  String current_temperature_2 = String(get_temperature(1)); //Get current temperature
  String current_temperature_3 = String(get_temperature(2)); //Get current temperature
  String current_temperature_4 = String(get_temperature(3)); //Get current temperature
  String current_temperature_5 = String(get_temperature(4)); //Get current temperature
  String printString = elapsedTime + ", " + current_temperature_1 + ", " + current_temperature_2 + ", " + current_temperature_3 + ", " + current_temperature_4 + ", " + current_temperature_5;
  Serial.println(printString);
}

float get_temperature(int platenum) { //Receive temperature measurement
  uint8_t i;
  float average=0; 
  for (i=0; i< NUMSAMPLES; i++) {
    sample[i]= analogRead(THERMISTOR_PIN[platenum]);
    delay(5);
  }
  for (i=0; i< NUMSAMPLES; i++) {
     average += sample[i];
  }
  average /= NUMSAMPLES;
  average = 1023 / average - 1;
  average = SERIESRESISTOR / average;
  //Serial.print("Thermistor resistance "); 
  //Serial.println(average);
 
  float steinhart;
  steinhart = average / THERMISTORNOMINAL[platenum];     // (R/Ro)
  steinhart = log(steinhart);                  // ln(R/Ro)
  steinhart /= BCOEFFICIENT;                   // 1/B * ln(R/Ro)
  steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;                 // Invert
  steinhart -= 273.15;                         // convert to Cs
  return steinhart;
}
