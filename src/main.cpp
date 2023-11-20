/*
   MIT License

  Copyright (c) 2023 Felix Biego

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

  ______________  _____
  ___  __/___  /_ ___(_)_____ _______ _______
  __  /_  __  __ \__  / _  _ \__  __ `/_  __ \
  _  __/  _  /_/ /_  /  /  __/_  /_/ / / /_/ /
  /_/     /_.___/ /_/   \___/ _\__, /  \____/
                              /____/

*/

#define LGFX_USE_V1
#include "Arduino.h"
#include <LovyanGFX.hpp>

#include <ChronosESP32.h>
#include <Timber.h>
#include "FS.h"
#include "SPIFFS.h"

#ifdef DEVKIT
#include "Button2.h"
#else
#include "CST816D.h"
#endif

#include "main.h"

#define buf_size 50

#define DIALS_MAX 20

class LGFX : public lgfx::LGFX_Device
{

  lgfx::Panel_GC9A01 _panel_instance;
  lgfx::Light_PWM _light_instance;
  lgfx::Bus_SPI _bus_instance;

public:
  LGFX(void)
  {
    {
      auto cfg = _bus_instance.config();

      // SPIバスの設定
      cfg.spi_host = SPI; // 使用するSPIを選択  ESP32-S2,C3 : SPI2_HOST or SPI3_HOST / ESP32 : VSPI_HOST or HSPI_HOST
      // ※ ESP-IDFバージョンアップに伴い、VSPI_HOST , HSPI_HOSTの記述は非推奨になるため、エラーが出る場合は代わりにSPI2_HOST , SPI3_HOSTを使用してください。
      cfg.spi_mode = MODE;               // SPI通信モードを設定 (0 ~ 3)
      cfg.freq_write = 80000000;         // 传输时的SPI时钟（最高80MHz，四舍五入为80MHz除以整数得到的值）
      cfg.freq_read = 20000000;          // 接收时的SPI时钟
      cfg.spi_3wire = true;              // 受信をMOSIピンで行う場合はtrueを設定
      cfg.use_lock = true;               // 使用事务锁时设置为 true
      cfg.dma_channel = SPI_DMA_CH_AUTO; // 使用するDMAチャンネルを設定 (0=DMA不使用 / 1=1ch / 2=ch / SPI_DMA_CH_AUTO=自動設定)
      // ※ ESP-IDFバージョンアップに伴い、DMAチャンネルはSPI_DMA_CH_AUTO(自動設定)が推奨になりました。1ch,2chの指定は非推奨になります。
      cfg.pin_sclk = SCLK; // SPIのSCLKピン番号を設定
      cfg.pin_mosi = MOSI; // SPIのCLKピン番号を設定
      cfg.pin_miso = MISO; // SPIのMISOピン番号を設定 (-1 = disable)
      cfg.pin_dc = DC;     // SPIのD/Cピン番号を設定  (-1 = disable)

      _bus_instance.config(cfg);              // 設定値をバスに反映します。
      _panel_instance.setBus(&_bus_instance); // バスをパネルにセットします。
    }

    {                                      // 表示パネル制御の設定を行います。
      auto cfg = _panel_instance.config(); // 表示パネル設定用の構造体を取得します。

      cfg.pin_cs = CS;   // CSが接続されているピン番号   (-1 = disable)
      cfg.pin_rst = RST; // RSTが接続されているピン番号  (-1 = disable)
      cfg.pin_busy = -1; // BUSYが接続されているピン番号 (-1 = disable)

      // ※ 以下の設定値はパネル毎に一般的な初期値が設定さ BUSYが接続されているピン番号 (-1 = disable)れていますので、不明な項目はコメントアウトして試してみてください。

      cfg.memory_width = 240;   // ドライバICがサポートしている最大の幅
      cfg.memory_height = 240;  // ドライバICがサポートしている最大の高さ
      cfg.panel_width = 240;    // 実際に表示可能な幅
      cfg.panel_height = 240;   // 実際に表示可能な高さ
      cfg.offset_x = 0;         // パネルのX方向オフセット量
      cfg.offset_y = 0;         // パネルのY方向オフセット量
      cfg.offset_rotation = 0;  // 值在旋转方向的偏移0~7（4~7是倒置的）
      cfg.dummy_read_pixel = 8; // 在读取像素之前读取的虚拟位数
      cfg.dummy_read_bits = 1;  // 读取像素以外的数据之前的虚拟读取位数
      cfg.readable = false;     // 如果可以读取数据，则设置为 true
      cfg.invert = true;        // 如果面板的明暗反转，则设置为 true
      cfg.rgb_order = false;    // 如果面板的红色和蓝色被交换，则设置为 true
      cfg.dlen_16bit = false;   // 对于以 16 位单位发送数据长度的面板，设置为 true
      cfg.bus_shared = false;   // 如果总线与 SD 卡共享，则设置为 true（使用 drawJpgFile 等执行总线控制）

      _panel_instance.config(cfg);
    }

    {                                      // Set backlight control. (delete if not necessary)
      auto cfg = _light_instance.config(); // Get the structure for backlight configuration.

      cfg.pin_bl = BL;     // pin number to which the backlight is connected
      cfg.invert = false;  // true to invert backlight brightness
      cfg.freq = 44100;    // backlight PWM frequency
      cfg.pwm_channel = 1; // PWM channel number to use

      _light_instance.config(cfg);
      _panel_instance.setLight(&_light_instance); // Sets the backlight to the panel.
    }

    setPanel(&_panel_instance); // 使用するパネルをセットします。
                                //    { // バックライト制御の設定を行います。(必要なければ削除）
                                //    auto cfg = _light_instance.config();// バックライト設定用の構造体を取得します。
                                //    cfg.pin_bl = 8;             // バックライトが接続されているピン番号 BL
                                //    cfg.invert = false;          // バックライトの輝度を反転させる場合 true
                                //    cfg.freq   = 44100;          // バックライトのPWM周波数
                                //    cfg.pwm_channel = 7;         // 使用するPWMのチャンネル番号
                                //    _light_instance.config(cfg);
                                //    _panel_instance.setLight(&_light_instance);//バックライトをパネルにセットします。
                                //    }
  }
};

LGFX tft;

ChronosESP32 watch("Chronos C3");

#ifdef DEVKIT
Button2 button = Button2(0);
#else
CST816D touch(I2C_SDA, I2C_SCL, TP_RST, TP_INT);
#endif



String dials[DIALS_MAX];
int dNo = 0;
int cNo = 0;


void loadDial();

void listDir(fs::FS &fs, const char *dirname, uint8_t levels)
{

  Timber.i("Available space: %d", (SPIFFS.totalBytes() - SPIFFS.usedBytes()));

  Serial.printf("Listing directory: %s\r\n", dirname);

  File root = fs.open(dirname);
  if (!root)
  {
    Serial.println("- failed to open directory");
    return;
  }
  if (!root.isDirectory())
  {
    Serial.println(" - not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file)
  {
    if (file.isDirectory())
    {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels)
      {
        listDir(fs, file.path(), levels - 1);
      }
    }
    else
    {

      if (String(file.name()).endsWith(".bin"))
      {
        Serial.print("  DIAL: ");
        Serial.print(file.name());

        if (dNo < DIALS_MAX)
        {
          dials[dNo] = "/" + String(file.name());
          Serial.print("\tADDED: ");
          Serial.print(dials[dNo]);
          dNo++;
        }
        Serial.print("\tSIZE: ");
        Serial.println(file.size());
      }
      else
      {
        Serial.print("  FILE: ");
        Serial.print(file.name());
        Serial.print("\tSIZE: ");
        Serial.println(file.size());
      }
    }
    file = root.openNextFile();
  }
}

bool readDialBytes(uint8_t *data, size_t offset, size_t size)
{
  File file = SPIFFS.open(dials[cNo].c_str(), "r");
  if (!file)
  {
    Serial.println("Failed to open file for reading");
    return false;
  }

  if (!file.seek(offset))
  {
    Serial.println("Failed to seek file");
    file.close();
    return false;
  }

  int bytesRead = file.readBytes((char *)data, size);

  if (bytesRead <= 0)
  {
    Serial.println("Error reading file");
    file.close();
    return false;
  }

  file.close();
  return true;
}

bool isKnown(uint8_t id)
{
  if (id < 0x1E)
  {
    if (id != 0x04 || id != 0x05 || id != 0x12 || id != 0x18 || id != 0x20)
    {
      return true;
    }
  }
  else
  {
    if (id == 0xFA || id == 0xFD)
    {
      return true;
    }
  }
  return false;
}

void connectionCallback(bool state)
{
  // Timber.d("Connection change");
}

void notificationCallback(Notification notification)
{
  Timber.d("Notification Received from " + notification.app + " at " + notification.time);
  Timber.d(notification.message);
}

void configCallback(Config config, uint32_t a, uint32_t b)
{
  switch (config)
  {
  case CF_WEATHER:

    break;
  case CF_FONT:

    break;
  case CF_CAMERA:

    break;
  }
}

void rawDataCallback(uint8_t *data, int length)
{
  // Serial.print(length);
  // Serial.print("->");
  for (int i = 0; i < length; i++)
  {
    Serial.printf("%02X ", data[i]);
  }
  Serial.println();
}
void logCallback(Level level, unsigned long time, String message)
{
  Serial.print(message);
}

#ifdef DEVKIT
void handler(Button2 &btn)
{
  switch (btn.getClickType())
  {
  case SINGLE_CLICK:
    cNo++;
    if (cNo >= dNo)
    {
      cNo = 0;
    }
    loadDial();
    break;
  case DOUBLE_CLICK:
    cNo--;
    if (cNo < 0)
    {
      cNo = dNo - 1;
    }
    loadDial();
    break;
  case TRIPLE_CLICK:
    Serial.print("triple ");
    // SPIFFS.format();
    break;
  case LONG_CLICK:
    Serial.print("long");

    break;
  }
  Serial.print("click");
  Serial.print(" (");
  Serial.print(btn.getNumberOfClicks());
  Serial.println(")");
}

#else

#endif

void setup()
{

  Serial.begin(115200); 
  Timber.setLogCallback(logCallback);

  Timber.i("Starting up device");

  if (!SPIFFS.begin(true))
  {
    Serial.println("SPIFFS Mount Failed");
    ESP.restart();
  }


  listDir(SPIFFS, "/", 0);

  tft.init();
  tft.initDMA();
  tft.startWrite();

#ifdef DEVKIT
  button.setClickHandler(handler);
  button.setLongClickHandler(handler);
  button.setDoubleClickHandler(handler);
  button.setTripleClickHandler(handler);
#else
  touch.begin();
#endif

  watch.setConnectionCallback(connectionCallback);
  watch.setNotificationCallback(notificationCallback);
  watch.setConfigurationCallback(configCallback);
  watch.setRawDataCallback(rawDataCallback);
  watch.begin();
  watch.set24Hour(true);

  String about = "v1.0 [fbiego]\nESP32 C3 Mini\n" + watch.getAddress();

  tft.setBrightness(100);

  Timber.i("Setup done");
  Timber.i(about);

  loadDial();
}

void loop()
{
  watch.loop();

#ifdef DEVKIT
  button.loop();
#else
  bool touched;
  uint8_t gesture;
  uint16_t touchX;
  uint16_t touchY;

  if (touch.getTouch(&touchX, &touchY, &gesture))
  {
    uint8_t g = gesture;
    // Timber.e("Gesture: %d", g);
    switch (g)
    {
    case 0x03:
      // left
      cNo++;
      if (cNo >= dNo)
      {
        cNo = 0;
      }
      loadDial();
      break;
    case 0x04:
      // right
      cNo--;
      if (cNo < 0)
      {
        cNo = dNo - 1;
      }
      loadDial();
      break;
    }
  }
#endif
}


void loadDial()
{

  tft.clear();
  tft.setCursor(10, 120);

  if (dNo <= 0)
  {
    tft.print("No dials found");
    Serial.println("No dials found");
    return;
  }
  else
  {
    Serial.print("Loading dial:");
    Serial.println(dials[cNo]);
  }


  uint8_t az[1];
  readDialBytes(az, 0, 1);
  uint8_t j = az[0];

  static uint8_t item[20];
  static uint8_t table[512];

  uint8_t lid = 0;
  int a = 0;
  int lan = 0;
  int tp = 0;
  int wt = 0;

  for (int i = 0; i < j; i++)
  {
    if (i >= 60)
    {
      break;
    }

    readDialBytes(item, (i * 20) + 4, 20);

    uint8_t id = item[0];

    uint16_t xOff = item[5] * 256 + item[4];
    uint16_t yOff = item[7] * 256 + item[6];

    uint16_t xSz = item[9] * 256 + item[8];
    uint16_t ySz = item[11] * 256 + item[10];

    uint32_t clt = item[15] * 256 * 256 * 258 + item[14] * 256 * 256 + item[13] * 256 + item[12];
    uint32_t dat = item[19] * 256 * 256 * 256 + item[18] * 256 * 256 + item[17] * 256 + item[16];

    bool isG = (item[1] & 0x80) == 0x80;
    uint8_t cmp = isG ? (item[1] & 0x7F) : 1;

    int aOff = item[2];

    bool isM = (item[3] & 0x80) == 0x80;
    uint8_t cG = isM ? (item[3] & 0x7F) : 1;

    if (id == 0x16 && (item[1] == 0x06 || item[1] == 0x00))
    {
      // weather (-) label
      continue;
    }
    if (isM)
    {
      lan++;
    }

    if (tp == 0x09 && id == 0x09)
    {
      a++;
    }
    else if (tp != id)
    {
      tp = id;
      a++;
    }
    else if (lan == 1)
    {
      a++;
    }

    if (!isKnown(id))
    {
      continue;
    }

    if (xSz == 0 || ySz == 0)
    {
      continue;
    }

    if (isM)
    {
      if (lan == cG)
      {
        lan = 0;
      }
      else if (id == 0x0d && (lan == 1 || lan == 32 || lan == 40))
      {
        yOff -= (ySz - aOff);
        xOff -= aOff;
      }
      else
      {
        continue;
      }
    }
    if (id == 0x17)
    {
      wt++;
      if (wt != 1)
      {
        continue;
      }
    }

    Serial.printf("i:%d, id:%d, xOff:%d, yOff:%d, xSz:%d, ySz:%d, clt:%d, dat:%d, cmp:%d\n", i, id, xOff, yOff, xSz, ySz, clt, dat, cmp);

    readDialBytes(table, clt, 512);

    ySz = uint16_t(ySz / cmp);

    File file = SPIFFS.open(dials[cNo].c_str(), "r");
    if (!file)
    {
      Serial.println("Failed to open file for reading");
      break;
    }
    int offset = (xSz * ySz) * random(cmp);

    if (!file.seek(dat + offset))
    {
      Serial.println("Failed to seek file");
      file.close();
      break;
    }

    int x = 0;
    if (id == 0x19)
    {
      for (int z = 0; z < (xSz * ySz); z++)
      {
        uint16_t pixel = item[13] * 256 + item[12];
        if (pixel != 0)
        {
          // 0 black -> transparent
          tft.drawPixel((z % xSz) + xOff, (z / xSz) + yOff, pixel);
        }
      }
    }
    else
    {
      while (file.available())
      {
        uint16_t index = file.read();

        uint16_t pixel = table[(index * 2) + 1] * 256 + table[index * 2];
        if (pixel != 0)
        {
          // 0 black -> transparent
          tft.drawPixel((x % xSz) + xOff, (x / xSz) + yOff, pixel);
        }
        x++;
        if (x >= (xSz * ySz))
        {
          break;
        }
      }
    }

    file.close();
  }
}
