//raspberry pi pico向け

void setup() {
  // put your setup code here, to run once:
  Serial1.setTX(0);
  Serial1.setRX(1);
  Serial1.begin(9600);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial1.println("send from raspberry pi pico");
  Serial.println("send from raspberry pi pico");
  delay(200);
}
