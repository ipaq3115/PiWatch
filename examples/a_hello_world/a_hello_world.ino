/*
 *
 * This example provides a very simple Hello World program 
 * that prints this text to the screen
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

    // Print the text 'Hello World' in the center of the screen
    watch.print("Hello World");
    
    // The brightness starts out at zero so this turns the backlight on
    watch.setBrightness(100);
    
}

void loop() {
  
}