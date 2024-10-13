#ifdef _CORES3
#include <SD.h>
//#include <SPIFFS.h>
#include <M5Unified.h>
#elif defined _ATOMS3R
#include <SPIFFS.h>
#include "lgfx.h"
#include "Button.h"
Button extBtn = Button(41, true, 1);
#ifdef USE_EXT_LCD
LGFX lcd;
#else
M5GFX lcd;
#endif
#else
#include <M5Stack.h>
#include <M5StackUpdater.h>  // https://github.com/tobozo/M5Stack-SD-Updater/
#endif
#include<Wire.h>

extern"C"{
#include "quasi88.h"
}

void setup(){
#ifdef _CORES3  
  auto cfg = M5.config();
  cfg.output_power = true;
  M5.begin(cfg);
#elif defined _ATOMS3R
  USBSerial.begin(115200);
  lcd.init();
#else
  M5.begin();
#endif

#ifndef _ATOMS3R
Wire.begin();
#else
//Wire.begin(1,2);
#endif

#if not defined(_CORES3) && not defined(_ATOMS3R)  
  if(digitalRead(BUTTON_A_PIN) == 0) {
     Serial.println("Will Load menu binary");
     updateFromFS(SD);
     ESP.restart();
  }
#elif defined _ATOMS3R
  //ATOM S3Rは UpDater対応なし
  if (!SPIFFS.begin()) { 
     Serial.println("SPIFFS Mount Failed");
     lcd.println("SPIFFS Mount Failed");
    return;
  }
#else
  while (false == SD.begin(GPIO_NUM_4, SPI, 25000000)) {
    Serial.println("SD Wait...");
    delay(500);
  }
  //Serial1.begin(115200, SERIAL_8N1,17,18);
  //while(!Serial1);
  //Serial1.println("Serial1 Start");

  //if (!SPIFFS.begin()) { 
  //  Serial.println("SPIFFS Mount Failed");
  //  return;
  //}

#endif

#ifdef _CORES3  
  M5.Power.setUsbOutput( true );
#elif defined _ATOMS3R

#else
  M5.Speaker.begin();
  M5.Speaker.mute();
#endif
  quasi88main();
}

void loop(){
  
}

void M5SerialPrintln(const char* logString){
  Serial.println(logString);
}
void M5SerialPrint(const char* logString){
  Serial.print(logString);
}
void M5SerialPrintHex(int value){
  Serial.print(value,HEX);
}
