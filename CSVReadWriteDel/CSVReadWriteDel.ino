/**
    Project: CSVFile read write Delete - examples
    @author: Sandhiya
    @contact: sandhiya.neer@gmail.com
    @date: 24.03.2019
    @version: 1.0.1

*/

/**
    Read Write Delete CSV file data
*/
#include <SdFat.h>
#include <CSVFile.h>

// =*= CONFIG =*=
// SPI pinout
//#define PIN_SPI_CLK 13
//#define PIN_SPI_MOSI 11
//#define PIN_SPI_MISO 12
//#define PIN_SD_CS 4

#define PIN_SPI_CLK 14
#define PIN_SPI_MOSI 13
#define PIN_SPI_MISO 12
#define PIN_SD_CS 15

#define PIN_OTHER_DEVICE_CS -1
#define SD_CARD_SPEED SPI_QUARTER_SPEED

#define FILENAME "OfflineData4.csv"
#define FILENAME1 "OfflineData5.csv"

SdFat sd;
CSVFile csv;
void setup() {
  // put your setup code here, to run once:
  pinMode(PIN_SPI_MOSI, OUTPUT);
  pinMode(PIN_SPI_MISO, INPUT);
  pinMode(PIN_SPI_CLK, OUTPUT);
  //Disable SPI devices
  pinMode(PIN_SD_CS, OUTPUT);
  digitalWrite(PIN_SD_CS, HIGH);

#if PIN_OTHER_DEVICE_CS > 0
  pinMode(PIN_OTHER_DEVICE_CS, OUTPUT);
  digitalWrite(PIN_OTHER_DEVICE_CS, HIGH);
#endif //PIN_OTHER_DEVICE_CS > 0

  Serial.begin(9600);
  while (!Serial) {
    /* wait for Leonardo */
  }
  // Setup SD card
  if (!sd.begin(PIN_SD_CS, SD_CARD_SPEED))
  {
    Serial.println("SD card begin error");
    return;
  }
}
void initSdFile()
{
  /*
    if (sd.exists(FILENAME) && !sd.remove(FILENAME))
    {
      Serial.println("Failed init remove file");
      return;
    }
  */
  // Important note!
  // You should use flag O_RDWR even if you use CSV File
  // only for writting.
  if (!csv.open(FILENAME, O_RDWR | O_CREAT)) {
    Serial.println("Failed open file");
  }else{
    Serial.println("Open Success");
  }
}
void loop() {
  // put your main code here, to run repeatedly:
  // panic();
  initSdFile();
  csv.gotoBeginOfFile();
  const byte BUFFER_SIZE = 15;
  char buffer[BUFFER_SIZE + 1];
  buffer[BUFFER_SIZE] = '\0';
  do
  {
    Serial.println(csv.getNumberOfLine());
    csv.readField(buffer, BUFFER_SIZE);
    sprintf ("buffer %s", buffer);
    Serial.print(buffer);

    Serial.print("\n");
   /* csv.markLineAsDelete();
    Serial.print(F("Deleted"));
     Serial.print("\n");
     */
    csv.nextField();
  }
  while (csv.nextLine());
  csv.isEndOfField() + 1;
  csv.nextField();
  //Serial.println(csv.isEndOfField());
  csv.addLine();
  csv.addField(F("001,11,08:00:00"));//CompanyId
  csv.addLine();
  csv.addField("001,13,08:10:00");//COmpanyId
  csv.addLine();
 /* csv.addField("001,14,08:10:00");//COmpanyId
  csv.addLine();
  csv.addField("001,15,08:20:00");//COmpanyId
  csv.addLine();
  csv.addField("001,16,08:20:00");//COmpanyId
  csv.addLine();
  csv.addField("001,17,08:20:00");//COmpanyId
  csv.addLine();
  csv.addField("001,18,08:20:00");//COmpanyId
  csv.addLine();
  csv.addField("001,19,08:20:00");//COmpanyId
  csv.addLine();
  csv.addField("001,20,08:20:00");//COmpanyId
  csv.addLine();
  */
  unsigned int num = csv.getNumberOfLine();
  Serial.println("Size");
  Serial.println(num);
  while (num > -1)
  {
    csv.readField(buffer, BUFFER_SIZE);
    sprintf ("buffer %s", buffer);
    Serial.print(buffer);

    Serial.print("\n");

    csv.nextField();
    num = num - 1;
    Serial.print("num \n");
  }
   csv.gotoBeginOfFile();
    csv.gotoLine(120);
    csv.nextLine();
    csv.markLineAsDelete();
    csv.nextLine();
    csv.nextLine();
    csv.nextLine();
    csv.markLineAsDelete();
  
  csv.close();
  initSdFile();
  Serial.println("Success"); 
  delay(50000);
  Serial.println("Operation Done");
  //waitForKey();
}
void waitForKey()
{
  while (Serial.read() >= 0) { }
  //Serial.println("Type any character to repeat.\n");
  while (Serial.read() <= 0) { }
}
