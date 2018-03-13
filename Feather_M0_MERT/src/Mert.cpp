#include "Mert.h"

Mert::Mert()
{
  
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

bool Mert::sendtoWait(String data)
{
  Serial.println("Sending to rf95_reliable_datagram_server");
    data = data + "," + MY_TYPE;
    char buff[data.length()+1];
    data.toCharArray(buff,(data.length()+1));

    char req[251];    
    returnRequest(req, SEND_CMD, TEMP_KEY, buff);
    
  // Send a message to manager_server
  if (_manager.sendtoWait((uint8_t*)req, sizeof(req), SERVER_ADDRESS))
  {

    
     
    // Now wait for a reply from the server
    uint8_t len = sizeof(rcvBuf);
    uint8_t from;   
    if (_manager.recvfromAckTimeout(rcvBuf, &len, 2000, &from))
    {
      Serial.print("got reply from : 0x");
      Serial.print(from, HEX);
      Serial.print(": ");
      Serial.println((char*)rcvBuf);
    }
    else
    {
      Serial.println("No reply, is rf95_reliable_datagram_server running?");
    }
  }
  else
    Serial.println("sendtoWait failed");
}
bool Mert::recvfromAckTimeout()
{
 // Wait for a message addressed to us from the client
    uint8_t len = sizeof(rcvBuf);
    uint8_t from;
    if (_manager.recvfromAckTimeout(rcvBuf, &len, 2000, &from))
    { 
      
    }
}

bool Mert::recvfromAck()
{

  // Wait for a message addressed to us from the client
      uint8_t len = sizeof(rcvBuf);
      uint8_t from;
      
  if (_manager.recvfromAck(rcvBuf, &len, &from))
    {     
      Serial.print(from, DEC);
      Serial.print(",");
      Serial.println((char*)rcvBuf);
      Serial.println(sizeof((char*)rcvBuf));

       //Send a reply back to the originator client
       uint8_t ackBuff[] = "ack";
      if (!_manager.sendtoWait(ackBuff, sizeof(ackBuff), from))
        Serial.println("sendtoWait failed");        
    }
    
}

void Mert::checkSerial() 
{
  while (Serial.available()) {
    if(_stringComplete)
    {
      _inputString = "";
      _stringComplete = false;
    }
      
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    _inputString += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
//    Serial.println(inputString);
    if (inChar == '\n') 
    {
      _stringComplete = true;
      serialEvent(_inputString);  
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
//  String serialData = String((char[len])Serial.read());  
//  char buff[len+1];
//  
//  for(int i = 0; i < len; i++)
//  {
//    buff[i] = Serial.read();
//  }
//
//  buff[len] = '\0';
  Serial.println("Beginning");
#ifdef DEBUG_3
  Serial.print("Serial event data: ");
  Serial.println(serialData);
#endif
  
  request req;
  
  int len = serialData.length();
  char buff[len+1];
  serialData.toCharArray(buff, len+1);

  parseRequest(&req, buff);
   
  if(req.isVerified == 0)
    return;
#ifdef DEBUG_2
  else
    Serial.println("Do something with the valid data\n\n");
#endif

  //Do something with the data
  if(req.address != MY_ADDRESS)
    forwardMessage(req.address, req.message);
  else
    processReq(req);  
}

void Mert::processReq(request req)
{
#ifdef DEBUG_3
  Serial.print("cmd: ");
  Serial.println(req.cmd[0]);
  Serial.print("cmd to int: ");
  Serial.println((int)req.cmd[0]);
  Serial.print("#define update: ");
  Serial.println(UPDATE_CMD);
  Serial.print("#define update to int: ");
  Serial.println((int)UPDATE_CMD);
#endif

  switch((int)req.cmd[0])
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

void Mert::processUpdateCmd(request req)
{
#ifdef DEBUG_1
  Serial.println("Got an update Command");
#endif

  if(strcmp(req.key, SAMPLE_RATE_KEY) == 0)
    _sampleRate = atoi(req.value);
    
#ifdef DEBUG_1
  Serial.print("_sampleRate: ");
  Serial.println(_sampleRate);
#endif

}

void Mert::processRequestCmd(request req)
{
#ifdef DEBUG_1
  Serial.println("Got a request Command");
  Serial.print("Request Key: ");
  Serial.println(req.key);
#endif
  
  if(strcmp(req.key, TYPE_KEY) == 0)
  {
    char req[251];
    returnRequest(req, REQUEST_RESPONSE_CMD, TYPE_KEY, MY_TYPE);
  }
    
}

void Mert::returnRequest(char req[], char cmd[], char key[], char value[])
{
  strcpy(req, SOF);
  strcat(req, ",");
  char add[16];
  strcat(req, itoa(SERVER_ADDRESS, add, 10)); 
  strcat(req, ",");
  strcat(req, cmd);
  strcat(req, ",");
  strcat(req, key);
  strcat(req, ",");
  strcat(req, value);
  signed char sum = checksum(req);
  char strSum[8];
  itoa(sum, strSum, 10);
  strcat(req, ",");
  strcat(req, strSum);
  strcat(req, ",");
  strcat(req, EOF);
#ifdef DEBUG_1
  Serial.println("Send response message:");
  Serial.println(req);
#endif
Serial.println("END");
}


void Mert::forwardMessage(uint8_t address, char message[])
{
  Serial.print("Send: " );
  Serial.println(message);
  Serial.print("To address: ");
  Serial.println(address);
}


void Mert::parseRequest(request *req, char* str)
{
  strcpy(req->message, str);

#ifdef DEBUG_3
  Serial.print("Message: ");
  Serial.println(str);
#endif

  
  const char s[2] = ",";
  char *token;
  uint8_t i = 0;
  /* get the first token */
  token = strtok(str, s);
  /* walk through other tokens */

#ifdef DEBUG_3
  Serial.print(i);
  Serial.print(" - token: ");
  Serial.println(token);
#endif
  
  while( token != NULL ) 
  {
    if((i == 0) && (strcmp(token, SOF) != 0))
    {
      Serial.println(token);
      Serial.println("SOF bad!");
      return; 
    }
    
    switch(i)
    {
      case 1:
      {      
        char buff[strlen(token)];
        strcpy(buff, token);
        req->address = atoi(buff);
      }break;
      case 2:
        strcpy(req->cmd, token);
        break;
      case 3:
        strcpy(req->key, token);
      break;
      case 4:
        strcpy(req->value, token);
        break;
      case 5:
        strcpy(req->checksum, token);
        verifyChecksum(req, token);
        break;
      case 6:
        if(strcmp(token, EOF) == 0)
          req->fullTransmission = 1;
      default:
        break;
    }
    i++;
    token = strtok(NULL, s);
  }
    printRequestStruct(req);
}

uint8_t Mert::verifyChecksum(request *req, char *token)
{
  char add[8];
  itoa(req->address, add, 10);
  //for the 2 !! and the 4 commas
  uint8_t l1 = 6;
  uint8_t l2 = strlen(add);
  uint8_t l3 = strlen(req->cmd);
  uint8_t l4 = strlen(req->key);
  uint8_t l5 = strlen(req->value);
  uint8_t len = l1 + l2 + l3 + l4 + l5;
  char reqStr[len+1];
  
  strcpy(reqStr, "!!,");
  strcat(reqStr, add);
  strcat(reqStr, ",");
  strcat(reqStr, req->cmd);
  strcat(reqStr, ",");
  strcat(reqStr, req->key);
  strcat(reqStr, ",");
  strcat(reqStr, req->value);
  Serial.println(reqStr);
                  
  signed char sum = checksum(reqStr);
  char buff[8];
  itoa(sum, buff, 10);
#ifdef DEBUG_2
  Serial.print("Checksum converted to char[]: ");
  Serial.println(buff);
#endif
  if(strcmp(buff, token) == 0)
    req->isVerified = 1;  
}


void Mert::printRequestStruct(request *req)
{
#ifdef DEBUG_2
  Serial.print("Stored Request: ");
  Serial.println(req->message);
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
  Serial.print("Stored isVerified: ");
  Serial.println(req->isVerified);
  Serial.print("Stored fullTransmission: ");
  Serial.println(req->fullTransmission);
  Serial.println("\n");
#endif
}
