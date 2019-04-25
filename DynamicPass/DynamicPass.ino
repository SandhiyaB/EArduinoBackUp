#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

ESP8266WebServer server(80);

const char* ssid = "Tictoks_V1" ;
const char* passphrase = "12345678";
String st;
String content;
int statusCode;
String esid;
String epass = "";

void setup() {
  Serial.begin(9600);
  EEPROM.begin(512);

  delay(10);
  Serial.println();
  Serial.println();
  Serial.println("System Online");
  // read eeprom for ssid and pass
  Serial.println("Reading ssid");
  for (int i = 0; i < 32; ++i)
  {
    esid += char(EEPROM.read(i));
  }
  Serial.print("SSID: ");
  Serial.println(esid);
  Serial.println("Reading  password");
  for (int i = 32; i < 96; ++i)
  {
    epass += char(EEPROM.read(i));
  }
  Serial.print("PASS: ");
  Serial.println(epass);

  
  if ( esid.length() > 1 ) {
  /*  WiFi.persistent(false);
    WiFi.mode(WIFI_OFF);   // this is a temporary line, to be removed after SDK update to 1.5.4
    WiFi.mode(WIFI_STA);*/
    
    WiFi.begin(esid.c_str(), epass.c_str());
     if (testWifi()) {
      Serial.println("  LAUNCH WEB");
      launchWeb(0);
      return;
    }
  }


  setupAP();

}

bool testWifi(void) {
  int c = 0;
  Serial.println("Waiting for Wifi to connect to ESP8266");
  while ( c < 20 ) {
 /*   WiFi.begin(esid.c_str(), epass.c_str());

    // if(WiFi.localIP().toString() == "0.0.0.0"){
     Serial.println(" connectin %s" + WiFi.isConnected());
   */ 
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println(" WIFI Status Connected %s" + WiFi.isConnected());
      return true;
    }
    delay(500);
    Serial.println(" WIFI Statusss notConnected %s" + WiFi.status());

    Serial.println(WiFi.status());
    c++;
  }
  Serial.println("");
  Serial.println("Connect timed out, opening AP");
  return false;
}

void launchWeb(int webtype) {
  Serial.println("" + webtype);
  Serial.println("WiFi not connected");
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("SoftAP IP: ");
  Serial.println(WiFi.softAPIP());
  createWebServer(webtype);
  // Start the server
  server.begin();
  Serial.println("Server started");
}

void setupAP(void) {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0)
    Serial.println("no networks found");
  else
  {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
      delay(10);
    }
  }
  Serial.println("");
  st = "<ol>";
  for (int i = 0; i < n; ++i)
  {
    // Print SSID and RSSI for each network found
    st += "<li>";
    st += WiFi.SSID(i);
    st += " (";
    st += WiFi.RSSI(i);
    st += ")";
    st += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*";
    st += "</li>";
  }
  st += "</ol>";
  delay(100);
  WiFi.softAP(ssid, passphrase, 6);
  Serial.println("softap");
  launchWeb(1);
  Serial.println("over");
}

void createWebServer(int webtype)
{
  webtype = 1;
  Serial.println("Inside CreateWebserver" + webtype);
  if ( webtype == 1 ) {
    Serial.println("WebType " + webtype);
    server.on("/", []() {
      IPAddress ip = WiFi.softAPIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
      content = "<!DOCTYPE HTML>\r\n<html>WiFi configuration page for :  ";
      content += ipStr;
      content += "<p>";
      content += st;
      content += "</p><form method='get' action='setting'><label>SSID: </label><input name='ssid' length=32><input name='pass' length=64><input type='submit'></form>";
      content += "</html>";
      server.send(200, "text/html", content);
    });
    server.on("/setting", []() {
      String qsid = server.arg("ssid");
      String qpass = server.arg("pass");
      if (qsid.length() > 0 && qpass.length() > 0) {
        Serial.println("clearing eeprom");
        for (int i = 0; i < 96; ++i) {
          EEPROM.write(i, 0);
        }
        Serial.println(qsid);
        Serial.println("");
        Serial.println(qpass);
        Serial.println("");

        Serial.println("writing eeprom ssid:");
        for (int i = 0; i < qsid.length(); ++i)
        {
          EEPROM.write(i, qsid[i]);
          Serial.print("Wrote: ");
          Serial.println(qsid[i]);
        }
        Serial.println("writing eeprom pass:");
        for (int i = 0; i < qpass.length(); ++i)
        {
          EEPROM.write(32 + i, qpass[i]);
          Serial.print("Wrote: ");
          Serial.println(qpass[i]);
        }
        EEPROM.commit();
        content = "{\"Success\":\"saved to eeprom....... reset to boot into new wifi\"}";
        statusCode = 200;

        String esid;
        for (int i = 0; i < 32; ++i)
        {
          esid += char(EEPROM.read(i));
        }
        Serial.print("SSID: ");
        Serial.println(esid);
        Serial.println("Reading  password");
        String epass = "";
        for (int i = 32; i < 96; ++i)
        {
          epass += char(EEPROM.read(i));
        }
        Serial.print("PASS: ");
        Serial.println(epass);
        if ( esid.length() > 1 ) {
          WiFi.begin(esid.c_str(), epass.c_str());
          Serial.println("Wifi st 12323 %d" + testWifi());
          Serial.println("wifi name %s" + esid);
          Serial.println("wifi password %s" + epass);
          if (!testWifi()) {
            launchWeb(0);
            return;
          }
        }

      } else {
        content = "{\"Error\":\"404 not found\"}";
        statusCode = 404;
        Serial.println("Sending 404");
      }
      server.send(statusCode, "application/json", content);
    });
  } else if (webtype == 0) {
    server.on("/", []() {
      IPAddress ip = WiFi.localIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
      server.send(200, "application/json", "{\"IP\":\"" + ipStr + "\"}");
    });
    server.on("/cleareeprom", []() {
      content = "<!DOCTYPE HTML>\r\n<html>";
      content += "<p>Clearing the EEPROM</p></html>";
      server.send(200, "text/html", content);
      Serial.println("clearing eeprom");
      for (int i = 0; i < 96; ++i) {
        EEPROM.write(i, 0);
      }
      EEPROM.commit();
      ESP.restart();
    });
  }
}

void loop() {
  server.handleClient();
}
