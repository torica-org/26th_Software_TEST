//送信用

#include <Arduino.h>
#include "hardware/dma.h"

// 送信データ
const char tx_msg[32] = "$send from rpi pico. test"; // 32バイト固定にする
int tx_dma_chan;

void setup() {
    Serial.begin(115200);
    Serial1.setTX(0);
    Serial1.setRX(1);
    Serial1.begin(460800);
    pinMode(LED_BUILTIN, OUTPUT);

    // DMAチャンネルの確保
    tx_dma_chan = dma_claim_unused_channel(true);
    dma_channel_config c = dma_channel_get_default_config(tx_dma_chan);

    // 設定（8bit転送、読み出し増分あり、書き込み固定、UART0_TXと同期）
    channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
    channel_config_set_read_increment(&c, true);
    channel_config_set_write_increment(&c, false);
    channel_config_set_dreq(&c, DREQ_UART0_TX); // GP0/1はUART0

    // 基本設定の適用（まだ開始はしない）
    dma_channel_configure(
        tx_dma_chan, &c,
        &uart0_hw->dr, // 書き込み先：UART0のデータレジスタ
        tx_msg,        // 読み出し元：文字列
        32,            // 転送バイト数
        false          // 今すぐ開始はしない
    );
}

void setup1(){

}

bool send = false;

void loop() {
    // 200msごとにDMAを再起動
    send = false;
    
    if (!dma_channel_is_busy(tx_dma_chan)) {
        
        dma_channel_set_read_addr(tx_dma_chan, tx_msg, true);
        send = true;
    }
    digitalWrite(LED_BUILTIN, HIGH);
    delay(200);
    digitalWrite(LED_BUILTIN, LOW);
}

void loop1(){
    if (send == true){
        Serial.println("DMA TX working...");
    } else {
        Serial.println("Failed to transmit");
    }
}