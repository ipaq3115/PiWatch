
// #if 0
#ifndef PITOUCH_
#define PITOUCH_

#include <Arduino.h>
#include <LowPower_Teensy3.h>

const uint8_t pin2tsi[] = {
//0    1    2    3    4    5    6    7    8    9
  9,  10, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255,  13,   0,   6,   8,   7,
255, 255,  14,  15, 255,  12, 255, 255, 255, 255,
255, 255,  11,   5
};

struct touch_event {

    int touchType;
    int finePos;
    int activeTouches;
    int touchIndex;

};

int const TOUCH_BUFFER_MAX = 20;
int const SENSOR_BUFFER_MAX = 2;
// int const SENSOR_BUFFER_MAX = 20;
int const MAX_TOUCHES = 5;
int const PI_TOUCH_PIN_TOTAL = 10;

// #define DBG(x)
// #define DBGf(x,y) 
// #define ERROR(x)
#define ERROR(x) Serial.printf("PiTouch: %s\r\n",x);
#define DBG(x) Serial.println(x);
#define DBGf(x,y) Serial.printf(x,y);
// #define DEBUG

// The number of counts higher that the cal value that are 
// needed for a touch to register
#define SENSITIVITY 40

// #define SENSOR_CLIP 1500
#define SENSOR_CLIP 6000

#define PRESSED  0
#define MOVING   1
#define RELEASED 2

#define CURRENT   2 // 0 to 15 - current to use, value is 2*(current+1)
#define NSCAN     9 // number of times to scan, 0 to 31, value is nscan+1
#define PRESCALE  2 // prescaler, 0 to 7 - value is 2^(prescaler+1)

// #define CURRENT   2 // 0 to 15 - current to use, value is 2*(current+1)
// #define NSCAN     9 // number of times to scan, 0 to 31, value is nscan+1
// #define PRESCALE  2 // prescaler, 0 to 7 - value is 2^(prescaler+1)

// teensy pin numbers, the first is at the top of the ring
// then they proceed clockwise around the ring

// #ifdef HARDWARE_REVB
    // const int touchPin[PI_TOUCH_PIN_TOTAL] = {17,16,33,23,22,15,25,32, 1, 0}; // New
// #else
    // const int touchPin[PI_TOUCH_PIN_TOTAL] = {25,16,17, 0, 1,32,22,23,33,15}; // Old
// #endif
    
volatile int touchPin[PI_TOUCH_PIN_TOTAL] = {17,16,33,23,22,15,25,32, 1, 0};
    
// const int touchPinA[PI_TOUCH_PIN_TOTAL] = {25,16,17, 0, 1,32,22,23,33,15}; // Old
// const int touchPinB[PI_TOUCH_PIN_TOTAL] = {17,16,33,23,22,15,25,32, 1, 0}; // New

// Uncomment this line to go int loop mode, this is mainly just for debugging
// without all of the funness that is interrupts
// #define LOOP_MODE

class PiTouch {

private:

static IntervalTimer_LP primaryTimer;

volatile static int rotation;

enum {

    IDLE,
    START_WAIT,
    RESULT_WAIT,
    
};

// Tracks the state of the the current touch read index
volatile static uint8_t    state;

// The current pad that is being sampled
volatile static uint8_t    padIndex;

// The time of the last touch event
volatile static uint32_t    lastTouchTime;

// Touch interpolating
volatile static int     touchPos[MAX_TOUCHES];
volatile static int     touchPad[MAX_TOUCHES];
volatile static int     touchID[MAX_TOUCHES];
volatile static int     touchIDcounter;
volatile static int     touchTotal;
volatile static bool    touchUpdated[MAX_TOUCHES];

volatile static int     tempArray[3];
volatile static int     diff;

volatile static int     clockwiseIndex;
volatile static int     cclockwiseIndex;
volatile static int     pos;
volatile static bool    foundPreviousTouch;
volatile static int     writePtr,readPtr;

// Sensor Buffering
volatile static int     sensorBuffer[PI_TOUCH_PIN_TOTAL][SENSOR_BUFFER_MAX];
volatile static int     sensorBufferCounter;
volatile static int     responseRate;

// Touch buffering
volatile static touch_event touchBuffer[TOUCH_BUFFER_MAX];
volatile static int         touchBufferPointer;
volatile static int         touchBufferTotal;
volatile static bool        touchBufferOverrun;
volatile static int         touchBufferMoving[PI_TOUCH_PIN_TOTAL];

volatile static long        timeTmp;
volatile static long        lastRecTime;

static void (*callback)(int,int,int,int);

public:

volatile static int touchVals[PI_TOUCH_PIN_TOTAL];
volatile static int calValue[PI_TOUCH_PIN_TOTAL];

PiTouch() { }

static void touchInit(void (*callbackTmp)(int,int,int,int), bool _old_watch = false) {

    DBG("INIT");

    if(_old_watch) {
    
        int const touchPin_old[PI_TOUCH_PIN_TOTAL] = {25,16,17, 0, 1,32,22,23,33,15}; // Old
        
        for(int i=0;i<PI_TOUCH_PIN_TOTAL;i++) touchPin[i] = touchPin_old[i];
    
    }
    
    touchBufferOverrun = false;
    for(int i=0;i<PI_TOUCH_PIN_TOTAL;i++) touchBufferMoving[i] = false;

    callback = callbackTmp;
    
    // Setup the Touch Sense stuff on the cortex
    
    SIM_SCGC5 |= SIM_SCGC5_TSI;
    
    for(int i=0;i<PI_TOUCH_PIN_TOTAL;i++) {
    
        uint8_t pin = (touchPin[i]);
    
        *portConfigRegister(pin) = PORT_PCR_MUX(0);
        TSI0_PEN |= (1 << pin2tsi[pin]);
    
    }
    
    TSI0_SCANC = TSI_SCANC_REFCHRG(3) | TSI_SCANC_EXTCHRG(CURRENT);
    // TSI0_SCANC = (0xF << 24) | (0xF << 16);
    
    // Calibrate the pads
    
    calCapacitive();

    // Start the sampling

    state = START_WAIT;

    touchReader();
    
}

static void restartTouch() {

    primaryTimer.end();

    TSI0_GENCS = 0;

    SIM_SCGC5 |= SIM_SCGC5_TSI;
    
    for(int i=0;i<PI_TOUCH_PIN_TOTAL;i++) {
    
        uint8_t pin = (touchPin[i]);
    
        *portConfigRegister(pin) = PORT_PCR_MUX(0);
        TSI0_PEN |= (1 << pin2tsi[pin]);
    
    }
    
    TSI0_SCANC = TSI_SCANC_REFCHRG(3) | TSI_SCANC_EXTCHRG(CURRENT);
    // TSI0_SCANC = (0xF << 24) | (0xF << 16);
    


    touchBufferOverrun = false;
    for(int i=0;i<PI_TOUCH_PIN_TOTAL;i++) touchBufferMoving[i] = false;

    touchBufferPointer = 0;
    touchBufferTotal = 0;
    
    sensorBufferCounter = 0;
    
    // Start the sampling

    state = START_WAIT;

    touchReader();

}

static void touchEnd() {

    primaryTimer.end();

    TSI0_GENCS = 0;

}

// 0 is normal side up 1 is rotated 90 degrees and so on...
static void setTouchOrientation(int orient) {

    if(orient >= 0 && orient <= 3) rotation = orient * 90; 
    else rotation = 0;

    DBGf("setOrientation %d\r\n",rotation);
    
}

static int getTouchOrientation() {

    return rotation;

}

static void setResponseRate(int value) {

    if(value > 100) value = 100;
    if(value < 0) value = 0;
    
    int tmpResponseRate = SENSOR_BUFFER_MAX * (float)value/100.0;
    
    if(tmpResponseRate >= SENSOR_BUFFER_MAX) tmpResponseRate = SENSOR_BUFFER_MAX - 1;
    if(tmpResponseRate < 1) tmpResponseRate = 1;
    
    cli();
    
    responseRate = tmpResponseRate;
    
    sei();

}

static void setResponseRateRaw(int value) {

    if(value >= SENSOR_BUFFER_MAX) value = SENSOR_BUFFER_MAX - 1;
    if(value < 1) value = 1;
    
    cli();
    
    responseRate = value;
    
    sei();

}

static int getResponseRateRaw() {

    return responseRate;

}

static void calCapacitive() {

    DBG("calCapacitive");
    
    uint32_t ch;
    uint8_t pin;
    
    int const CAL_POINTS = 10;
    
    for(int n=0;n<PI_TOUCH_PIN_TOTAL;n++) calValue[n] = 0;

    for(int i=0;i<CAL_POINTS;i++) {

        // Start touch read 
        TSI0_GENCS = 0;
        // TSI0_GENCS = TSI_GENCS_NSCN(0) | TSI_GENCS_PS(0) | TSI_GENCS_TSIEN | TSI_GENCS_SWTS;
        TSI0_GENCS = TSI_GENCS_NSCN(NSCAN) | TSI_GENCS_PS(PRESCALE) | TSI_GENCS_TSIEN | TSI_GENCS_SWTS;
        
        delayMicroseconds(10);
        while(TSI0_GENCS & TSI_GENCS_SCNIP); // wait
        delayMicroseconds(1);

        for(int n=0;n<PI_TOUCH_PIN_TOTAL;n++) {
        
            ch = pin2tsi[touchPin[n]];
            
            calValue[n] += *((volatile uint16_t *)(&TSI0_CNTR1) + ch);
            
        }
        
    }

    for(int n=0;n<PI_TOUCH_PIN_TOTAL;n++) {
        calValue[n] /= CAL_POINTS;
        // Serial.printf("Cal: %d\r\n",calValue[n]);
    }
    
}

static void touchReader() {
    
    static uint32_t   ch;
    
    if(state == START_WAIT) {
        
        primaryTimer.end();

        // Start touch read 
        TSI0_GENCS = 0;
        // TSI0_GENCS = TSI_GENCS_NSCN(0) | TSI_GENCS_PS(0) | TSI_GENCS_TSIEN | TSI_GENCS_SWTS;
        TSI0_GENCS = TSI_GENCS_NSCN(NSCAN) | TSI_GENCS_PS(PRESCALE) | TSI_GENCS_TSIEN | TSI_GENCS_SWTS;
        
        // Set a primaryTimer to go off right before the value should be returned
        
        delayMicroseconds(5);

        // send this off to keep watching for results
        state = RESULT_WAIT;
        primaryTimer.begin(touchReader, 5);
        
    } else if(state == RESULT_WAIT) {
        
        // Check to see if the process is done yet
        if(TSI0_GENCS & TSI_GENCS_SCNIP) return; 
    
        // The read is done so kill the interrupt before it hits again
        primaryTimer.end();
        
        delayMicroseconds(1);
        
        // Read all of the pad values out
        for(int i=0;i<PI_TOUCH_PIN_TOTAL;i++) {
        
            ch = pin2tsi[touchPin[i]];
            
            sensorBuffer[i][sensorBufferCounter] = *((volatile uint16_t *)(&TSI0_CNTR1) + ch) - calValue[i];
            
            if(sensorBuffer[i][sensorBufferCounter] > SENSOR_CLIP) sensorBuffer[i][sensorBufferCounter] = SENSOR_CLIP;
            
        }
        
        // Increment the counter for the sensor buffer
        if(++sensorBufferCounter >= responseRate) sensorBufferCounter = 0;
        
        // Average the values together
        for(int i=0;i<PI_TOUCH_PIN_TOTAL;i++) {
        
            touchVals[i] = 0;
            
            for(int k=0;k<responseRate;k++) touchVals[i] += sensorBuffer[i][k];
            
            touchVals[i] /= responseRate;
        
        }
        
        // Handle the touches
        sampleTouch();
    
        // prep the next update
        state = START_WAIT;
        // primaryTimer.begin(touchReader, 2000);
        // primaryTimer.begin(touchReader, 5000);
        primaryTimer.begin(touchReader, 20000);
        
    }

}

static void newTouchEvent(int touchType,int finePos,int activeTouches,int touchIndex) {

    if(touchBufferTotal < TOUCH_BUFFER_MAX && !touchBufferOverrun) {
    
        if(touchType == MOVING) {
        
            if(!touchBufferMoving[touchIndex]) touchBufferMoving[touchIndex] = true;
            else return;
        
        }

        touchBuffer[touchBufferPointer].touchType  = touchType;
        touchBuffer[touchBufferPointer].finePos    = finePos;
        touchBuffer[touchBufferPointer].activeTouches     = activeTouches;
        touchBuffer[touchBufferPointer].touchIndex = touchIndex;
        
        touchBufferPointer++;
        touchBufferPointer %= TOUCH_BUFFER_MAX;
        
        touchBufferTotal++;
        
        // if(touchType == MOVING && touchBufferTotal >= 1) return;

    } else {
    
        touchBufferOverrun = true;
        
    }
    
}

static void sampleTouch() {

    for(int i=0;i<MAX_TOUCHES;i++) touchUpdated[i] = false;

    // Find the peaks and find the new dot position and draw them
    for(int i=0;i<PI_TOUCH_PIN_TOTAL;i++) {

        cclockwiseIndex = i - 1; 
        if(cclockwiseIndex < 0) cclockwiseIndex = PI_TOUCH_PIN_TOTAL - 1;
        
        clockwiseIndex = i + 1; 
        if(clockwiseIndex >= PI_TOUCH_PIN_TOTAL) clockwiseIndex = 0;
    
        // Checking to see if this value is above threshold and is a peak
        if( touchVals[i] > SENSITIVITY && 
        touchVals[cclockwiseIndex] <= touchVals[i] && 
        touchVals[clockwiseIndex] <= touchVals[i]) {
        
            tempArray[0] = touchVals[cclockwiseIndex];
            tempArray[1] = touchVals[i];
            tempArray[2] = touchVals[clockwiseIndex];
            
            pos = calculatePointMagnitudePosition(tempArray,3,36) + cclockwiseIndex * 36; 
            pos -= rotation;
            while(pos < 0) pos += 360;
            while(pos >= 360) pos -= 360;
            
            // See if there are any previous touches that are close to this one
            foundPreviousTouch = false;
            for(int n=0;n<touchTotal;n++) {
            
                diff = abs(touchPos[n] - pos);
                if(diff > 180) diff = abs((max(pos,touchPos[n]) - 360) - min(pos,touchPos[n]));
            
                if(diff < 36) {
                    
                    touchPos[n] = pos;
                    
                    touchPad[n] = i;
                    
                    newTouchEvent(MOVING,pos,touchTotal,touchID[n]);
                    
                    touchUpdated[n] = true;
                    
                    foundPreviousTouch = true;
                    
                    break;
                    
                }
            
            }
            
            // Section for handling a new touch
            if(!foundPreviousTouch && touchTotal < MAX_TOUCHES) {
            
                touchPos[touchTotal] = pos;
                
                touchPad[touchTotal] = touchTotal;
                
                // Make sure touchID we are going to use isn't in use already
                // Might be a better way to do this
                searchLoop:
                for(int n=0;n<touchTotal;n++) {
                
                    if(touchID[n] == touchIDcounter) {
                    
                        touchIDcounter++; if(touchIDcounter > 9) touchIDcounter = 0;
                        
                        goto searchLoop;
                    
                    }
                
                }
                
                touchID[touchTotal] = touchIDcounter;
                
                touchUpdated[touchTotal] = true;
                
                // if(D) USB.printf("Pressed touchID[touchTotal] %d touchTotal %d\r\n",touchID[touchTotal],touchTotal);
                
                newTouchEvent(PRESSED,pos,touchTotal + 1,touchID[touchTotal]);
                
                touchTotal++;
                
                if(touchTotal >= MAX_TOUCHES) {
                
                    // Serial.println("touchTotal is full");
                
                }
            
            }
        
        }
    
    }

    // Remove the touches that weren't updated on this sample
    int tmpTouchTotal = touchTotal;
    writePtr = 0;
    for(readPtr=0;readPtr<touchTotal;readPtr++) {
    
        if(!touchUpdated[readPtr]) {
    
            newTouchEvent(RELEASED,touchPos[readPtr],--tmpTouchTotal,touchID[readPtr]);
            
        } else {
        
            if(writePtr != readPtr) {
            
                touchUpdated[writePtr] = touchUpdated[readPtr];
    
                touchPos[writePtr] = touchPos[readPtr];
    
                touchID[writePtr] = touchID[readPtr];
            
            }
        
            writePtr++;
        
        }
    
    }
    
    if(writePtr == 0 && touchTotal != 0) lastTouchTime = millis();
    
    touchTotal = writePtr;
    
}

static int calculatePointMagnitudePosition(volatile int * values,int pointTotal,int unitLength) { //,int &sizeresult) {

    // This function takes a set of values [evenly spaced along a line] and their magnitudes and
    // returns a point on that line

    // if(D) dbB << pstr("calculatePointMagnitudePosition") << endl;
  
    int indexRank[pointTotal]; // Ranks the indexes from highest to lowest
    int tempValues[pointTotal];

    for(int i=0;i<pointTotal;i++) tempValues[i] = values[i]; 
    
    for(int i=0;i<pointTotal;i++) {
    
        // Set this index to zero
        indexRank[i] = 0;
        
        // Find the highest value in tempValues and set it to indexRank[i]
        for(int d=0;d<pointTotal;d++) { if(tempValues[d]>tempValues[indexRank[i]]) indexRank[i] = d; }
        
        // if(D) dbB << dec << setw(6) << int(indexRank[i]) << ' ';
        
        // Zero out the index that I just used so that it isn't counted again
        tempValues[indexRank[i]] = 0;
        
    }
    // if(D) dbB << endl;

    int finalPos;
    int finalsize;

    for(int count=0;count<pointTotal;count++) {

        int pos = indexRank[count] * unitLength;

        if(count == 0) {

            finalPos = pos;
            finalsize = values[indexRank[count]];

        } else {

            int xdiff = finalPos - pos;

            // if(D) dbB << pstr("xdiff ") << dec << xdiff << endl;

            if(xdiff) {

                float onepercent = (finalsize + values[indexRank[count]]);   

                // if(D) dbB << pstr("onepercent ") << dec << onepercent << endl;

                onepercent /= (float)100;   

                // if(D) dbB << pstr("onepercent ") << dec << onepercent << endl;

                float percentChange = values[indexRank[count]] / onepercent;

                // if(D) dbB << pstr("percentChange ") << dec << percentChange << endl;

                float pixelChange = xdiff * (percentChange / (float)100);

                // if(D) dbB << pstr("pixelChange ") << dec << pixelChange << endl;

                finalPos -= pixelChange;

            }

            finalsize += values[indexRank[count]];

            // if(D) dbB << pstr("finalsize ") << dec << finalsize << endl;

        }
        
        // if(D) dbB 
        // << dec << count 
        // << pstr(" finalPos ") << dec << finalPos 
        // << pstr(" finalsize ") << dec << finalsize 
        // << pstr(" pos ") << dec << pos << endl;

    }

    //    if(D) dbB << pstr("row ") << dec << row << pstr(" column ") << dec << column << endl;
    //    if(D) dbB << pstr("finalPos ") << dec << finalPos << pstr(" finaly ") << dec << finaly << endl;

    //  if(D) { dbB << pstr("indexRank ");for(int i=0;i<16;i++) dbB << dec << int(indexRank[i]) << ' '; dbB << endl; }

    // sizeresult = finalsize;
    
    return finalPos;
 
 
}

static void loop() {

    static elapsedMillis time;
    
    if(time > 10) {
    
        time = 0;
    
        cli();
        
        if(touchBufferOverrun) {
        
            #ifdef DEBUG
            
                Serial.println("touchBufferOverrun:");
            
                for(int i=0;i<touchBufferTotal;i++) Serial.printf("%05d %05d %05d %05d\r\n",
                touchBuffer[i].touchType,
                touchBuffer[i].finePos,
                touchBuffer[i].activeTouches,
                touchBuffer[i].touchIndex);
                
                while(true);
        
            #endif
        
        }

        while(touchBufferTotal > 0) {
        
            int tmp = touchBufferPointer - touchBufferTotal;
            if(tmp < 0) tmp += TOUCH_BUFFER_MAX;
        
            callback(
            touchBuffer[tmp].touchType,
            touchBuffer[tmp].finePos,
            touchBuffer[tmp].activeTouches,
            touchBuffer[tmp].touchIndex);
            
            touchBufferMoving[touchBuffer[tmp].touchIndex] = false;
           
            touchBufferTotal--;
           
        }
        
        sei();
        
    }
    
}

};

volatile int PiTouch::rotation = 0;

IntervalTimer_LP PiTouch::primaryTimer;

// Tracks the state of the the current touch read index
volatile uint8_t    PiTouch::state = 0;

// The current pad that is being sampled
volatile uint8_t    PiTouch::padIndex = 0;

// The time of the last touch event
volatile uint32_t   PiTouch::lastTouchTime = 0;

volatile int PiTouch::touchVals[PI_TOUCH_PIN_TOTAL];
volatile int PiTouch::calValue[PI_TOUCH_PIN_TOTAL];

// Touch interpolating
volatile int     PiTouch::touchPos[MAX_TOUCHES];
volatile int     PiTouch::touchPad[MAX_TOUCHES];
volatile int     PiTouch::touchID[MAX_TOUCHES];
volatile int     PiTouch::touchIDcounter;
volatile int     PiTouch::touchTotal;
volatile bool    PiTouch::touchUpdated[MAX_TOUCHES];

volatile int     PiTouch::tempArray[3];
volatile int     PiTouch::diff;

volatile int     PiTouch::clockwiseIndex;
volatile int     PiTouch::cclockwiseIndex;
volatile int     PiTouch::pos;
volatile bool    PiTouch::foundPreviousTouch;
volatile int     PiTouch::writePtr,PiTouch::readPtr;


// Sensor Buffering
volatile int     PiTouch::sensorBuffer[PI_TOUCH_PIN_TOTAL][SENSOR_BUFFER_MAX];
volatile int     PiTouch::sensorBufferCounter = 0;
volatile int     PiTouch::responseRate = SENSOR_BUFFER_MAX;

// Touch buffering
volatile touch_event PiTouch::touchBuffer[TOUCH_BUFFER_MAX];
volatile int         PiTouch::touchBufferPointer = 0;
volatile int         PiTouch::touchBufferTotal = 0;
volatile bool        PiTouch::touchBufferOverrun = false;
volatile int         PiTouch::touchBufferMoving[PI_TOUCH_PIN_TOTAL];

volatile long         PiTouch::timeTmp = 0;
volatile long         PiTouch::lastRecTime = 0;

void (*PiTouch::callback)(int,int,int,int);

#endif