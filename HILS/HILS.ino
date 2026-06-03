#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

// UART用
const int SERIAL_TX = 0;
const int SERIAL_RX = 1;

// SD用SPI
const int MISO = 4;
const int MOSI = 7;
const int CS = 5;
const int SCK = 6;

const String DATA_FILE = "data.csv";
const int DATA_SIZE = 5; // CSVの列数


File myfile;

void setup() {
  // put your setup code here, to run once:
  // PCとの通信用
  Serial.begin(115200);

  // UART0を使用
  Serial1.setTX(SERIAL_TX);
  Serial1.setRX(SERIAL_RX);
  Serial1.begin(460800); // 各基板との通信速度を設定
  delay(1000);

  Serial.println("Initializing SPI...");

  // SPI設定
  SPI.setRX(MISO);
  SPI.setTX(MOSI);
  SPI.setSCK(SCK);

  // SDカードの接続判定
  if (SD.begin(CS) == false){
    // SDカードに接続できなかった場合
    Serial.println("SD initialization failed.");
  } else {
    Serial.println("SD initialization done.");
  }
  
  // ファイルを読み込みモードでopen
  myfile = SD.open(DATA_FILE, FILE_READ);


  // データファイルがあるかどうか確認
  if(SD.exists(DATA_FILE)) {
    // SDカード内に読み出し対象のデータがある場合
    Serial.println(DATA_FILE + "exists.");
  } else {
    Serial.println("Cannot find " + DATA_FILE);
  }

}


void readLog(){
  if (myfile) {
    Serial.println("reading...");
    String buffer = ""; // 読み込んだ文字を保存するバッファ
    float read_data[DATA_SIZE]; // 読み込んだ数値を保存する配列
    int data_count = 0; // カウンターを初期化

    while (myfile.available() > 0){
      char c = myfile.read(); // 1文字読み込む

      // ','（カンマ）が来た場合
      if (c == ','){
        Serial.println(buffer);
        read_data[data_count] = buffer;
        data_count++;
        buffer = "";
      }

      // 改行コードが来た場合
      else if (c == '\n') {
        Serial.println(buffer);
        buffer = "";
        read_data[data_count] = buffer;
        data_count = 0;
      }

      else if (c != '\r') {
        // CRは無視してそれ以外の文字はバッファに追加
        buffer += c;
      }
    }
    // ファイルの終端に達したとき，バッファにデータが残っていれば出力
    if (buffer.length() > 0){
      Serial.println(buffer);
    }

    // ファイルを閉じる
    myfile.close();
    Serial.println("finish reading");

  } else {
    Serial.println("Cannot open " + DATA_FILE );
  }
}


void loop() {
  // put your main code here, to run repeatedly:

}