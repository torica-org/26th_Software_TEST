# Teseo LIV3FLの使い方と留意事項
使ってて，ん？となった部分や，困ったこと＆その対処法などを残しておきます．


## マイコンとの接続
UARTで接続するだけ．電源が5Vではなく**3.3V**の点に注意．
電源を入れると1PPSから信号が出始めます．

## 各種設定
基本的にはSoftware Manualに従って送信すればOK.

## 設定変更コマンドのフォーマット


### Baud rate設定



## 留意事項とかトラブルシューティング

### Teseoから出力される文字列が滅茶苦茶なものしか出なくなった

[症状] 正しいBaud rateに設定しても，Teseoから出力される文字が文字化け？したような状態で出力される．
[原因] おそらく設定変更コマンドをむやみやたらに送信しまくって，モジュール内フラッシュメモリにある設定保存領域を破壊したこと．
[試したこと] 電源ON/OFFを繰り返す・すべてのbaud rateで設定クリア＆リセットコマンドを送信・リセットピン(9番)をGNDに落とす．→どれも解決に至らず
[成功した解決策] Teseo Suite ProをPCにインストールし，ファームウェア書き換え．
（必要なもの）Windows PC, Arduino Uno R3（部室に複数ある），Teseo Suite ProとTeseo LIV3FLのファームウェア（どちらもSTMicroのサイトからダウンロードできる）
1. TeseoとPCを接続．USB-シリアル変換器を持ってないので，Arduino Uno R3にあるRX/TX（0,1番ピン）と接続．なおArduino Uno R3には空のスケッチを書き込んでおく．
注）Teseoと接続する前に空スケッチを書き込んでおくこと．Teseoの電源を落とした状態でも繋げたままだと書き込むことができない．Teseoの電源OFFでも書き込み時にUSBの通信と干渉してるっぽい
2. 上のメニューバーから`Tools`->`Teseo Firmware Upgrade`を選択．
3. `Port Settings`からCOMポートを選択，`Product`を`Other products`，`Loader baud rate`を設定したbaud rateに（私は115200bpsで成功），`Settings`では`Recovery`，`Restore factory settings`にチェックを入れる．次に`Firmware`の`Binary`ボタンを押して，ダウンロードしておいたFirmwareを選択．
なおこの設定では，TeseoがRecoveryモードに入り，ファームウェアが書き換えられ，工場出荷時の設定に戻される．
4. Teseoのリセットピン(9番)をGNDに落としながらTeseo Firmware Upgradeにある`START`ボタンを押す．9番ピンをGNDから接続を切り離し，あとは待つだけ．`Status`が`Sync OK`になると，自動的にファームウェア書き換えに移行する．