//xiao rp2040向け
//受信テスト

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial1.setTX(0);
  Serial1.setRX(1);
  Serial1.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (Serial1.available() > 0) {
    // シリアルデータの受信 (改行まで)
    String data = Serial1.readStringUntil('\n');

    // 受信したデータを出力
    Serial.print("Receive:");
    Serial.println(data);
  }
}
