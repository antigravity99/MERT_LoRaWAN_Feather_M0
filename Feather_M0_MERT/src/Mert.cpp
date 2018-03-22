#include "Mert.h"

Mert::Mert()
{
}

void Mert::init(bool isServer)
{
  if(isServer)
    _moteAddress = SERVER_ADDRESS;
  else
    _moteAddress = getAddress();

    /* Assign a unique ID to this sensor at the same time */
    _accel = Adafruit_ADXL345_Unified(ADXL345_ADDRESS);
    /* Initialise the sensor */
    if(!_accel.begin())
    {
      /* There was a problem detecting the ADXL345 ... check your connections */
      Serial.println("Ooops, no ADXL345 detected ... Check your wiring!. The program will not progress!");
      while(1);
    }

    /* Set the range to whatever is appropriate for your project */
    // _accel.setRange(ADXL345_RANGE_16_G);
    // _accel.setRange(ADXL345_RANGE_8_G);
    // _accel.setRange(ADXL345_RANGE_4_G);
    _accel.setRange(ADXL345_RANGE_2_G);
}

String Mert::getMoteType()
{
  return _moteType;
}

uint8_t Mert::getMoteAddress()
{
  return _moteAddress;
}

bool Mert::managerInit()
{
  if (!_manager.init())
  {
    //driver.setModeTx()
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
    //driver.setTxPower(23, false);
  }
  return true;
}

uint8_t Mert::serailizeRequest(Request req, char *buff)
{
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  root[ADDRESS] = req.address;
  root[CMD] = req.cmd;
  root[KEY] = req.key;
  root[VALUE] = req.value;
  root[CHECKSUM] = req.checksum;

  root.printTo(buff, root.measureLength()+1);

  return root.measureLength();
}

bool Mert::sendtoWait(Request req)
{
  bool successful = false;
  char *temp = (char *) malloc(sizeof(char) * 251);
  int j = serailizeRequest(req, temp);
  char buff[j];
  strncpy_PF(buff, temp, j);
  free(temp);
#ifdef DEBUG_2
  Serial.print("sendToWait buff: "); Serial.println(buff);
#endif
  // Send a message to manager_server
  if (_manager.sendtoWait((uint8_t*)buff, sizeof(buff), SERVER_ADDRESS))
  {
    // Now wait for a reply from the server
    uint8_t len = sizeof(_rcvBuf);
    uint8_t from;
    if (_manager.recvfromAckTimeout(_rcvBuf, &len, 2000, &from))
    {
#ifdef DEBUG_1
      Serial.print("got reply from : 0x");
      Serial.print(from, HEX);
      Serial.print(": ");
      Serial.println((char*)_rcvBuf);
#endif
      if(String((char*)_rcvBuf) == String(ACK))
      {
        successful = true;
      }

    }
    else
    {
#ifdef DEBUG_1
      Serial.println("No reply, is rf95_reliable_datagram_server running?");
#endif
    }
  }
  else
  {
#ifdef DEBUG_1
      Serial.println("sendtoWait failed");
#endif
  }
  return successful;

}

bool Mert::recvfromAckTimeout(Request *req, char* json)
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
    #endif

    parseJsonRequest(req, (char*)_rcvBuf);
     //Send a reply back to the originator client
     successful = true;
     uint8_t ackBuff[] = "ack";
     if (!_manager.sendtoWait(ackBuff, sizeof(ackBuff), from))
      Serial.println("sendtoWait failed");
  }
  else
  {
    Serial.println("recvfromAckTimeout faild");
    return successful;
  }
  return successful;
}

bool Mert::recvfromAck(Request *req)
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
    parseJsonRequest(req, (char*)_rcvBuf);
    //Send a reply back to the originator client
    successful = true;
    uint8_t ackBuff[] = "ack";
    if (!_manager.sendtoWait(ackBuff, sizeof(ackBuff), from))
    Serial.println("sendtoWait failed");
  }
  return successful;
}

void Mert::checkSerial()
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

/** Quick and dirty checksum **/
char Mert::checksum(char* s)
{
  signed char sum = -1;
  while (*s != 0)
  {
    sum += *s;
    s++;
  }
  return sum;
}

void Mert::serialEvent(String serialData)
{

#ifdef DEBUG_3
  Serial.println("Beginning of serialEvent");
  Serial.print("Serial event data: ");
  Serial.println(serialData);
#endif

  Request jReq;
  int len = serialData.length();
  char buff[len+1];
  serialData.toCharArray(buff, len+1);

  parseJsonRequest(&jReq, buff);

  Serial.print("JSON to struct key: ");
  Serial.println(jReq.key);

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

void Mert::processReq(Request req)
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

  switch((int)req.cmd.charAt(0))
  {
    case (int)UPDATE_CMD[0]:
      processUpdateCmd(req);
      break;
    case (int)REQUEST_CMD[0]:
#ifdef DEBUG_2
      Serial.println("Got a request Command");
#endif
      processRequestCmd(req);
      break;
    default:
      Serial.println("Not a valid command");
      break;
  }
}

void Mert::processUpdateCmd(Request req)
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

void Mert::processRequestCmd(Request req)
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
    // char req[251];
    // returnRequest(req, REQUEST_RESPONSE_CMD, TYPE_KEY, MY_TYPE);
  }

}

void Mert::forwardMessage(uint8_t address, char message[])
{
  Serial.print("Send: " );
  Serial.println(message);
  Serial.print("To address: ");
  Serial.println(address);
  Serial.println("Not yet implemented");
}

void Mert::parseJsonRequest(Request *req, char *json)
{
  StaticJsonBuffer<200> jsonBuffer;
#ifdef DEBUG_1
  Serial.print("ParseJsonRequest json: "); Serial.println(json);
#endif
  JsonObject &root = jsonBuffer.parseObject(json);
  if (!root.success())
    Serial.println("Could not parse the json message!");

    req->address = root[ADDRESS];
    req->cmd = root[CMD].as<String>();
    req->key = root[KEY].as<String>();
    req->value = root[VALUE].as<String>();
    req->checksum = root[CHECKSUM].as<String>();

}

Temp Mert::getTemp()
{
  Temp temp;

  // Output data to serial monitor
  temp.irTemp = _tmp007.readObjTempC();
  Serial.print("Object Temperature: "); Serial.print(temp.irTemp); Serial.println("*C");
  temp.dieTemp = _tmp007.readDieTempC();
  Serial.print("Die Temperature: "); Serial.print(temp.dieTemp); Serial.println("*C");

  return temp;
}

uint16_t Mert::getAccelMag()
{
  // /* Get a new sensor event */
    sensors_event_t event;
    _accel.getEvent(&event);

    /* Display the results (acceleration is measured in m/s^2) */
    //changed to be int instead of m/s^
    Serial.println("Accelerometer readings are:");
    Serial.print("X: "); Serial.print(event.acceleration.x); Serial.print("  ");
    Serial.print("Y: "); Serial.print(event.acceleration.y); Serial.print("  ");
    Serial.print("Z: "); Serial.print(event.acceleration.z); Serial.print("  ");Serial.println("m/s^2 ");

    uint16_t magnitude = sqrt((event.acceleration.x * event.acceleration.x) + (event.acceleration.y * event.acceleration.y) + (event.acceleration.z * event.acceleration.z));
    return magnitude;
}



uint8_t Mert::getAddress()
{
  uint8_t val[4] = {0,1,1,1};
  uint8_t address = -1;
    if(val[0])
      address += 1;
    if(val[1])
      address += 2;
    if(val[2])
      address += 4;
    if(val[3])
      address += 8;
    address += 1;
  return address;
}


// uint8_t getAddress()
// {
//   uint8_t address = -1;
//   if(digitalRead(PIN_1))
//       address += 1;
//   if(digitalRead(PIN_2))
//       address += 2;
//   if(digitalRead(PIN_3))
//       address += 4;
//   if(digitalRead(PIN_4))
//       address += 8;
//     address += 1;
//   return address;
// }


void Mert::printRequestStruct(Request *req)
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
  Serial.print("Stored key: ");
  Serial.println(req->key);
  Serial.print("Stored checksum: ");
  Serial.println(req->checksum);
  // Serial.print("Stored isVerified: ");
  // Serial.println(req->isVerified);
  // Serial.print("Stored fullTransmission: ");
  // Serial.println(req->fullTransmission);
  Serial.println("\n");
// #endif
}


// void Mert::parseRequest(request *req, char* str)
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

// void Mert::verifyChecksum(Request *req, char *token)
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
