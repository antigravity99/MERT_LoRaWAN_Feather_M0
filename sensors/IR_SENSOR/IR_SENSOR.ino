// Distributed with a free-will license.
// Use it any way you want, profit or free, provided it fits in the licenses of its associated works.
// TMP007
// This code is designed to work with the TMP007_I2CS I2C Mini Module available from ControlEverything.com.
// https://www.controleverything.com/content/Temperature?sku=TMP007_I2CS#tabs-0-product_tabset-2

#include<Wire.h>

// TMP007 I2C address is 0x40(64)
#define Addr 0x40

void setup() 
{
  // Initialise I2C communication as MASTER 
  Wire.begin();
  // Initialise serial communication, set baud rate = 9600
  Serial.begin(9600);

  // Start I2C Transmission
  Wire.beginTransmission(Addr);
  // Select configuration register
  Wire.write(0x02);
  // Send config command
  Wire.write(0x15);
  Wire.write(0x40);
  // Stop I2C Transmission
  Wire.endTransmission();
  delay(300);  
}

void loop()
{
  unsigned data[2];
  
  // Start I2C Transmission
  Wire.beginTransmission(Addr);
  // Select object temp data register
  Wire.write(0x03);
  // Stop I2C Transmission
  Wire.endTransmission();
  delay(300);
  
  // Request 2 bytes of data
  Wire.requestFrom(Addr, 2);
  
  // Read 2 bytes of data
  // temp msb, temp lsb
  if(Wire.available() == 2)
  {
    data[0] = Wire.read();
    data[1] = Wire.read();
  }
  
  // Convert the data to 14-bits
  int temp = (((data[0] * 256) + (data[1] & 0xFC)) / 4);
  if (temp > 8191)
  {
    temp -= 16384;
  }
  float cTemp = temp * 0.03125;
  float fTemp = (cTemp * 1.8) + 32;
  
  // Output data to serial monitor
  Serial.print("Object Temperature in Celsius : ");
  Serial.print(cTemp);
  Serial.println(" C");
  Serial.print("Object Temperature in Fahrenheit : ");
  Serial.print(fTemp);
  Serial.println(" F");
  delay(500); 
}
