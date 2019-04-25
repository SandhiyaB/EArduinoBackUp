#include <ESP8266WiFi.h>
#include <time.h>

#include <SPI.h>
#include <Wire.h>


const char* ssid = "Sandhiya";
const char* password = "shinchan";

int ledPin = 13;

int timezone = 7 * 3600;
int dst = 0;



void setup() {

  
  pinMode(ledPin,OUTPUT);
  digitalWrite(ledPin,LOW);

  Serial.begin(115200);

 
  
  
  WiFi.begin(ssid,password);
 
  
  while( WiFi.status() != WL_CONNECTED ){
      delay(500);
     }

  configTime(timezone, dst, "pool.ntp.org","time.nist.gov");
  
  while(!time(nullptr)){
     Serial.print("*");
     
     delay(1000);
  delay(1000);

 }
}

void loop() {
  
  time_t now = time(nullptr);
  struct tm* p_tm = localtime(&now);
  
  Serial.print(p_tm->tm_mday);
  Serial.print("/");
  Serial.print(p_tm->tm_mon + 1);
  Serial.print("/");
  Serial.print(p_tm->tm_year + 1900);
  
  Serial.print(" ");
  
  Serial.print(p_tm->tm_hour);
  Serial.print(":");
  Serial.print(p_tm->tm_min);
  Serial.print(":");
  Serial.println(p_tm->tm_sec);
  
  
  delay(1000); // update every 1 sec

}
