/***********************************************************************
 * グラフィック処理 (システム依存)
 *
 *	詳細は、 graph.h 参照
 ************************************************************************/
#pragma GCC optimize ("O3")
#include<M5Stack.h>
#define LGFX_USE_V0
#include <LovyanGFX.hpp>
extern "C"{
#include <stdio.h>
#include <stdlib.h>
#include "quasi88.h"
#include "device.h"
#include "graph.h"
}

static	T_GRAPH_SPEC	graph_spec;		/* 基本情報		*/

static	int		graph_exist;		/* 真で、画面生成済み	*/
static	T_GRAPH_INFO	graph_info;		/* その時の、画面情報	*/

static LGFX lcd;

uint8_t needDrawUpdateFlag;
uint8_t nowDrawingFlag;

/************************************************************************
 *	グラフィック処理の初期化
 *	グラフィック処理の動作
 *	グラフィック処理の終了
 ************************************************************************/

const T_GRAPH_SPEC	*graph_init(void)
{
    if (verbose_proc) {
	printf("Initializing Graphic System ... ");
    }
    lcd.init();

    lcd.setSwapBytes(true); // バイト順変換を有効にする。
    lcd.fillScreen(TFT_BLACK);

    //graph_spec.window_max_width      = 640;
    //graph_spec.window_max_height     = 400;
    graph_spec.window_max_width      = 320;
    graph_spec.window_max_height     = 240;

    graph_spec.fullscreen_max_width  = 320;
    graph_spec.fullscreen_max_height = 240;
    //graph_spec.fullscreen_max_width  = 640;
    //graph_spec.fullscreen_max_height = 400;

    graph_spec.forbid_status         = FALSE;
    graph_spec.forbid_half           = FALSE;

    if (verbose_proc)
	printf("OK\n");

   needDrawUpdateFlag = FALSE;
   nowDrawingFlag = FALSE;
    xTaskCreatePinnedToCore(graph_updae_thread, "graph_updae_thread", 8192, NULL, 1, NULL, 0);

    return &graph_spec;
}

/************************************************************************/

static unsigned char *buffer = NULL;

const T_GRAPH_INFO	*graph_setup(int width, int height,
				     int fullscreen, double aspect)
{
    if (graph_exist == FALSE) {

	/* まだウインドウが無いならば、ウインドウを生成する */

	if (buffer == NULL) {
        Serial.print("MakeBuffer:");
        Serial.print(width);
        Serial.print(":");
        Serial.println(height);
	    buffer = (unsigned char*)ps_malloc(width * height * sizeof(unsigned short));
	    if (buffer == FALSE) {
		return NULL;
	    }
	}
    } else {

	/* すでにウインドウが有るならば、リサイズする */
    }

    /* graph_info に諸言をセットする */

    graph_info.fullscreen	= FALSE;
    graph_info.width		= width;
    graph_info.height		= height;
    //graph_info.byte_per_pixel	= 2;
    graph_info.byte_per_line	= width * 2;
    graph_info.byte_per_pixel	= 2;
    //graph_info.byte_per_line	= width;

    graph_info.buffer		= buffer;
    //graph_info.nr_color		= 255;
    graph_info.nr_color		= 255;
    graph_info.write_only	= TRUE;
    graph_info.broken_mouse	= FALSE;
    graph_info.draw_start	= NULL;
    graph_info.draw_finish	= NULL;
    graph_info.dont_frameskip	= FALSE;

    graph_exist = TRUE;

    return &graph_info;
}

/************************************************************************/

void	graph_exit(void)
{
    if (buffer) {
	free(buffer);
    }
}

/************************************************************************
 *	色の確保
 *	色の解放
 ************************************************************************/

void	graph_add_color(const PC88_PALETTE_T color[],
			int nr_color, unsigned long pixel[])
{
    int i;
    for (i=0; i<nr_color; i++) {
    //https://forum.arduino.cc/index.php?topic=487698.0
	//pixel[i] = ((((unsigned long) color[i].red   >> 3) << 10) |
	//   	    (((unsigned long) color[i].green >> 3) <<  5) |
	//  	    (((unsigned long) color[i].blue  >> 3)));

//RGB565 
//	pixel[i] = ((((unsigned long) color[i].red  & 0xF8) << 8) | //11111000
//  	   	    (((unsigned long) color[i].green & 0xFC) <<  3) | //11111100
//	  	    (((unsigned long) color[i].blue & 0xF8) >> 3));     //11111000

//RGB332
  pixel[i] = ((((unsigned long) color[i].red  & 0xE0) ) | //11100000
            (((unsigned long) color[i].green & 0xE0) >>  3) | //11100000
          (((unsigned long) color[i].blue & 0xC0) >> 6));     //11000000
 
/*    Serial.print("pixelRed=");
    Serial.println((unsigned long) color[i].red,HEX);    
    Serial.print("pixelGreen=");
    Serial.println((unsigned long) color[i].green,HEX);
    Serial.print("pixelBlue=");
    Serial.println((unsigned long) color[i].blue,HEX);
    Serial.print("pixel[");
    Serial.print(i);
    Serial.print("]=");
    Serial.println(pixel[i],HEX);
    */
    }
}

/************************************************************************/

void	graph_remove_color(int nr_pixel, unsigned long pixel[])
{
}

/************************************************************************
 *	グラフィックの更新
 ************************************************************************/
//グラフィックの更新は別スレッドで行う。

void graph_updae_thread(void *pvParameters){
  while(1){
    if(needDrawUpdateFlag == TRUE){
      nowDrawingFlag = TRUE;
       lcd.pushImageRotateZoom
        ( 160  // 描画先の中心座標X
        , 120  // 描画先の中心座標Y
        , 320  // 画像の中心座標X
        , 120  // 画像の中心座標Y
        , 0              // 回転角度
        , 0.5                // X方向の描画倍率 (マイナス指定で反転可能)
        , 1                // Y方向の描画倍率 (マイナス指定で反転可能)
        , 640        // 画像データの幅
        , 220       // 画像データの高さ
        , buffer             // 画像データのポインタ
        );
        nowDrawingFlag = FALSE;
        needDrawUpdateFlag = FALSE;
    }
    delay(10);
  }
}

void waitDrawing(){
  while(nowDrawingFlag == TRUE){
    delay(10);    
  }
  return;
}

void	graph_update(int nr_rect, T_GRAPH_RECT rect[])
{
  needDrawUpdateFlag = TRUE;
  /*
     lcd.pushImageRotateZoom
      ( 160  // 描画先の中心座標X
      , 120  // 描画先の中心座標Y
      , 320  // 画像の中心座標X
      , 120  // 画像の中心座標Y
      , 0              // 回転角度
      , 0.5                // X方向の描画倍率 (マイナス指定で反転可能)
      , 1                // Y方向の描画倍率 (マイナス指定で反転可能)
      , 640        // 画像データの幅
      , 220       // 画像データの高さ
      , buffer             // 画像データのポインタ
      );
    */  
/*    
    for (int i=0; i<nr_rect; i++) {
        int x = rect[i].x;
        int y = rect[i].y;
        int w = rect[i].width;
        int h = rect[i].height;

     lcd.pushImageRotateZoom
      ( (x+w) >> 2  // 描画先の中心座標X
      , (y+h) >> 1  // 描画先の中心座標Y
      , (x+w) >> 1  // 画像の中心座標X
      , (y+h) >> 1  // 画像の中心座標Y
      , 0              // 回転角度
      , 0.5                // X方向の描画倍率 (マイナス指定で反転可能)
      , 1                // Y方向の描画倍率 (マイナス指定で反転可能)
      , w        // 画像データの幅
      , h       // 画像データの高さ
      , buffer             // 画像データのポインタ
      );

      //  
      //  fb.drawRect(x, y, w, h, TFT_BLACK);
      //  for(int yPos = y;yPos < y + h;yPos++){
      //      for(int xPos = x;xPos < x + w;xPos++){
      //          if(xPos >= 320){break;}  
      //          fb.drawPixel(xPos, yPos, *(buffer + yPos * 640 + xPos*2) | *(buffer + yPos * 640 + xPos*2 + 1)<<8);    
      //      }
      //      if(yPos >= 240){break;}  
      //  }
    }
    lcd.display();
    //fb.pushSprite(0, 0);
    */
}


/************************************************************************
 *	タイトルの設定
 *	属性の設定
 ************************************************************************/

void	graph_set_window_title(const char *title)
{
}

/************************************************************************/

void	graph_set_attribute(int mouse_show, int grab, int keyrepeat_on)
{
}
