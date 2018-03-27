#include <Arduino.h>
#include <RHReliableDatagram.h>
#include <RH_RF95.h>
#include <ArduinoJson.h>
#include "Adafruit_ADXL345_U.h"
#include "Adafruit_TMP007.h"
#include "Adafruit_Sensor.h"
#include <Wire.h>
// #include <SPI.h>

//Debugging Serial prints
#define DEBUG_1
#define DEBUG_2
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
#define ADXL345_ADDRESS 12345


//Dip switch pins for addressing
#define PIN_1 10
#define PIN_2 11
#define PIN_3 12
#define PIN_4 13

#define HUMIDITY_KEY "HUMIDITY"
#define TEMP_IR_KEY "TEMP_IR"
#define TEMP_DIE_KEY "TEMP_DIE"
#define TYPE_KEY "TYPE"
#define VIBRATION_KEY "VIBRATION"
#define SAMPLE_RATE_KEY "SAMPLE_RATE"
#define INIT_KEY "INIT"

#define SERVER_VALUE "Server"
#define MOTE_VALUE "Mote"
#define INIT_VALUE "INIT";

//Acknowledgement reply
#define ACK "ack"

//Command types
#define REQUEST_CMD "R"
#define UPDATE_CMD "U"
#define SEND_CMD "S"
#define REQUEST_RESPONSE_CMD "Q"
//Json keys
#define ADDRESS "Address"
#define CMD "Cmd"
#define KEY "Key"
#define VALUE "Value"
#define CHECKSUM "Checksum"

//Request structure - Json will be deserialized to this
typedef struct Request
{
  uint8_t address = -1;
  String cmd;
  String key;
  String value;
  String checksum;
} Request;

typedef struct Temp
{
  float irTemp;
  float dieTemp;
} Temp;

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
    uint8_t serailizeRequest(Request req, char *buff);

  public:
    //public methods
    Mert();
    void init(bool isServer);
    String getMoteType();
    uint8_t getMoteAddress();
    void checkSerial();
    void serialEvent(String serialData);
    void processReq(Request req);
    void verifyChecksum(Request *req, char *token);
    void processUpdateCmd(Request req);
    void processRequestCmd(Request req);
    // void returnRequest(char req[], char cmd[], char key[], char value[]);
    void forwardMessage(uint8_t address, char message[]);
    // void parseRequest(request *req, char* str);//maybe able to delete this
    void parseJsonRequest(Request *req, char* str);
    void printRequestStruct(Request *req);
    bool sendtoWait(Request req);
    bool recvfromAckTimeout(Request *req, char* json);
    bool recvfromAck(Request *req);
    bool managerInit();
    uint8_t getAddress();
    Temp getTemp();
    uint16_t getAccelMag();

};
