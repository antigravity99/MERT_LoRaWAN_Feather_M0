// rf95_reliable_datagram_server.pde
// -*- mode: C++ -*-
// Example sketch showing how to create a simple addressed, reliable messaging server
// with the RHReliableDatagram class, using the RH_RF95 driver to control a RF95 radio.
// It is designed to work with the other example rf95_reliable_datagram_client
// Tested with Anarduino MiniWirelessLoRa, Rocket Scream Mini Ultra Pro with the RFM95W 


#include <Wire.h>
#include "Adafruit_TMP007.h"
#include <SPI.h>
#include "Mert.h"



Adafruit_TMP007 tmp007;

Mert mert;

void setup() 
{
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);
  Serial.begin(9600);
  //while (!Serial) ; // Wait for serial port to be available
  if(!mert.managerInit())
  {
    Serial.println("Error init");
    while(1);
  }
}

void loop()
{   
    if (mert.recvfromAck())
    {
      Serial.println("WoooHoo!");
    }
  delay(500);
}
