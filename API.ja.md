# Xiamocon API リファレンス

## Platform API

Platform API は MCU やボードの違いを吸収し抽象化するレイヤです。
GPIO、I2C、SPI、DMA、タイマ、電源などを制御するための低レベル API を提供します。

### GPIO (`xmc/hw/gpio.h`)

XIAO ボード本体の GPIO を制御します。

#### xmc_gpio_set_dir

GPIO の方向を設定します。

```c
void xmc_gpio_set_dir(int pin, bool output);
```

- pin: GPIO ピン番号
- output: false にすると GPIO を入力に、true にすると GPIO を出力に設定します。

#### xmc_gpio_write

GPIO の出力値を設定します。

設定された出力値の状態は GPIO の方向に関わらず保持されますが、その値は GPIO が出力に設定されている間だけ出力されます。

```c
void xmc_gpio_write(int pin, bool value);
```

- pin: GPIO ピン番号
- value: true にすると GPIO に high を出力し、そうでなければ low を出力します。

#### xmc_gpio_read

GIPO の入力値を読み取ります。

```c
bool xmc_gpio_read(int pin);
```

- pin: GPIO ピン番号
- 戻り値: low であれば false を、high であれば true を返します。

#### xmc_gpio_set_pullup

GPIO のプルアップ抵抗を設定します。

プルアップの強さは MCU の仕様に依存します。

```c
void xmc_gpio_set_pullup(int pin, bool enable);
```

- pin: GPIO ピン番号
- enable: true にすると GPIO のプルアップ抵抗を有効に設定し、そうでなければ無効にします。

### I2C (`xmc/hw/i2c.h`)

XIAO ボード本体の I2C ペリフェラルを制御します。

#### xmc_i2c_device_t

Xiamicon に搭載されている I2C デバイスを指定するための列挙型です。

|識別子|デバイス|アドレス|説明|
|:--|:--|:--:|:--|
|`XMC_I2C_DEV_IOEX`|PCA9555PW|0x20|IO エキスパンダー|
|`XMC_I2C_DEV_BAT_MON`|T.B.D.|T.B.D.|バッテリー電圧 ADC|

#### xmc_i2c_get_preferred_frequency

指定されたデバイスに対する I2C クロック周波数の推奨値を返します。

```c
uint32_t xmc_i2c_get_preferred_frequency(xmc_i2c_device_t device);
```

- device: I2C デバイス
- 戻り値: Hz 単位の SCL 周波数

#### xmc_status_t xmc_i2c_init

MCU の I2C ペリフェラルと関連するリソースを初期化します。

```c
xmc_status_t xmc_i2c_init();
```

- 戻り値: エラーコード

#### void xmc_i2c_deinit

MCU の I2C ペリフェラルと関連するリソースを解放します。

```c
void xmc_i2c_deinit();
```

#### xmc_i2c_start_transaction

I2C ペリフェラルをロックして占有します。
I2C デバイスへのアクセスや設定変更を行う前に呼び出す必要があります。
`xmc_i2c_end_transaction` が呼び出されるまでの間、他のタスクによる I2C デバイスへのアクセスはブロックされます。

```c
xmc_status_t xmc_i2c_start_transaction();
```

- 戻り値: エラーコード

#### xmc_status_t xmc_i2c_end_transaction

I2C ペリフェラルのロックを解除します。
I2C デバイスへのアクセスや設定変更を行った後に呼び出す必要があります。

```c
xmc_status_t xmc_i2c_end_transaction();
```

- 戻り値: エラーコード

#### xmc_i2c_set_baudrate

SCL 周波数を Hz 単位で指定します。

```c
xmc_status_t xmc_i2c_set_baudrate(uint32_t baudrate);
```

- baudrate: Hz 単位の SCL 周波数
- 戻り値: エラーコード

#### xmc_i2c_write_blocking

I2C デバイスに書き込みアクセスを行います。

```c
xmc_status_t xmc_i2c_write_blocking(
    uint8_t dev_addr, const uint8_t *data, uint32_t size, bool nostop);
```

- dev_addr: デバイスアドレス
- data: 書き込むバイト列へのポインタ
- size: 書き込むバイト数 (1以上)
- nostop: true に設定すると、書き込みの後に Stop コンディションを送信せずに次の書き込みを続けます。
- 戻り値: エラーコード

#### xmc_i2c_read_blocking

I2C デバイスに読み出しアクセスを行います。

```c
xmc_status_t xmc_i2c_read_blocking(
    uint8_t dev_addr, uint8_t *data, uint32_t size, bool nostop);
```

- dev_addr: デバイスアドレス
- data: 読み出したデータの格納先ポインタ
- size: 読み出すバイト数 (1以上)
- nostop: true に設定すると、読み出しの後に Stop コンディションを送信せずに次の読み出しを続けます。
- 戻り値: エラーコード
