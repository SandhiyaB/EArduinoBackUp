// Library FOr Wifi
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <EEPROM.h>

//Library For SD CARD CSV
#include <SdFat.h>
#include <CSVFile.h>

//Library for LCD
#include <LiquidCrystal_I2C.h>

//For Keypad and Bio
#include <Keypad.h>
#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>
SoftwareSerial mySerial(13, 15);

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

//For Wifi Connection
const char* ssid = "Tictoks_V1" ;
const char* passphrase = "12345678";
String st;
String content;
int statusCode;
String esid;
String epass = "";


ESP8266WebServer server(80);
WiFiClient client;
char servername[] = "google.com";

//For Sending Data via Wifi

const char *host = "192.168.43.128";   //https://circuits4you.com website or IP address of server
StaticJsonBuffer<300> JSONbuffer;
JsonObject& JSONencoder = JSONbuffer.createObject();

//For SD card and CSV
// SPI pinout

#define PIN_SPI_CLK 14
#define PIN_SPI_MOSI 13
#define PIN_SPI_MISO 12
#define PIN_SD_CS 15

#define PIN_OTHER_DEVICE_CS -1
#define SD_CARD_SPEED SPI_QUARTER_SPEED

#define FILENAME "test5.csv"

SdFat sd;
CSVFile csv;

//For KeyPad
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
byte rowPins[n_cols] = {D7, D9, D5, D4};
int i = 0;
int firstvariable;
String companyId = "001";
Keypad myKeypad = Keypad( makeKeymap(keys), rowPins, colPins, n_rows, n_cols);

//Initialize LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);
void setup() {
  Serial.begin(9600);
  //For Wifi to get Data from EEPROM
  EEPROM.begin(512);

  lcd.begin();   // initializing the LCD
  lcd.backlight(); // Enable or Turn On the backlight

  Serial.println(companyId);
  //Initializing Wifi Function
  InitialWiFi();
  //Initialize Fingerprint
  InitialFingerPrint();
  //Initializing SD card
  InitialSDcard();
  //Initializing Csv File
  InitialCSV();


}
void loop() {
  server.handleClient();
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
        if (!  getFingerprintEnroll() ) {
          Serial.println("FingerPrint ");
        }

        firstvariable = 0;

      } else {
        firstvariable = firstvariable * 10 + key1 - '0';
        Serial.println("i value ");
        Serial.print (i);
        Serial.println(firstvariable);
        Serial.println("Sending Data");
        //SendData();
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
      Serial.print ("Going to Match FingerPrint" );
      int countMatch = 0;
      //while(getFingerprintIDez());
      int empId = -1;
      do {
        Serial.println("Finger Print Matching" );
        Serial.print(empId);
        empId = getFingerprintIDez();
        if (empId != -1) {
          Serial.println(" Id :" );

          Serial.print("Finger Matched so Send Data");
          //Function For Sending Data
          SendData(empId);
          countMatch = 100;
        } else {
          countMatch = countMatch + 1;
          Serial.println(" Count :" );
          Serial.println( countMatch);
        }
      } while (countMatch < 100);

  }

}
//Function For Initializing Wifi
void InitialWiFi() {

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Initializing WIFI....");

  Serial.println("Initialzing Wifi Connection");
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
    WiFi.begin(esid.c_str(), epass.c_str());
    if (!testWifi()) {
      Serial.println(" LAUNCH WEB");
      // launchWeb(0);
      setupAP();
      return;
    }
  }

}
//Function For Testing Wifi Connection
bool testWifi(void) {
  int c = 0;
  Serial.println("Waiting for Wifi to connect to ESP8266");
  while ( c < 20  ) {

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println(" WIFI Status Connected " + WiFi.isConnected());
      lcd.clear();
      lcd.print("WIFI Connected....");
      return true;
    }
    delay(500);
    Serial.println("WIFI Status Not Connected " + WiFi.status());
    c++;
  }
  Serial.println("");
  Serial.println("Connect timed out, opening AP");
  lcd.clear();
  lcd.print("WIFI Not Connected....");
  return false;
}
//Function For Launching Web to reset Password
void launchWeb(int webtype) {
  Serial.println("WiFi not Connected");
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("SoftAP IP: ");
  Serial.println(WiFi.softAPIP());
  createWebServer(webtype);
  // Start the server
  server.begin();
  Serial.println("Server started");
}
//Function for Browser page launching
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
//Function to on NodeMCU Wifi
void setupAP(void) {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0)
    Serial.println("No networks found");
  else
  {
    Serial.print(n);
    Serial.println(" Networks found");
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

//Function for Initializing SdCard
void InitialSDcard() {
  // Initialize Sd card Pins
  pinMode(PIN_SPI_MOSI, OUTPUT);
  pinMode(PIN_SPI_MISO, INPUT);
  pinMode(PIN_SPI_CLK, OUTPUT);
  Serial.println("Initializating SD card.... ");

  // Setup SD card
  if (sd.begin(PIN_SD_CS, SD_CARD_SPEED))
  {
    Serial.println("SD card Initialization is Success");
    return;
  } else {
    Serial.println("SD card begin error");
    delay(10000);
    InitialSDcard();

  }
}
//Function For Initializing Csv File
void InitialCSV() {
  Serial.println("Initializing Csv File");
  csv.close();
  if (!csv.open(FILENAME, O_RDWR | O_CREAT)) {
    Serial.println("Failed open file");
    delay(10000);
    InitialCSV();
  } else {
    Serial.println("Open Success");
  }
  /* if (csv.open(FILENAME, O_RDWR | O_CREAT)) {
     Serial.println("File open is Success");

    } else {
     Serial.println("Failed To Open File");
     delay(10000);
     InitialCSV();

    }*/
}
//Function For Sending Data to Back end
void SendData(int empId) {
  Serial.println(WiFi.status());
  if (WiFi.status() == WL_CONNECTED) {
    if (client.connect(servername, 80)) {
      Serial.println("connected");
      ServerSend(empId, companyId);
      return;
    }

    else {
      Serial.println("Wifi Is Not Connected Offline");
      // Writing Data in CSV Because WiFi is not Connected
      OfflineDataWrite(empId);
      //delay(10000);
      //Re-initialize to connect to Wifi
      // InitialWiFi();
      return;
    }
  } else {
    Serial.println("Wifi Is Not Connected Offline");
    // Writing Data in CSV Because WiFi is not Connected
    OfflineDataWrite(empId);
    //delay(10000);
    //Re-initialize to connect to Wifi
    // InitialWiFi();
    return;
  }
}

//Function for Writing Data in CSV
void OfflineDataWrite(int empId) {
  //Initialize and Open CSV File
  //csv.close();
  InitialSDcard();
  InitialCSV();
  //To move to beginning of file

  csv.gotoBeginOfFile();
  /*const byte BUFFER_SIZE = 50;
    char buffer[BUFFER_SIZE + 1];
    buffer[BUFFER_SIZE] = '\0';
  */
  do
  {
    const byte BUFFER_SIZE = 50;
    char buffer[BUFFER_SIZE + 1];
    buffer[BUFFER_SIZE] = '\0';
    Serial.println(csv.getNumberOfLine());
    buffer[BUFFER_SIZE] = '\0';
    csv.readField(buffer, BUFFER_SIZE);
    Serial.println("Buffer     :");
    sprintf ("buffer %s", buffer);
    Serial.print(buffer);
    Serial.print("\n");
    csv.nextField();
  } while (csv.nextLine());
  while (!csv.isEndOfField()) {
    const byte BUFFER_SIZE = 50;
    char buffer[BUFFER_SIZE + 1];
    buffer[BUFFER_SIZE] = '\0';
    Serial.println(csv.getNumberOfLine());
    buffer[BUFFER_SIZE] = '\0';
    csv.readField(buffer, BUFFER_SIZE);
    //sprintf ("buffer %s", buffer);
    Serial.println("Buffer     :");
    Serial.print(buffer);

    Serial.print("\n");
    csv.nextField();
    Serial.println("writing");
  }
  csv.nextField();
  csv.addLine();
  String senddata = "";
  senddata = "{employeeId:";
  senddata += empId;
  senddata += ",companyId:";
  senddata += "001,time:08:00:00}";
  Serial.println("String  :");
  Serial.print(F(senddata.c_str()));
  Serial.println("String  1 :");
  Serial.print(F(senddata));
  csv.addField(F(senddata.c_str()));
  Serial.println("Finish writing");
  csv.close();
}

//Function For Sending data to Backend
void ServerSend(int empId, String companyId) {

  Serial.println("Sending data TO Backend");
  HTTPClient http;    //Declare object of class HTTPClient

  JSONencoder["employeeId"] = empId;
  JSONencoder["companyId"] = companyId;
  JSONencoder["date"] = "2019-04-08";
  JSONencoder["time"] = "09:00:00";
  char JSONmessageBuffer[300];
  JSONencoder.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
  Serial.println(JSONmessageBuffer);

  http.begin("http://13.126.195.214:8080/EmployeeAttendenceAPI/employee/EmployeeCheckInOut");      //Specify request destination
  http.addHeader("Content-Type", "application/json");  //Specify content-type header

  int httpCode = http.POST(JSONmessageBuffer);   //Send the request
  /*  int httpCode = http.GET(JSONmessageBuffer);*/
  if (httpCode > 0) {

    const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(2) + 600;
    DynamicJsonBuffer jsonBuffer(capacity);

    // Parse JSON object
    JsonObject& root = jsonBuffer.parseObject(http.getString());
    const char*  code = root["employeeId"];
    const char*  department = root["department"];
    const char*  companyName = root["companyName"];
    const char*  companyType = root["companyType"];
    const char*  userName = root["userName"];
    const char*  employeeDepartmentlist = root["employeeDepartmentlist"];
    const char*  role = root["role"];
    Serial.println("resturn" + http.getString());
    Serial.print("Code return element = ");
    Serial.println("EmployeeId ");
    Serial.println(code);
    Serial.println("Employee name " );
    Serial.println(userName);
    Serial.println("companyName name " );
    Serial.println(companyName);
    Serial.println("companyType " );
    Serial.println(companyType);
    Serial.println("employeeDepartmentlist " );
    Serial.println(employeeDepartmentlist);
    Serial.println("Department" );
    Serial.println(department);
    Serial.println("Role" );
    Serial.println(role);

    Serial.println(httpCode);   //Print HTTP return code
  }
  http.end();  //Close connection
  Serial.println(httpCode);
  Serial.println("SuucessFully Send Data To BackEnd");
  i = 0;
  firstvariable = 0;
}

void InitialFingerPrint() {
  Serial.println("Initialize Finger Print");
  finger.begin(57600);

  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
    return;
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    delay(10000);
    InitialFingerPrint();
  }

}
//FingerPrint Module Function
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
  //Function for sending data o backend
  //SendData();

}
void printHex(int num, int precision) {
  char tmp[16];
  char format[128];

  sprintf(format, "%%.%dX", precision);

  sprintf(tmp, format, num);
  Serial.print(tmp);
}
//Function for Matching FingerPrint
// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;

  // found a match!
  Serial.print("Found ID #");
  Serial.print(finger.fingerID);
  Serial.print(" with confidence of ");
  Serial.println(finger.confidence);
  return finger.fingerID;
}
