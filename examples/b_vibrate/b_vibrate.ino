/*
 *
 * An example of using the vibrator built into the Pi Watch
 *
 */

#include <SD.h>
#include <SPI.h>

#include <EEPROM.h>
#include <PiWatch.h>

PiWatch watch;

void setup() {

  // This does all of the setup for the watch hardware
  watch.init();

  watch.print("Vibrate");
  
  // The brightness starts out at zero so this turns the backlight on
  watch.setBrightness(100);
    
}

void loop() {
  
  // This makes the watch vibrate for the desired time in milliseconds
  // 500 is half a second
  
  // NOTE: This returns immediately and continues your code while the vibrator
  // continues for the desired time
  
  watch.vibrate(500);
  
  delay(1000);
  
}