//#include <SD.h> //Load SD card library

//SD Info
const int chipSelect = 4; //chipSelect pin for the SD card Reader
//File SensorData;

// resistance at 25 degrees C
const int THERMISTORNOMINAL[] {10100, 10245, 10285, 10250, 9965};      
// temp. for nominal resistance (almost always 25 C)
#define TEMPERATURENOMINAL 25
// how many samples to take and average, more takes longer
// The beta coefficient of the thermistor (usually 3000-4000)
#define BCOEFFICIENT 3950



// for time delay
long lastTime = 0;

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

void loop() {
  if(abs(millis() - lastTime) > 1000)
  {
  lastTime = millis();
  Serial.print(lastTime); Serial.print(", ");
  float current_temperature_1 = get_temperature(0); //Get current temperature
  Serial.print(current_temperature_1); Serial.print(", ");
  float current_temperature_2 = get_temperature(1); //Get current temperature
  Serial.print(current_temperature_2); Serial.print(", ");
  float current_temperature_3 = get_temperature(2); //Get current temperature
  Serial.print(current_temperature_3); Serial.print(", ");
  float current_temperature_4 = get_temperature(3); //Get current temperature
  Serial.print(current_temperature_4); Serial.print(", ");
  float current_temperature_5 = get_temperature(4); //Get current temperature
  Serial.println(current_temperature_5);

/*
SensorData = SD.open("Ein_Data.txt", FILE_WRITE);//file name is SensorData
if (SensorData) {
  SensorData.print(current_temperature_1);       //write data to card if possible
  SensorData.print(",");
  SensorData.print(current_temperature_2);
  SensorData.print(",");
  SensorData.print(current_temperature_3);
  SensorData.print(",");
  SensorData.print(current_temperature_4);
  SensorData.print(",");
  SensorData.println(current_temperature_5);
  SensorData.close();
}
else {
  Serial.println("Error, failed to write to sd card file");
  
}
*/
}
}

float get_temperature(int platenum) { //Receive temperature measurement
  uint8_t i;
  float average=0; 
  for (i=0; i< NUMSAMPLES; i++) {
    sample[i]= analogRead(THERMISTOR_PIN[platenum]);
    average += sample[i];
    delay(50);
  }
  average /= NUMSAMPLES;
  average = SERIESRESISTOR / (1023 / average - 1);
//  return average;
 
  float steinhart;
  steinhart = log(average / THERMISTORNOMINAL[platenum]) / BCOEFFICIENT; // ln(R/Ro) * 1/B
  steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;                 // Invert
  steinhart -= 273.15;                         // convert to Cs
  return steinhart;
}
