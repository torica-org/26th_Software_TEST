#include "hardware/dma.h"

const int DATA_SIZE = 20000;
uint32_t src[DATA_SIZE], dest[DATA_SIZE];

// 足し算をさせるだけのコード
void heavy_calculation() {
  volatile uint32_t sum = 0;
  for (int i = 0; i < 100000; i++) {
    sum += i;
  }
}

void setup() {
  Serial.begin(9600);
  while (!Serial);

  delay(1500);

  for (int i = 0; i < DATA_SIZE; i++) {
    src[i] = i;
  }

  Serial.println("--- DMA Benchmark ---");
  delay(100);

  // CPUのみ（転送してから計算する）
  uint32_t start1 = micros();
  // 1. 転送
  for (int i = 0; i < DATA_SIZE; i++) {
    dest[i] = src[i];
  }
  // 2. 計算
  heavy_calculation();
  uint32_t end1 = micros();
  Serial.print("CPU Serial Work: ");
  Serial.print(end1 - start1);
  Serial.println(" us");

  delay(1000);

  // --- 【実験2】DMA使用（転送中に計算する） ---
  int chan = dma_claim_unused_channel(true);
  dma_channel_config c = dma_channel_get_default_config(chan);
  channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
  channel_config_set_read_increment(&c, true);
  channel_config_set_write_increment(&c, true);

  uint32_t start2 = micros();
  // 1. 転送開始（バックグラウンド）
  dma_channel_configure(chan, &c, dest, src, DATA_SIZE, true);
  // 2. 転送が終わるのを待たずに計算開始
  heavy_calculation();
  // 3. 両方が終わるのを待つ
  dma_channel_wait_for_finish_blocking(chan);
  uint32_t end2 = micros();
  Serial.print("DMA Parallel Work: ");
  Serial.print(end2 - start2);
  Serial.println(" us");

  dma_channel_unclaim(chan);
}

void loop() {
}