#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
 WiFiManager wifiManager;
void setup() {
    // put your setup code here, to run once:
    Serial.begin(9600);

    //WiFiManager
    //Local intialization. Once its business is done, there is no need to keep it around
   
    //reset saved settings
    //wifiManager.resetSettings();
    
    //set custom ip for portal
    //wifiManager.setAPStaticIPConfig(IPAddress(192,168,43,12), IPAddress(192,168,43,1), IPAddress(255,255,255,0));

    //fetches ssid and pass from eeprom and tries to connect
    //if it does not connect it starts an access point with the specified name
    //here  "AutoConnectAP"
    //and goes into a blocking loop awaiting configuration
    wifiManager.autoConnect("AutoConnectAP");
    //or use this for auto generated name ESP + ChipID
    //wifiManager.autoConnect();

    
    //if you get here you have connected to the WiFi
    Serial.println("connected...yeey :)");
}

void loop() {
    // put your main code here, to run repeatedly:
   Serial.println("resetar"); //tenta abrir o portal
      if(!wifiManager.startConfigPortal("ESP_AP", "12345678") ){
        Serial.println("Falha ao conectar");
        delay(2000);
        ESP.restart();
        delay(1000);
      }
      Serial.println("Conectou ESP_AP!!!");
    Serial.println("connected...yeey1 :)");
}
