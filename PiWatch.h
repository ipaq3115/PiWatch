
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

#include "PiHardware.h"
#include "PiScreen.h"
#include "PiTouch.h"

class PiWatch : public PiScreen, public PiTouch, public PiHardware {

public:

PiWatch() {}

static void init() {

    setupPins();
    
    InitLCD();
    clrScr();
    
}

static void init(void (*tmpTouchCallback)(int,int,int,int), bool _old_watch = false) {

    init();

    touchInit(tmpTouchCallback, _old_watch);
    
}

void setOrientation(int tmpOrient) {

    setTouchOrientation(tmpOrient);
    setLcdOrientation(tmpOrient);

}

};

#undef db

#endif