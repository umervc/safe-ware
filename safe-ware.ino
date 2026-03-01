#include <Arduino.h>;
#include <Wire.h>
#include <SoftwareSerial.h>
#include <TinyGPSPlus.h>

TinyGPSPlus gps;

//time delay code:
boolean time_count = true;
unsigned long startMillis;
unsigned long currentMillis;
const unsigned long period = 12000; //the value is a number of milliseconds

//IMU Sensor global setup:
const int MPU_addr=0x68; // I2C address of the MPU-6050
int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;
float ax=0, ay=0, az=0, gx=0, gy=0, gz=0;

//int data[STORE_SIZE][5]; //array for saving past data
//byte currentIndex=0; //stores current data array index (0-255)

boolean fall_detected=false;
boolean global_fall_detected =false; //sent so that it is used as a function
boolean trigger1=false; //stores if first trigger (lower threshold) has occurred
boolean trigger2=false; //stores if second trigger (upper threshold) has occurred
boolean trigger3=false; //stores if third trigger (orientation change) has occurred

byte trigger1count=0; //stores the counts past since trigger 1 was set true
byte trigger2count=0; //stores the counts past since trigger 2 was set true
byte trigger3count=0; //stores the counts past since trigger 3 was set true

int angleChange=0;

//Other global setup:

//Button 1 : Blue : I don't need help
int lS1 = LOW; // the previous state from the input pin
int cS1; // the current reading from the input pin

//Button 2 : Red : I need help
int lS2 = LOW; // the previous state from the input pin
int cS2; // the current reading from the input pin

int emergency_mode = 0;
/*If fall is detected, emergency mode is turned ON : 1*/

int button_1_ON = 1;
/*button_1_ON = 1 when "I don't need help button" is turned ON*/

int button_2_ON = 0;
/*button_2_ON = 1 when "I need help" button is turned ON*/

//gps setup
boolean gps_count = true; //number of times gps location sent to app

void setup() {

  delay(250);

  //setup for mpu6050:
  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B); // PWR_MGMT_1 register
  Wire.write(0); // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);

  Serial.begin(115200);

  //setup for GPS module:
  Serial2.begin(115200);
  delay(3000);

  pinMode(12, INPUT_PULLUP); // Button 1 : Blue : I don't need help
  pinMode(27, INPUT_PULLUP); // Button 2 : Red : I need help

  pinMode(25, OUTPUT); // LED1 Green always on
  digitalWrite(25, HIGH);

  pinMode(33, OUTPUT); // LED1 Blue
  digitalWrite(33, HIGH);

  pinMode(32, OUTPUT); // LED1 RED
  digitalWrite(32, LOW);

  pinMode(15, OUTPUT); // buzzer at pin 15
}

// function requiered to read the data sent by the mpu6050:
void mpu_read(){
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr,14,true);

  AcX=Wire.read()<<8|Wire.read();
  AcY=Wire.read()<<8|Wire.read();
  AcZ=Wire.read()<<8|Wire.read();
  Tmp=Wire.read()<<8|Wire.read();
  GyX=Wire.read()<<8|Wire.read();
  GyY=Wire.read()<<8|Wire.read();
  GyZ=Wire.read()<<8|Wire.read();
}

void detectFall(){
  mpu_read();

  ax = (AcX-2050)/16384.00;
  ay = (AcY-77)/16384.00;
  az = (AcZ-1947)/16384.00;

  gx = (GyX+270)/131.07;
  gy = (GyY-351)/131.07;
  gz = (GyZ+136)/131.07;

  float raw_acceleration_mag = pow(pow(ax,2)+pow(ay,2)+pow(az,2),0.5);
  int acceleration_mag = raw_acceleration_mag * 10;

  if (acceleration_mag<=2 && trigger2==false){
    trigger1=true;
    Serial.println("TRIGGER 1 ACTIVATED");
  }

  if (trigger1==true){
    trigger1count++;
    if (acceleration_mag>=12){
      trigger2=true;
      Serial.println("TRIGGER 2 ACTIVATED");
      trigger1=false; trigger1count=0;
    }
  }

  if (trigger2==true){
    trigger2count++;
    angleChange = pow(pow(gx,2)+pow(gy,2)+pow(gz,2),0.5);
    if (angleChange>=30 && angleChange<=400){
      trigger3=true; trigger2=false; trigger2count=0;
      Serial.println("TRIGGER 3 ACTIVATED");
    }
  }

  if(trigger3==true){
    trigger3count++;
    if (trigger3count>=10){
      angleChange = pow(pow(gx,2)+pow(gy,2)+pow(gz,2),0.5);
      if ((angleChange>=0) && (angleChange<=10)){
        fall_detected=true; trigger3=false; trigger3count=0;
      }
      else{
        trigger3=false; trigger3count=0;
      }
    }
  }

  if (trigger2count>=6){
    trigger2=false; trigger2count=0;
  }

  if (trigger1count>=6){
    trigger1=false; trigger1count=0;
  }

  delay(100);
}

//Functions for GPS module:

void updateSerial(){
  delay(500);
  while (Serial.available()) {
    Serial2.write(Serial.read());
  }
  while (Serial2.available()) {
    Serial.write(Serial2.read());
  }
}

void displayInfo()
{
  Serial.print(F("Location: "));
  if (gps.location.isValid()){
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(","));
    Serial.print(gps.location.lng(), 6);
  }
  else
  {
    Serial.print(F("INVALID"));
  }
}

void gpsCode(){
  updateSerial();
  while (Serial2.available() > 0)
    if (gps.encode(Serial2.read()))
      displayInfo();

  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println(F("No GPS detected: check wiring."));
  }
}

//last part of code:

void other_code(){
  detectFall;

  if(fall_detected == true){

    if(time_count == true){
      tone(15, 5000, 3000);
      startMillis = millis();
      time_count = false;
    }

    if(gps_count == true && fall_detected == true && (millis() - startMillis >= period)){
      gpsCode();
      gps_count = false;
      time_count = true;
    }
  }
}

void loop() {

  cS1 = digitalRead(12);
  cS2 = digitalRead(27);

  if (lS1 == HIGH && cS1 == LOW){
    button_1_ON = 1;
    button_2_ON = 0;
    digitalWrite(33, HIGH);
    digitalWrite(32, LOW);
    digitalWrite(15, LOW);
    noTone(15);
    fall_detected = false;
    gps_count = true;
    time_count = true;
  }

  if ((lS2 == HIGH && cS2 == LOW)){
    tone(15, 5000, 3000);
  }

  if ((lS2 == HIGH && cS2 == LOW) || fall_detected == true){
    button_1_ON = 0;
    button_2_ON = 1;
    digitalWrite(33, LOW);
    digitalWrite(32, HIGH);
    digitalWrite(15, LOW);
    fall_detected = true;
  }

  printf("%d,%d \n",button_1_ON , button_2_ON);

  lS1 = cS1;
  lS2 = cS2;

  other_code();
}
