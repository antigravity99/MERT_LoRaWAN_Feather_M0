#include <Arduino.h>

#define HUMIDITY_KEY "HUMIDITY"
#define TEMP_KEY "TEMP"
#define TYPE_KEY "TYPE"
#define VIBRATION_KEY "VIBRATION"
#define SAMPLE_RATE_KEY "SAMPLE_RATE"

#define REQUEST_RESPONSE "REQ_RES"
#define SOF "!!"
#define EOF "**\n"

#define REQUEST_CMD 'R'
#define UPDATE_CMD 'U'

#define SERVER_ADDRESS 0
#define MY_ADDRESS 0

#define MY_TYPE "TEMP_MOTE"

#define DEBUG_1
//#define DEBUG_2
//#define DEBUG_3

uint8_t _sampleRate = 10;

typedef struct request
{
  char message[255];
  uint8_t address = -1;
  char cmd[2];
  char key[63];
  char value[63];
  char checksum[15];
  uint8_t isVerified = 0;
  uint8_t fullTransmission = 0;
} request;


void setup() 
{
  Serial.begin(9600);
}
String inputString = "";
boolean stringComplete = false;

void loop() 
{
  while (Serial.available()) {
    if(stringComplete)
    {
      inputString = "";
      stringComplete = false;
    }
      
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
//    Serial.println(inputString);
    if (inChar == '\n') 
    {
      stringComplete = true;
      serialEvent(inputString);  
    }
  }
}

/** Quick and dirty checksum **/
char checksum(char* s)
{
  signed char sum = -1;
  while (*s != 0)
  {
    sum += *s;
    s++;
  }
  return sum;
}

void serialEvent(String serialData)
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

void processReq(request req)
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
    case (int)UPDATE_CMD:
      processUpdateCmd(req);
      break;
    case (int)REQUEST_CMD:
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

void processUpdateCmd(request req)
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

void processRequestCmd(request req)
{
#ifdef DEBUG_1
  Serial.println("Got a request Command");
  Serial.print("Request Key: ");
  Serial.println(req.key);
#endif
  
  if(strcmp(req.key, TYPE_KEY) == 0)
    returnRequest(TYPE_KEY, MY_TYPE);
}

void returnRequest(char key[], char value[])
{
  char buff[255];
  strcpy(buff, SOF);
  strcat(buff, ",");
  char add[16];
  strcat(buff, itoa(SERVER_ADDRESS, add, 10)); 
  strcat(buff, ",");
  strcat(buff, REQUEST_RESPONSE);
  strcat(buff, ",");
  strcat(buff, key);
  strcat(buff, ",");
  strcat(buff, value);
  signed char sum = checksum(buff);
  char strSum[8];
  itoa(sum, strSum, 10);
  strcat(buff, ",");
  strcat(buff, strSum);
  strcat(buff, ",");
  strcat(buff, EOF);
#ifdef DEBUG_1
  Serial.println("Send response message:");
  Serial.println(buff);
#endif
Serial.println("END");
}


void forwardMessage(uint8_t address, char message[])
{
  Serial.print("Send: " );
  Serial.println(message);
  Serial.print("To address: ");
  Serial.println(address);
}


void parseRequest(request *req, char* str)
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

uint8_t verifyChecksum(request *req, char *token)
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


void printRequestStruct(request *req)
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
