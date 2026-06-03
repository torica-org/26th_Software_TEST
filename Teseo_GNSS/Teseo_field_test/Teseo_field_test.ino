#include <TinyGPSPlus.h>
TinyGPSPlus gps;
#include <SPI.h>

#include <SdFat.h>
SdFs sd_fat;  // 名前をSDからsd_fatに変更して衝突を回避
FsFile myFile;

#define SDFAT_FILE_WRITE (O_RDWR | O_CREAT | O_AT_END)
#define SDFAT_FILE_APPEND (O_RDWR | O_APPEND)

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
volatile uint8_t fix_quality = 0;

void setup() {
  Serial.begin(921600);
  Serial1.setRxBufferSize(8192);

  // SPI初期化
  SPI.begin(D8, D9, D10, D2);  // SCK, MISO, MOSI, CS

  Serial.println("Initializing SD card...");

  // SdFatの初期化 (exFAT対応)
  // クロックを16MHzに制限して安定性を向上
  if (!sd_fat.begin(SdSpiConfig(chipSelect, SHARED_SPI, SD_SCK_MHZ(16)))) {
    Serial.println("SD.begin failed! (Check format or wiring)");
  } else {
    Serial.println("SD.begin() success");

    // 新しいCSVファイルを作成
    for (int i = 0; i < 1000; i++) {
      sprintf(fileName, "/LOG%03d.csv", i);
      if (!sd_fat.exists(fileName)) {
        break;
      }
    }
    Serial.print("New file name: ");
    Serial.println(fileName);

    // ヘッダー書き込み (SDFAT_FILE_WRITEを使用)
    myFile = sd_fat.open(fileName, SDFAT_FILE_WRITE);
    if (!myFile) {
      Serial.println("Create LOG.csv failed.");
    } else {
      Serial.println("Created LOG.csv");
      myFile.print("time,lat,lon,groundspeed,fix_quality,satellites\n");
      myFile.close();
      Serial.println("SD init done.");
    }
  }

  Serial1.begin(921600, SERIAL_8N1, D7, D6);
  delay(100);
  SerialWeb.begin(SSID, PASSWORD);
  delay(1000);

  Serial.println("GNSS Initialization Complete");
}

uint8_t loop_count = 0;

void loop() {
  while (Serial1.available() > 0) {
    char c = Serial1.read();
    gps.encode(c);
  }

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
    memset(SD_BUF, 0, sizeof(SD_BUF));
    sprintf(SD_BUF, "%02u:%02u:%02u:%03u,%.7f,%.7f,%.2f,%u,%u\n", hour, minute, second, centisecond, latitude, longitude, groundspeed, fix_quality, satellites);

    // 追記モードでオープン (SDFAT_FILE_APPENDを使用)
    myFile = sd_fat.open(fileName, SDFAT_FILE_APPEND);
    if (myFile) {
      myFile.print(SD_BUF);
      myFile.close();
      SD_active = true;
    }

    if (loop_count >= 10) {  // 1秒ごとにデバッグ表示
      loop_count = 0;

      char time[16];
      sprintf(time, "%02u:%02u:%02u:%03u", hour, minute, second, centisecond);
      Serial.print("time: ");
      Serial.print(time);
      Serial.print(" SD: ");
      Serial.print(SD_active);

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