#ifndef USER_USB_RAM
#error "This example needs to be compiled with a USER USB setting"
#endif

#include "src/userUsbHidMediaKeyboard/USBHIDMediaKeyboard.h"

#include <TouchKey.h>

#define LED_BUILTIN 30
#define VOL_STEP 3900

uint32_t lastPressTime = 0;
float volStartValue = 0;
float volCumValue = 0;
byte mode = 0;  // 0 - nothing; 1 - single press; 2 - volume control

void setup() {
  USBInit();
  pinMode(LED_BUILTIN, OUTPUT);

//Enable all 6 channels: TIN0(P1.0), TIN1(P1.1), TIN2(P1.4), TIN3(P1.5), TIN4(P1.6), TIN5(P1.7)
  TouchKey_begin(63); //Enable all 6 channels  (00111111)
  
  // In most cases you don't need to adjust parameters. But if you do, test with TouchKeyTuneParameter example
  /*
    //refer to AN3891 MPR121 Baseline System for more details
    TouchKey_SetMaxHalfDelta(5);      //increase if sensor value are more noisy
    TouchKey_SetNoiseHalfDelta(2);    //If baseline need to adjust at higher rate, increase this value
    TouchKey_SetNoiseCountLimit(10);  //If baseline need to adjust faster, increase this value
    TouchKey_SetFilterDelayLimit(5);  //make overall adjustment slopwer

    TouchKey_SetTouchThreshold(100);  //Bigger touch pad can use a bigger value
    TouchKey_SetReleaseThreshold(80); //Smaller than touch threshold
  */
}

void loop() {
  uint8_t j;
  uint8_t buttons_sum = 0;
  uint8_t buttons_count = 0;
  float avg_val = 0;
  float buttons_diff = 0;
  TouchKey_Process();
  uint8_t touchResult = TouchKey_Get();
  if (touchResult) {
    if (mode == 0) {
      lastPressTime = millis();
      mode = 1;
      digitalWrite(LED_BUILTIN, HIGH);
    } else if (millis() - lastPressTime > 150) {
      mode = 2;
    }
  } else {
    if (mode == 1) {
        Consumer_press(MEDIA_PLAY_PAUSE);
        delay(10);
        Consumer_release(MEDIA_PLAY_PAUSE);
    }
    mode = 0;
    volStartValue = 0;
    volCumValue = 0;
    digitalWrite(LED_BUILTIN, LOW);
  }
  for (uint8_t i = 0; i < 6; i++) {
    j = (i + 2) % 6;
    if (touchResult & (1 << j)) {
      buttons_sum += i;
      buttons_count += 1;
    } else { }    
  }
  
  if (mode > 0){
    if (buttons_count > 1){
      avg_val = buttons_sum;
      avg_val /= buttons_count;
    }
    if (volStartValue == 0){
      volStartValue = avg_val;
      }
    buttons_diff = avg_val - volStartValue;
    buttons_diff *= abs(buttons_diff);
    volCumValue += buttons_diff;
    
    if (volCumValue > VOL_STEP){
      digitalWrite(LED_BUILTIN, LOW);
      Consumer_press(MEDIA_VOL_UP);
      delay(10);
      Consumer_release(MEDIA_VOL_UP);
      delay(10);
      digitalWrite(LED_BUILTIN, HIGH);
      volCumValue -= VOL_STEP;
      mode = 2;
    }
    
    if (volCumValue < -VOL_STEP){
      digitalWrite(LED_BUILTIN, LOW);
      Consumer_press(MEDIA_VOL_DOWN);
      delay(10);
      Consumer_release(MEDIA_VOL_DOWN);
      delay(10);
      digitalWrite(LED_BUILTIN, HIGH);
      volCumValue += VOL_STEP;
      mode = 2;
    }
  }
}
