
#ifndef PI_WATCH_
#define PI_WATCH_

#include <Arduino.h>

#define db Serial

#define HARDWARE_REVA 0
#define HARDWARE_REVB 1

#define ON  1
#define OFF 0

#define UP   1
#define DOWN 0

volatile int hVer = 1;
#include "PiScreen.h"
#include "PiTouch.h"

namespace PIN { // Pin definitions

    int const
    
    // Analog In
    POWER_BUTTON    = A11,

    // Analog Out [PWM]
    LCD_BACKLIGHT   = 4,
    
    // Digital Out
    SD_CHIP_SELECT    = 30,
    SYSTEM_POWER      = 31,
    VIBRATOR          =  3,
    AMPLIFIER_ENABLE  = 24;
    
}

class PiWatch : public PiScreen, public PiTouch {

private:

static IntervalTimer vibrateTimer;
static bool vibrateState;
int brightness = 100;

static void vibrateOffISR() {

    pinMode(PIN::VIBRATOR, OUTPUT);
    digitalWrite(PIN::VIBRATOR, LOW);
    
    vibrateState = false;

    vibrateTimer.end();

}

static void setupPins() {

    using namespace PIN;

    // System Power [drop this to shut off]
    pinMode(SYSTEM_POWER, OUTPUT);
    digitalWrite(SYSTEM_POWER, HIGH);

    // Speaker amplifier
    pinMode(AMPLIFIER_ENABLE, OUTPUT);
    digitalWrite(AMPLIFIER_ENABLE, LOW);

    // Backlight
    pinMode(LCD_BACKLIGHT, OUTPUT);
    digitalWrite(LCD_BACKLIGHT, LOW);

    // Power Button
    pinMode(POWER_BUTTON,INPUT);
    
    // Vibrator
    pinMode(VIBRATOR, OUTPUT);
    digitalWrite(VIBRATOR, LOW);
    
}

int rawBrightness() {

    int actualBrightness = 255.0 * (float)brightness/100.0;
    if(actualBrightness < 10) actualBrightness = 10;
    // analogWrite(PIN::LCD_BACKLIGHT, actualBrightness);

    return actualBrightness;
    
}

public:

PiWatch() {}

static void init() {

    hVer = EEPROM.read(0);
    #ifdef db
        db.printf("hardware %d %s\r\n",hVer,hVer == 0 ? "REVA" : "REVB");
    #endif
    if(hVer != HARDWARE_REVA && hVer != HARDWARE_REVB) hVer = HARDWARE_REVB;
    
    setupPins();
    
    InitLCD();
    clrScr();
    
}

static void init(void (*tmpTouchCallback)(int,int,int,int)) {

    init();

    touchInit(tmpTouchCallback);
    
}

void setOrientation(int tmpOrient) {

    setTouchOrientation(tmpOrient);
    setLcdOrientation(tmpOrient);

}

void powerDown() {

    // delay(2000);
    
    digitalWrite(PIN::SYSTEM_POWER, LOW);
    
    while(true) {
    
        if(Serial) Serial.println("powered down");
    
        delay(1000);
    
    }

}

void vibrate(int millisTime) {

    pinMode(PIN::VIBRATOR, OUTPUT);
    digitalWrite(PIN::VIBRATOR, HIGH);

    vibrateState = true;

    vibrateTimer.begin(vibrateOffISR, millisTime * 1000);
    
}

bool getVibrateState() {

    return vibrateState;

}

// TODO: Make this non blocking
void rampBrightness(bool dir) {

    using namespace PIN;
    
    int const speed = 2;
    
    if(dir) {
    
        // Take about 1/4 of a second to fade in
        for(int i=0;i<=rawBrightness();i+=speed) {
        
            analogWrite(LCD_BACKLIGHT, i);
            delay(1);
            
        }
        
        analogWrite(LCD_BACKLIGHT, rawBrightness());
        
    } else {

        // Take about 1/4 of a second to fade out
        for(int i=rawBrightness();i>=0;i-=speed) {
        
            analogWrite(LCD_BACKLIGHT, i);
            delay(1);
            
        }
        
        analogWrite(LCD_BACKLIGHT, 0);
    
    }
    
}

void setBrightness(int value) {

    Serial.printf("setBrightness %d %d\r\n",value,brightness);

    if(value > 100) value = 100;
    if(value < 0) value = 0;

    brightness = value;

    analogWrite(PIN::LCD_BACKLIGHT, rawBrightness());
    
}

int getBrightness() {

    return brightness;

}

int speaker(bool value) {

    pinMode(PIN::AMPLIFIER_ENABLE, OUTPUT);
    digitalWrite(PIN::AMPLIFIER_ENABLE, value);

}

};

IntervalTimer PiWatch::vibrateTimer;

bool PiWatch::vibrateState = false;

#undef db

#endif