const int sensorPin = 2;
const int controlPin = 3;
const int ledPin = 9;
const int fanRelayPin = 4;

const int potA = 0;
const int potB = 1;

// Definitions for PWM fan control
// this is calculated as 16MHz divided by 8 (prescaler), divided by 25KHz (target PWM frequency from Intel specification)
const byte maxFanSpeed = 80;
byte currentFanDutyCycle = 0;

unsigned long lastRPMCheck = 0;
unsigned int pulseCount = 0;
volatile int fanRPM = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("Serial ok");

  TCCR2A = 0x23;
  
  TCCR2B = 0x09;  // select clock
  OCR2A = 79;  // aiming for 25kHz
  
  //TCCR2B = 0x02;  // select clock
  //OCR2A = 80;  // aiming for 25kHz
  
  // enable the PWM output.
  pinMode(controlPin, OUTPUT);
  setFanSpeed (1);
  
  pinMode(sensorPin, INPUT);
  digitalWrite(sensorPin, HIGH);
  attachInterrupt(0, fanSensorISR, FALLING);
  
  pinMode(ledPin, OUTPUT);
  analogWrite(ledPin, 0);
  
  pinMode(fanRelayPin, OUTPUT);
  digitalWrite(fanRelayPin, LOW);
  
  Serial.println("Config ok");
  delay(500);
  
  digitalWrite(fanRelayPin, HIGH);
}

void loop() {
  setFanSpeed(map(analogRead(potA), 0, 1023, 0, maxFanSpeed));
  setLedPWM(map(analogRead(potB), 0, 1023, 0, 254));
  
  static unsigned int delayedInfo;
  delayedInfo++;
  if (delayedInfo > 1024) {
    printFanInfo();
    delayedInfo = 0;
  }
}

void setLedPWM(byte pwm) {
  analogWrite(ledPin, pwm);
}


// Set the fan speed, where 0 <= fanSpeed <= maxFanSpeed
void setFanSpeed(byte fanSpeed) {
  byte val = min(fanSpeed, maxFanSpeed);
  currentFanDutyCycle = map(val, 0, maxFanSpeed, 0, 100);
  
  // set the PWM duty cycle
  OCR2B = val;
  
  digitalWrite(fanRelayPin, currentFanDutyCycle != 0);
}

void fanSensorISR() {
  pulseCount++;
  
  //If less then one second since last reading, return.
  unsigned long currentTime = millis();
  if (lastRPMCheck + 1000 > currentTime) {
    return;
  }
  
  fanRPM = pulseCount;
  pulseCount = 0;
  lastRPMCheck = currentTime;
  
  //Overflow protect
  if (lastRPMCheck + 1000 < lastRPMCheck) {
    lastRPMCheck = 0;
  }
}

int getFanRPM() {
  return fanRPM;
}

void printFanInfo() {
  Serial.print("Current RPM: ");
  Serial.println(getFanRPM());
  
  Serial.print("Current Duty cycle: ");
  Serial.println(currentFanDutyCycle);
  
  Serial.println("");
}


