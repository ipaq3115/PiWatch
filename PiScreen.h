/*
  
  This is a specialized library meant for running the R61505v display driver found 
  in 220 x 220 round displays and is mostly based on the UTFT library by Henning 
  Karlsen, Support for other displays has been removed and additional functions have
  been added.
  
*/

#ifndef PiScreen_
#define PiScreen_

// Uncomment this line to use sdfat instead of the normal Arduino SD library
#define USE_SDFAT

#ifdef USE_SDFAT
    #include <SdFat.h>
#else
    #include <SD.h>
    #define SdFile File
    #define seekSet seek
#endif

#define ERR(x) Serial.println(x);

#define LEFT 0
#define RIGHT 9999
#define CENTER 9998

#define PORTRAIT 0
#define LANDSCAPE_R 1
#define PORTRAIT_R 2
#define LANDSCAPE 3

// *** Hardwarespecific defines ***
#define cbi(reg, bitmask) *reg &= ~bitmask
#define sbi(reg, bitmask) *reg |= bitmask
#define pulse_high(reg, bitmask) sbi(reg, bitmask); cbi(reg, bitmask);
#define pulse_low(reg, bitmask) cbi(reg, bitmask); sbi(reg, bitmask);

#define cport(port, data) port &= data
#define sport(port, data) port |= data

#define swap(type, i, j) {type t = i; i = j; j = t;}

#define fontbyte(x) cfont.font[x]  

#define pgm_read_word(data) *data
#define pgm_read_byte(data) *data

#if defined(TEENSYDUINO) && TEENSYDUINO >= 117
  #define regtype volatile uint8_t
  #define regsize uint8_t
#else
  #define regtype volatile uint32_t
  #define regsize uint32_t
#endif

#define bitmapdatatype unsigned short*

// VGA color palette
#define VGA_BLACK       0x0000
#define VGA_WHITE       0xFFFF
#define VGA_RED         0xF800
#define VGA_GREEN       0x0400
#define VGA_BLUE        0x001F
#define VGA_SILVER      0xC618
#define VGA_GRAY        0x8410
#define VGA_MAROON      0x8000
#define VGA_YELLOW      0xFFE0
#define VGA_OLIVE       0x8400
#define VGA_LIME        0x07E0
#define VGA_AQUA        0x07FF
#define VGA_TEAL        0x0410
#define VGA_NAVY        0x0010
#define VGA_FUCHSIA     0xF81F
#define VGA_PURPLE      0x8010
#define VGA_TRANSPARENT 0xFFFFFFFF

#define BLACK           0x0000
#define WHITE           0xFFFF
#define RED             0xF800
#define GREEN           0x0400
#define BLUE            0x001F
#define SILVER          0xC618
#define GRAY            0x8410
#define MAROON          0x8000
#define YELLOW          0xFFE0
#define OLIVE           0x8400
#define LIME            0x07E0
#define AQUA            0x07FF
#define TEAL            0x0410
#define NAVY            0x0010
#define FUCHSIA         0xF81F
#define PURPLE          0x8010
#define TRANSPARENT     0xFFFFFFFF

#include "Arduino.h"

extern uint8_t BigFont[];

struct _current_font
{
    uint8_t* font;
    uint8_t x_size;
    uint8_t y_size;
    uint8_t offset;
    uint8_t numchars;
};

enum class imagetype : byte {

    AUTO,
    NONE,
    BITMAP,
    GCI,

};

struct image_info {
    
    // Position of the top left corner of the image
    int x;
    int y;
    
    // Dimenisons
    int width;
    int height;
    
    // Bounds, these determine which part of the image to actually print
    int x1;
    int y1;
    int x2;
    int y2;
    
    int frames;
    int frame_delay;
    int file_start;
    int bits;
    imagetype type;
    char filename[20];
    SdFile file;

};

enum lcd_corner_start {

    TOP_LEFT,
    TOP_RIGHT,
    BOTTOM_LEFT,
    BOTTOM_RIGHT,
    NONE

};

char const entryStrings[4][20] {

    "TOP_LEFT",
    "TOP_RIGHT",
    "BOTTOM_LEFT",
    "BOTTOM_RIGHT",

};

class PiScreen {

    public:
    
        PiScreen();
        
        static void     InitLCD(byte orientation = LANDSCAPE);
        static void     clrScr();
        void            drawPixel(int x, int y);
        void            drawLine(int x1, int y1, int x2, int y2);
        void            fillScr(byte r, byte g, byte b);
        void            fillScr(word color);
        void            drawRect(int x1, int y1, int x2, int y2);
        void            drawRoundRect(int x1, int y1, int x2, int y2);
        void            fillRect(int x1, int y1, int x2, int y2);
        void            fillRoundRect(int x1, int y1, int x2, int y2);
        void            drawCircle(int x, int y, int radius);
        void            fillCircle(int x, int y, int radius);
        static void     setColor(byte r, byte g, byte b);
        static void     setColor(word color);
        uint16_t        RGBto565(byte r, byte g, byte b) {
        
            byte thi = ((r & 248) | g >> 5);
            byte tlo = ((g & 28) << 3 | b >> 3);

            return (thi << 8) + tlo;
            
        }
        word            getColor();
        static void     setBackColor(byte r, byte g, byte b);
        static void     setBackColor(uint32_t color);
        word            getBackColor();
        void            print(char *st, int x=CENTER, int y=CENTER, int deg=0);
        void            print(String st, int x=CENTER, int y=CENTER, int deg=0);
        void            print(char *st,SdFile imageFont,int x,int y,SdFile imageBack,int xBack,int yBack,int frameBack,int transparencyColor=-1,int space=0);
        void            printNumI(long num, int x, int y, int length=0, char filler=' ');
        void            printNumF(double num, byte dec, int x, int y, char divider='.', int length=0, char filler=' ');
        static void     setFont(uint8_t* font);
        uint8_t*        getFont();
        uint8_t         getFontXsize();
        uint8_t         getFontYsize();
        void            drawBitmap(int x, int y, int sx, int sy, bitmapdatatype data, int scale=1);
        void            drawBitmap(int x, int y, int sx, int sy, bitmapdatatype data, int deg, int rox, int roy);
        int             getDisplayXSize();
        int             getDisplayYSize();
        void            setLcdOrientation(int tmpOrient) { orient = tmpOrient; setEntryMode(TOP_LEFT); }
        int             getLcdOrientation() { return orient; }
        
        int backX,backY,backImageStart,backWidth,backHeight,backBytes;
        SdFile backgroundImageFile;
        
        image_info backgroundInfo;

        bool loadImage(char * filename,image_info * info);
        void loadHeader(image_info * info);
        void printImageInfo(image_info * info);

        void setBackground(char * filename,int x,int y);
        void clrBackground();
        void printBackground();
        
        bool printImage(char * filename,int x,int y,int frame = 0);
        bool printImage(image_info * info,int frame = 0);
        bool printImage(image_info * info,int x1,int y1,int x2,int y2) { printImage(info,0,x1,y1,x2,y2); }
        bool printImage(image_info * info,int frame,int x1,int y1,int x2,int y2);
        
        void mergeImages(SdFile * newFile,SdFile * backFile,SdFile * frontFile,int x,int y,int frame);

        
        void printBitmap(SdFile tmpFile,int x,int y); 
        void printBitmap(SdFile tmpFile,int x,int y,int imageXa,int imageYa,int imageXb,int imageYb); 
        void printBitmap(SdFile tmpFile,int x,int y,bool partialPrint,int imageXa,int imageYa,int imageXb,int imageYb,bool isbackground); 


        int  loadVideo(SdFile tmpFile,int x,int y);
        void videoFrame(int frame) { videoFrame(frame,9999,9999); }
        void videoFrame(int frame,int x,int y);

        void printRawTransparent(int transparencyColor,SdFile tmpFile,int x,int y,int frame,SdFile backFile,int xB,int yB,int frameBack);
        void printRawPartialBitmap16(
            SdFile tempFile,SdFile backFile,
            int transparencyColor,
            int imageStart,int x,int y,int imageWidth,int imageHeight,
            int imageStartBack,int xBack,int yBack,int imageWidthBack,int imageHeightBack,
            int imageXa,int imageYa,int imageXb,int imageYb);
            
        void printRaw(SdFile tmpFile,int x,int y); 
        void printRaw(SdFile tmpFile,int x,int y,int frame); 
        void printRaw(SdFile tmpFile,int x,int y,int frame,int imageXa,int imageYa,int imageXb,int imageYb,bool inverse = false); 
        void printRaw(SdFile tmpFile,int x,int y,int frame,bool partialPrint,bool inverse,int imageXa,int imageYa,int imageXb,int imageYb); 
        
        void printErrorImage(int x1,int y1);
        void printErrorImage(int x1,int y1,int frame);
        void printErrorImage(int x1,int y1,int x2,int y2);
        void printErrorImage(int x1,int y1,int x2,int y2,int frame);

        void printRawBitmap16(image_info * info,int frame); 
        void printRawBitmap24(image_info * info,int frame);
        
        void printPartialRawBitmap16(image_info * info,int frame); 
        void printPartialRawBitmap24(image_info * info,int frame);
        
        void printBitmap16(image_info * info); 
        void printBitmap24(image_info * info); 
        void printBitmap32(image_info * info); 
        
        void printPartialBitmap16(SdFile tempFile,int imageStart,int x,int y,int imageWidth,int imageHeight,int imageXa,int imageYa,int imageXb,int imageYb);
        


    // protected:

        static byte fch, fcl, bch, bcl;
        static byte orient;
        static long disp_x_size, disp_y_size;
        // byte            display_model, display_transfer_mode, display_serial_mode;
        static regtype            *P_RS, *P_WR, *P_RST, *P_SDA, *P_SCL, *P_ALE;
        static regsize            B_RS, B_WR, B_RST, B_SDA, B_SCL, B_ALE;
        static _current_font cfont;
        static bool _transparent;
        static lcd_corner_start entryMode;

        static void LCD_Writ_Bus(char VH,char VL);
        static void LCD_Write_COM(char VL);
        static void LCD_Write_DATA(char VH,char VL);
        static void LCD_Write_DATA(char VL);
        static void LCD_Write_COM_DATA(char com1,int dat1);
        void _hw_special_init();
        void setPixel(word color);
        void drawHLine(int x, int y, int l);
        void drawVLine(int x, int y, int l);
        void printChar(byte c, int x, int y);
        static void setXY(word x1, word y1, word x2, word y2);
        static void setEntryMode(lcd_corner_start id);
        static void gotoXY(word x0,word y0);
        static void clrXY();
        void rotateChar(byte c, int x, int y, int pos, int deg);
        void _set_direction_registers();
        static void _fast_fill_8(int ch, long pix);
        void _convert_float(char *buf, double num, int width, byte prec);
        
        // static bool D;
        // bool D = true;
        int const BITMAP_LINES_TO_BUFFER = 90;
        // int const BITMAP_LINES_TO_BUFFER = 113;
        
        int strMatch(char* mystring,char* searchstring);
        int strLength(char* string);
        
        
        static int const BUFFER_SIZE = 35000;
        // static int const BUFFER_SIZE = 40000;
        byte readBuffer[BUFFER_SIZE];
        
        // Storing variables for a loaded video so that frames can just be pulled
        SdFile video;
        bool videoLoaded = false;
        int videoStart,videoX,videoY,videoW,videoH,videoFrames;
        
        // saving the current info about the background image
        SdFile backgroundFile;
        int background_x,background_y;
        
};

// bool PiScreen::D = true;

#ifndef USE_SDFAT
#undef SdFile
#endif

#endif