// rf95_reliable_datagram_client.pde
// -*- mode: C++ -*-
// Example sketch showing how to create a simple addressed, reliable messaging client
// with the RHReliableDatagram class, using the RH_RF95 driver to control a RF95 radio.
// It is designed to work with the other example rf95_reliable_datagram_server
// Tested with Anarduino MiniWirelessLoRa, Rocket Scream Mini Ultra Pro with the RFM95W 

#include <Wire.h>
#include "Adafruit_TMP007.h"
#include <SPI.h>
#include "Mert.h"

Adafruit_TMP007 tmp007;

Mert mert;

static const String DEVICE_TYPE = HUMIDITY_KEY;

void setup() 
{
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);
  Serial.begin(9600);
  if (!mert.managerInit())
  {
    Serial.println("Error init");
    while(1);
  }
}

void loop()
{
  String s = "data";
  // Send a message to manager_server
  if (mert.sendtoWait(s))
  {
    
  }
  delay(500);
}











