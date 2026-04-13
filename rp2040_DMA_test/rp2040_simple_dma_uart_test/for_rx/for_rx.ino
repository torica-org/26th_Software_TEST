//受信用

#include <Arduino.h>
#include "hardware/dma.h"

char rx_buffer[64]; // 受信バッファ
int rx_dma_chan;
dma_channel_config rx_cfg;

void setup() {
    Serial.begin(9600);
    Serial1.setTX(0);
    Serial1.setRX(1);
    Serial1.begin(460800);

    // 1. DMAチャンネルの確保
    rx_dma_chan = dma_claim_unused_channel(true);
    rx_cfg = dma_channel_get_default_config(rx_dma_chan);

    // 2. 設定（8bit転送、読み出し固定、書き込み増分あり、UART0_RXと同期）
    channel_config_set_transfer_data_size(&rx_cfg, DMA_SIZE_8);
    channel_config_set_read_increment(&rx_cfg, false); // UART窓口は固定
    channel_config_set_write_increment(&rx_cfg, true);  // 配列側をずらす
    channel_config_set_dreq(&rx_cfg, DREQ_UART0_RX);

    // 3. 受信待機開始（32バイト分受け取るまでCPUを介さず裏で動く）
    start_rx_dma();
}

// DMA受信をリセットして開始する関数
void start_rx_dma() {
    memset(rx_buffer, 0, sizeof(rx_buffer)); // バッファを掃除
    dma_channel_configure(
        rx_dma_chan, &rx_cfg,
        rx_buffer,      // 書き込み先：配列
        &uart0_hw->dr,  // 読み出し元：UART0のデータレジスタ
        32,             // 32バイト分溜まるまで待つ
        true            // 即座に受信待機開始
    );
}

// 受信側 (XIAO)
void loop() {
    static bool dma_busy = false;

    if (!dma_busy) {
        // 1. CPUが先頭の '$' を探す
        if (Serial1.available() > 0) {
            if (Serial1.read() == '$') {
                // '$' を見つけた！
                rx_buffer[0] = '$';
                
                // 2. 残りの31バイトをDMAで一気に拾う
                dma_channel_configure(
                    rx_dma_chan, &rx_cfg,
                    &rx_buffer[1],  // [1]から書き込む
                    &uart0_hw->dr,
                    31,             // 残り31バイト
                    true            // 今すぐ開始
                );
                dma_busy = true;
            }
        }
    } else {
        // 3. 31バイト拾い終わったか確認
        if (!dma_channel_is_busy(rx_dma_chan)) {
            dma_busy = false;
            rx_buffer[31] = '\0'; // 終端処理
            Serial.print("Fixed Receive: ");
            Serial.println(rx_buffer);
            
            // 4. UARTバッファに残っているゴミ（あれば）を掃除
            while(Serial1.available()) Serial1.read();
        }
    }
}