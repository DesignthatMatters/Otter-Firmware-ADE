const int MEDIUM_LEDS = 11;
const int HIGH_LEDS = 12;

const int HIGH_INTERVAL = freqToTime(2.0); // 2Hz flashing for high priority
const int MEDIUM_INTERVAL = freqToTime(0.5);  // 0.5Hz flashing for medium priority

int mediumState = 0; // State of the alarms -- should the be flashing
int highState = 1;

int mediumLast = 0; // Last state of light of given alarm
int highLast = 0;

long mediumMillis = 0;
long highMillis = 0;


void setup() {
  pinMode(MEDIUM_LEDS, OUTPUT);
  pinMode(HIGH_LEDS, OUTPUT);
}

void loop() {
  unsigned long currentMillis = millis();
  if (highState) {
    if (toggleState(currentMillis, highMillis, HIGH_INTERVAL)) {
      highMillis = currentMillis;
      highLast = !highLast;
    }
  }
  else if (mediumState) {
    // Only flash medium if high is not active
    highLast = 0;
    if (toggleState(currentMillis, mediumMillis, MEDIUM_INTERVAL)) {
      mediumMillis = currentMillis;
      mediumLast = !mediumLast;
    }
  }
  else {
    highLast = 0;
    mediumLast = 0;
  }
  
  digitalWrite(HIGH_LEDS, highLast);
  digitalWrite(MEDIUM_LEDS, mediumLast);
  
}

boolean toggleState(long current, long last, int interval) {
  // returns true if enough time has expired
  return (current-last > interval);
}

int freqToTime(float hz) {
  float tmp = 1000.0/hz;
  return (int)tmp;
}

