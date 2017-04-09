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
double start = 0;
boolean hasFallen = false, alerted = false;

void setup() {
  Wire.begin();
  Serial.begin(9600);
  delay(100);
  //Turning on the ADXL345
  adxl.powerOn();

  //set activity/ inactivity thresholds (0-255)
  adxl.setActivityThreshold(75); //62.5mg per increment
  adxl.setInactivityThreshold(75); //62.5mg per increment
  adxl.setTimeInactivity(10); // how many seconds of no activity is inactive?
 
  //look of activity movement on this axes - 1 == on; 0 == off 
  adxl.setActivityX(1);
  adxl.setActivityY(1);
  adxl.setActivityZ(1);
 
  //look of inactivity movement on this axes - 1 == on; 0 == off
  adxl.setInactivityX(1);
  adxl.setInactivityY(1);
  adxl.setInactivityZ(1);
 
  //look of tap movement on this axes - 1 == on; 0 == off
  adxl.setTapDetectionOnX(0);
  adxl.setTapDetectionOnY(0);
  adxl.setTapDetectionOnZ(1);
 
  //set values for what is a tap, and what is a double tap (0-255)
  adxl.setTapThreshold(50); //62.5mg per increment
  adxl.setTapDuration(15); //625us per increment
  adxl.setDoubleTapLatency(80); //1.25ms per increment
  adxl.setDoubleTapWindow(200); //1.25ms per increment
 
  //set values for what is considered freefall (0-255)
  adxl.setFreeFallThreshold(7); //(5 - 9) recommended - 62.5mg per increment
  adxl.setFreeFallDuration(45); //(20 - 70) recommended - 5ms per increment
 
  //setting all interrupts to take place on int pin 1
  //I had issues with int pin 2, was unable to reset it
  adxl.setInterruptMapping( ADXL345_INT_SINGLE_TAP_BIT,   ADXL345_INT1_PIN );
  adxl.setInterruptMapping( ADXL345_INT_DOUBLE_TAP_BIT,   ADXL345_INT1_PIN );
  adxl.setInterruptMapping( ADXL345_INT_FREE_FALL_BIT,    ADXL345_INT1_PIN );
  adxl.setInterruptMapping( ADXL345_INT_ACTIVITY_BIT,     ADXL345_INT1_PIN );
  adxl.setInterruptMapping( ADXL345_INT_INACTIVITY_BIT,   ADXL345_INT1_PIN );
 
  //register interrupt actions - 1 == on; 0 == off  
  adxl.setInterrupt( ADXL345_INT_SINGLE_TAP_BIT, 1);
  adxl.setInterrupt( ADXL345_INT_DOUBLE_TAP_BIT, 1);
  adxl.setInterrupt( ADXL345_INT_FREE_FALL_BIT,  1);
  adxl.setInterrupt( ADXL345_INT_ACTIVITY_BIT,   1);
  adxl.setInterrupt( ADXL345_INT_INACTIVITY_BIT, 1);
}

void loop() {
  X_Read = readRegister(ADXL345_ADDRESS, ADXL345_DATAX0, ADXL345_DATAX1);
  Y_Read = readRegister(ADXL345_ADDRESS, ADXL345_DATAY0, ADXL345_DATAY1);
  Z_Read = readRegister(ADXL345_ADDRESS, ADXL345_DATAZ0, ADXL345_DATAZ1);  
  getAcceleration();
  detectFall();
  if (hasFallen && !alerted) {
    alert();
  }
  buzzerToggle();
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

   //Boring accelerometer stuff   
  int x,y,z;  
  adxl.readXYZ(&x, &y, &z); //read the accelerometer values and store them in variables  x,y,z
  // Output x,y,z values 
  Serial.print("values of X , Y , Z: ");
  Serial.print(x);
  Serial.print(" , ");
  Serial.print(y);
  Serial.print(" , ");
  Serial.println(z);
  
  double xyz[3];
  double ax,ay,az;
  adxl.getAcceleration(xyz);
  ax = xyz[0];
  ay = xyz[1];
  az = xyz[2];
  Serial.print("X=");
  Serial.print(ax);
    Serial.println(" g");
  Serial.print("Y=");
  Serial.print(ay);
    Serial.println(" g");
  Serial.print("Z=");
  Serial.println(az);
    Serial.println(" g");
  Serial.println("**********************");
  delay(500);

   byte interrupts = adxl.getInterruptSource();
   if(adxl.triggered(interrupts, ADXL345_FREE_FALL)){
    Serial.println("freefall");
    //add code here to do when freefall is sensed
  } 
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

/*****************************************************************************/
//  Function:    Get the accelemeter of X/Y/Z axis and print out on the 
//          serial monitor.
//  Hardware:    3-Axis Digital Accelerometer(��16g)
//  Arduino IDE: Arduino-1.0
//  Author:  Frankie.Chu    
//  Date:    Jan 11,2013
//  Version: v1.0
//  by www.seeedstudio.com
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//
/*******************************************************************************/
