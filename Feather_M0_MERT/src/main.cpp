// rf95_reliable_datagram_server.pde
// -*- mode: C++ -*-
// Example sketch showing how to create a simple addressed, reliable messaging server
// with the RHReliableDatagram class, using the RH_RF95 driver to control a RF95 radio.
// It is designed to work with the other example rf95_reliable_datagram_client
// Tested with Anarduino MiniWirelessLoRa, Rocket Scream Mini Ultra Pro with the RFM95W

#include <Wire.h>
#include <Adafruit_TMP007.h>
#include <SPI.h>
#include "Mert.h"

bool isServer = true;

void server();
void client();
uint8_t getAddress();

Adafruit_TMP007 tmp007;
Mert mert;

void setup()
{

  if (isServer)
    mert.init(String(SERVER_TYPE), SERVER_ADDRESS);
  else
    mert.init(String(AMB_TEMP_TYPE), 1);

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
    if(isServer)
      server();
    else
      client();
}


void server()
{

  Serial.println(getAddress());
  Request req;
  if (mert.recvfromAckTimeout(&req))
  {
    Serial.print("Address: ");
    Serial.println(req.address);
    Serial.print("CMD: ");
    Serial.println(req.cmd);
    Serial.print("Key: ");
    Serial.println(req.key);
    Serial.print("Value: ");
    Serial.println(req.value);
    Serial.print("Checksum: ");
    Serial.println(req.checksum);
  }
  mert.checkSerial();
  delay(500);
}

void client()
{
  Request req;
  req.address = mert.getMoteAddress();
  req.cmd = SEND_CMD;
  req.key = TEMP_KEY;
  req.value = String(380);

  // Send a message to manager_server
  if (mert.sendtoWait(req))
  {

  }
  delay(500);
}

uint8_t getAddress()
{
  uint8_t val[4] = {1,1,1,1};
  uint8_t address = -1;
    if(val[0] == 1)
      address += 1;
    if(val[1] == 1)
      address += 2;
    if(val[2] == 1)
      address += 4;
    if(val[3] == 1)
      address += 8;
    address += 1;
  return address;
}
