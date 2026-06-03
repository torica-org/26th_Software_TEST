#include <Arduino.h>

TaskHandle_t xSensorTaskHandle = NULL;

// ハードウェアタイマーの割り込み関数（10msごとに発火）
void IRAM_ATTR onTimer() { // プログラムをIRAMに配置して高速読み出し
  BaseType_t xHigherPriorityTaskWoken = pdFALSE; // 割り込みにより今動いていたタスクよりも優先度が高いタスクが動いたかどうか判別するフラグ
  vTaskNotifyGiveFromISR(xSensorTaskHandle, &xHigherPriorityTaskWoken); // xSensorTaskHandleを使いsensorTaskに起動シグナルを送る

  // xHigherPriorityTaskWokenが成立した（＝最優先のsensorTaskが起きた）なら，割り込みから戻る瞬間にsensorTaskへと移行させる
  if (xHigherPriorityTaskWoken) {
    portYIELD_FROM_ISR();
  }
}

// delay()を使わずに500msおきに反転させる関数
void LED_blink() {
  static int counter = 0; // 起きた回数を記録する変数（関数が終わっても値を記憶する）

  counter++;              // 10msごとに1ずつ増える

  if (counter >= 50) {    // 50回増える ＝ 500ms 経った！
    counter = 0;          // カウンターをリセット
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN)); // LEDの状態を反転（点灯⇔消灯）
  }
}

void sensorTask(void *pvParameters) {
  // LEDピンを出力モードに設定
  pinMode(LED_BUILTIN, OUTPUT);

  while (1) {
    // 10ms経つまでここで完全休止
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    // ---- [10msごとに実行されるエリア] ----
    
    // 1. LEDの点滅制御（1回あたり数ナノ秒で終わる超ノンブロッキング処理）
    LED_blink();

    // 2. I2Cセンサー読み取り
    // 3. UART（GPS）読み取り
    // 4. キューへのデータ送信
    // ------------------------------------
  }
}

void setup() {
  Serial.begin(115200);

  // タスクを生成
  xTaskCreatePinnedToCore(sensorTask, "SensorTask", 4096, 
  NULL, 3, &xSensorTaskHandle, 0);
  /* xTaskCreatePinnedToCore(
  1. pvTaskCode (実行する関数名) ex) sensorTask
  2. pcName （タスクの識別名） ex) "SensorTask"
  3. usStackDepth（メモリの割り当て量[byte]） ex)4096
  4. pvParameters（関数に渡す引数） ex) NULL
  5. uxPriority（優先順位） ex) 3
  6. pxCreatedtask（操作用のリモコン） ex) &xSensorTaskHandle
  7. xCoreID（配属するコア） ex) 0
  ) */

  // ハードウェアタイマーの設定（10ms周期）
  hw_timer_t *timer = timerBegin(1000000); // タイマーの周波数．Hz単位．1000000Hz = 1μs
  timerAttachInterrupt(timer, &onTimer); // 第一引数：タイマーの指定，第二引数：タイマーがなった瞬間に実行したい関数名
  timerAlarm(timer, 10000, true, 0); // 第一引数：タイマーの名前，第二引数：何カウント目でアラームを鳴らすか(10000μs = 10ms)，第三引数：自動リピートするか，第四引数：自動リピートしたときにカウントをいくつから再スタートさせるか
}

void loop() {
  // ここは空っぽ。Core 1は一切CPUを消費せず、いつでもSD書き込みやBluetoothを動かせる状態。
}