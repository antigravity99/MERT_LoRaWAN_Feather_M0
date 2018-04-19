#include "Reza.h"

#define SERVER


int count = 0;
//
void server();
void client();
uint8_t getAddress();

Adafruit_TMP007 tmp007;
Reza reza;

void setup()
{
  delay(5000);
#ifdef SERVER
  reza.init(true);
#else
  reza.init(false);
#endif

  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);
  //dip switch address pins
  pinMode(DIP_1, INPUT_PULLUP);
  pinMode(DIP_2, INPUT_PULLUP);
  pinMode(DIP_3, INPUT_PULLUP);
  pinMode(DIP_4, INPUT_PULLUP);
  // pinMode(13, INPUT_PULLUP);
  // pinMode(5, OUTPUT);
  // digitalWrite(5, HIGH);

  Serial.begin(9600);
  //while (!Serial) ; // Wait for serial port to be available
  if(!reza.managerInit())
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
  request_t req;
  String json;
  if (reza.recvfromAckTimeout(&req, &json))
  {
    // reza.printRequestStruct(&req);
    Serial.println(json);
  }
  reza.checkSerial();
  // delay(50);
}
#else
void client()
{
  uint16_t *vibBuff = reza.getAccelMagArray();

  request_t reqAccel;
  reqAccel.address = reza.getMoteAddress();
  reqAccel.cmd = SEND_CMD;
  reqAccel.key = VIBRATION_KEY;
  reqAccel.value = "";
  reqAccel.vibBuff = vibBuff;

  reza.sendtoWait(reqAccel);

  temp_t temp = reza.getTemp();

  request_t reqDie;
  reqDie.address = reza.getMoteAddress();
  reqDie.cmd = SEND_CMD;
  reqDie.key = TEMP_DIE_KEY;
  reqDie.value = String(temp.dieTemp);

  reza.sendtoWait(reqDie);

  request_t reqIR;
  reqIR.address = reza.getMoteAddress();
  reqIR.cmd = SEND_CMD;
  reqIR.key = TEMP_IR_KEY;
  reqIR.value = String(temp.irTemp);

  reza.sendtoWait(reqIR);

  // Serial.print("IR temp: "); Serial.println(t.irTemp);
  // Serial.print("Die temp: "); Serial.println(t.dieTemp);

  // Send a message to manager_server
  // if (reza.sendtoWait(req))
  // {
  //
  // }
  delay(3000);
  reza.checkSerial();
}
#endif
