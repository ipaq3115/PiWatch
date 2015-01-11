

#ifndef PI_HARDWARE_H_
#define PI_HARDWARE_H_

#define db Serial

namespace PIN { // Pin definitions

    int const
    
    // Analog In
    POWER_BUTTON    = A11,
    BATTERY         = A10,

    // Analog Out [PWM]
    LCD_BACKLIGHT     = 4,
    
    // Digital Out
    SD_CHIP_SELECT    = 30,
    SYSTEM_POWER      = 31,
    VIBRATOR          =  3,
    AMPLIFIER_ENABLE  = 24;
    
}

class PiHardware {

protected:

static IntervalTimer vibrateTimer;
static bool vibrateState;
int brightness = 0;

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

    return rawBrightness(brightness);
    
}

int rawBrightness(int tmpBrightness) {

    int actualBrightness = 255.0 * (float)tmpBrightness/100.0;
    if(actualBrightness < 10) actualBrightness = 10;
    // analogWrite(PIN::LCD_BACKLIGHT, actualBrightness);

    return actualBrightness;
    
}

public:


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
void rampBrightness(int value) {

    using namespace PIN;
    
    int const speed = 2;
    
    if(value > 100) value = 100;
    if(value < 0) value = 0;

    if(value > brightness) {
    
        // Take about 1/4 of a second to fade in
        for(int i=rawBrightness();i<=rawBrightness(value);i+=speed) {
        
            analogWrite(LCD_BACKLIGHT, i);
            delay(1);
            
        }
        
        brightness = value;
        
        analogWrite(LCD_BACKLIGHT, rawBrightness());
        
    } else if(value < brightness) {

        // Take about 1/4 of a second to fade out
        for(int i=rawBrightness();i>=rawBrightness(value);i-=speed) {
        
            analogWrite(LCD_BACKLIGHT, i);
            delay(1);
            
        }
        
        brightness = value;
        
        analogWrite(LCD_BACKLIGHT, rawBrightness());
        
    }
    

}

void setBrightness(int value) {

    Serial.printf("setBrightness %d %d\r\n",value,brightness);

    if(value > 100) value = 100;
    if(value < 0) value = 0;

    if(brightness != value) analogWrite(PIN::LCD_BACKLIGHT, rawBrightness(value));

    brightness = value;
    
}

int getBrightness() {

    return brightness;

}

int speaker(bool value) {

    pinMode(PIN::AMPLIFIER_ENABLE, OUTPUT);
    digitalWrite(PIN::AMPLIFIER_ENABLE, value);

}

int getBatteryRaw() {

    int value;

    analogReadResolution(10);
    
    value = analogRead(PIN::BATTERY);
    
    analogReadResolution(8);
    
    return value;

}


};

IntervalTimer PiHardware::vibrateTimer;

bool PiHardware::vibrateState = false;

#undef db

#endif


