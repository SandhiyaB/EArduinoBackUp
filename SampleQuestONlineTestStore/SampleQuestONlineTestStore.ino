#include <LiquidCrystal.h>
#include <SD.h>
#include <SPI.h>
#define NUM_OF_FINGERS_IN_READER
// Initialize the library with the numbers of the interface pins
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
Sd2Card card;
SdVolume volume;
SdFile root;
File myFile;
// change this to match your SD shield or module;
// Arduino Ethernet shield: pin 4
// Adafruit SD shields and modules: pin 10
// Sparkfun SD shield: pin 8
const int chipSelect = 53;    
//States for the menu.
int currentMenuItem = 0;
int lastState = 0;
unsigned int i, p;
uint8_t id1, id2;
#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include "ds3231.h"
int ffpin = A14;

#define BUFF_MAX 128

uint8_t time[8];
char recv[BUFF_MAX];
unsigned int recv_size = 0;
unsigned long prev, interval = 1000;


SoftwareSerial mySerial(A14,A15);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);



void setup() {
  //Set the characters and column numbers.
  lcd.begin(16, 2);
  //Print default title.
  clearPrintTitle();

  Serial.begin(9600);
  finger.begin(57600);

  if (finger.verifyPassword())
  {
    Serial.println("Found fingerprint sensor!");
  }
  else
  {
    Serial.println("Did not find fingerprint sensor :(");
    while (1);
  }
  Serial.begin(9600);
  Wire.begin();
  DS3231_init(DS3231_INTCN);
  memset(recv, 0, BUFF_MAX);
  Serial.println("GET time");
  lcd.begin(16, 2);
  lcd.clear();

  //Serial.println("Setting time");
  //parse_cmd("T004704209032016",16);
   {
lcd.begin(16, 2);
  lcd.setCursor (0,0);
  lcd.print("Initializing");
  lcd.setCursor (0,1);
  lcd.print("SD card...");

  
  // On the Ethernet Shield, CS is pin 4. It's set as an output by default.
  // Note that even if it's not used as the CS pin, the hardware SS pin 
  // (10 on most Arduino boards, 53 on the Mega) must be left as an output 
  // or the SD library functions will not work. 
  pinMode(53, OUTPUT);     // change this to 53 on a mega


  // we'll use the initialization code from the utility libraries
  // since we're just testing if the card is working!
  if (!card.init(SPI_HALF_SPEED, chipSelect)) {
    Serial.println("\nInitialization failed. Things to check:");
    Serial.println("* is a card is inserted?");
    Serial.println("* Is your wiring correct?");
    Serial.println("* did you change the chipSelect pin to match your shield or module?");
    return;
  } else {
   delay (1500);
   lcd.clear();
   lcd.setCursor (0,0);
   lcd.print("Installation");
      lcd.setCursor (0,1);
   lcd.print("Success");
   delay (2500);
   lcd.clear();
   delay (2500);
   lcd.setCursor (0,0);
   lcd.print("SD Card Ready to");
      lcd.setCursor (0,1);
   lcd.print("Read and Write");
    
  }

  // print the type of card
  Serial.print("\nCard type: ");
  switch(card.type()) {
    case SD_CARD_TYPE_SD1:
      Serial.println("SD1");
      break;
    case SD_CARD_TYPE_SD2:
      Serial.println("SD2");
      break;
    case SD_CARD_TYPE_SDHC:
      Serial.println("SDHC");
      break;
    default:
      Serial.println("Unknown");
  }

  // Now we will try to open the 'volume'/'partition' - it should be FAT16 or FAT32
  if (!volume.init(card)) {
    Serial.println("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card");
    return;
  }
pinMode (ffpin , INPUT);

  // print the type and size of the first FAT-type volume
  uint32_t volumesize;
  Serial.print("\nVolume type is FAT");
  Serial.println(volume.fatType(), DEC);
  Serial.println();
  
  volumesize = volume.blocksPerCluster();    // clusters are collections of blocks
  volumesize *= volume.clusterCount();       // we'll have a lot of clusters
  volumesize *= 512;                            // SD card blocks are always 512 bytes
  Serial.print("Volume size (bytes): ");
  Serial.println(volumesize);
  Serial.print("Volume size (Kbytes): ");
  volumesize /= 1024;
  Serial.println(volumesize);
  Serial.print("Volume size (Mbytes): ");
  volumesize /= 1024;
  Serial.println(volumesize);

  
  Serial.println("\nFiles found on the card (name, date and size in bytes): ");
  root.openRoot(volume);
  
  // list all files in the card with date and size
  root.ls(LS_R | LS_DATE | LS_SIZE);}
}





void loop() {
  //Call the main menu.
  mainMenu();
}



void mainMenu() {
  //State = 0 every loop cycle.
  int state = 0;
  //Refresh the button pressed.
  int x = analogRead (0);
  //Set the Row 0, Col 0 position.
  lcd.setCursor(0, 0);

  //Check analog values from LCD Keypad Shield
  if (x < 100) {
    //Right
  } else if (x < 200) {
    //Up
    state = 1;
  } else if (x < 400) {
    //Down
    state = 2;
  } else if (x < 600) {
    //Left
  } else if (x < 800) {
    //Select
    state = 3;
  }

  //If we are out of bounds on th menu then reset it.
  if (currentMenuItem < 0 || currentMenuItem > 4) {
    currentMenuItem = 0;
  }

  //If we have changed Index, saves re-draws.
  if (state != lastState) {
    if (state == 1) {
      //If Up
      currentMenuItem = currentMenuItem - 1;
      displayMenu(currentMenuItem);
    } else if (state == 2) {
      //If Down
      currentMenuItem = currentMenuItem + 1;
      displayMenu(currentMenuItem);
    } else if (state == 3) {
      //If Selected
      selectMenu(currentMenuItem);
    }
    //Save the last State to compare.
    lastState = state;
  }
  //Small delay
  delay(5);
}

//Display Menu Option based on Index.
void displayMenu(int x) {
  switch (x) {
    case 1:
      clearPrintTitle();
      lcd.print ("-> ENROLL L01");
      break;
    case 2:
      clearPrintTitle();
      lcd.print ("-> ENROLL LO2");
      break;
    case 3:
      clearPrintTitle();
      lcd.print ("-> UPDATE L01");
      break;
    case 4:
      clearPrintTitle();
      lcd.print ("-> UPDATE L02");
      break;
  }
}

//Print a basic header on Row 1.
void clearPrintTitle() {
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print(" BIO-AMS  ");
  lcd.setCursor(0, 1);
  char in;
  char tempF[6];
  float temperature;
  char buff[BUFF_MAX];
  unsigned long now = millis();
  struct ts t;

  // show time once in a while
  if ((now - prev > interval) && (Serial.available() <= 0)) {
    DS3231_get(&t); //Get time
    parse_cmd("C", 1);
    temperature = DS3231_get_treg(); //Get temperature
    dtostrf(temperature, 5, 1, tempF);

    lcd.setCursor(0, 1);

    lcd.print(t.mday);
    lcd.print(".");
    printMonth(t.mon);
    lcd.print(".");
    lcd.print(t.year);

    lcd.setCursor(12, 1); //Go to second line of the LCD Screen
    lcd.print(t.hour);
    lcd.print(":");
    if (t.min < 10)
    {
      lcd.print("0");
    }
    lcd.print(t.min);
    delay (1000);
    lcd.clear ();

  }


  if (Serial.available() > 0) {
    in = Serial.read();

    if ((in == 10 || in == 13) && (recv_size > 0)) {
      parse_cmd(recv, recv_size);
      recv_size = 0;
      recv[0] = 0;
    } else if (in < 48 || in > 122) {
      ;       // ignore ~[0-9A-Za-z]
    } else if (recv_size > BUFF_MAX - 2) {   // drop lines that are too long
      // drop
      recv_size = 0;
      recv[0] = 0;
    } else if (recv_size < BUFF_MAX - 2) {
      recv[recv_size] = in;
      recv[recv_size + 1] = 0;
      recv_size += 1;
    }

  }

}


//Show the selection on Screen.
void selectMenu(int x) {
  switch (x) {
    case 1:
      clearPrintTitle();
      lcd.clear();
      lcd.setCursor(3, 0);
      lcd.print ("Selected");
      lcd.setCursor(3, 1);
      lcd.print ("ENROLL L01");

      lcd.clear();
      lcd.setCursor (0, 0);
      lcd.print("Enrolling ID #");
      lcd.setCursor(0, 1);
      lcd.print(id1);

      id1 = 1;
      id1 = id1++ ;

      delay (1000);
      getFingerprintEnroll(id1);
      delay (1000);
      getdatabase();
      lcd.clear();

      //Call the function that belongs to Option 1
      delay (1000);
      break;
    case 2:
      clearPrintTitle();
      lcd.clear();
      lcd.setCursor(3, 0);
      lcd.print ("Selected");
      lcd.setCursor(3, 1);
      lcd.print ("ENROLL L02");
      lcd.clear();
      lcd.setCursor (0, 0);
      lcd.print("Enrolling ID #");
      lcd.setCursor(0, 1);
      lcd.println(id2);
      id2 = 20;
      id2 = id2++ ;


      delay (1000);
      getFingerprintEnroll(id2);

      delay (1000);

      //Call the function that belongs to Option 2
      break;
    case 3:
      clearPrintTitle();
      lcd.clear();
      lcd.setCursor(3, 0);
      lcd.print ("Selected");
      lcd.setCursor(3, 1);
      lcd.print ("UPDATE L01");

      {
        getFingerprintID();
      }
      delay (1000);


      //Call the function that belongs to Option 3
      break;

    case 4:
      clearPrintTitle();
      lcd.clear();
      lcd.setCursor(3, 0);
      lcd.print ("Selected");
      lcd.setCursor(3, 1);
      lcd.print ("UPDATE L02");
      {
        getFingerprintID();
      }
      delay (1000);

      //Call the function that belongs to Option 4
      break;
  }
}
//----------------------------------------------------//
int getFingerprintEnroll(uint8_t id)
{
  p = -1;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Waiting finger to enroll");
  lcd.setCursor(0, 1);
  lcd.print("to enroll");
  while (p != FINGERPRINT_OK)
  {
    p = finger.getImage();
    switch (p)
    {
      case FINGERPRINT_OK:
#if !defined (VB)
        Serial.println("??");
#endif
        lcd.clear();
        lcd.setCursor(1, 0);
        lcd.print("Image taken");
        delay (1000);
        break;
      case FINGERPRINT_NOFINGER:
#if !defined (VB)
        Serial.print(".");
#endif
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
#if !defined (VB)
        Serial.println();
#endif
        lcd.clear();
        lcd.setCursor(3, 0);
        lcd.print("Communication error");
        delay (1000);
        break;
      case FINGERPRINT_IMAGEFAIL:
#if !defined (VB)
        Serial.println();
#endif
        lcd.clear();
        lcd.setCursor(3, 0);
        lcd.print("Imaging error");
        delay (1000);
        break;
      default:
#if !defined (VB)
        Serial.println("??");
#endif
        lcd.clear();
        lcd.setCursor(3, 0);
        lcd.print("Unknown error");
        delay (1000);
        break;
    }
  }

  p = finger.image2Tz(1);
  switch (p)
  {
    case FINGERPRINT_OK:
      lcd.clear();
      lcd.setCursor(1, 0);
      Serial.print("Image converted");
      delay (1000);
      break;
    case FINGERPRINT_IMAGEMESS:
      lcd.clear();
      lcd.setCursor(3, 0);
      Serial.print("Image too messy");
      delay (1000);
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      lcd.clear();
      lcd.setCursor(3, 0);
      Serial.print("Communication error");
      delay (1000);
      return p;
    case FINGERPRINT_FEATUREFAIL:
      lcd.clear();
      lcd.setCursor(3, 0);
      Serial.print("Could not find fingerprint features");
      delay (1000);
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      lcd.clear();
      lcd.setCursor(3, 0);
      Serial.print("Could not find fingerprint features");
      delay (1000);
      return p;
    default:
      lcd.clear();
      lcd.setCursor(0, 0);
      Serial.print("Unknown error");
      delay (1000);
      return p;
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Remove finger");
  delay (2000);
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER)
  {
    p = finger.getImage();
  }

  p = -1;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Place same");
  lcd.setCursor(0, 1);
  lcd.print("finger again");
  delay (1000);
  while (p != FINGERPRINT_OK)
  {
    p = finger.getImage();
    switch (p)
    {
      case FINGERPRINT_OK:
#if !defined (VB)
        Serial.println();
#endif
        lcd.clear();
        lcd.setCursor(1, 0);
        lcd.print("Image taken");
        delay (1000);
        break;
      case FINGERPRINT_NOFINGER:
#if !defined (VB)
        Serial.print(".");
#endif
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
#if !defined (VB)
        Serial.println();
#endif
        lcd.clear();
        lcd.setCursor(3, 0);
        Serial.print("Communication error");
        delay (1000);
        break;
      case FINGERPRINT_IMAGEFAIL:
#if !defined (VB)
        Serial.println();
#endif
        lcd.clear();
        lcd.setCursor(3, 0);
        Serial.print("Imaging error");
        delay (1000);
        break;
      default:
#if !defined (VB)
        Serial.println();
#endif
        lcd.clear();
        lcd.setCursor(1, 0);
        Serial.print("Unknown error");
        delay (1000);
        break;
    }
  }

  p = finger.image2Tz(2);
  switch (p)
  {
    case FINGERPRINT_OK:
      lcd.clear();
      lcd.setCursor(1, 0);
      Serial.print("Image converted");
      delay (1000);
      break;
    case FINGERPRINT_IMAGEMESS:
      lcd.clear();
      lcd.setCursor(3, 0);
      Serial.print("Image too messy");
      delay (1000);
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      lcd.clear();
      lcd.setCursor(3, 0);
      Serial.print("Communication error");
      delay (1000);
      return p;
    case FINGERPRINT_FEATUREFAIL:
      lcd.clear();
      lcd.setCursor(3, 0);
      Serial.print("Could not find fingerprint features");
      delay (1000);
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      lcd.clear();
      lcd.setCursor(3, 0);
      Serial.print("Could not find fingerprint features");
      delay (1000);
      return p;
    default:
      lcd.clear();
      lcd.setCursor(3, 0);
      Serial.print("Unknown error");
      delay (1000);
      return p;
  }

  p = finger.createModel();
  switch (p)
  {
    case FINGERPRINT_OK:
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.print("Prints matched!");
      delay (1000);
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      lcd.clear();
      lcd.setCursor(3, 0);
      Serial.print("Communication error");
      delay (1000);
      return -1;
      break;
    case FINGERPRINT_ENROLLMISMATCH:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Fingerprints");
      lcd.setCursor(0, 1);
      lcd.print("did not match");
      delay (1000);
      return -1;
      break;
    default:
      lcd.clear();
      lcd.setCursor(0, 0);
      Serial.print("Unknown error");
      delay (1000);
      return -1;
      break;
  }

  p = finger.storeModel(id);
  switch (p)
  {
    case FINGERPRINT_OK:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Stored as #ID");
      lcd.setCursor(3, 1);
      lcd.print(id1);
      delay (1000);
      getdatabase();
     delay(1000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Slct to cont");
      lcd.setCursor(0, 1);
      lcd.print("Down to Menu");
      delay (5000);

      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      lcd.clear();
      lcd.setCursor(3, 0);
      Serial.print("Communication error");
      delay (1000);
      return -1;
      break;
    case FINGERPRINT_BADLOCATION:
      lcd.clear();
      lcd.setCursor(3, 0);
      lcd.print("Could not store in that location");
      delay (1000);
      return -1;
      break;
    case FINGERPRINT_FLASHERR:
      lcd.clear();
      lcd.setCursor(3, 0);
      Serial.print("Error writing to flash");
      delay (1000);
      return -1;
      break;
    default:
      lcd.clear();
      lcd.setCursor(3, 0);
      Serial.print("Unknown error");
      delay (1000);
      return p;
      break;
  }
}
//---------------------------------------------------------------------//
int getFingerprintID()
{
  p = -1;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Waiting for ");
  lcd.setCursor(0, 1);
  lcd.print("finger to enroll");

  while (p != FINGERPRINT_OK)
  {
    p = finger.getImage();
    switch (p)
    {
      case FINGERPRINT_OK:
#if !defined (VB)
        Serial.println();
#endif
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Image taken");
        delay (1000);
        break;
      case FINGERPRINT_NOFINGER:
#if !defined (VB)
        Serial.print(".");
#endif
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
#if !defined (VB)
        Serial.println();
#endif
        lcd.clear();
        lcd.setCursor(3, 0);
        Serial.print("Communication error");
        delay (1000);
        break;
      case FINGERPRINT_IMAGEFAIL:
#if !defined (VB)
        Serial.println();
#endif
        lcd.clear();
        lcd.setCursor(3, 0);
        Serial.print("Imaging error");
        delay (1000);
        break;
      default:
#if !defined (VB)
        Serial.println();
#endif
        lcd.clear();
        lcd.setCursor(3, 0);
        Serial.print("Unknown error");
        delay (1000);
        break;
    }
  }

  p = finger.image2Tz();
  switch (p)
  {
    case FINGERPRINT_OK:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Image converted");
      delay (1000);
      break;
    case FINGERPRINT_IMAGEMESS:
      lcd.clear();
      lcd.setCursor(3, 0);
      Serial.print("Image too messy");
      delay (1000);
      return -1;
    case FINGERPRINT_PACKETRECIEVEERR:
      lcd.clear();
      lcd.setCursor(3, 0);
      Serial.print("Communication error");
      delay (1000);
      return -1;
    case FINGERPRINT_FEATUREFAIL:
      lcd.clear();
      lcd.setCursor(3, 0);
      Serial.print("Could not find fingerprint features");
      delay (1000);
      return -1;
    case FINGERPRINT_INVALIDIMAGE:
      lcd.clear();
      lcd.setCursor(3, 0);
      Serial.print("Could not find fingerprint features");
      delay (1000);
      return -1;
    default:
      lcd.clear();
      lcd.setCursor(3, 0);
      Serial.print("Unknown error");
      delay (1000);
      return -1;
  }

  p = finger.fingerFastSearch();
  switch (p)
  {
    case FINGERPRINT_OK:
      lcd.clear();
      lcd.setCursor(3, 0);
      lcd.print("Found a ");
      lcd.setCursor(3, 1);
      lcd.print("print match!");
      delay (1000);

      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      lcd.clear();
      lcd.setCursor(3, 0);
      Serial.print("Communication error");
      delay (1000);
      return -1;
      break;
    case FINGERPRINT_NOTFOUND:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Did not ");
      lcd.setCursor(0, 1);
      lcd.print("find a match");
      delay (1000);
      return -1;
      break;
    default:
      lcd.clear();
      lcd.setCursor(3, 0);
      lcd.print("Unknown error");
      delay (1000);
      return -1;
      break;
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Found ID #");


  lcd.setCursor(11, 0);
  lcd.print(finger.fingerID);

  lcd.setCursor(0, 1);
  lcd.print(" with cdf ");

  lcd.setCursor(12, 1);
  lcd.print(finger.confidence);
  delay (2000);
  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("SAVED");
  delay (2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Slct to cont");
  lcd.setCursor(0, 1);
  lcd.print("Down to Menu");
  delay (5000);
  return finger.fingerID;
}

int getFingerprintIDez()
{
  p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;

  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("Found ID #");

  lcd.setCursor(11, 0);
  lcd.print(finger.fingerID);

  lcd.setCursor(0, 1);
  lcd.print(" with cdf ");

  lcd.setCursor(12, 1);
  lcd.print(finger.confidence);
  delay (2000);
  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("SAVED");
  delay (2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Slct to cont");
  lcd.setCursor(0, 1);
  lcd.print("Down to Menu");
  delay (5000);
  return finger.fingerID;

}

boolean FingerprintExists(uint16_t id)
{
  uint8_t p = finger.loadModel(id1);
  switch (p) {
    case FINGERPRINT_OK:
      return true;
      break;
    default:
      return false;
  }
}

void parse_cmd(char *cmd, int cmdsize)
{
  uint8_t i;
  uint8_t reg_val;
  char buff[BUFF_MAX];
  struct ts t;

  //snprintf(buff, BUFF_MAX, "cmd was '%s' %d\n", cmd, cmdsize);
  //Serial.print(buff);

  // TssmmhhWDDMMYYYY aka set time
  if (cmd[0] == 84 && cmdsize == 16) {
    //T355720619112011
    t.sec = inp2toi(cmd, 1);
    t.min = inp2toi(cmd, 3);
    t.hour = inp2toi(cmd, 5);
    t.wday = inp2toi(cmd, 7);
    t.mday = inp2toi(cmd, 8);
    t.mon = inp2toi(cmd, 10);
    t.year = inp2toi(cmd, 12) * 100 + inp2toi(cmd, 14);
    DS3231_set(t);
    Serial.println("OK");
  } else if (cmd[0] == 49 && cmdsize == 1) {  // "1" get alarm 1
    DS3231_get_a1(&buff[0], 59);
    Serial.println(buff);
  } else if (cmd[0] == 50 && cmdsize == 1) {  // "2" get alarm 1
    DS3231_get_a2(&buff[0], 59);
    Serial.println(buff);
  } else if (cmd[0] == 51 && cmdsize == 1) {  // "3" get aging register
    Serial.print("aging reg is ");
    Serial.println(DS3231_get_aging(), DEC);
  } else if (cmd[0] == 65 && cmdsize == 9) {  // "A" set alarm 1
    DS3231_set_creg(DS3231_INTCN | DS3231_A1IE);
    //ASSMMHHDD
    for (i = 0; i < 4; i++) {
      time[i] = (cmd[2 * i + 1] - 48) * 10 + cmd[2 * i + 2] - 48; // ss, mm, hh, dd
    }
    byte flags[5] = { 0, 0, 0, 0, 0 };
    DS3231_set_a1(time[0], time[1], time[2], time[3], flags);
    DS3231_get_a1(&buff[0], 59);
    Serial.println(buff);
  } else if (cmd[0] == 66 && cmdsize == 7) {  // "B" Set Alarm 2
    DS3231_set_creg(DS3231_INTCN | DS3231_A2IE);
    //BMMHHDD
    for (i = 0; i < 4; i++) {
      time[i] = (cmd[2 * i + 1] - 48) * 10 + cmd[2 * i + 2] - 48; // mm, hh, dd
    }
    byte flags[5] = { 0, 0, 0, 0 };
    DS3231_set_a2(time[0], time[1], time[2], flags);
    DS3231_get_a2(&buff[0], 59);
    Serial.println(buff);
  } else if (cmd[0] == 67 && cmdsize == 1) {  // "C" - get temperature register
    Serial.print("temperature reg is ");
    Serial.println(DS3231_get_treg(), DEC);
  } else if (cmd[0] == 68 && cmdsize == 1) {  // "D" - reset status register alarm flags
    reg_val = DS3231_get_sreg();
    reg_val &= B11111100;
    DS3231_set_sreg(reg_val);
  } else if (cmd[0] == 70 && cmdsize == 1) {  // "F" - custom fct
    reg_val = DS3231_get_addr(0x5);
    Serial.print("orig ");
    Serial.print(reg_val, DEC);
    Serial.print("month is ");
    Serial.println(bcdtodec(reg_val & 0x1F), DEC);
  } else if (cmd[0] == 71 && cmdsize == 1) {  // "G" - set aging status register
    DS3231_set_aging(0);
  } else if (cmd[0] == 83 && cmdsize == 1) {  // "S" - get status register
    Serial.print("status reg is ");
    Serial.println(DS3231_get_sreg(), DEC);
  } else {
    Serial.print("unknown command prefix ");
    Serial.println(cmd[0]);
    Serial.println(cmd[0], DEC);
  }
}

void printMonth(int month)
{
  switch (month)
  {
    case 1: lcd.print("01"); break;
    case 2: lcd.print("02"); break;
    case 3: lcd.print("03"); break;
    case 4: lcd.print("04"); break;
    case 5: lcd.print("05"); break;
    case 6: lcd.print("06"); break;
    case 7: lcd.print("07"); break;
    case 8: lcd.print("08"); break;
    case 9: lcd.print("09"); break;
    case 10: lcd.print("10"); break;
    case 11: lcd.print("11"); break;
    case 12: lcd.print("12"); break;
    default: lcd.print("Error"); break;
  }
}


void getdatabase()
{
  analogRead (ffpin);
  myFile = SD.open ("Database.csv", FILE_WRITE);
  if (myFile)
  {
     struct ts t;
    myFile.print(finger.fingerID);
    Serial.print(finger.fingerID);
    myFile.print(",");
    Serial.print(",");
    myFile.print(t.mday);
    
      myFile.print(",");
      
    myFile.print(t.mon);
    
      myFile.print(",");
      
    myFile.print(t.year);
    
      myFile.print(",");
      
    myFile.print(t.hour);
    
      myFile.print(",");
      
    myFile.print(t.min);
    
      myFile.print(",");
      
    myFile.print(t.sec);
    delay (2000);
    lcd.setCursor(0,0);
    lcd.print("ID SAVED");
    lcd.setCursor(0,1);
    lcd.print("IN DATABASE");
    myFile.close();
  }
  else{
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Error");
    
  }
}
