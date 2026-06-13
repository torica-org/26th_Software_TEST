#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

// UART用
const int SERIAL_TX = 0;
const int SERIAL_RX = 1;

// SD用SPI
const int SD_MISO = 4;
const int SD_MOSI = 7;
const int SD_CS = 5;
const int SD_SCK = 6;

const String DATA_FILE = "data.csv"; // 読み込むログデータ
const int COLUMN_SIZE = 5; // CSVの列数
int ROW_SIZE = 0;          // 行カウント用

// 読み込んだ1行分のデータを保持するグローバル配列
String read_data[COLUMN_SIZE]; 

File myfile;

void setup() {
  // PCとの通信用
  Serial.begin(115200);

  // UART0を使用
  Serial1.setTX(SERIAL_TX);
  Serial1.setRX(SERIAL_RX);
  Serial1.begin(460800); // 各基板との通信速度を設定
  delay(2000);

  Serial.println("Initializing SPI...");
  delay(1000);

  // SPI設定
  SPI.setRX(SD_MISO);
  SPI.setTX(SD_MOSI);
  SPI.setSCK(SD_SCK);

  // SDカードの接続判定
  if (SD.begin(SD_CS) == false){
    Serial.println("SD initialization failed.");
  } else {
    Serial.println("SD initialization done.");
  }

  // データファイルがあるかどうか確認
  if(SD.exists(DATA_FILE)) {
    Serial.println(DATA_FILE + " exists.");
  } else {
    Serial.println("Cannot find " + DATA_FILE);
  }
}

// 戻り値: 1行読み込み成功したらtrue、ファイル末尾に達したらfalse
bool readLog(void){
  // ファイルが閉じていれば、新しく開く（最初の1回目や、全行読み終わった後の再スタート用）
  if (!myfile) {
    myfile = SD.open(DATA_FILE, FILE_READ);
    if (!myfile) {
      Serial.println("Cannot open " + DATA_FILE);
      return false;
    }
    Serial.println("reading start...");
    ROW_SIZE = 0;
  }

  String buffer = ""; 
  int column_count = 0; 

  // 1行分、またはファイルの終端まで読み進める
  while (myfile.available() > 0){
    char c = myfile.read(); 

    // ','（カンマ）が来た場合
    if (c == ','){
      if (column_count < COLUMN_SIZE) {
        read_data[column_count] = buffer;
      }
      column_count++;
      buffer = "";
    }
    // 改行コードが来た場合（ここで1行分が確定）
    else if (c == '\n') {
      if (column_count < COLUMN_SIZE) {
        read_data[column_count] = buffer;
      }
      ROW_SIZE++;
      return true; // 1行読めたので、一旦関数を抜けて送信処理へ
    }
    else if (c != '\r') {
      buffer += c;
    }
  }

  // ファイルの終端に達したとき、改行コードがなくバッファにデータが残っていれば最後の行として処理
  if (buffer.length() > 0){
    if (column_count < COLUMN_SIZE) {
      read_data[column_count] = buffer;
    }
    ROW_SIZE++;
    return true;
  }

  // ファイルの全行を読み切った場合
  myfile.close(); // ファイルを完全に閉じる
  Serial.print("finish reading. Total rows: ");
  Serial.println(ROW_SIZE);
  return false; 
}

// 1行まるごと","区切りでSerial1（各基板）へ送信する関数
void transmitLog(void) {
  String send_line = "";
  
  for (int i = 0; i < COLUMN_SIZE; i++) {
    send_line += read_data[i];
    if (i < COLUMN_SIZE - 1) {
      send_line += ","; // 要素の間にカンマを挟む
    }
  }
  
  Serial1.println(send_line); // 各基板へ送信（末尾に改行を付与）
}

uint32_t last_time = 0;
void timer_100Hz(void){
  // 10ms以上経過したか判定
  if (millis() - last_time >= 10){
    last_time = millis(); //
    
    // 1行読み込みに成功したら、それを送信する
    if (readLog()) {
      transmitLog();
    }
  }
}

void loop() {
  timer_100Hz();
}