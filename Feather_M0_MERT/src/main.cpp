#include "Mert.h"

// #define SERVER

int count = 0;

void server();
void client();
uint8_t getAddress();

Adafruit_TMP007 tmp007;
Mert mert;

void setup()
{
  delay(5000);
#ifdef SERVER
  mert.init(true);
#else
  mert.init(false);
#endif

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
    // Serial.print("\rloop ");
    // Serial.print(count++);
#ifdef SERVER
  server();
#else
  client();
#endif

}

#ifdef SERVER
void server()
{
  Request req;
  String json;
  if (mert.recvfromAckTimeout(&req, &json))
  {
    // mert.printRequestStruct(&req);
    Serial.println(json);
  }
  mert.checkSerial();
  // delay(50);
}
#else
void client()
{
  // Temp t = mert.getTemp();
  // uint16_t m = mert.getAccelMag();

  String accelArr = mert.getAccelMag();



  Request req;
  req.address = mert.getMoteAddress();
  req.cmd = SEND_CMD;
  req.key = VIBRATION_KEY;
  req.value = accelArr;
  // req.key = TEMP_IR_KEY;
  // req.value = String(t.irTemp);


  // Serial.print("IR temp: "); Serial.println(t.irTemp);
  // Serial.print("Die temp: "); Serial.println(t.dieTemp);
  // Serial.print("Accel magnitude: "); Serial.println(m);

  // Send a message to manager_server
  if (mert.sendtoWait(req))
  {

  }
  delay(1000);
  mert.checkSerial();
}
#endif
