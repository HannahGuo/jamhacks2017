#include <Due.h>
#include <MifareClassic.h>
#include <MifareUltralight.h>
#include <Ndef.h>
#include <NdefMessage.h>
#include <NdefRecord.h>
#include <NfcAdapter.h>
#include <NfcTag.h>
#include <Wire.h>
#include <ADXL345.h>

#define ADXL345_DEVICE 0x00
#define ADXL345_POWER_CTL 0x2D
#define ADXL345_DATAX0 0x32
#define ADXL345_DATAX1 0x33
#define ADXL345_DATAY0 0x34
#define ADXL345_DATAY1 0x35
#define ADXL345_DATAZ0 0x36
#define ADXL345_DATAZ1 0x37
#define ADXL345_ADDRESS  0x53

ADXL345 adxl; //variable adxl is an instance of the ADXL345 library
//PN532_I2C pn532_i2c(Wire);
//NfcAdapter nfc = NfcAdapter(pn532_i2c);
int X_Read, Y_Read, Z_Read;
double ax, ay, az;
boolean hasFallen = false, alerted = false;

void setup() {
  Wire.begin();
  Serial.begin(9600);
  delay(100);
  adxl.powerOn();   //Turning on the ADXL345
  adxl.setActivityX(1);  //look of activity movement on this axes - 1 == on; 0 == off
  adxl.setActivityY(1);
  adxl.setActivityZ(1);
  pinMode(11, OUTPUT);
  pinMode(13, OUTPUT);
}

void loop() {
  X_Read = readRegister(ADXL345_ADDRESS, ADXL345_DATAX0, ADXL345_DATAX1);
  Y_Read = readRegister(ADXL345_ADDRESS, ADXL345_DATAY0, ADXL345_DATAY1);
  Z_Read = readRegister(ADXL345_ADDRESS, ADXL345_DATAZ0, ADXL345_DATAZ1);
  getAcceleration();
//  buzzerToggle();
  if (hasFallen && !alerted) {
    alert();
  }
}

int readRegister(int deviceAddress, int address1, int address2) {
  long int value;
  int readValue1, readValue2;
  Wire.beginTransmission(deviceAddress);
  Wire.write(address1); // register to read
  Wire.write(address2); // register to read
  Wire.endTransmission();
  Wire.requestFrom(deviceAddress, 2); // read two byte
  if (Wire.available() <= 2) {
    readValue1 = Wire.read();
    readValue2 = Wire.read();
  }
  readValue2 = readValue2 << 8;
  value = readValue1 + readValue2;
  delay(100);
  return value;
}

void getAcceleration() {
  double gains = 0.00390625;
  ax = X_Read * gains;
  ay = Y_Read * gains;
  az = Z_Read * gains;

  //Boring accelerometer stuff
  int x, y, z;
  adxl.readXYZ(&x, &y, &z); //read the accelerometer values and store them in variables  x,y,z
  // Output x,y,z values
  Serial.print("values of X , Y , Z: ");
  Serial.print(x);
  Serial.print(" , ");
  Serial.print(y);
  Serial.print(" , ");
  Serial.println(z);
  detectFall(x);
}

void detectFall(double ax) {
  if (ax > 50000 && !hasFallen) {
    hasFallen = true;
    Serial.println("FALL DETECTED AHHHHHHHH!");
  }
}

void alert() {
  while (hasFallen && !alerted) {
    buzzerToggle();
    cancelAlert();
  }
}

void cancelAlert() {
  if (hasFallen) {
    alerted = true;
    reset();
    //&& nfc.tagPresent()
  }
}

void buzzerToggle() {
//  Serial.print("BUZZER HAS BEEN ACTIVATED");
  digitalWrite(11, HIGH);
  digitalWrite(13, HIGH);
  delayMicroseconds(150);
  digitalWrite(11, LOW);
  digitalWrite(13, LOW);
}

void reset() {
  alerted = false;
  hasFallen = true;
  digitalWrite(11, LOW);
  digitalWrite(13, LOW);
}
