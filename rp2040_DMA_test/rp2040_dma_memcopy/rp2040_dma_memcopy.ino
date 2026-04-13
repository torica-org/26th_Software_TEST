/*
CPUを使わずにDMAを使って配列Aの内容を配列Bに高速コピーするだけのコード
*/

#define count 50
#define benchmark_loops 1000

#include "hardware/dma.h"

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while (!Serial) {
    ;
  }
  delay(1000);
  test_dma_mem_to_mem();
  delay(500);
  test_cpu_mem_to_mem();
}

void loop() {
  // put your main code here, to run repeatedly:

}


void test_dma_mem_to_mem(){
  //初期条件設定
  float source[count];
  float dest[count];

  //テストデータの準備
  for (int i=0; i < count; i++) {
    source[i] = (float)i * 1.1f; //0 ~ 49*1.1までのfloat型データを用意
    dest[i] = 0.0f;  
  }

  //空いているDMAチャンネルを確保
  int chan = dma_claim_unused_channel(true);
  dma_channel_config c = dma_channel_get_default_config(chan);

  //float用に32bit単位に，読み出し＆書き出しアドレスを増やす
  channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
  channel_config_set_read_increment(&c, true);
  channel_config_set_write_increment(&c, true);

  uint32_t start1 = micros();
  for (int n = 0; n < benchmark_loops; n++) {
    //転送を実行
    dma_channel_configure(
      chan, /*チャンネル*/
      &c, /*設定*/
      dest, /*書き込み先(dest)*/
      source, /*読み出し元*/
      count, /*転送回数*/
      true /*即実行*/
    );

    //完了を待つ（テスト用コードなのでブロッキング）
    dma_channel_wait_for_finish_blocking(chan);
  }
  uint32_t end1 = micros();

  //結果確認
  bool success = true;
  //配列の中身を全確認
  for (int i=0; i<count; i++){
    if (dest[i] != source[i]) {
      success = false;
    }
  }

  if (success == true) {
    Serial.println("DMA copy Success!");
    Serial.print("Total Time (us): ");
    Serial.println(end1 - start1);
    Serial.print("Per copy (us): ");
    Serial.println((end1 - start1) / (float)benchmark_loops, 3);

  } else {
    Serial.println("Failed");
  }

  //DMAチャンネルを解放
  dma_channel_unclaim(chan);
}

void test_cpu_mem_to_mem(){
  //初期条件設定
  float source2[count];
  float dest2[count];

  //テストデータの準備
  for (int i=0; i < count; i++) {
    source2[i] = (float)i * 1.1f; //0 ~ 49*1.1までのfloat型データを用意
    dest2[i] = 0.0f;  
  }

  uint32_t start2 = micros();

  for (int n = 0; n < benchmark_loops; n++) {
    //データコピー
    for (int i=0; i < count; i++) {
      dest2[i] = source2[i];
    }
  }

  uint32_t end2 = micros();

  bool success = true;
  for (int i=0; i<count; i++) {
    if (dest2[i] != source2[i]) {
      success = false;
    }
  }

  if (success) {
    Serial.println("CPU only end.");
    Serial.print("Total Time (us): ");
    Serial.println(end2 - start2);
    Serial.print("Per copy (us): ");
    Serial.println((end2 - start2) / (float)benchmark_loops, 3);
  } else {
    Serial.println("CPU copy Failed");
  }
}


