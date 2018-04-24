const int MEDIUM_LEDS = 10;
const int HIGH_LEDS = 11;

// change this for brightness
const int BRITE = 255;

const int HIGH_INTERVAL = freqToTime(2.0); // 2Hz flashing for high priority
const int MEDIUM_INTERVAL = freqToTime(0.5);  // 0.5Hz flashing for medium priority

int mediumState = 1; // State of the alarms -- should the be flashing
int highState = 0;

int mediumLast = 0; // Last state of light of given alarm
int highLast = 0;

long mediumMillis = 0;
long highMillis = 0;
int readInt = 1;

void setup() {
  pinMode(MEDIUM_LEDS, OUTPUT);
  pinMode(HIGH_LEDS, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  if (Serial.available() > 0) {    // This is to easily change the state of the medium alarm for testing
    readInt = Serial.parseInt();
    if (readInt == 2) {
      mediumState = 1;
      highState = 0;
    } else if (readInt == 1) {
      mediumState = 0;
      highState = 0;
    } else if (readInt == 3) {
      highState = 1;
      mediumState = 0;
    }
  }

  unsigned long currentMillis = millis();
  if (highState) {
    mediumLast = 0;
    // Use red to make red2
    if (toggleState(currentMillis, highMillis, HIGH_INTERVAL)) {
      highMillis = currentMillis;
      highLast = !highLast;
    }
  }
  else if (mediumState) {
    // Revision: use green and red to make yellow
    if (toggleState(currentMillis, mediumMillis, MEDIUM_INTERVAL)) {
      mediumMillis = currentMillis;
      mediumLast = !mediumLast;
      highLast = mediumLast;
    }
  }
  else {
    highLast = 0;
    mediumLast = 0;
  }
  
  
  analogWrite(HIGH_LEDS, highLast*BRITE);
  analogWrite(MEDIUM_LEDS, mediumLast*BRITE);
  
}

boolean toggleState(long current, long last, int interval) {
  // returns true if enough time has expired
  return (current-last > interval);
}

int freqToTime(float hz) {
  float tmp = 1000.0/hz;
  return (int)tmp;
}

