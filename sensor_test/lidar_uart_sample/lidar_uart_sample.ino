//https://akizukidenshi.com/goodsaffix/TSD20%20user%20manual.pdf

/*
UARTで送られてくるデータの構造
Byte    内容           例
1     Frame Header    0x5C（固定）
2     Distance_L      0x02
3     Distance_H      0x11
4      Checksum       0xEC

距離データはlittle-endianのため下位バイト→上位バイトの順に送られてくる．
0x02,0x11の順に来た場合，距離は0x1102 = 4354mmとなる．

チェックサムの仕組み：0x02,0x11の場合
0x02+0x11 = 0x13
ビット反転(NOT)→ ~0x13=0xFF - 0x13 = 0xEC 

*/

#include <SerialPIO.h>

SerialPIO myserial(3, 2, 256);  // RX=3, TX=2


//Checksumの計算．
uint8_t calcChecksum(uint8_t *data, uint8_t len) {
  uint8_t sum = 0;
  for (int i = 0; i < len; i++) sum += data[i];
  return ~sum;
}

void setup() {
  Serial.begin(460800);
  while (!Serial) {}
  Serial.println("Serial begin");

  myserial.begin(460800);
}

void loop() {
  if (myserial.available() >= 4) {
    if (myserial.read() == 0x5C) {  // Frame Header
      uint8_t dist_L = myserial.read();
      uint8_t dist_H = myserial.read();
      uint8_t recv_chk = myserial.read();

      uint8_t data_for_chk[2] = {dist_L, dist_H};
      uint8_t calc_chk = calcChecksum(data_for_chk, 2);

      if (recv_chk == calc_chk) {
        uint16_t distance_mm = (dist_H << 8) | dist_L;  // Little-endian
        if (distance_mm == 5000)
          Serial.println("Out of range");
        else {
          Serial.print("Distance: ");
          Serial.print(distance_mm);
          Serial.println(" mm");
        }
      } else {
        Serial.println("Checksum error");
      }
    }
  }
}
