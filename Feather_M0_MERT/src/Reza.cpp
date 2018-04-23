#include "Reza.h"

Reza::Reza(){}

void Reza::init(bool isServer)
{
  if(isServer)
  {
    _moteAddress = SERVER_ADDRESS;
    _moteType = SERVER_VALUE;
  }
  else
  {
    _moteAddress = getAddress();
    _moteType = MOTE_VALUE;
    /* Assign a unique ID to this sensor at the same time */
    _accel = Adafruit_ADXL345_Unified(ADXL345_ADDRESS);
    /* Initialise the sensor */
    if(!_accel.begin())
    {
      /* There was a problem detecting the ADXL345 ... check your connections */
      Serial.println("Ooops, no ADXL345 detected ... Check your wiring!.");
      // while(1);
    }

    /* Set the range to whatever is appropriate for your project */
    // _accel.setRange(ADXL345_RANGE_16_G);
    // _accel.setRange(ADXL345_RANGE_8_G);
    _accel.setRange(ADXL345_RANGE_4_G);
    // _accel.setRange(ADXL345_RANGE_2_G);
  }
}

bool Reza::managerInit()
{
  if (!_manager.init())
  {
    Serial.println("init failed");
    return false;
  }
  else
  {
    if (!_driver.setFrequency(FREQ))
    {
      Serial.println("setFrequency failed");
      while (1);
    }
    Serial.print("Set Freq to: "); Serial.println(FREQ);
    //http://www.airspayce.com/mikem/arduino/RadioHead/classRH__RF95.html#ab273e242758e3cc2ed2679ef795a7196
    //http://www.airspayce.com/mikem/arduino/RadioHead/classRH__RF95.html#ab9605810c11c025758ea91b2813666e3

    //http://www.hoperf.com/upload/rf/RFM95_96_97_98W.pdf
    //Note that the spreading factor,
    //SpreadingFactor, must be known in advance on both transmit and receive sides of the link as different spreading factors are orthogonal to each other
    //Default Bw125Cr45Sf128
    //_driver.setModemConfig(RH_RF95::Bw31_25Cr48Sf512);  //set for pre-configured long range
    //After init(), the power will be set to 13dBm, with useRFO false (ie PA_BOOST enabled). See documentation link above
    //dBm to mw conversion table http://www.fab-corp.com/pages.php?pageid=1
    //not sure going over 20dBm really makes much difference. Looking at the RH_RF95.cpp and RH_RF95.h files and the
    //set setTxPower method comments seem to indicate that there should be be the current consuption does not indicate this
    // _driver.setTxPower(20, false);  //set for 100mw
    //driver.setTxPower(23, false);
    //TX power register is 9 and will be 0x88 by default, which is equal to 13dBm (per the documentation);
    _driver.printRegisters();  //Print all the RFM95 register values
  }
  return true;
}

String Reza::getMoteType()
{
  return _moteType;
}

uint8_t Reza::getMoteAddress()
{
  return _moteAddress;
}

String Reza::serailizeRequest(request_t req)
{
  StaticJsonBuffer<500> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  root[ADDRESS] = req.address;
  root[CMD] = req.cmd;
  root[KEY] = req.key;
  root[VALUE] = req.value;
  // root[CHECKSUM] = req.checksum;
  String s;
  root.printTo(s);
  return s;
  // return root.measureLength();
}

void Reza::deserializeJsonRequest(request_t *req, char *json)
{
  StaticJsonBuffer<800> jsonBuffer;
#ifdef DEBUG_1
  Serial.print("ParseJsonRequest json: ");
   Serial.println(json);
#endif
  JsonObject &root = jsonBuffer.parseObject(json);
  if (!root.success())
    Serial.println("Could not parse the json message!");

    // root.prettyPrintTo(Serial);

    req->address = root[ADDRESS];
    req->cmd = root[CMD].as<String>();
    req->key = root[KEY].as<String>();
    req->value = root[VALUE].as<String>();
    // req->checksum = root[CHECKSUM].as<String>();

    // printRequestStruct(req);
}

bool Reza::sendtoWait(request_t req)
{
  bool successful = false;

  String json = serailizeRequest(req);
  char buff[171];
  json.toCharArray(buff, json.length()+1);

  //currently set to send the 64 Accelerometer samples
  //The indexes should be changed to be dynamically set based on the length of the
  //samples array instead of hard coded. Also, you can send up to 105 samples,
  //where the last sample is actually the average sample rate
  if(req.key == VIBRATION_KEY)
  {
    //This breaks the 16bit integers into its uppper and lower bytes and
    //stores them as a char and then appends/inserts them to the VAL string
    uint8_t j = 0;
    for (uint8_t i = 38; i < 168; i++)
    {
      char lo = req.vibBuff[j] & 0xFF;
      char hi = req.vibBuff[j++] >> 8;
      buff[i++] = hi;
      // Serial.println(buff[i]);
      buff[i] = lo;
    }
    //close off the json string since it was overwritten
    buff[168] = '"';
    buff[169] = '}';
    //null char to end the json string
    buff[170] = (uint8_t)0;

    free(req.vibBuff);
  }



#ifdef DEBUG_2
  for(int i = 0; i < 171; i++)
  {
      Serial.print(buff[i]);
  }
  // Serial.print("sendToWait buff: ");
  Serial.println("");
#endif
  // Send a message to manager_server
  if (_manager.sendtoWait((uint8_t*)buff, sizeof(buff), SERVER_ADDRESS))
  {
    // Now wait for a reply from the server
    // uint8_t len = sizeof(_rcvBuf);
    // uint8_t from;
    //Decided not to wait for an acknowledgement
//     if (_manager.recvfromAckTimeout(_rcvBuf, &len, 2000, &from))
//     {
// #ifdef DEBUG_1
//       Serial.print("got reply from : 0x");
//       Serial.print(from, HEX);
//       Serial.print(": ");
//       Serial.println((char*)_rcvBuf);
// #endif
//       if(String((char*)_rcvBuf) == String(ACK))
//       {
//         successful = true;
//       }
//     }
//     else
//     {
// #ifdef DEBUG_1
//       Serial.println("No reply, is rf95_reliable_datagram_server running?");
// #endif
//     }
  }
  else
  {
#ifdef DEBUG_1
      Serial.println("sendtoWait failed");
#endif
  }
  #ifdef DEBUG_1
        Serial.print("Successful: "); Serial.println(successful);
  #endif

  return successful;

}

bool Reza::recvfromAckTimeout(request_t *req, String *json)
{
  bool successful = false;
  // Wait for a message addressed to us from the client
  uint8_t len = sizeof(_rcvBuf);
  uint8_t from;

  if (_manager.recvfromAckTimeout(_rcvBuf, &len, 2000, &from))
  {
    #ifdef DEBUG_2
      Serial.print("Received data from: ");
      Serial.println(from, DEC);
      Serial.print("Data: ");
      Serial.println((char*)_rcvBuf);
      for(int i = 0; i < 170; i++)
      {
          Serial.print((char)_rcvBuf[i]);
      }
    #endif
    // Serial.println((char*)_rcvBuf);

    // Serial.print("Size of _rcvBuf: "); Serial.println(sizeof(_rcvBuf));
    // Serial.print("Size of json string: "); Serial.println(sizeof(json));
    // Serial.println((char)_rcvBuf[26]);

    StaticJsonBuffer<1000> jsonBuffer;
    JsonArray &array = jsonBuffer.createArray();

    if((char)_rcvBuf[26] == 'V' && (char)_rcvBuf[27] == 'I' && (char)_rcvBuf[28] == 'B')
    {
      for (uint8_t i = 38; i < 168; i++)
      {
        uint16_t mag = _rcvBuf[i+1] | uint16_t(_rcvBuf[i]) << 8;
        i++;
        array.add(mag);
      }
      // array.printTo(Serial);

      _rcvBuf[38] = '\0';

      json->concat((char*)_rcvBuf);
      String s;
      array.printTo(s);
      json->concat(s);
      json->concat('"');
      json->concat('}');

      char buff[json->length()];
      json->toCharArray(buff, json->length()+1);
      deserializeJsonRequest(req, buff);
    }
    else
    {
      json->concat((char*)_rcvBuf);
      deserializeJsonRequest(req, (char *)_rcvBuf);
    }

     successful = true;
     //Decided not to send an acknowledgement
     //Send a reply back to the originator client
     // uint8_t ackBuff[] = "ack";
     // if (!_manager.sendtoWait(ackBuff, sizeof(ackBuff), from))
     //  Serial.println("sendtoWait failed");
  }
  else
  {
    Serial.println("recvfromAckTimeout failed");
    return successful;
  }
  return successful;
}

bool Reza::recvfromAck(request_t *req)
{
  bool successful = false;
  // Wait for a message addressed to us from the client
  uint8_t len = sizeof(_rcvBuf);
  uint8_t from;

  if (_manager.recvfromAck(_rcvBuf, &len, &from))
  {
#ifdef DEBUG_2
    Serial.print("Received data from: ");
    Serial.println(from, DEC);
    Serial.print("Data: ");
    Serial.println((char*)_rcvBuf);
#endif
    deserializeJsonRequest(req, (char*)_rcvBuf);


    successful = true;
    //Decided not to send an acknowledgement
    //Send a reply back to the originator client
    // uint8_t ackBuff[] = "ack";
    // if (!_manager.sendtoWait(ackBuff, sizeof(ackBuff), from))
    //   Serial.println("sendtoWait failed");
  }
  return successful;
}

void Reza::checkSerial()
{
  String inputString = "";
  while (Serial.available())
  {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n')
    {
      Serial.println(inputString);
      // _stringComplete = true;
      serialEvent(inputString);
    }
  }
}

//no longer used
/** Quick and dirty checksum **/
// char Reza::checksum(char* s)
// {
//   signed char sum = -1;
//   while (*s != 0)
//   {
//     sum += *s;
//     s++;
//   }
//   return sum;
// }

void Reza::serialEvent(String serialData)
{

#ifdef DEBUG_3
  Serial.println("Beginning of serialEvent");
  Serial.print("Serial event data: ");
  Serial.println(serialData);
#endif

  request_t req;
  int len = serialData.length();
  char buff[len+1];
  serialData.toCharArray(buff, len+1);
  deserializeJsonRequest(&req, buff);
  processReq(req);

  // Serial.print("JSON to struct key: ");
  // Serial.println(jReq.key);

  // parseRequest(&req, buff);

//   if(req.isVerified == 0)
//     return;
// #ifdef DEBUG_2
//   else
//     Serial.println("Do something with the valid data\n\n");
// #endif

  //Do something with the data
  // if(jReq.address != MY_ADDRESS)
    //forwardMessage(req.address, req.message);
  // else
    // processReq(req);
}

void Reza::processReq(request_t req)
{
#ifdef DEBUG_3
  Serial.print("cmd: ");
  Serial.println(req.cmd);
  Serial.print("cmd to int: ");
  Serial.println((int)req.cmd.charAt(0));
  Serial.print("#define update: ");
  Serial.println(UPDATE_CMD);
  Serial.print("#define update to int: ");
  Serial.println((int)UPDATE_CMD);
#endif

if(req.cmd == REQUEST_CMD)
  processRequestCmd(req);
else if(req.cmd == UPDATE_CMD)
  processUpdateCmd(req);
}

void Reza::processUpdateCmd(request_t req)
{
#ifdef DEBUG_1
  Serial.println("Got an update Command");
#endif

  if(req.key == String(SAMPLE_RATE_KEY))
    _sampleRate = req.value.toInt();

#ifdef DEBUG_1
  Serial.print("_sampleRate: ");
  Serial.println(_sampleRate);
#endif

}

void Reza::processRequestCmd(request_t req)
{
#ifdef DEBUG_1
  Serial.println("Got a request Command");
  Serial.print("Request Key: ");
  Serial.println(req.key);
#endif

  if(req.key == String(TYPE_KEY))
  {
#ifdef DEBUG_1
    Serial.print("Process CMD type: ");
    Serial.println(TYPE_KEY);
#endif
    request_t returnReq;
    returnReq.address = _moteAddress;
    returnReq.cmd = REQUEST_RESPONSE_CMD;
    returnReq.key = TYPE_KEY;
    returnReq.value = _moteType;
    // returnReq.checksum = "";

    // char *buff = (char *) malloc(sizeof(char) * 251);
    String json = serailizeRequest(returnReq);
#ifdef DEBUG_1
    Serial.println(json);
#endif

    // free(buff);

    // char req[251];
    // returnRequest(req, REQUEST_RESPONSE_CMD, TYPE_KEY, MY_TYPE);
  }
  else if(req.key == String())
  {
#ifdef DEBUG_1
    Serial.print("Process CMD type: ");
    Serial.println(TYPE_KEY);
#endif
    request_t returnReq;
    returnReq.address = _moteAddress;
    returnReq.cmd = REQUEST_RESPONSE_CMD;
    returnReq.key = TYPE_KEY;
    returnReq.value = _moteType;
    // returnReq.checksum = "";

    // char *buff = (char *) malloc(sizeof(char) * 251);
    String json = serailizeRequest(returnReq);
    #ifdef DEBUG_1
        Serial.println(json);
    #endif
    // free(buff);

    // char req[251];
    // returnRequest(req, REQUEST_RESPONSE_CMD, TYPE_KEY, MY_TYPE);
  }

}

void Reza::forwardMessage(uint8_t address, char message[])
{
  Serial.print("Send: " );
  Serial.println(message);
  Serial.print("To address: ");
  Serial.println(address);
  Serial.println("Not yet implemented");
}

temp_t Reza::getTemp()
{
  temp_t temp;
  // Output data to serial monitor
  temp.irTemp = _tmp007.readObjTempC();
  temp.dieTemp = _tmp007.readDieTempC();

#ifdef DEBUG_2
  Serial.print("Object Temperature: "); Serial.print(temp.irTemp); Serial.println("*C");
  Serial.print("Die Temperature: "); Serial.print(temp.dieTemp); Serial.println("*C");
#endif
  return temp;
}

uint16_t* Reza::getAccelMagArray()
{
  uint16_t *buffer = (uint16_t*)malloc(sizeof(uint16_t) * 64);

  unsigned long t = micros();

  for (uint8_t i = 0; i < VIB_SAMPLES-1; i++)
  {
    // /* Get a new sensor event */
    sensors_event_t event;
    _accel.getEvent(&event);

    /* Display the results (acceleration is measured in m/s^2) */
    //changed to be int instead of m/s^
    // Serial.println("Accelerometer readings are:");
    // Serial.print("X: "); Serial.print(event.acceleration.x); Serial.print("  ");
    // Serial.print("Y: "); Serial.print(event.acceleration.y); Serial.print("  ");
    // Serial.print("Z: "); Serial.print(event.acceleration.z); Serial.print("  ");Serial.println("m/s^2 ");

    // uint16_t mag = ;
    buffer[i] = sqrt((event.acceleration.x * event.acceleration.x) + (event.acceleration.y * event.acceleration.y) + (event.acceleration.z * event.acceleration.z)) * 1000;

    //Makes the sample rate 300Hz
    // delayMicroseconds(1662);
  }

  double sampleRate = 1 / ((micros() - t) / 1000000.0 / VIB_SAMPLES);
#ifdef DEBUG_3
  Serial.print("Sample rate: "); Serial.print(sampleRate); Serial.println("Hz");
#endif
  buffer[VIB_SAMPLES-1] = sampleRate * 100;
  return buffer;
}



// uint8_t Reza::getAddress()
// {
//   uint8_t val[4] = {0,0,1,0};
//   uint8_t address = -1;
//     if(val[0])
//       address += 1;
//     if(val[1])
//       address += 2;
//     if(val[2])
//       address += 4;
//     if(val[3])
//       address += 8;
//     address += 1;
//   return address;
// }


uint8_t Reza::getAddress()
{
  // Serial.println(digitalRead(DIP_4));
  // Serial.println(digitalRead(DIP_3));
  // Serial.println(digitalRead(DIP_2));
  // Serial.println(digitalRead(DIP_1));
  uint8_t address = -1;
  if(digitalRead(DIP_4))
      address += 1;
  if(digitalRead(DIP_3))
      address += 2;
  if(digitalRead(DIP_2))
      address += 4;
  if(digitalRead(DIP_1))
      address += 8;
  address += 1;
  Serial.print("Address: "); Serial.println(address);
  return address;
}


void Reza::printRequestStruct(request_t *req)
{
// #ifdef DEBUG_2
  Serial.print("Address: ");
  Serial.println(req->address);
  Serial.print("Cmd: ");
  Serial.println(req->cmd);
  Serial.print("Stored key: ");
  Serial.println(req->key);
  Serial.print("Stored value: ");
  Serial.println(req->value);
  // Serial.print("Stored checksum: ");
  // Serial.println(req->checksum);
  // Serial.print("Stored isVerified: ");
  // Serial.println(req->isVerified);
  // Serial.print("Stored fullTransmission: ");
  // Serial.println(req->fullTransmission);
  Serial.println("\n");
// #endif
}


// void Reza::parseRequest(request *req, char* str)
// {
//   strcpy(req->message, str);
//
// #ifdef DEBUG_3
//   Serial.print("Message: ");
//   Serial.println(str);
// #endif
//
//
//   const char s[2] = ",";
//   char *token;
//   uint8_t i = 0;
//   /* get the first token */
//   token = strtok(str, s);
//   /* walk through other tokens */
//
// #ifdef DEBUG_3
//   Serial.print(i);
//   Serial.print(" - token: ");
//   Serial.println(token);
// #endif
//
//   while( token != NULL )
//   {
//     if((i == 0) && (strcmp(token, SOF) != 0))
//     {
//       Serial.println(token);
//       Serial.println("SOF bad!");
//       return;
//     }
//
//     switch(i)
//     {
//       case 1:
//       {
//         char buff[strlen(token)];
//         strcpy(buff, token);
//         req->address = atoi(buff);
//       }break;
//       case 2:
//         strcpy(req->cmd, token);
//         break;
//       case 3:
//         strcpy(req->key, token);
//       break;
//       case 4:
//         strcpy(req->value, token);
//         break;
//       case 5:
//         strcpy(req->checksum, token);
//         verifyChecksum(req, token);
//         break;
//       case 6:
//         if(strcmp(token, EOF) == 0)
//           req->fullTransmission = 1;
//       default:
//         break;
//     }
//     i++;
//     token = strtok(NULL, s);
//   }
//     printRequestStruct(req);
// }

// void Reza::verifyChecksum(Request *req, char *token)
// {
  // String address = String(req->address);
  // //for the 2 !! and the 4 commas
  // uint8_t l1 = 6;
  // uint8_t l2 = address.length();
  // uint8_t l3 = req->cmd.length();
  // uint8_t l4 = req->key.length();
  // uint8_t l5 = req->value.length();
  // uint8_t len = l1 + l2 + l3 + l4 + l5;
  // char reqStr[len+1];

//   strcpy(reqStr, "!!,");
//   strcat(reqStr, add);
//   strcat(reqStr, ",");
//   strcat(reqStr, req->cmd);
//   strcat(reqStr, ",");
//   strcat(reqStr, req->key);
//   strcat(reqStr, ",");
//   strcat(reqStr, req->value);
//   Serial.println(reqStr);
//
//   signed char sum = checksum(reqStr);
//   char buff[8];
//   itoa(sum, buff, 10);
// #ifdef DEBUG_2
//   Serial.print("Checksum converted to char[]: ");
//   Serial.println(buff);
// #endif
//   if(strcmp(buff, token) == 0)
//     req->isVerified = 1;
// }
