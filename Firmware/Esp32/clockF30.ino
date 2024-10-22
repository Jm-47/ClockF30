#include <AccelStepper.h>

#define HALFSTEP 8

#define motorPin11  32     // IN1 on ULN2003 ==> Blue   on 28BYJ-48
#define motorPin12  33     // IN2 on ULN2004 ==> Pink   on 28BYJ-48
#define motorPin13  25    // IN3 on ULN2003 ==> Yellow on 28BYJ-48
#define motorPin14  26    // IN4 on ULN2003 ==> Orange on 28BYJ-48

#define motorPin21  5     // IN1 on ULN2003 ==> Blue   on 28BYJ-48
#define motorPin22  18     // IN2 on ULN2004 ==> Pink   on 28BYJ-48
#define motorPin23  19    // IN3 on ULN2003 ==> Yellow on 28BYJ-48
#define motorPin24  21    // IN4 on ULN2003 ==> Orange on 28BYJ-48


int endPoint1 = 4096;        // Move this many steps - 1024 = approx 1/4 turn
int endPoint2 = 4096;        // Move this many steps - 1024 = approx 1/4 turn

// NOTE: The sequence 1-3-2-4 is required for proper sequencing of 28BYJ-48
AccelStepper stepper1(HALFSTEP, motorPin11, motorPin13, motorPin12, motorPin14);

AccelStepper stepper2(HALFSTEP, motorPin21, motorPin23, motorPin22, motorPin24);

void setup()
{
  Serial.begin(9600);
  Serial.println(stepper1.currentPosition());
  Serial.println(stepper2.currentPosition());
  delay(100);
  stepper1.setMaxSpeed(3000.0);
  stepper1.setAcceleration(1000.0);
  stepper1.setSpeed(2000);
  stepper1.moveTo(endPoint1);

  stepper2.setMaxSpeed(3000.0);
  stepper2.setAcceleration(1000.0);
  stepper2.setSpeed(2000);
  stepper2.moveTo(endPoint2);
}

void loop()
{
  //Change direction at the limits
    if (stepper1.distanceToGo() == 0)
   {
     Serial.println(stepper1.currentPosition());
     stepper1.setCurrentPosition(0);
     endPoint1 = -endPoint1;
     stepper1.moveTo(endPoint1);
     Serial.println(stepper1.currentPosition());
   }
    if (stepper2.distanceToGo() == 0)
   {
     Serial.println(stepper1.currentPosition());
     stepper2.setCurrentPosition(0);
     endPoint2 = -endPoint2;
     stepper2.moveTo(endPoint2);
     Serial.println(stepper2.currentPosition());
   }
    stepper1.run();
    stepper2.run();
}

