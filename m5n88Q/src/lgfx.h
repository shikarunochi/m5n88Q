#ifndef LGFX_H
#define  LGFX_H

#include <M5GFX.h>
#ifdef USE_EXT_LCD
#include <lgfx/v1/panel/Panel_ST7789.hpp>
class LGFX : public lgfx::LGFX_Device
{
  lgfx::Panel_ST7789      _panel_instance;
  lgfx::Bus_SPI       _bus_instance;   // SPIバスのインスタンス
  public:
    LGFX(void)
    {
      { // バス制御の設定を行います。
        auto cfg = _bus_instance.config();    // バス設定用の構造体を取得します。
        cfg.spi_host = SPI2_HOST ;     // ESP32-S2,C3 : SPI2_HOST or SPI3_HOST / ESP32 : VSPI_HOST or HSPI_HOST
        cfg.spi_mode = 3;             // SPI通信モードを設定 (0 ~ 3)
        cfg.freq_write = 40000000;    // 送信時のSPIクロック (最大80MHz, 80MHzを整数で割った値に丸められます)
        cfg.freq_read  = 16000000;    // 受信時のSPIクロック
        cfg.spi_3wire  = false;        // 受信をMOSIピンで行う場合はtrueを設定
        cfg.use_lock   = true;        // トランザクションロックを使用する場合はtrueを設定
        cfg.dma_channel = 1;          // Set the DMA channel (1 or 2. 0=disable)   使用するDMAチャンネルを設定 (0=DMA不使用)
        cfg.pin_sclk = EXT_SPI_SCLK;            // SPIのSCLKピン番号を設定
        cfg.pin_mosi = EXT_SPI_MOSI;            // SPIのMOSIピン番号を設定
        cfg.pin_miso = -1;            // SPIのMISOピン番号を設定 (-1 = disable)
        cfg.pin_dc   = EXT_SPI_DC;            // SPIのD/Cピン番号を設定  (-1 = disable)
        _bus_instance.config(cfg);    // 設定値をバスに反映します。
        _panel_instance.setBus(&_bus_instance);      // バスをパネルにセットします。
      }

      { // 表示パネル制御の設定を行います。
        auto cfg = _panel_instance.config();    // 表示パネル設定用の構造体を取得します。
        cfg.pin_cs           =    -1;  // CSが接続されているピン番号   (-1 = disable)
        cfg.pin_rst          =    EXT_SPI_RST;  // RSTが接続されているピン番号  (-1 = disable)
        cfg.pin_busy         =    -1;  // BUSYが接続されているピン番号 (-1 = disable)

        // ※ 以下の設定値はパネル毎に一般的な初期値が設定されていますので、不明な項目はコメントアウトして試してみてください。
        cfg.memory_width     =   240;  // ドライバICがサポートしている最大の幅
        cfg.memory_height    =   320;  // ドライバICがサポートしている最大の高さ
        cfg.panel_width      =   240;  // 実際に表示可能な幅
        cfg.panel_height     =   240;  // 実際に表示可能な高さ
        
        cfg.offset_y         =     0;  // パネルのY方向オフセット量
        cfg.offset_x         =     0;  // パネルのX方向オフセット量
        cfg.offset_rotation  =     3;  // 回転方向の値のオフセット 0~7 (4~7は上下反転)
        cfg.invert           =  true;  // パネルの明暗が反転してしまう場合 trueに設定
        cfg.rgb_order        = false;  // パネルの赤と青が入れ替わってしまう場合 trueに設定
        cfg.dummy_read_pixel =     8;  // ピクセル読出し前のダミーリードのビット数
        cfg.dummy_read_bits  =     1;  // ピクセル以外のデータ読出し前のダミーリードのビット数
        cfg.readable         =  false;  // データ読出しが可能な場合 trueに設定        
        cfg.dlen_16bit       = false;  // データ長を16bit単位で送信するパネルの場合 trueに設定
        cfg.bus_shared       =  false;  // SDカードとバスを共有している場合 trueに設定(drawJpgFile等でバス制御を行います)

        _panel_instance.config(cfg);
      }
      setPanel(&_panel_instance); // 使用するパネルをセットします。
    }
};
extern LGFX lcd;

#else
extern M5GFX lcd;
#endif
#endif