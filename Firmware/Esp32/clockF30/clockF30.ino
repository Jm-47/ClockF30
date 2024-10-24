#include <AccelStepper.h>
#include <WiFi.h>
#include <time.h>


// WiFi credentials
const char* ssid = "iPhone";
const char* password = " nonomomo";

// NTP Server
const char* ntpServer = "europe.pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

const float MAX_SPEED = 1000.0;
const float ACCELERATION = 1000.0;


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

#define MAX_POT 4096
#define MIDDLE_POT 2048
#define MARGIN 256
#define SENSITIVITY 512

#define REVOLUTION 4096

// Potentiometers
#define ARRAY_SIZE 64
int readingIndex = 0;
int hourPotReading[ARRAY_SIZE];
int hourPot = 2048;
int minPotReading[ARRAY_SIZE];
int minPot = 2048;

boolean ignoreHourPot = false;
boolean ignoreMinPot = false;

// Mode
#define CLOCK 1
#define TWO_PAST_SIX 2
#define SPEED 3
int mode = 0;

// hand speed
float hourSpeed = 0;
float minSpeed = 0;

// Buttons
int buttonPin1 = 12;
int buttonPin2 = 13;
int buttonPin3 = 14;
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
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");

  // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  hourStepper.setSpeed(MAX_SPEED);
  hourStepper.setMaxSpeed(MAX_SPEED);
  hourStepper.setAcceleration(ACCELERATION);

  minStepper.setSpeed(MAX_SPEED);
  minStepper.setMaxSpeed(MAX_SPEED);
  minStepper.setAcceleration(ACCELERATION);
}

void loop() {
  // Read potentiometers
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
  if (++readingIndex == ARRAY_SIZE) { readingIndex = 0; }

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

  if (false) { serialBypass(); }

  if (button3 == LOW) {
    if (hourStepper.currentPosition() != 0) {
      Serial.println("Hours current position is now 0");
    }
    if (minStepper.currentPosition() != 0) {
      Serial.println("Minutes current position is now 0");
    }

    hourStepper.setCurrentPosition(0);
    minStepper.setCurrentPosition(0);
    return;
  }

  // -------------------------
  // --- Two past six mode ---
  // -------------------------
  if (button2 == LOW || (mode == TWO_PAST_SIX && ignoreHourPot && ignoreMinPot)) {
    if (mode != TWO_PAST_SIX) {
      Serial.println("06:02");
      mode = TWO_PAST_SIX;
    }

    moveHandToPosition(&hourStepper, REVOLUTION * 6 / 12);
    moveHandToPosition(&minStepper, REVOLUTION * 2 / 60);

    ignoreHourPot = true;
    ignoreMinPot = true;
    return;
  }

  // ------------------
  // --- Clock mode ---
  // ------------------
  if (button1 == LOW || (mode == CLOCK && ignoreHourPot && ignoreMinPot)) {
    if (!clockMode) {
      Serial.println("Clock mode");
      mode = CLOCK;
    }

    struct tm time_info;
    if (!getLocalTime(&time_info)) {
      Serial.println("Failed to obtain time");
      return;
    }

    moveHandToPosition(&hourStepper, REVOLUTION * (time_info.tm_hour % 12) / 12);
    moveHandToPosition(&minStepper, REVOLUTION * time_info.tm_sec / 60);

    ignoreHourPot = true;
    ignoreMinPot = true;
    return;
  }

  // ------------------
  // --- Speed mode ---
  // ------------------
  if (hourPot > MIDDLE_POT - MARGIN && hourPot < MIDDLE_POT + MARGIN) {
    if (hourSpeed != 0) {
      hourSpeed = 0;
      Serial.print("Stop hours at ");
      Serial.print(hourStepper.currentPosition());
      Serial.print(" ");
      Serial.println(hourStepper.currentPosition() * 12 / REVOLUTION);
    }
    hourSpeed = 0;
    ignoreHourPot = false;
  } else if (ignoreHourPot) {
    if (hourSpeed != 0) {
      Serial.print("Ignore hour pot");
      hourSpeed = 0;
    }
  }
  else {
    if (mode != speed) {
      Serial.println("Stop clock mode");
      mode = SPEED;
    }
    float speed = MAX_SPEED * (hourPot - MIDDLE_POT - MARGIN) / (MIDDLE_POT - MARGIN);
    hourStepper.setSpeed(speed);
    if (hourPot > MIDDLE_POT) {
      hourStepper.move(1000);
    }
    else {
      hourStepper.move(-1000);
    }
    hourStepper.runSpeed();
    if (speed != hourSpeed) {
      hourSpeed = speed;
      Serial.print("Run hours hand at ");
      Serial.println(hourSpeed);
    }
  }

  // Control direction and speed
  if (minPot > MIDDLE_POT - MARGIN && minPot < MIDDLE_POT + MARGIN) {
    if (minSpeed != 0) {
      minSpeed = 0;
      Serial.print("Stop minutes at");
      Serial.print(minStepper.currentPosition());
      Serial.print(" ");
      Serial.println(minStepper.currentPosition() * 12 / REVOLUTION);
    }
    minSpeed = 0;
    ignoreMinPot = false;
  } else if (ignoreMinPot) {
    if (minSpeed != 0) {
      minSpeed = 0;
      Serial.print("Ignore min pot");
    }
  } else {
    if (mode != speed) {
      Serial.println("Stop clock mode");
      mode = SPEED;
    }
    float speed = MAX_SPEED * (minPot - MIDDLE_POT - MARGIN) / MIDDLE_POT;
      minStepper.setSpeed(speed);
      if (minPot > MIDDLE_POT) {
        minStepper.move(1000);
      }
      else {
        minStepper.move(-1000);
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

      int currentPosition = stepper->currentPosition() % REVOLUTION;

      if (currentPosition != destination) {

        if (destination - currentPosition > REVOLUTION / 2) {
          stepper->setCurrentPosition(currentPosition + REVOLUTION);
        }
        else if (destination - currentPosition < -REVOLUTION / 2 + 1) {
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

void serialBypass() {
  // serial bypass
  if (Serial.available() > 0) {
    int thisChar = Serial.read();
    // Serial.print("Command ");
    // Serial.println(thisChar);

    switch (thisChar) {
      case 49:  //1
        hourPot = 0;
        break;
      case 50:  //2
        hourPot = 1024;
        break;
      case 51:  //3
        hourPot = 2048;
        break;
      case 52:  //4
        hourPot = 3072;
        break;
      case 53:  //5
        hourPot = 4096;
        break;

      case 54:  //6
        minPot = 0;
        break;
      case 55:  //7
        minPot = 1024;
        break;
      case 56:  //8
        minPot = 2048;
        break;
      case 57:  //9
        minPot = 3072;
        break;
      case 48:  //0
        minPot = 4096;
        break;

      case 113:  //q
        button1 = LOW;
        break;
      case 119:  //w
        button1 = HIGH;
        break;

      case 97:  //a
        button2 = LOW;
        break;
      case 115:  //s
        button2 = HIGH;
        break;

      case 122:  //z
        button3 = LOW;
        break;
      case 120:  //x
        button3 = HIGH;
        break;
    }
  }
}