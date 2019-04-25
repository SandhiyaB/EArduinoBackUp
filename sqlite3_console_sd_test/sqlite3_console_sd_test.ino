/*
  Open SQLite databass on the SD Card
  and execute SQL statements throught this console.
  Output is in tab-delimited format.
  Increase stack size in cores/esp8266/cont.h
  to atleast 6144 (from 4096)
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <sqlite3.h>
#include <vfs.h>
#include <SPI.h>

#include "SdFat.h"
extern "C" {
#include "user_interface.h"
}
#include <ESP8266WiFi.h>

#define MAX_FILE_NAME_LEN 100
#define MAX_STR_LEN 500

void WiFiOff() {
   wifi_station_disconnect();
   wifi_set_opmode(NULL_MODE);
   wifi_set_sleep_type(MODEM_SLEEP_T);
   wifi_fpm_open();
   wifi_fpm_do_sleep(0xFFFFFFF);
}

char db_file_name[MAX_FILE_NAME_LEN] = "\0";
sqlite3 *db = NULL;
int rc;

bool first_time = false;
static int callback(void *data, int argc, char **argv, char **azColName) {
  int i;
  if (first_time) {
     Serial.println((const char *) data);
     for (i = 0; i<argc; i++) {
         if (i)
           Serial.print((char) '\t');
         Serial.printf("%s", azColName[i]);
     }
     Serial.printf("\n");
     first_time = false;
  }
  for (i = 0; i<argc; i++) {
    if (i)
      Serial.print((char) '\t');
    Serial.printf("%s", argv[i] ? argv[i] : "NULL");
  }
  Serial.printf("\n");
  return 0;
}

int db_open() {
  if (db != NULL)
    sqlite3_close(db);
  int rc = sqlite3_open(db_file_name, &db);
  if (rc) {
    Serial.print(F("Can't open database: "));
    Serial.println(sqlite3_errmsg(db));
    return rc;
  } else
    Serial.println(F("Opened database successfully"));
  return rc;
}

char *zErrMsg = 0;
const char* data = "Output:";
int db_exec(const char *sql) {
  if (db == NULL) {
    Serial.println("No database open");
    return 0;
  }
  first_time = true;
  long start = micros();
  int rc = sqlite3_exec(db, sql, callback, (void*)data, &zErrMsg);
  if (rc != SQLITE_OK) {
    Serial.print(F("SQL error: "));
    Serial.println(zErrMsg);
    sqlite3_free(zErrMsg);
  } else
    Serial.println(F("Operation done successfully"));
  Serial.print(F("Time taken:"));
  Serial.print(micros()-start);
  Serial.println(F(" us"));
  return rc;
}

int input_string(char *str, int max_len) {
  max_len--;
  int ctr = 0;
  str[ctr] = 0;
  while (str[ctr] != '\n') {
    if (Serial.available()) {
        str[ctr] = Serial.read();
        if (str[ctr] >= ' ' && str[ctr] <= '~')
          ctr++;
        if (ctr >= max_len)
          break;
    }
  }
  str[ctr] = 0;
  Serial.println(str);
}

int input_num() {
  char in[20];
  int ctr = 0;
  in[ctr] = 0;
  while (in[ctr] != '\n') {
    if (Serial.available()) {
        in[ctr] = Serial.read();
        if (in[ctr] >= '0' && in[ctr] <= '9')
            ctr++;
        if (ctr >= sizeof(in))
          break;
    }
  }
  in[ctr] = 0;
  int ret = atoi(in);
  Serial.println(ret);
  return ret;
}

enum {CHOICE_OPEN_DB = 1, CHOICE_EXEC_SQL, CHOICE_EXEC_MULTI_SQL, CHOICE_CLOSE_DB,
    CHOICE_LIST_FOLDER, CHOICE_RENAME_FILE, CHOICE_DELETE_FILE};
int askChoice() {
  Serial.println();
  Serial.println(F("Welcome to SQLite console!!"));
  Serial.println(F("---------------------------"));
  Serial.println();
  Serial.print(F("Database file: "));
  Serial.println(db_file_name);
  Serial.println();
  Serial.println(F("1. Open database           2. Execute SQL"));
  Serial.println(F("3. Execute Multiple SQL    4. Close database"));
  Serial.println();
  Serial.print(F("Enter choice: "));
  return input_num();
}

void displayPrompt(const char *title) {
  Serial.print(F("Enter "));
  Serial.println(title);
}

void setup() {
  Serial.begin(74880);
  system_update_cpu_freq(SYS_CPU_160MHZ);
  WiFiOff();
  SPI.begin();
  vfs_mount("/SD0", SS);
  sqlite3_initialize();
}

char str[MAX_STR_LEN];
void loop() {
  int choice = askChoice();
  switch (choice) {
    case CHOICE_OPEN_DB:
      displayPrompt("file name (prefix /SD0/): ");
      input_string(str, MAX_FILE_NAME_LEN);
      if (str[0] != 0) {
        strncpy(db_file_name, str, MAX_FILE_NAME_LEN);
        db_open();
      }
      break;
    case CHOICE_EXEC_SQL:
      Serial.print(F("Enter SQL (max "));
      Serial.print(MAX_STR_LEN);
      Serial.println(F(" characters):"));
      input_string(str, MAX_STR_LEN);
      if (str[0] != 0)
        db_exec(str);
      break;
    case CHOICE_EXEC_MULTI_SQL:
      Serial.println(F("(Copy paste may not always work due to limited serial buffer)"));
      Serial.println(F("Keep entering SQL, empty to stop:"));
      do {
        input_string(str, MAX_STR_LEN);
        if (str[0] != 0)
          db_exec(str);
      } while (str[0] != 0);
      break;
    case CHOICE_CLOSE_DB:
      if (db_file_name[0] != 0) {
        db_file_name[0] = 0;
        sqlite3_close(db);
      }
      break;
    default:
      Serial.println(F("Invalid choice. Try again."));
  }

}
