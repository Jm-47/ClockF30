#include <AccelStepper.h>
#include <WiFi.h>
#include <time.h>


// WiFi credentials FIXME
#define SSID "iPhone"
#define PASSWORD  " nonomomo"

// NTP Server
#define NTP_SERVER "pool.ntp.org"
#define EUROPE_PARIS "CET-1CEST,M3.5.0/02,M10.5.0/03"

#define motorPin11 32  // IN1 on ULN2003 ==> Blue   on 28BYJ-48
#define motorPin12 33  // IN2 on ULN2004 ==> Pink   on 28BYJ-48
#define motorPin13 25  // IN3 on ULN2003 ==> Yellow on 28BYJ-48
#define motorPin14 26  // IN4 on ULN2003 ==> Orange on 28BYJ-48

#define motorPin21 5   // IN1 on ULN2003 ==> Blue   on 28BYJ-48
#define motorPin22 18  // IN2 on ULN2004 ==> Pink   on 28BYJ-48
#define motorPin23 19  // IN3 on ULN2003 ==> Yellow on 28BYJ-48
#define motorPin24 21  // IN4 on ULN2003 ==> Orange on 28BYJ-48

#define hourPotPin 34
#define minPotPin 35

#define buttonPin1 12
#define buttonPin2 13
#define buttonPin3 14

#define HOUR_DIRECTION -1
#define MIN_DIRECTION -1

#define MAX_POT 4096
#define MIDDLE_POT 2048
#define SENSITIVITY 256

#define REVOLUTION 4096

#define MAX_SPEED 1500.0
#define ACCELERATION 1000.0


// Potentiometers
#define ARRAY_SIZE 8
int readingIndex = 0;
int hourPotReading[ARRAY_SIZE];
int hourPot = 0;
int minPotReading[ARRAY_SIZE];
int minPot = 0;

unsigned long lastReadingMillis;
#define READING_INTERVAL 20

boolean ignoreHourPot = false;
boolean ignoreMinPot = false;

// Mode
#define CALIBRATION 0
#define CLOCK 1
#define TWO_PAST_SIX 2
#define SPEED 3
int mode = 0;

// hand speed
float hourSpeed = 0;
float minSpeed = 0;

// Buttons
int button1 = HIGH;
int button2 = HIGH;
int button3 = HIGH;

// NOTE: The sequence 1-3-2-4 is required for proper sequencing of 28BYJ-48
AccelStepper hourStepper(AccelStepper::HALF4WIRE, motorPin11, motorPin13, motorPin12, motorPin14);
AccelStepper minStepper(AccelStepper::HALF4WIRE, motorPin21, motorPin23, motorPin22, motorPin24);

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB port only
  }

  // // init buttons
  pinMode(buttonPin1, INPUT_PULLUP);
  pinMode(buttonPin2, INPUT_PULLUP);
  pinMode(buttonPin3, INPUT_PULLUP);
  pinMode(hourPotPin, INPUT);
  pinMode(minPotPin, INPUT);

  // Connect to WiFi
  Serial.println("Connecting to WiFi");
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");

  configTzTime(EUROPE_PARIS, NTP_SERVER);


  struct tm time_info;
  if (!getLocalTime(&time_info)) {
    Serial.println("Failed to obtain time");
  }
  Serial.println(&time_info, "%A, %B %d %Y %H:%M:%S zone %Z %z ");

  hourStepper.setSpeed(MAX_SPEED);
  hourStepper.setMaxSpeed(MAX_SPEED);
  hourStepper.setAcceleration(ACCELERATION);

  minStepper.setSpeed(MAX_SPEED);
  minStepper.setMaxSpeed(MAX_SPEED);
  minStepper.setAcceleration(ACCELERATION);

  ignoreHourPot = true;
  ignoreMinPot = true;
}

void loop() {
  // Read potentiometers
  unsigned long currentMillis = millis();
  if (currentMillis - lastReadingMillis >= READING_INTERVAL) {
    hourPotReading[readingIndex] = analogRead(hourPotPin);
    int newHourPot = mean(hourPotReading) / SENSITIVITY * SENSITIVITY;
    if (newHourPot != hourPot) {
      hourPot = newHourPot;
      Serial.print("HourPot ");
      Serial.println(hourPot);
    }

    minPotReading[readingIndex] = analogRead(minPotPin);
    int newMinPot = mean(minPotReading) / SENSITIVITY * SENSITIVITY;
    if (newMinPot != minPot) {
      minPot = newMinPot;
      Serial.print("MinPot ");
      Serial.println(minPot);
    }

    lastReadingMillis = currentMillis;
    if (++readingIndex == ARRAY_SIZE) { readingIndex = 0; }
  }

  // Read buttons
  int status = digitalRead(buttonPin1);
  if (button1 == HIGH && status == LOW) { Serial.println("Button 1 is low"); }
  if (button1 == LOW && status == HIGH) { Serial.println("Button 1 is high"); }
  button1 = status;

  status = digitalRead(buttonPin2);
  if (button2 == HIGH && status == LOW) { Serial.println("Button 2 is low"); }
  if (button2 == LOW && status == HIGH) { Serial.println("Button 2 is high"); }
  button2 = status;

  status = digitalRead(buttonPin3);
  if (button3 == HIGH && status == LOW) { Serial.println("Button 3 is low"); }
  if (button3 == LOW && status == HIGH) { Serial.println("Button 3 is high"); }
  button3 = status;

  if (button1 == LOW && button3 == HIGH && mode != CLOCK) {
    Serial.println("Clock mode");
    mode = CLOCK;
    ignoreHourPot = true;
    ignoreMinPot = true;
  }
  else if (button2 == LOW && button3 == HIGH && mode != TWO_PAST_SIX) {
    Serial.println("06:02");
    mode = TWO_PAST_SIX;
    ignoreHourPot = true;
    ignoreMinPot = true;
  }
  else if (button3 == LOW && (button1 == LOW ||button2 == LOW) && mode != CALIBRATION) {
    Serial.println("Calibration");
    mode = CALIBRATION;
    ignoreHourPot = true;
    ignoreMinPot = true;
  }

  // ------------------
  // --- Clock mode ---
  // ------------------
  if (mode == CLOCK) {
    struct tm time_info;
    if (!getLocalTime(&time_info)) {
      Serial.println("Failed to obtain time");
    }
    else {
      moveHandToPosition(&hourStepper, HOUR_DIRECTION * ((REVOLUTION * (time_info.tm_hour % 12)) / 12 +
                                                         (REVOLUTION * time_info.tm_min) / 720));
      // moveHandToPosition(&minStepper, MIN_DIRECTION * REVOLUTION * time_info.tm_sec / 60); // seconds
      moveHandToPosition(&minStepper, MIN_DIRECTION * ((REVOLUTION * time_info.tm_min) / 60 +
                                                       (REVOLUTION * time_info.tm_sec) / 3600)); // FIXME
    }
  }

  // -------------------------
  // --- Two past six mode ---
  // -------------------------
  if (mode == TWO_PAST_SIX) {
    moveHandToPosition(&hourStepper, HOUR_DIRECTION * REVOLUTION * 6 / 12);
    moveHandToPosition(&minStepper, MIN_DIRECTION * REVOLUTION * 2 / 60);
  }

  // ------------------------
  // --- Calibration mode ---
  // ------------------------
  if (mode == CALIBRATION) {
    hourStepper.setCurrentPosition(0);
    minStepper.setCurrentPosition(0);
  }

  // ------------------
  // --- Speed mode ---
  // ------------------
  if (hourPot == MIDDLE_POT) {
    if (hourSpeed != 0) {
      Serial.println("Hours pot reset");
      hourSpeed = 0;
    }
    ignoreHourPot = false;
  } else if (ignoreHourPot) {
    // Ignore
  } else {
    if (mode != SPEED) {
      Serial.println("Speed mode");
      mode = SPEED;
    }
    float speed = MAX_SPEED * (hourPot - MIDDLE_POT) / MIDDLE_POT;
    hourStepper.setSpeed(speed);
    if (hourPot > MIDDLE_POT) {
      hourStepper.move(100 * HOUR_DIRECTION);
    }
    else {
      hourStepper.move(-100 * HOUR_DIRECTION);
    }
    hourStepper.runSpeed();
    if (speed != hourSpeed) {
      hourSpeed = speed;
      Serial.print("Run hours hand at ");
      Serial.println(hourSpeed);
    }
  }

  // Control direction and speed
  if (minPot == MIDDLE_POT) {
    if (minSpeed != 0) {
      Serial.println("Minutes pot reset");
      minSpeed = 0;
    }
    ignoreMinPot = false;
  } else if (ignoreMinPot) {
    // Ignore
  } else {
    if (mode != SPEED) {
      Serial.println("Speed mode");
      mode = SPEED;
    }
    float speed = MAX_SPEED * (minPot - MIDDLE_POT) / MIDDLE_POT;
      minStepper.setSpeed(speed);
      if (minPot > MIDDLE_POT) {
        minStepper.move(100 * MIN_DIRECTION);
      }
      else {
        minStepper.move(-100 * MIN_DIRECTION);
      }
      minStepper.runSpeed();
    if (speed != minSpeed) {
      minSpeed = speed;
      Serial.print("Run minutes hand at ");
      Serial.println(minSpeed);
    }
  }
}

void moveHandToPosition(AccelStepper *stepper, int destination) {
    if (stepper->distanceToGo() == 0) {

      long currentPosition = stepper->currentPosition();

      currentPosition = currentPosition % REVOLUTION;
      stepper->setCurrentPosition(currentPosition);

      if (currentPosition != destination) {
        Serial.print("Current position ");
        Serial.print(currentPosition);
        Serial.print(" move to  ");
        Serial.println(destination);

        if (destination - currentPosition > REVOLUTION / 2) {
          stepper->setCurrentPosition(currentPosition + REVOLUTION);
        } else if (destination - currentPosition < -REVOLUTION / 2 + 1) {
          stepper->setCurrentPosition(currentPosition - REVOLUTION);
        }
        stepper->moveTo(destination);
      }
    }
    stepper->run();
}

int mean(int array[]) {
  int sum = 0;
  for (int i = 0; i < ARRAY_SIZE; i++) { sum = sum + array[i]; }
  return sum / ARRAY_SIZE;
}
