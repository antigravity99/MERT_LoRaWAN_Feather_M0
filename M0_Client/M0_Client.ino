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
#include "mert.h"

#define TMP007_DEBUG 1
Adafruit_TMP007 tmp007;

#define SERVER_ADDRESS 0
#define CLIENT_ADDRESS 2
#define FREQ 915.0

/* for feather m0 */ 
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3

static const String DEVICE_TYPE = HUMIDITY_TYPE;

// Singleton instance of the radio driver
RH_RF95 driver(RFM95_CS, RFM95_INT);

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

//    if (! tmp007.begin()) {
//    Serial.println("No sensor found");
//    while (1);
  }
}

// Dont put this on the stack:
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
//uint8_t data[] = "Hi Mom! Look, no hands!";

void loop()
{
  String tempStr = String(tmp007.readDieTempC());
  tempStr = tempStr + "," + DEVICE_TYPE;
  char data[tempStr.length()+1];
  tempStr.toCharArray(data, sizeof(data));
  
//  Serial.print("Object Temp#:      "); Serial.print(data); Serial.println("*C");
  
//  Serial.println("Sending to rf95_reliable_datagram_server");
    Serial.println("Send");
  // Send a message to manager_server
  if (manager.sendtoWait((uint8_t *)data, sizeof(data), SERVER_ADDRESS))
  {
    // Now wait for a reply from the server
    uint8_t len = sizeof(buf);
    uint8_t from;   
    if (manager.recvfromAckTimeout(buf, &len, 2000, &from))
    {
      
      if(from == SERVER_ADDRESS && buf[0] == 1)
        Serial.println("Ack");
    }
    else
    {
      Serial.println("No reply, is rf95_reliable_datagram_server running?");
    }
  }
  else 
      Serial.println("Send to wait failed");
  delay(500);
}

//void resetDevice()
//{
//  // manual reset
//  int pinReset = 4;
//  digitalWrite(pinReset, LOW);
//  delay(10);
//  digitalWrite(pinReset, HIGH);
//  delay(10);
//}
