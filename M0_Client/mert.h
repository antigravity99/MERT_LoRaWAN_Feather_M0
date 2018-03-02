#include <Arduino.h>
#include <RH_RF95.h>
#include <RHReliableDatagram.h>

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
#define MY_ADDRESS 1

#define MY_TYPE "TEMP_MOTE"

/* for feather m0 */ 
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3

#define FREQ 915.0

#define DEBUG_1
#define DEBUG_2
#define DEBUG_3

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
    
class Mert 
{
  private:
    uint8_t _sampleRate = 10;
    String _inputString = "";
    boolean _stringComplete = false;
    // Dont put this on the stack:
    uint8_t _rcvBuf[RH_RF95_MAX_MESSAGE_LEN];
    //uint8_t data[] = "Hi Mom! Look, no hands!";
    // Singleton instance of the radio driver
    RH_RF95 _driver = RH_RF95(RFM95_CS, RFM95_INT);
    // Class to manage message delivery and receipt, using the driver declared above
    RHReliableDatagram _manager = RHReliableDatagram(_driver, MY_ADDRESS);
  
  public:
    Mert();
    char checksum(char* s);
    void checkSerial();
    void serialEvent(String serialData);
    void processReq(request req);
    uint8_t verifyChecksum(request *req, char *token);
    void processUpdateCmd(request req);
    void processRequestCmd(request req);
    void returnRequest(char key[], char value[]);
    void forwardMessage(uint8_t address, char message[]);
    void parseRequest(request *req, char* str);
    void printRequestStruct(request *req);
    bool sendtoWait(uint8_t* buf, uint8_t len, uint8_t address);
    bool recvfromAckTimeout();
    bool managerInit();
};
