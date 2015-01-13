

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

// The interrupt timer for brightness fading
static IntervalTimer brightnessTimer;

// The brightness that has been requested -1 - 255 [-1 indicates no fading is going on currently]
static volatile int brightnessTarget;

// Keeps track of brightness while fading is going on 0 - 255
static volatile int currentBrightness;

// Keeps track of the user set brigtness 0 - 100
static volatile int brightness;

static IntervalTimer vibrateTimer;
static bool vibrateState;

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

static int rawBrightness() {

    return rawBrightness(brightness);
    
}

static int rawBrightness(int tmpBrightness) {

    int actualBrightness = 255.0 * (float)tmpBrightness/100.0;
    if(actualBrightness < 10) actualBrightness = 10;
    // analogWrite(PIN::LCD_BACKLIGHT, actualBrightness);

    return actualBrightness;
    
}

static void brightnessUpdate() {

    if(currentBrightness < brightnessTarget) {
    
        currentBrightness++;
    
    } else if(currentBrightness > brightnessTarget) {
    
        currentBrightness--;
    }

    analogWrite(PIN::LCD_BACKLIGHT, currentBrightness);
    
    if(currentBrightness == brightnessTarget) {
    
        brightnessTimer.end();
        
        brightnessTarget = -1;
        
        setBrightness(currentBrightness / 2.55);
    
    }

}

public:

void powerDown() {

    // Turn power off
    digitalWrite(PIN::SYSTEM_POWER, LOW);
    
    // Give the system time to go down
    delay(1000);
    
    // Turn power on because if we are here we are clearly still ticking and something has gone wrong
    digitalWrite(PIN::SYSTEM_POWER, HIGH);
    
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

void rampBrightnessWait(int value,int speed = 100) {
    
    rampBrightness(value,speed);

    elapsedMillis time;
    
    while(1) {
    
        if(time > 5) {
        
            time = 0;
            
            int tmp;
            
            cli(); 
            tmp = brightnessTarget; 
            sei();
            
            if(tmp == -1) return;
            
        }
    
    }
    
}

void rampBrightness(int value,int speed = 100) {

    if(value == brightness) return;

    brightnessTimer.end();
    
    brightnessTarget = rawBrightness(value);
    currentBrightness = rawBrightness();

    // The time between updates needed to create
    int tmpTime = speed * 1000 / abs(brightnessTarget-currentBrightness);

    // this must be bigger than zero or this will crash
    if(tmpTime < 1) tmpTime = 1;
    
    brightnessTimer.begin(brightnessUpdate, tmpTime);
    
}

static void setBrightness(int value) {

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

IntervalTimer PiHardware::brightnessTimer;
volatile int PiHardware::brightnessTarget = -1;
volatile int PiHardware::brightness = 0;
volatile int PiHardware::currentBrightness = 0;

IntervalTimer PiHardware::vibrateTimer;
bool PiHardware::vibrateState = false;

#undef db

#endif


