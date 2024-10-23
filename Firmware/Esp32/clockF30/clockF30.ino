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


int endPoint1 = 4096;  // Move this many steps - 1024 = approx 1/4 turn
int endPoint2 = 4096;  // Move this many steps - 1024 = approx 1/4 turn

// Potentiometers
int hourPotPin = 31;
int minPotPin = 32;
int hourPot = 512;
int minPot = 512;
boolean ignoreHourPot = false;
boolean ignoreMinPot = false;

const int MARGIN = 32;

int hours = 0;
int minutes = 0;

// hand speed
float hourSpeed = 0;
float minSpeed = 0;

// calibration
int hourZero = 0;
int minZero = 0;

// Buttons
int buttonPin1 = 2;
int buttonPin2 = 3;
int buttonPin3 = 4;
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
  // pinMode(buttonPin1, INPUT);
  // pinMode(buttonPin2, INPUT);
  // pinMode(buttonPin3, INPUT);

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


  // Serial.println(hourStepper.currentPosition());
  // Serial.println(minStepper.currentPosition());
  // delay(100);

  hourStepper.setMaxSpeed(MAX_SPEED);
  hourStepper.setAcceleration(ACCELERATION);
  hourStepper.setSpeed(1000);
  hourStepper.moveTo(endPoint1);

  minStepper.setMaxSpeed(MAX_SPEED);
  minStepper.setAcceleration(ACCELERATION);
  minStepper.setSpeed(1000);
  minStepper.moveTo(endPoint2);
}

void loop() {
  // Read potentiometers
  // hourPot = analogRead(hourPotPin);
  // minPot = analogRead(minPotPin);
  // Read buttons
  // button1 = digitalRead(buttonPin1);
  // button2 = digitalRead(buttonPin2);
  // button3 = digitalRead(buttonPin3);

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
        hourPot = 256;
        break;
      case 51:  //3
        hourPot = 512;
        break;
      case 52:  //4
        hourPot = 768;
        break;
      case 53:  //5
        hourPot = 1024;
        break;

      case 54:  //6
        minPot = 0;
        break;
      case 55:  //7
        minPot = 256;
        break;
      case 56:  //8
        minPot = 512;
        break;
      case 57:  //9
        minPot = 768;
        break;
      case 48:  //0
        minPot = 1024;
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



  if (button3 == LOW) {
    if (hourStepper.currentPosition() != 0) {
      Serial.print("Hours current position ");
      Serial.print(hourStepper.currentPosition());
      Serial.print(" ");
      Serial.print(hourStepper.currentPosition() * 12 / 4096);

      Serial.println("  - current position is now 0");
    }
    if (minStepper.currentPosition() != 0) {
      Serial.print("Minutes current position ");
      Serial.print(minStepper.currentPosition());
      Serial.print(" ");
      Serial.print(minStepper.currentPosition() * 60 / 4096);

      Serial.print("  - current position is now 0");
    }

    hourStepper.setCurrentPosition(0);
    minStepper.setCurrentPosition(0);
    return;
  }

  if (button2 == LOW) {
    if (hourStepper.distanceToGo() != 0) {
      Serial.println("Move hours to 6");
      hourStepper.moveTo(4096*6/12);
    }
    if (minStepper.distanceToGo() != 0) {
      Serial.println("Move minutes to 2");
      minStepper.moveTo(4096*2/60);
    }
    // Ignore potentiometers until they get back to 0
    ignoreHourPot = true;
    ignoreMinPot = true;

    hourStepper.run();
    minStepper.run();
    return;
  }

  if (button1 == LOW) {
    struct tm time_info;
    if (!getLocalTime(&time_info)) {
      Serial.println("Failed to obtain time");
      return;
    }

    if (hourStepper.distanceToGo() == 0) {

      hours = time_info.tm_hour % 12;
      int destination = 4096 * hours / 12;
      if (hourStepper.currentPosition() != destination) {
        Serial.print("Move hours to ");
        Serial.print(hours);
        Serial.print(" - ");
        Serial.println(destination);
        hourStepper.moveTo(destination);
        ignoreHourPot = true; // Ignore potentiometer until it is reset
      }
      hourStepper.run();
    }

    if (minStepper.distanceToGo() == 0) {
      minutes = time_info.tm_sec; // FIXME minutes, not seconds
      int destination = 4096 * minutes / 60;
      if (minStepper.currentPosition() != destination) {
        Serial.print("Move minutes to ");
        Serial.print(minutes);
        Serial.print(" - ");
        Serial.println(destination);
        minStepper.moveTo(destination);
        ignoreMinPot = true; // Ignore potentiometer until it is reset
      }
      minStepper.run();
    }

    return;
  }


  // Control direction and speed
  if (hourPot > 512 - MARGIN && hourPot < 512 + MARGIN) {
    if (hourSpeed != 0) {
      hourSpeed = 0;
      Serial.print("Stop hours at ");
      Serial.print(hourStepper.currentPosition());
      Serial.print(" ");
      Serial.println(hourStepper.currentPosition() * 12 / 4096);
    }
    hourSpeed = 0;
    ignoreHourPot = false;
  } else if (!ignoreHourPot) {
    float speed = (hourPot - 512) * MAX_SPEED / 512;
    hourStepper.setSpeed(speed);
    if (hourPot > 512) {
      hourStepper.move(100);
    }
    else {
      hourStepper.move(-100);
    }
    hourStepper.runSpeed();
    if (speed != hourSpeed) {
      hourSpeed = speed;
      Serial.print("Run hours hand at ");
      Serial.println(hourSpeed);
    }
  }

  // Control direction and speed
  if (minPot > 512 - MARGIN && minPot < 512 + MARGIN) {
    if (minSpeed != 0) {
      minSpeed = 0;
      Serial.print("Stop minutes at");
      Serial.print(hourStepper.currentPosition());
      Serial.print(" ");
      Serial.println(hourStepper.currentPosition() * 12 / 4096);
    }
    minSpeed = 0;
    ignoreMinPot = false;
  } else if (!ignoreMinPot) {
    float speed = (minPot - 512) * MAX_SPEED / 512;
      minStepper.setSpeed(speed);
      if (minPot > 512) {
        minStepper.move(100);
      }
      else {
        minStepper.move(-100);
      }
      minStepper.runSpeed();
    if (speed != minSpeed) {
      minSpeed = speed;
      Serial.print("Run minutes hand at ");
      Serial.println(minSpeed);
    }
  }

  // // //Change direction at the limits
  // if (hourStepper.distanceToGo() == 0) {
  //   Serial.println(hourStepper.currentPosition());
  //   hourStepper.setCurrentPosition(0);
  //   endPoint1 = -endPoint1;
  //   hourStepper.moveTo(endPoint1);
  //   Serial.println(hourStepper.currentPosition());
  // }

  // if (minStepper.distanceToGo() == 0) {
  //   Serial.println(hourStepper.currentPosition());
  //   minStepper.setCurrentPosition(0);
  //   endPoint2 = -endPoint2;
  //   minStepper.moveTo(endPoint2);
  //   Serial.println(minStepper.currentPosition());
  // }
  // hourStepper.run();
  // minStepper.run();

  // sleep(1000);
}
