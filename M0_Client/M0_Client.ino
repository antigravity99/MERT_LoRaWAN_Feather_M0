// rf95_reliable_datagram_client.pde
// -*- mode: C++ -*-
// Example sketch showing how to create a simple addressed, reliable messaging client
// with the RHReliableDatagram class, using the RH_RF95 driver to control a RF95 radio.
// It is designed to work with the other example rf95_reliable_datagram_server
// Tested with Anarduino MiniWirelessLoRa, Rocket Scream Mini Ultra Pro with the RFM95W 

#include <RHReliableDatagram.h>
#include <Arduino.h>
#include <Wire.h>
#include "Adafruit_TMP007.h"
#include <SPI.h>
#include <RH_RF95.h>

Adafruit_TMP007 tmp007;

#define CLIENT_ADDRESS 1
#define SERVER_ADDRESS 2
#define FREQ 915.0

// Singleton instance of the radio driver
RH_RF95 driver;

// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram manager(driver, CLIENT_ADDRESS);

void setup() 
{
  Serial.begin(9600);
  //while (!Serial) ; // Wait for serial port to be available
  if (!manager.init())
  {
    Serial.println("init failed");
  }
  else
  {
    if (!driver.setFrequency(FREQ)) 
    {
      Serial.println("setFrequency failed");
      while (1);
    }
    Serial.print("Set Freq to: "); Serial.println(FREQ);
    driver.setTxPower(23, false);
  }
}

// Dont put this on the stack:
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];

void loop()
{
  String tempStr = String(tmp007.readDieTempC());
  char data[sizeof(tempStr)];
  tempStr.toCharArray(data, sizeof(data));
  
  Serial.print("Object Temp#:      "); Serial.print(data); Serial.println("*C");
  
  Serial.println("Sending to rf95_reliable_datagram_server");
    
  // Send a message to manager_server
  if (manager.sendtoWait((uint8_t *)data, sizeof(data), SERVER_ADDRESS))
  {
    // Now wait for a reply from the server
    uint8_t len = sizeof(buf);
    uint8_t from;   
    if (manager.recvfromAckTimeout(buf, &len, 2000, &from))
    {
      Serial.print("got reply from : 0x");
      Serial.print(from, HEX);
      Serial.print(": ");
      Serial.println((char*)buf);
    }
    else
    {
      Serial.println("No reply, is rf95_reliable_datagram_server running?");
    }
  }
  else
    Serial.println("sendtoWait failed");
  delay(500);
}

void resetDevice()
{
  // manual reset
  int pinReset = 4;
  digitalWrite(pinReset, LOW);
  delay(10);
  digitalWrite(pinReset, HIGH);
  delay(10);
}
