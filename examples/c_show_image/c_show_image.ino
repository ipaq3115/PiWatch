/*
 *
 * An example showing how to write a bitmap file to the sdcard
 *
 * NOTE: This image must be either a bitmap in the 565, RGB or RGBA formats or a
 * image in the pi format. (These are generated by the Gimp plugin)
 * 
 * RGBA will probably be fased out of the library in favor of a pi image with
 * transparency. If you think you would use this format with the pi or think it 
 * would be helpful to keep for any reason shoot me an email at tim@thepiwatch.com
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
  
  // Now we are just going to load the sdcard and then load a file, normal Arduino stuff.
  // There are more examples under the SD library if you need help understanding SD cards
  // in general.

  // Mount the SD card
  if(!SD.begin(PIN::SD_CHIP_SELECT)) {
    
    // Print an error if the sdcard couldn't be found
    watch.print("No SDcard");
    watch.setBrightness(100);
    
    // Stop here
    while(1);
  
  }
  
  // Create the file object
  File myImage;
  
  // Load the image file into the file object
  myImage = SD.open("PiSplash.bmp");
  
  // Check to see if the image actually loaded
  if(!myImage) {
  
    // Print an error if the image file couldn't be found
    watch.print("No Image");
    watch.setBrightness(100);
  
    // Stop here
    while(1);
  
  }
  
  // Now that we have loaded the image we can send it to the screen!
  
  // Print the image
  watch.printBitmap(myImage,0,0);
  
  // The brightness starts out at zero so this line is what turns the 
  // backlight on so we can actually see what's on the screen. Doing 
  // this after the image has been printed to the screen gives the 
  // effect of the image 'fading' in
  watch.rampBrightness(UP);
  
  // If you didn't want to fade the brightness in you could comment out
  // the line above and simply set the brightness to 100% like this:
  watch.setBrightness(100);

}

void loop() {}
