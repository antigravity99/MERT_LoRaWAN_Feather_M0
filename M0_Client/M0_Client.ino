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
  Serial.begin(9600);
  if (!mert.managerInit())
  {
    Serial.println("Error init");
    while(1);
  }
}

void loop()
{
// 
//  String tempStr = String(tmp007.readDieTempC());
//  tempStr = tempStr + "," + DEVICE_TYPE;
//  char data[tempStr.length()+1];
//  tempStr.toCharArray(data, sizeof(data));
//  
//    Serial.println("Sending to rf95_reliable_datagram_server");
//
//  // Send a message to manager_server
//  if (manager.sendtoWait((uint8_t *)data, sizeof(data), SERVER_ADDRESS))
//  {
//    // Now wait for a reply from the server
//    if (manager.recvfromAckTimeout(_rcvBuf, &len, 2000, &from))
//    { 
//      if(from == SERVER_ADDRESS && _rcvBuf[0] == 1)
//        Serial.println("Acknowledged");
//    }
//    else
//    {
//      Serial.println("No reply, is rf95_reliable_datagram_server running?");
//    }
//  }
//  else 
//      Serial.println("Send to wait failed");
  delay(500);
}
