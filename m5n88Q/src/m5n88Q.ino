#include<M5Stack.h>
#include<Wire.h>
#include <M5StackUpdater.h>  // https://github.com/tobozo/M5Stack-SD-Updater/
extern"C"{
#include "quasi88.h"
}

void setup(){
  M5.begin();
  Wire.begin();
  
  if(digitalRead(BUTTON_A_PIN) == 0) {
     Serial.println("Will Load menu binary");
     updateFromFS(SD);
     ESP.restart();
  }
  M5.Speaker.begin(); 
  M5.Speaker.mute();
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
