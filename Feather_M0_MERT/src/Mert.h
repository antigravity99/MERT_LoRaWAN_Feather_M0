#include <Arduino.h>
#include <RHReliableDatagram.h>
#include <RH_RF95.h>
#include <ArduinoJson.h>


#define HUMIDITY_KEY "HUMIDITY"
#define TEMP_KEY "TEMP"
#define TYPE_KEY "TYPE"
#define VIBRATION_KEY "VIBRATION"
#define SAMPLE_RATE_KEY "SAMPLE_RATE"

#define SERVER_TYPE "Server"
#define AMB_TEMP_TYPE "Ambient_Temp"


// #define SOF "!!"
// #define EOF "**\n"

#define REQUEST_CMD "R"
#define UPDATE_CMD "U"
#define SEND_CMD "S"
#define REQUEST_RESPONSE_CMD "Q"
//Json keys
#define ADDRESS "address"
#define CMD "cmd"
#define KEY "key"
#define VALUE "value"
#define CHECKSUM "checksum"

#define SERVER_ADDRESS 0
#define MY_ADDRESS 1

#define MY_TYPE AMB_TEMP_TYPE //"SERVER"

/* for feather m0 */
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3

#define FREQ 915.0

#define DEBUG_1
#define DEBUG_2
#define DEBUG_3

// typedef struct request
// {
//   char message[255];
//   uint8_t address = -1;
//   char cmd[2];
//   char key[63];
//   char value[63];
//   char checksum[15];
//   uint8_t isVerified = 0;
//   uint8_t fullTransmission = 0;
// } request;

typedef struct ReqJson
{
  uint8_t address = -1;
  String cmd;
  String key;
  String value;
  String checksum;
} ReqJson;

class Mert
{

  private:
    String _moteType;
    uint8_t _moteAddress;
    uint8_t _sampleRate = 10;
    String _inputString = "";
    boolean _stringComplete = false;
    // Singleton instance of the radio driver
    RH_RF95 _driver = RH_RF95(RFM95_CS, RFM95_INT);
    // Class to manage message delivery and receipt, using the driver declared above
    RHReliableDatagram _manager = RHReliableDatagram(_driver, SERVER_ADDRESS);
    // Dont put this on the stack:
    uint8_t _rcvBuf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t _sendBuff[256];

  public:
    Mert();
    void init(String moteType, uint8_t moteAddress);
    String getMoteType();
    uint8_t getMoteAddress();
    void checkSerial();
    char checksum(char* s);
    void serialEvent(String serialData);
    void processReq(ReqJson req);
    void verifyChecksum(ReqJson *req, char *token);
    void processUpdateCmd(ReqJson req);
    void processRequestCmd(ReqJson req);
    // void returnRequest(char req[], char cmd[], char key[], char value[]);
    void forwardMessage(uint8_t address, char message[]);
    // void parseRequest(request *req, char* str);//maybe able to delete this
    void parseJsonRequest(ReqJson *req, char* str);
    void printRequestStruct(ReqJson *req);
    bool sendtoWait(ReqJson data);
    bool recvfromAckTimeout();
    bool recvfromAck();
    bool managerInit();

};
