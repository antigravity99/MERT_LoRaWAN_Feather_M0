#include <Arduino.h>
bool done = false;

void setup() {
Serial.begin(9600);
delay(6000);
}

void loop() {

if(!done)
{
  Serial.print("Size of uint16_t: ");
  Serial.println(sizeof(uint16_t));
  Serial.print("Size of char arr: ");
  char *arr = "1024";
  Serial.println(sizeof(arr));
delay(20000);
  for(uint16_t i = 0; i<1024; i++)
  {
    char lo = i & 0xFF;
    char hi = i >> 8;
    Serial.print("\ni = ");
    Serial.print(i);
    Serial.print(" lo = ");
    Serial.print(lo);
    Serial.print(" hi = ");
    Serial.print(hi);
     uint16_t value = lo | uint16_t(hi) << 8;
    Serial.print(" value = ");
    Serial.println(value);
    // delay(2000);
    done = true;
  }

}

}
