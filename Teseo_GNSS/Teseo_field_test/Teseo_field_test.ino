#include <TinyGPSPlus.h>
TinyGPSPlus gps;
#include <SPI.h>
#include <SD.h>

/*
#include <TORICA_SD.h>
TORICA_SD mySD;
*/

File myFile;
const int chipSelect = D2;
bool SD_active = false;
char fileName[32];
char SD_BUF[512];

TinyGPSCustom fixQualityGP(gps, "GPGGA", 6);
TinyGPSCustom fixQualityGN(gps, "GNGGA", 6);

unsigned long lastDisplayTime = 0;

#include <SerialWeb.h>
constexpr char SSID[] = "SerialWeb";
constexpr char PASSWORD[] = "12345678";

volatile uint8_t hour = 0;
volatile uint8_t minute = 0;
volatile uint8_t second = 0;
volatile uint8_t centisecond = 0;
volatile double latitude = 0;
volatile double longitude = 0;
volatile double altitude = 0;
volatile double groundspeed = 0;
volatile uint8_t satellites = 0;
volatile uint8_t fix_quality = 0;  // ★追加: Fix Quality保存用変数

void setup() {
  Serial.begin(921600);
  Serial1.setRxBufferSize(8192);

  // SDピン設定
  // SPI.setCS(D2); // CS
  // SPI.setSCK(D8); // SCK
  // SPI.setTX(D10); // MOSI
  // SPI.setRX(D9); //MISO
  SPI.begin(D8, D9, D10, D2);  // SCK, MISO, MOSI, CS

  // mySD.begin(D2); //CSをD2に設定 for TORICA_SD
  Serial.println("Initializing SD card...");

  // SD.h使用
  if (!SD.begin(chipSelect)) {
    Serial.println("SD.begin failed");
  } else {
    Serial.println("SD.begin() success");

    // 新しいCSVファイルを作成
    for (int i = 0; i < 1000; i++) {
      sprintf(fileName, "/LOG%03d.csv", i);  // LOG000.csv, LOG001.csv ...
      if (!SD.exists(fileName)) {
        // もしその名前のファイルが存在しなければ、それが新しいファイル名に決定！
        break;
      }
      Serial.print("New file name: ");
      Serial.println(fileName);
    }

    myFile = SD.open(fileName, FILE_WRITE);
    if (!myFile) {
      Serial.println("Create LOG.csv failed.");
    } else {
      Serial.println("Created LOG.csv");
      myFile.print("time,lat,lon,fix_quality,satellites\n");  // ヘッダーを書き込み
    }
    myFile.close();
    Serial.println("SD init done.");
  }


  Serial1.begin(921600, SERIAL_8N1, D7, D6);
  delay(100);
  SerialWeb.begin(SSID, PASSWORD);

  delay(1000);  // GPSレシーバの起動を待機

  Serial.println("GNSS Initialization Complete");
}


uint8_t loop_count = 0;

void loop() {
  // GNSSからのデータ読み込みとパース
  while (Serial1.available() > 0) {
    char c = Serial1.read();
    gps.encode(c);
  }

  // 100msに1回の画面表示とWeb送信処理
  if (millis() - lastDisplayTime > 100) {
    lastDisplayTime = millis();

    if (fixQualityGN.isValid()) {
      fix_quality = atoi(fixQualityGN.value());
    } else if (fixQualityGP.isValid()) {
      fix_quality = atoi(fixQualityGP.value());
    }

    hour = gps.time.hour();
    minute = gps.time.minute();
    second = gps.time.second();
    centisecond = gps.time.centisecond();
    latitude = gps.location.lat();
    longitude = gps.location.lng();
    altitude = gps.altitude.meters();
    groundspeed = gps.speed.kmph() * 1000 / 3600;
    satellites = gps.satellites.value();

    SD_active = false;
    // SD書き込み
    memset(SD_BUF, 0, sizeof(SD_BUF));  // バッファクリア
    sprintf(SD_BUF, "%02u:%02u:%02u:%03u,%.7f,%.7f,%.2f,%u,%u\n", hour, minute, second, centisecond, latitude, longitude, groundspeed, fix_quality, satellites);
    // mySD.add_str(SD_BUF);
    // mySD.flash();
    myFile = SD.open(fileName, FILE_APPEND);
    if (myFile) {
      myFile.print(SD_BUF);
      myFile.close();
      Serial.println("LOG ADDED");
      SD_active = true;
    } else {
      SD_active = false;
    }

    if (loop_count == 10 /* 10秒に一回更新 */) {
      // デバッグ表示
      loop_count = 0;  //リセット

      char time[16];
      sprintf(time, "%02u:%02u:%02u:%03u", hour, minute, second, centisecond);
      Serial.print("time: ");
      Serial.print(time);

      Serial.print(" Sat: ");
      Serial.print(satellites);
      Serial.print(" Quality: ");
      Serial.print(fix_quality);

      Serial.print(" Lat: ");
      Serial.print(latitude, 10);
      Serial.print(" long: ");
      Serial.print(longitude, 10);
      Serial.print(" Alt: ");
      Serial.print(altitude, 5);
      Serial.print(" gnd_spd: ");
      Serial.println(groundspeed, 5);

      // SerialWebへの送信 (元のまま)
      SerialWeb.send("time", time);

      char lat_value[16];
      sprintf(lat_value, "%lf", latitude);
      SerialWeb.send("Lat", lat_value);

      char long_value[16];
      sprintf(long_value, "%lf", longitude);
      SerialWeb.send("Long", long_value);

      char sat_value[16];
      sprintf(sat_value, "%d", satellites);
      SerialWeb.send("satellites", sat_value);

      char fix_value[8];
      sprintf(fix_value, "%d", fix_quality);
      SerialWeb.send("Quality", fix_value);

      char SD_active_value[8];
      sprintf(SD_active_value, "%d", SD_active);
      SerialWeb.send("SD active", SD_active_value);
    }

    loop_count++;
  }
}