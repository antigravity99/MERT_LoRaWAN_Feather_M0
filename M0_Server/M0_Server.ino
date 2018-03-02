// rf95_reliable_datagram_server.pde
// -*- mode: C++ -*-
// Example sketch showing how to create a simple addressed, reliable messaging server
// with the RHReliableDatagram class, using the RH_RF95 driver to control a RF95 radio.
// It is designed to work with the other example rf95_reliable_datagram_client
// Tested with Anarduino MiniWirelessLoRa, Rocket Scream Mini Ultra Pro with the RFM95W 

#include <RHReliableDatagram.h>
#include <Arduino.h>
#include <Wire.h>
#include "Adafruit_TMP007.h"
#include <SPI.h>
#include <RH_RF95.h>
#include "mert.h"

Adafruit_TMP007 tmp007;

#define FREQ 915.0

/* for feather m0 */ 
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3

// Singleton instance of the radio driver
RH_RF95 driver(RFM95_CS, RFM95_INT);

// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram manager(driver, SERVER_ADDRESS);

void setup() 
{
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);
  Serial.begin(9600);
  //while (!Serial) ; // Wait for serial port to be available

  if (!manager.init())
  {
    //driver.setModeTx()
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
    //driver.setTxPower(23, false);
  }
}

uint8_t ackBuff[] = {1};
// Dont put this on the stack:
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];

void loop()
{
  checkSerial();
//if (manager.available())
//Serial.println("I'm available");
//else
//Serial.println("I'm not available");

    // Wait for a message addressed to us from the client
    uint8_t len = sizeof(buf);
//    Serial.println(len);
    uint8_t from;
    if (manager.recvfromAck(buf, &len, &from))
    {
      Serial.print(from, DEC);
      Serial.print(",");
      Serial.println((char*)buf);

      // Send a reply back to the originator client
      if (!manager.sendtoWait(ackBuff, sizeof(ackBuff), from))
        Serial.println("sendtoWait failed");
    }
  delay(500);
}
