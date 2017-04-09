#include <Due.h>
#include <MifareClassic.h>
#include <MifareUltralight.h>
#include <Ndef.h>
#include <NdefMessage.h>
#include <NdefRecord.h>
#include <NfcAdapter.h>
#include <NfcTag.h>
#include <Wire.h>

#define ADXL345_DEVICE 0x00
#define ADXL345_POWER_CTL 0x2D
#define ADXL345_DATAX0 0x32
#define ADXL345_DATAX1 0x33
#define ADXL345_DATAY0 0x34
#define ADXL345_DATAY1 0x35
#define ADXL345_DATAZ0 0x36
#define ADXL345_DATAZ1 0x37
#define ADXL345_ADDRESS  0x53

//PN532_I2C pn532_i2c(Wire);
//NfcAdapter nfc = NfcAdapter(pn532_i2c);
int X_Read, Y_Read, Z_Read;
double ax, ay, az;
double start = 0;
boolean hasFallen = false, alerted = false;

void setup() {
  Wire.begin();
  Serial.begin(19200);
  delay(100);
  //Turning on the ADXL345
  Wire.beginTransmission(ADXL345_DEVICE); // start transmission to device
  Wire.write(ADXL345_POWER_CTL);
  Wire.write(8);                          //measuring enable
  Wire.endTransmission();                 // end transmission
}

void loop() {
  X_Read = readRegister(ADXL345_ADDRESS, ADXL345_DATAX0, ADXL345_DATAX1);
  Y_Read = readRegister(ADXL345_ADDRESS, ADXL345_DATAY0, ADXL345_DATAY1);
  Z_Read = readRegister(ADXL345_ADDRESS, ADXL345_DATAZ0, ADXL345_DATAZ1);  
  getAcceleration();
  Serial.print("x: " + ax);
  Serial.print("y: " + ay);
  Serial.print("z: " + az);
  detectFall();
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
  //Wire.endTransmission();
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
}

void detectFall() {
  double prev = start;
  double current = 0;
  if (current - prev < 1 && current == 0) {
    hasFallen = true;
  }
}

void alert() {
  buzzerToggle();
  cancelAlert();
}

void cancelAlert() {
  if (hasFallen) {
    alerted = true;
    reset();
    //&& nfc.tagPresent()
  }
}

void buzzerToggle() {
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

