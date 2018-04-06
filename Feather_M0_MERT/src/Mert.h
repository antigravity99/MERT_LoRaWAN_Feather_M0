#include <Arduino.h>
#include <RHReliableDatagram.h>
#include <RH_RF95.h>
#include <ArduinoJson.h>
#include "Adafruit_ADXL345_U.h"
#include "Adafruit_TMP007.h"
#include "Adafruit_Sensor.h"
// #include <Wire.h>
// #include <SPI.h>

//Debugging Serial prints
// #define DEBUG_1
// #define DEBUG_2
#define DEBUG_3

/* for feather m0 */
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3

#define SERVER_ADDRESS 99
//LoRa Radio frequency
#define FREQ 915.0

// TMP007 I2C address is 0x40(64)
#define TMP007_ADDRESS 0x40
//Temp data register
#define TMP_DATA_REG 0x03
//Adafruit_ADXL345 Accelerometer ID
// #define ADXL345_ADDRESS 12345


//Dip switch pins for addressing
#define DIP_1 12
#define DIP_2 11
#define DIP_3 10
#define DIP_4 6

#define HUMIDITY_KEY "HUMIDITY"
#define TEMP_IR_KEY "TEMP_IR"
#define TEMP_DIE_KEY "TEMP_DIE"
#define TYPE_KEY "TYPE"
#define VIBRATION_KEY "VIB"
#define SAMPLE_RATE_KEY "SAMPLE_RATE"

#define SERVER_VALUE "Server"
#define MOTE_VALUE "Mote"

//Acknowledgement reply
#define ACK "ack"

//Command types
#define REQUEST_CMD "R"
#define UPDATE_CMD "U"
#define SEND_CMD "S"
#define REQUEST_RESPONSE_CMD "Q"
//Json keys
#define ADDRESS "Add"
#define CMD "Cmd"
#define KEY "Key"
#define VALUE "Val"
// #define CHECKSUM "Checksum"

#define VIB_SAMPLES 65

//Request structure - Json will be deserialized to this
typedef struct
{
  uint8_t address = -1;
  String cmd;
  String key;
  String value;
  uint16_t *vibBuff;
  // String checksum;
} request_t;

typedef struct
{
  float irTemp;
  float dieTemp;
} temp_t;

//Mert communication class
class Mert
{

  private:
    //Privarte class variables
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
    Adafruit_ADXL345_Unified _accel;
    Adafruit_TMP007 _tmp007;

    //Private method
    char checksum(char* s);
    String serailizeRequest(request_t req);

  public:
    //public methods
    Mert();
    void init(bool isServer);
    String getMoteType();
    uint8_t getMoteAddress();
    void checkSerial();
    void serialEvent(String serialData);
    void processReq(request_t req);
    void verifyChecksum(request_t *req, char *token);
    void processUpdateCmd(request_t req);
    void processRequestCmd(request_t req);
    // void returnRequest(char req[], char cmd[], char key[], char value[]);
    void forwardMessage(uint8_t address, char message[]);
    // void parseRequest(request *req, char* str);//maybe able to delete this
    void parseJsonRequest(request_t *req, char* str);
    void printRequestStruct(request_t *req);
    bool sendtoWait(request_t req);
    bool recvfromAckTimeout(request_t *req, String *json);
    bool recvfromAck(request_t *req);
    bool managerInit();
    uint8_t getAddress();
    temp_t getTemp();
    uint16_t* getAccelMagArray();

};
