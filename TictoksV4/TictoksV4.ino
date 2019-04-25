#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

#include <Keypad.h>
#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>

//Sqllite SdFat

#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <vfs.h>
#include <SPI.h>
#include <FS.h>
extern "C" {
#include "user_interface.h"
}

SoftwareSerial mySerial(13, 15);

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
ESP8266WebServer server(80);

const char* ssid = "Tictoks_V1" ;
const char* passphrase = "12345678";
String st;
String content;
int statusCode;

//uint8_t id;
int id;

const byte n_rows = 4;
const byte n_cols = 4;
int num[] = {0, 0, 0, 0};
char keys[n_rows][n_cols] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte colPins[n_rows] = {D3, D2, D1, D0};
byte rowPins[n_cols] = {D7, D6, D5, D4};
int i = 0;
int firstvariable;
Keypad myKeypad = Keypad( makeKeymap(keys), rowPins, colPins, n_rows, n_cols);

void WiFiOff() {
   wifi_station_disconnect();
   wifi_set_opmode(NULL_MODE);
   wifi_set_sleep_type(MODEM_SLEEP_T);
   wifi_fpm_open();
   wifi_fpm_do_sleep(0xFFFFFFF);
}
void setup() {
  Serial.begin(9600);
   EEPROM.begin(512);

  delay(10);
  while (!Serial);  // For Yun/Leo/Micro/Zero/...
  delay(100);
  Serial.println();
  Serial.println();
  Serial.println("System Online");
  // read eeprom for ssid and pass
  Serial.println("Reading ssid");
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
    //WiFi.disconnect();
    if (!testWifi()) {
       //setupAP();

      launchWeb(0);
      return;
    }
  }
  
 Serial.println("\n\nAdafruit Fingerprint sensor enrollment");

  // set the data rate for the sensor serial port

  finger.begin(57600);

    if(finger.verifyPassword()){
    Serial.println("Found fingerprint sensor!");
      } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) {
      delay(1);
    }
  }
  
  //Sqllite

  sqlite3 *db1;
  sqlite3 *db2;
  int rc;

  system_update_cpu_freq(SYS_CPU_80MHZ);
  //WiFiOff();

  if (!SPIFFS.begin()) {
    Serial.println("Failed to mount file system");
    return;
  }

  // list SPIFFS contents
  Dir dir = SPIFFS.openDir("/");
  while (dir.next()) {
    String fileName = dir.fileName();
    size_t fileSize = dir.fileSize();
    Serial.printf("FS File: %s, size: %ld\n", fileName.c_str(), (long) fileSize);
  }
  Serial.printf("\n");

  // remove existing file
  SPIFFS.remove("/EmployeeTable.db");
  SPIFFS.remove("/Offline.db");
  sqlite3_initialize();
  File db_file_obj_1;
  vfs_set_spiffs_file_obj(&db_file_obj_1);
  if (db_open("/FLASH/EmployeeTable.db", &db1))
    return;
  File db_file_obj_2;
  vfs_set_spiffs_file_obj(&db_file_obj_2);
  if (db_open("/FLASH/Offline.db", &db2))
    return;
    
  rc = db_exec(db1, "create table if not exists EmployeeTable  (Id INTEGER PRIMARY KEY, EmployeeId integer,FingerPrint1 BLOB,RFIDNo text);");
   if (rc != SQLITE_OK) {
       //sqlite3_close(db1);
       //sqlite3_close(db2);
       Serial.println("EmployeeTable Not Created");
       return;
   }
   /*
  rc = db_exec(db2, "CREATE TABLE  if not exists Offline (Id INTEGER PRIMARY KEY, Data text,CheckInOut text);");
   if (rc != SQLITE_OK) {
       //sqlite3_close(db1);
       //sqlite3_close(db2);
       Serial.println("Offline Table Not Created");
       
       return;
   }
   */
 
  
}

void loop() {
  char key1 = myKeypad.getKey();
  switch (key1) {
    case NO_KEY:
      break;

    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
      //lcd.print(key1);
      if (i == 3) {
        i = 0;
        firstvariable = firstvariable * 10 + key1 - '0';

        Serial.println("4 digit num ");
        Serial.print (firstvariable);
        Serial.print("Enrolling ID #");
        id = firstvariable;
        Serial.println(id);
        finger.getImage();
        while (!  getFingerprintEnroll() );

        firstvariable = 0;

      } else {
        firstvariable = firstvariable * 10 + key1 - '0';
        Serial.println("i value ");
        Serial.print (i);

        i = i + 1;
      }
      break;

    case '#':
      Serial.println ();
      Serial.print ("Value for 12V is: ");
      Serial.print (firstvariable );
      break;

    case '*':
      firstvariable = 0;
      // lcd.clear();
      Serial.println ();
      Serial.print ("Reset value:");
      Serial.print (firstvariable );
  }
  /*Serial.print("Enrolling ID #");
    Serial.println(id);
    finger.getImage();
     while (!  getFingerprintEnroll() );
     i=0;
*/

    
  

}
uint8_t getFingerprintEnroll() {

  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #"); Serial.println(id);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    Serial.print("p Status");
    Serial.print(String(p));
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.println(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        break;
      default:
        Serial.println("Unknown error");
        break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  Serial.print("p Status 2nd ");
  Serial.println(String(p));
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  Serial.println("Remove finger");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID "); Serial.println(id);
  p = -1;
  Serial.println("Place same finger again");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.print(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        break;
      default:
        Serial.println("Unknown error");
        break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  Serial.print("Creating model for #");  Serial.println(id);

  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  Serial.print("ID "); Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  p = finger.loadModel(id);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.print("Template AFEER "); Serial.print(id); Serial.println(" loaded");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    default:
      Serial.print("Unknown error "); Serial.println(p);
      return p;
  }
  Serial.print("Attempting to get #"); Serial.println(id);
  p = finger.getModel();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.print("Template "); Serial.print(id); Serial.println(" transferring:");
      break;
    default:
      Serial.print("Unknown error "); Serial.println(p);
      return p;
  }

  // one data packet is 267 bytes. in one data packet, 11 bytes are 'usesless' :D
  uint8_t bytesReceived[534]; // 2 data packets
  memset(bytesReceived, 0xff, 534);

  uint32_t starttime = millis();
  int i = 0;
  while (i < 534 && (millis() - starttime) < 20000) {
    if (mySerial.available()) {
      bytesReceived[i++] = mySerial.read();
    }
  }
  Serial.print(i); Serial.println(" bytes read.");
  Serial.println("Decoding packet...");

  uint8_t fingerTemplate[512]; // the real template
  memset(fingerTemplate, 0xff, 512);

  // filtering only the data packets
  int uindx = 9, index = 0;
  while (index < 534) {
    while (index < uindx) ++index;
    uindx += 256;
    while (index < uindx) {
      fingerTemplate[index++] = bytesReceived[index];
    }
    uindx += 2;
    while (index < uindx) ++index;
    uindx = index + 9;
  }
  for (int i = 0; i < 512; ++i) {
    //Serial.print("0x");
    printHex(fingerTemplate[i], 2);
    //Serial.print(", ");
  }
  Serial.println("\ndone.");

}
void printHex(int num, int precision) {
  char tmp[16];
  char format[128];

  sprintf(format, "%%.%dX", precision);

  sprintf(tmp, format, num);
  Serial.print(tmp);
}

//WIFI

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


void launchWeb(int webtype) {
  Serial.println("" + webtype);
  Serial.println("WiFi connected");
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("SoftAP IP: ");
  Serial.println(WiFi.softAPIP());
  createWebServer(webtype);
  // Start the server
  server.begin();
  Serial.println("Server started");
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



bool testWifi(void) {
  int c = 0;
  Serial.println("Waiting for Wifi to connect to ESP8266");
  while ( c < 20 ) {
    if (WiFi.status() == WL_CONNECTED) {
      return true;
    }
    delay(500);
    Serial.print(WiFi.status());
    c++;
  }
  Serial.println("");
  Serial.println("Connect timed out, opening AP");
  return false;
}



//SqlLite

const char* data = "Callback function called";
static int callback(void *data, int argc, char **argv, char **azColName) {
  int i;
  Serial.printf("%s: ", (const char*)data);
  for (i = 0; i < argc; i++) {
    Serial.printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
  }
  Serial.printf("\n");
  return 0;
}

int db_open(const char *filename, sqlite3 **db) {
  int rc = sqlite3_open(filename, db);
  if (rc) {
    Serial.printf("Can't open database: %s\n", sqlite3_errmsg(*db));
    return rc;
  } else {
    Serial.printf("Opened database successfully\n");
  }
  return rc;
}

char *zErrMsg = 0;
int db_exec(sqlite3 *db, const char *sql) {
  Serial.println(sql);
  long start = micros();
  int rc = sqlite3_exec(db, sql, callback, (void*)data, &zErrMsg);
  if (rc != SQLITE_OK) {
    Serial.printf("SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  } else {
    Serial.printf("Operation done successfully\n");
  }
  Serial.print(F("Time taken:"));
  Serial.println(micros() - start);
  return rc;
}
