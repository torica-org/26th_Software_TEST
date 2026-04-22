char data; //データ格納用の変数を準備

void setup() {
	// put your setup code here, to run once:
	Serial1.begin(460800);
  Serial.begin(460800); //UART0をボーレート：115200bpsで初期化
	pinMode(2,OUTPUT); //GP02を出力ピンとして初期化
}

void loop() {
	// put your main code here, to run repeatedly:
	//UART0でデータを受信したら
		data = Serial1.read(); //受信したデータをdataに格納
  
     Serial.print(data);
}