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

const float MAX_SPEED = 3000.0;
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

int hourPosition = 0;
int minPosition = 0;

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
AccelStepper stepper1(AccelStepper::HALF4WIRE, motorPin11, motorPin13, motorPin12, motorPin14);
AccelStepper stepper2(AccelStepper::HALF4WIRE, motorPin21, motorPin23, motorPin22, motorPin24);

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


  // Serial.println(stepper1.currentPosition());
  // Serial.println(stepper2.currentPosition());
  // delay(100);

  // stepper1.setMaxSpeed(MAX_SPEED);
  // stepper1.setAcceleration(ACCELERATION);
  // stepper1.setSpeed(2000);
  // stepper1.moveTo(endPoint1);

  // stepper2.setMaxSpeed(MAX_SPEED);
  // stepper2.setAcceleration(ACCELERATION);
  // stepper2.setSpeed(2000);
  // stepper2.moveTo(endPoint2);
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





  if (button2 == LOW) {
    if (hourPosition != 6) {
      Serial.println("Move hours to 6");
      hourPosition = 6;
    }
    if (minPosition != 2) {
      Serial.println("Move minutes to 2");
      minPosition = 2;
    }

    // stepper1.setMaxSpeed(MAX_SPEED);
    // stepper1.setAcceleration(ACCELERATION);
    // stepper1.setSpeed(2000);
    // stepper1.moveTo(endPoint1);

    // Ignore potentiometers until they get back to 0
    ignoreHourPot = true;
    ignoreMinPot = true;
  }

  if (button1 == LOW) {
    struct tm time_info;
    if (!getLocalTime(&time_info)) {
      Serial.println("Failed to obtain time");
      return;
    }

    if (hourPosition != time_info.tm_hour % 12) {
      Serial.print("Move hours to ");
      Serial.println(time_info.tm_hour % 12);
      hourPosition = time_info.tm_hour % 12;
    }

    if (minPosition != time_info.tm_min) {
      Serial.print("Move minutes to ");
      Serial.println(time_info.tm_min);
      minPosition = time_info.tm_min;
    }

    // Ignore potentiometers until they get back to 0
    ignoreHourPot = true;
    ignoreMinPot = true;
  }


  // Control direction and speed
  if (hourPot > 512 - MARGIN && hourPot < 512 + MARGIN) {
    if (hourSpeed != 0) {
      hourSpeed = 0;
      Serial.print("Stop hours hand");
    }
    hourSpeed = 0;
    ignoreHourPot = false;
  } else if (!ignoreHourPot) {
    float speed = (hourPot - 512) * MAX_SPEED / 512;
    //   // setMaxSpeed((512 - hourPot) * MAX_SPEED / 512);
    //   // move(-100);
    //   // runSpeed();
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
      Serial.print("Stop minutes hand");
    }
    minSpeed = 0;
    ignoreMinPot = false;
  } else if (!ignoreMinPot) {
    float speed = (minPot - 512) * MAX_SPEED / 512;
    //   // setMaxSpeed((512 - hourPot) * MAX_SPEED / 512);
    //   // move(-100);
    //   // runSpeed();
    if (speed != minSpeed) {
      minSpeed = speed;
      Serial.print("Run minutes hand at ");
      Serial.println(minSpeed);
    }
  }

  // //Change direction at the limits
  // if (stepper1.distanceToGo() == 0) {
  //   Serial.println(stepper1.currentPosition());
  //   stepper1.setCurrentPosition(0);
  //   endPoint1 = -endPoint1;
  //   stepper1.moveTo(endPoint1);
  //   Serial.println(stepper1.currentPosition());
  // }

  // if (stepper2.distanceToGo() == 0) {
  //   Serial.println(stepper1.currentPosition());
  //   stepper2.setCurrentPosition(0);
  //   endPoint2 = -endPoint2;
  //   stepper2.moveTo(endPoint2);
  //   Serial.println(stepper2.currentPosition());
  // }
  // stepper1.run();
  // stepper2.run();

  // sleep(1000);
}
