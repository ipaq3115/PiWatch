
#include <pins_arduino.h>
#include "PiScreen.h"

#ifdef USE_SDFAT
#else
    #define SdFile File
    #define seekSet seek
    #define sync flush
#endif

// Define this to send the the debug to the desired
// Serial port
// #define db Serial

byte                PiScreen::orient;
_current_font       PiScreen::cfont;
long PiScreen::disp_x_size, PiScreen::disp_y_size;
bool                PiScreen::_transparent;
byte                PiScreen::fch, PiScreen::fcl, PiScreen::bch, PiScreen::bcl;

regtype             *PiScreen::P_RS, *PiScreen::P_WR, *PiScreen::P_RST, *PiScreen::P_SDA, *PiScreen::P_SCL, *PiScreen::P_ALE;
regsize             PiScreen::B_RS, PiScreen::B_WR, PiScreen::B_RST, PiScreen::B_SDA, PiScreen::B_SCL, PiScreen::B_ALE;
lcd_corner_start    PiScreen::entryMode = TOP_LEFT;

PiScreen::PiScreen() {

    disp_x_size           = 219;
    disp_y_size           = 219;
    
    _set_direction_registers();
    
    int const RS  = 27;
    int const WR  = 28;
    int const RST = 29;
    
    P_RS     = portOutputRegister(digitalPinToPort(RS));
    P_WR     = portOutputRegister(digitalPinToPort(WR));
    P_RST    = portOutputRegister(digitalPinToPort(RST));
    
    B_RS     = digitalPinToBitMask(RS);
    B_WR     = digitalPinToBitMask(WR);
    B_RST    = digitalPinToBitMask(RST);
    
    pinMode(RS,OUTPUT);
    pinMode(WR,OUTPUT);
    pinMode(RST,OUTPUT);
    
}

void PiScreen::LCD_Write_COM(char VL) {

    cbi(P_RS, B_RS);
    LCD_Writ_Bus(0x00,VL);

}

void PiScreen::LCD_Write_DATA(char VH,char VL) {


    sbi(P_RS, B_RS);
    LCD_Writ_Bus(VH,VL);
    
}

void PiScreen::LCD_Write_DATA(char VL) {


    sbi(P_RS, B_RS);
    LCD_Writ_Bus(0x00,VL);
    
}

void PiScreen::LCD_Write_COM_DATA(char com1,int dat1) {

    LCD_Write_COM(com1);
    LCD_Write_DATA(dat1>>8,dat1);
}

void PiScreen::InitLCD(byte orientation) {
    
    orient = orientation;

    sbi(P_RST, B_RST);
    
    delay(5);
    
    cbi(P_RST, B_RST);
    
    delay(15);
    
    sbi(P_RST, B_RST);
    
    delay(15);

    LCD_Write_COM_DATA(0xe5, 0x8000);        
    LCD_Write_COM_DATA(0x00, 0x0001);        
    LCD_Write_COM_DATA(0x01, 0x0100);
    LCD_Write_COM_DATA(0x02, 0x0000); // LCD_Write_COM_DATA(0x02, 0x0700);
    LCD_Write_COM_DATA(0x03, 0x1030); // LCD_Write_COM_DATA(0x03, 0x1000); 
    LCD_Write_COM_DATA(0x04, 0x0000);
    LCD_Write_COM_DATA(0x08, 0x0202);        
    LCD_Write_COM_DATA(0x09, 0x002F); // LCD_Write_COM_DATA(0x09, 0x0000);        
    LCD_Write_COM_DATA(0x0A, 0x0000);
    LCD_Write_COM_DATA(0x0C, 0x0000);        
    LCD_Write_COM_DATA(0x0D, 0x0000);        
    LCD_Write_COM_DATA(0x0F, 0x0000);        
    //-----Power On sequence-----------------------        
    LCD_Write_COM_DATA(0x10, 0x0000);        
    LCD_Write_COM_DATA(0x11, 0x0007);        
    LCD_Write_COM_DATA(0x12, 0x0000);        
    LCD_Write_COM_DATA(0x13, 0x0000);        
    delay(50);
    LCD_Write_COM_DATA(0x10, 0x0730); // LCD_Write_COM_DATA(0x10, 0x17B0);        
    LCD_Write_COM_DATA(0x11, 0x0007);        
    delay(10);
    LCD_Write_COM_DATA(0x12, 0x0138); // LCD_Write_COM_DATA(0x12, 0x013A);        
    delay(10);
    LCD_Write_COM_DATA(0x13, 0x1A00);        
    LCD_Write_COM_DATA(0x29, 0x000c);                
    delay(10);
    //-----Gamma control-----------------------        
    LCD_Write_COM_DATA(0x30, 0x0000);        
    LCD_Write_COM_DATA(0x31, 0x0505);        
    LCD_Write_COM_DATA(0x32, 0x0004);        
    LCD_Write_COM_DATA(0x35, 0x0006);        
    LCD_Write_COM_DATA(0x36, 0x0707);        
    LCD_Write_COM_DATA(0x37, 0x0105);        
    LCD_Write_COM_DATA(0x38, 0x0002);        
    LCD_Write_COM_DATA(0x39, 0x0707);        
    LCD_Write_COM_DATA(0x3C, 0x0704);        
    LCD_Write_COM_DATA(0x3D, 0x0807);        
    //-----Set RAM area-----------------------        
    LCD_Write_COM_DATA(0x50, 0x0000);
    LCD_Write_COM_DATA(0x51, 0x00EF);
    LCD_Write_COM_DATA(0x52, 0x0000);
    LCD_Write_COM_DATA(0x53, 0x013F);
    LCD_Write_COM_DATA(0x60, 0x2700);
    LCD_Write_COM_DATA(0x61, 0x0001);
    LCD_Write_COM_DATA(0x6A, 0x0000);
    LCD_Write_COM_DATA(0x21, 0x0000);        
    LCD_Write_COM_DATA(0x20, 0x0000);        
    //-----Partial Display Control------------        
    LCD_Write_COM_DATA(0x80, 0x0000);        
    LCD_Write_COM_DATA(0x81, 0x0000);
    LCD_Write_COM_DATA(0x82, 0x0000);
    LCD_Write_COM_DATA(0x83, 0x0000);
    LCD_Write_COM_DATA(0x84, 0x0000);        
    LCD_Write_COM_DATA(0x85, 0x0000);
    //-----Panel Control----------------------
    LCD_Write_COM_DATA(0x90, 0x0010);        
    LCD_Write_COM_DATA(0x92, 0x0000);
    LCD_Write_COM_DATA(0x93, 0x0003);
    LCD_Write_COM_DATA(0x95, 0x0110);
    LCD_Write_COM_DATA(0x97, 0x0000);        
    LCD_Write_COM_DATA(0x98, 0x0000);
    //-----Display on-----------------------        
    LCD_Write_COM_DATA(0x07, 0x0173);        
    delay(50);
    
    setColor(255, 255, 255);
    setBackColor(0, 0, 0);
    // cfont.font = 0;
    setFont(BigFont);
    _transparent = false;
    setEntryMode(TOP_LEFT);
    
}

void PiScreen::setXY(word x1, word y1, word x2, word y2) {

    #ifdef db
        db.printf("setXY %d %d %d %d\r\n",x1, y1, x2, y2);
    #endif
    
    switch(entryMode) {
    
        case TOP_LEFT:      gotoXY(x1,y1); break;
        case TOP_RIGHT:     gotoXY(x2,y1); break;
        case BOTTOM_LEFT:   gotoXY(x1,y2); break;
        case BOTTOM_RIGHT:  gotoXY(x2,y2); break;
    
    }
    
    if(orient == LANDSCAPE) {
    
        swap(word, x1, y1);
        swap(word, x2, y2);
        
        y1 = disp_y_size - y1;
        y2 = disp_y_size - y2;
        
        swap(word, y1, y2);
        
        // word x1t,x2t,y1t,y2t;
    
        // y2t = disp_y_size - x1;
        // y1t = disp_y_size - x2;
        // x1t = y1;
        // x2t = y2;
        
        // x1 = x1t;
        // x2 = x2t;
        // y1 = y1t;
        // y2 = y2t;
        
    } else if(orient == LANDSCAPE_R) {
        
        swap(word, x1, y1);
        swap(word, x2, y2);
        
        x1 = disp_x_size - x1;
        x2 = disp_x_size - x2;
        
        swap(word, x1, x2);
        
        // word x1t,x2t,y1t,y2t;
    
        // y1t = x1;
        // y2t = x2;
        // x2t = disp_x_size - y1;
        // x1t = disp_x_size - y2;
        
        // x1 = x1t;
        // x2 = x2t;
        // y1 = y1t;
        // y2 = y2t;
        
    } else if(orient == PORTRAIT) {
    
    
    } else if(orient == PORTRAIT_R) {
    
        word x1t,x2t,y1t,y2t;
    
        x2t = disp_x_size - x1;
        x1t = disp_x_size - x2;
        y2t = disp_y_size - y1;
        y1t = disp_y_size - y2;
        
        x1 = x1t;
        x2 = x2t;
        y1 = y1t;
        y2 = y2t;
        
    }

    x1 += 10;
    x2 += 10;
    
    #ifdef db
        db.printf("setXY B %d %d %d %d\r\n",x1, y1, x2, y2);
    #endif
    
    LCD_Write_COM_DATA(0x50,x1);
    LCD_Write_COM_DATA(0x52,y1);
    LCD_Write_COM_DATA(0x51,x2);
    LCD_Write_COM_DATA(0x53,y2);
    
    LCD_Write_COM(0x22); 
    
}

void PiScreen::setEntryMode(lcd_corner_start id) {

    static int lastOrient = -1;

    // #ifdef db
    //     db.printf("setEntryMode %d %s\r\n",id,entryStrings[id]);
    // #endif
    
    if(entryMode == id && orient == lastOrient) return;
    
    lastOrient = orient;
    
    entryMode = id;

    if(orient == PORTRAIT) {
    
        switch(id) {
        
            case TOP_LEFT:      LCD_Write_COM_DATA(0x03, 0b0001000000110000); break;
            case TOP_RIGHT:     LCD_Write_COM_DATA(0x03, 0b0001000000100000); break;
            case BOTTOM_LEFT:   LCD_Write_COM_DATA(0x03, 0b0001000000010000); break;
            case BOTTOM_RIGHT:  LCD_Write_COM_DATA(0x03, 0b0001000000000000); break;
        
        }

    } else if(orient == PORTRAIT_R) {
    
        switch(id) {
        
            case TOP_LEFT:      LCD_Write_COM_DATA(0x03, 0b0001000000000000); break;
            case TOP_RIGHT:     LCD_Write_COM_DATA(0x03, 0b0001000000010000); break;
            case BOTTOM_LEFT:   LCD_Write_COM_DATA(0x03, 0b0001000000100000); break;
            case BOTTOM_RIGHT:  LCD_Write_COM_DATA(0x03, 0b0001000000110000); break;
        
        }

    } else if(orient == LANDSCAPE) {
    
        switch(id) {
        
            case TOP_LEFT:      LCD_Write_COM_DATA(0x03, 0b0001000000011000); break;
            case TOP_RIGHT:     LCD_Write_COM_DATA(0x03, 0b0001000000111000); break;
            case BOTTOM_LEFT:   LCD_Write_COM_DATA(0x03, 0b0001000000001000); break;
            case BOTTOM_RIGHT:  LCD_Write_COM_DATA(0x03, 0b0001000000101000); break;
        
        }


    } else if(orient == LANDSCAPE_R) {
    
        switch(id) {
        
            case TOP_LEFT:      LCD_Write_COM_DATA(0x03, 0b0001000000101000); break;
            case TOP_RIGHT:     LCD_Write_COM_DATA(0x03, 0b0001000000001000); break;
            case BOTTOM_LEFT:   LCD_Write_COM_DATA(0x03, 0b0001000000111000); break;
            case BOTTOM_RIGHT:  LCD_Write_COM_DATA(0x03, 0b0001000000011000); break;
        
        }

    } else {
    
        #ifdef D
            db.printf("ERROR: orient %d\r\n",orient);
        #endif
    
    }
    
}

void PiScreen::gotoXY(word x0,word y0) {

    #ifdef db
        db.printf("gotoXY %d %d\r\n",x0, y0);
    #endif
    
    if(orient == PORTRAIT_R) {
    
        x0 = disp_x_size - x0;
        y0 = disp_y_size - y0;
    
    } else if(orient == LANDSCAPE) {
        
        swap(word, x0, y0);
        y0 = disp_y_size - y0;
        
    } else if(orient == LANDSCAPE_R) {
        
        swap(word, x0, y0);
        x0 = disp_x_size - x0;
    
    }

    #ifdef db
        db.printf("gotoXY B %d %d\r\n",x0, y0);
    #endif
    
    LCD_Write_COM_DATA(0x20,x0 + 10);
    LCD_Write_COM_DATA(0x21,y0);

}

void PiScreen::clrXY() {

    if (orient==PORTRAIT)   setXY(0,0,disp_x_size,disp_y_size);
    else                    setXY(0,0,disp_y_size,disp_x_size);

}

void PiScreen::drawRect(int x1, int y1, int x2, int y2) {

    if (x1>x2) swap(int, x1, x2);
    if (y1>y2) swap(int, y1, y2);

    drawHLine(x1, y1, x2-x1);
    drawHLine(x1, y2, x2-x1);
    drawVLine(x1, y1, y2-y1);
    drawVLine(x2, y1, y2-y1);
    
}

void PiScreen::drawRoundRect(int x1, int y1, int x2, int y2) {


    if (x1>x2) swap(int, x1, x2);
    if (y1>y2) swap(int, y1, y2);
    
    if ((x2-x1)>4 && (y2-y1)>4) {
    
        drawPixel(x1+1,y1+1);
        drawPixel(x2-1,y1+1);
        drawPixel(x1+1,y2-1);
        drawPixel(x2-1,y2-1);
        drawHLine(x1+2, y1, x2-x1-4);
        drawHLine(x1+2, y2, x2-x1-4);
        drawVLine(x1, y1+2, y2-y1-4);
        drawVLine(x2, y1+2, y2-y1-4);
        
    }
}

void PiScreen::fillRect(int x1, int y1, int x2, int y2) {

    if (x1 > x2) swap(int, x1, x2);
    if (y1 > y2) swap(int, y1, y2);
    
    if (fch==fcl) {

        setEntryMode(TOP_LEFT);
        
        setXY(x1, y1, x2, y2);
        
        sbi(P_RS, B_RS);
        
        _fast_fill_8(fch,((long(x2 - x1) + 1) * (long(y2 - y1) + 1)));
        
    } else {
    
        for (int i=0; i<((y2-y1)/2)+1; i++) {
        
            drawHLine(x1, y1+i, x2-x1);
            drawHLine(x1, y2-i, x2-x1);
            
        }
        
    }
}

void PiScreen::fillRoundRect(int x1, int y1, int x2, int y2) {

    setEntryMode(TOP_LEFT);
    
    if (x1>x2)
    {
        swap(int, x1, x2);
    }
    if (y1>y2)
    {
        swap(int, y1, y2);
    }

    if ((x2-x1)>4 && (y2-y1)>4)
    {
        for (int i=0; i<((y2-y1)/2)+1; i++)
        {
            switch(i)
            {
            case 0:
                drawHLine(x1+2, y1+i, x2-x1-4);
                drawHLine(x1+2, y2-i, x2-x1-4);
                break;
            case 1:
                drawHLine(x1+1, y1+i, x2-x1-2);
                drawHLine(x1+1, y2-i, x2-x1-2);
                break;
            default:
                drawHLine(x1, y1+i, x2-x1);
                drawHLine(x1, y2-i, x2-x1);
            }
        }
    }
}

void PiScreen::drawCircle(int x, int y, int radius) {

    setEntryMode(TOP_LEFT);
    
    int f = 1 - radius;
    int ddF_x = 1;
    int ddF_y = -2 * radius;
    int x1 = 0;
    int y1 = radius;
 
    setXY(x, y + radius, x, y + radius);
    LCD_Write_DATA(fch,fcl);
    setXY(x, y - radius, x, y - radius);
    LCD_Write_DATA(fch,fcl);
    setXY(x + radius, y, x + radius, y);
    LCD_Write_DATA(fch,fcl);
    setXY(x - radius, y, x - radius, y);
    LCD_Write_DATA(fch,fcl);
 
    while(x1 < y1)
    {
        if(f >= 0) 
        {
            y1--;
            ddF_y += 2;
            f += ddF_y;
        }
        x1++;
        ddF_x += 2;
        f += ddF_x;    
        setXY(x + x1, y + y1, x + x1, y + y1);
        LCD_Write_DATA(fch,fcl);
        setXY(x - x1, y + y1, x - x1, y + y1);
        LCD_Write_DATA(fch,fcl);
        setXY(x + x1, y - y1, x + x1, y - y1);
        LCD_Write_DATA(fch,fcl);
        setXY(x - x1, y - y1, x - x1, y - y1);
        LCD_Write_DATA(fch,fcl);
        setXY(x + y1, y + x1, x + y1, y + x1);
        LCD_Write_DATA(fch,fcl);
        setXY(x - y1, y + x1, x - y1, y + x1);
        LCD_Write_DATA(fch,fcl);
        setXY(x + y1, y - x1, x + y1, y - x1);
        LCD_Write_DATA(fch,fcl);
        setXY(x - y1, y - x1, x - y1, y - x1);
        LCD_Write_DATA(fch,fcl);
    }
    
    clrXY();
}

void PiScreen::fillCircle(int x, int y, int radius) {

    setEntryMode(TOP_LEFT);
    
    for(int y1=-radius; y1<=0; y1++) 
        for(int x1=-radius; x1<=0; x1++)
            if(x1*x1+y1*y1 <= radius*radius) 
            {
                drawHLine(x+x1, y+y1, 2*(-x1));
                drawHLine(x+x1, y-y1, 2*(-x1));
                break;
            }
}

void PiScreen::clrScr() {
    
    setEntryMode(TOP_LEFT);
    
    clrXY();
    
    sbi(P_RS, B_RS);
    
    _fast_fill_8(0,((disp_x_size+1)*(disp_y_size+1)));
    
}

void PiScreen::fillScr(byte r, byte g, byte b) {

    word color = ((r&248)<<8 | (g&252)<<3 | (b&248)>>3);
    fillScr(color);

}

void PiScreen::fillScr(word color) {

    setEntryMode(TOP_LEFT);
    
    long i;
    char ch, cl;
    
    ch = byte(color >> 8);
    cl = byte(color & 0xFF);

    clrXY();
    
    sbi(P_RS, B_RS);
    
    if (ch == cl) _fast_fill_8(ch,((disp_x_size+1)*(disp_y_size+1)));
    
    else  for (i=0; i<((disp_x_size + 1) * (disp_y_size + 1)); i++) LCD_Writ_Bus(ch,cl);
    
}

void PiScreen::setColor(byte r, byte g, byte b) {

    fch = ((r & 248) | g >> 5);
    fcl = ((g & 28) << 3 | b >> 3);
}

void PiScreen::setColor(word color) {

    fch = byte(color >> 8);
    fcl = byte(color & 0xFF);
}

word PiScreen::getColor() {

    return (fch<<8) | fcl;
    
}

void PiScreen::setBackColor(byte r, byte g, byte b) {

    bch=((r&248)|g>>5);
    bcl=((g&28)<<3|b>>3);
    _transparent=false;
}

void PiScreen::setBackColor(uint32_t color) {

    if (color==VGA_TRANSPARENT)
        _transparent=true;
    else
    {
        bch=byte(color>>8);
        bcl=byte(color & 0xFF);
        _transparent=false;
    }
}

word PiScreen::getBackColor() {

    return (bch<<8) | bcl;
    
}

void PiScreen::setPixel(word color) {

    LCD_Write_DATA((color>>8),(color&0xFF));    // rrrrrggggggbbbbb
}

void PiScreen::drawPixel(int x, int y) {

    // entry mode doesn't matter for single pixels
    // setEntryMode(TOP_LEFT);
    
    setXY(x, y, x, y);
    setPixel((fch<<8)|fcl);
    clrXY();
    
}

void PiScreen::drawLine(int x1, int y1, int x2, int y2) {

    if (y1==y2)
        drawHLine(x1, y1, x2-x1);
    else if (x1==x2)
        drawVLine(x1, y1, y2-y1);
    else
    {
        unsigned int    dx = (x2 > x1 ? x2 - x1 : x1 - x2);
        short            xstep =  x2 > x1 ? 1 : -1;
        unsigned int    dy = (y2 > y1 ? y2 - y1 : y1 - y2);
        short            ystep =  y2 > y1 ? 1 : -1;
        int                col = x1, row = y1;
        
        // entry mode doesn't matter for single pixels
        // setEntryMode(TOP_LEFT);
        
        if (dx < dy)
        {
            int t = - (dy >> 1);
            while (true)
            {
                setXY (col, row, col, row);
                LCD_Write_DATA (fch, fcl);
                if (row == y2)
                    return;
                row += ystep;
                t += dx;
                if (t >= 0)
                {
                    col += xstep;
                    t   -= dy;
                }
            } 
        }
        else
        {
            int t = - (dx >> 1);
            while (true)
            {
                setXY (col, row, col, row);
                LCD_Write_DATA (fch, fcl);
                if (col == x2)
                    return;
                col += xstep;
                t += dy;
                if (t >= 0)
                {
                    row += ystep;
                    t   -= dx;
                }
            } 
        }
        
    }
    clrXY();
}

void PiScreen::drawHLine(int x, int y, int l) {

    // entry mode doesn't matter for a line
    // setEntryMode(TOP_LEFT);
    
    if (l<0) {
    
        l = -l;
        x -= l;
        
    }
    
    setXY(x, y, x+l, y);
    
    if (fch == fcl) {
        
        sbi(P_RS, B_RS);
        _fast_fill_8(fch,l);
    
    } else {
        
        for (int i=0; i<l+1; i++) LCD_Write_DATA(fch, fcl);
        
    }
    
    clrXY();
    
}

void PiScreen::drawVLine(int x, int y, int l) {

    // entry mode doesn't matter for a line
    // setEntryMode(TOP_LEFT);
    
    if (l<0) {
    
        l = -l;
        y -= l;
        
    }
    
    setXY(x, y, x, y+l);
    
    if (fch == fcl) {
    
        sbi(P_RS, B_RS);
        _fast_fill_8(fch,l);
        
    } else {
    
        for (int i=0; i < l + 1; i++) LCD_Write_DATA(fch, fcl);
        
    }
    
    clrXY();
    
}

void PiScreen::printChar(byte c, int x, int y) {

    // otherwise the characters wrap around
    if(x + cfont.x_size >= disp_x_size || y + cfont.y_size >= disp_y_size || x < 0 || y < 0) return;

    byte i,ch;
    word j;
    word temp; 

    if (!_transparent) {
    
        setXY(x,y,x+cfont.x_size-1,y+cfont.y_size-1);
  
        temp=((c-cfont.offset)*((cfont.x_size/8)*cfont.y_size))+4;
        for(j=0;j<((cfont.x_size/8)*cfont.y_size);j++) {
        
            ch = pgm_read_byte(&cfont.font[temp]);
            for(i=0;i<8;i++) {   
            
                if((ch&(1<<(7-i))) != 0) setPixel((fch<<8)|fcl);
                else setPixel((bch<<8)|bcl);
                
            }
            temp++;
        }
        
    } else {
    
        temp=((c-cfont.offset)*((cfont.x_size/8)*cfont.y_size))+4;
        for(j=0;j<cfont.y_size;j++) 
        {
            for (int zz=0; zz<(cfont.x_size/8); zz++)
            {
                ch=pgm_read_byte(&cfont.font[temp+zz]); 
                for(i=0;i<8;i++)
                {   
                    setXY(x+i+(zz*8),y+j,x+i+(zz*8)+1,y+j+1);
                
                    if((ch&(1<<(7-i)))!=0)   
                    {
                        setPixel((fch<<8)|fcl);
                    } 
                }
            }
            temp+=(cfont.x_size/8);
        }
        
    }
    
    clrXY();
}

void PiScreen::rotateChar(byte c, int x, int y, int pos, int deg) {

    byte i,j,ch;
    word temp; 
    int newx,newy;
    double radian;
    radian=deg*0.0175;  

    temp=((c-cfont.offset)*((cfont.x_size/8)*cfont.y_size))+4;
    for(j=0;j<cfont.y_size;j++) 
    {
        for (int zz=0; zz<(cfont.x_size/8); zz++)
        {
            ch=pgm_read_byte(&cfont.font[temp+zz]); 
            for(i=0;i<8;i++)
            {   
                newx=x+(((i+(zz*8)+(pos*cfont.x_size))*cos(radian))-((j)*sin(radian)));
                newy=y+(((j)*cos(radian))+((i+(zz*8)+(pos*cfont.x_size))*sin(radian)));

                setXY(newx,newy,newx+1,newy+1);
                
                if((ch&(1<<(7-i)))!=0)   
                {
                    setPixel((fch<<8)|fcl);
                } 
                else  
                {
                    if (!_transparent)
                        setPixel((bch<<8)|bcl);
                }   
            }
        }
        temp+=(cfont.x_size/8);
    }
    
    clrXY();
}

void PiScreen::print(char *st, int x, int y, int deg) {

    setEntryMode(TOP_LEFT);
    
    int stl, i;

    stl = strlen(st);
    
    if(y == CENTER) y = disp_y_size / 2 - cfont.x_size / 2;

    if (x == RIGHT)  x =  (disp_x_size + 1) - (stl * cfont.x_size);
    if (x == CENTER) x = ((disp_x_size + 1) - (stl * cfont.x_size)) / 2;

    int tmpX = x;
    
    for (i=0; i<stl; i++) {
    
        if (deg==0) {
        
            // if(tmpX + cfont.x_size > disp_x_size) {
            // 
            //     y += cfont.y_size;
            //     tmpX = 0;
            // 
            // }
            
            printChar(*st++, tmpX, y);
            
            tmpX += cfont.x_size;
            
        } else rotateChar(*st++, x, y, i, deg);
    
    }
    
}

void PiScreen::print(char *st,SdFile imageFont,int x,int y,SdFile imageBack,int xBack,int yBack,int frameBack,int transparencyColor,int space) {

    #ifdef USE_SDFAT
        if(!imageFont.isOpen()) return;
    #else
        if(!imageFont) return;
    #endif
    
    
    int strLength, i;
    
    imageFont.seekSet(0);

    int fontWidth = (imageFont.read() << 8) + imageFont.read();
    int fontHeight = (imageFont.read() << 8) + imageFont.read(); 

    strLength = strlen(st);
    
    int stringWidth = strLength * fontWidth + (strLength - 1) * space;

    if(orient == PORTRAIT) {
    
        if(x == RIGHT)  x =  (disp_x_size + 1) - stringWidth;
        if(x == CENTER) x = ((disp_x_size + 1) - stringWidth) / 2;
        if(y == CENTER) y = ((disp_y_size + 1) - fontHeight) / 2;
    
    } else {
    
        if(x == RIGHT)  x =  (disp_y_size + 1) - stringWidth;
        if(x == CENTER) x = ((disp_y_size + 1) - stringWidth) / 2;
        if(y == CENTER) y = ((disp_y_size + 1) - fontHeight) / 2;
    
    }

    if(transparencyColor == -1) {
        
        for(i=0; i<strLength; i++) printRaw(imageFont,x + fontWidth * i - space * i,y,st[i] - '!');
    
    } else {
       
        for(i=0; i<strLength; i++) printRawTransparent(
        transparencyColor,
        imageFont,x + fontWidth * i - space * i,y,st[i] - '!',
        imageBack,xBack,yBack,frameBack);
    
    }
    
}

void PiScreen::print(String st, int x, int y, int deg) {

    char buf[st.length()+1];

    st.toCharArray(buf, st.length()+1);
    print(buf, x, y, deg);
}

void PiScreen::printNumI(long num, int x, int y, int length, char filler) {

    char buf[25];
    char st[27];
    boolean neg=false;
    int c=0, f=0;
  
    if (num==0)
    {
        if (length!=0)
        {
            for (c=0; c<(length-1); c++)
                st[c]=filler;
            st[c]=48;
            st[c+1]=0;
        }
        else
        {
            st[0]=48;
            st[1]=0;
        }
    }
    else
    {
        if (num<0)
        {
            neg=true;
            num=-num;
        }
      
        while (num>0)
        {
            buf[c]=48+(num % 10);
            c++;
            num=(num-(num % 10))/10;
        }
        buf[c]=0;
      
        if (neg)
        {
            st[0]=45;
        }
      
        if (length>(c+neg))
        {
            for (int i=0; i<(length-c-neg); i++)
            {
                st[i+neg]=filler;
                f++;
            }
        }

        for (int i=0; i<c; i++)
        {
            st[i+neg+f]=buf[c-i-1];
        }
        st[c+neg+f]=0;

    }

    print(st,x,y);
}

void PiScreen::printNumF(double num, byte dec, int x, int y, char divider, int length, char filler) {

    char st[27];
    boolean neg=false;

    if (dec<1)
        dec=1;
    else if (dec>5)
        dec=5;

    if (num<0)
        neg = true;

    _convert_float(st, num, length, dec);

    if (divider != '.')
    {
        for (int i=0; i<sizeof(st); i++)
            if (st[i]=='.')
                st[i]=divider;
    }

    if (filler != ' ')
    {
        if (neg)
        {
            st[0]='-';
            for (int i=1; i<sizeof(st); i++)
                if ((st[i]==' ') || (st[i]=='-'))
                    st[i]=filler;
        }
        else
        {
            for (int i=0; i<sizeof(st); i++)
                if (st[i]==' ')
                    st[i]=filler;
        }
    }

    print(st,x,y);
}

void PiScreen::setFont(uint8_t* font) {

    cfont.font     = font;
    cfont.x_size   = fontbyte(0);
    cfont.y_size   = fontbyte(1);
    cfont.offset   = fontbyte(2);
    cfont.numchars = fontbyte(3);

}

uint8_t* PiScreen::getFont() {

    return cfont.font;

}

uint8_t PiScreen::getFontXsize() {

    return cfont.x_size;
    
}

uint8_t PiScreen::getFontYsize() {

    return cfont.y_size;
    
}

void PiScreen::drawBitmap(int x, int y, int sx, int sy, bitmapdatatype data, int scale) {

    unsigned int col;
    int tx, ty, tc, tsx, tsy;

    if (scale==1) {

        setXY(x, y, x+sx-1, y+sy-1);
        for (tc=0; tc<(sx*sy); tc++)
        {
        
            col=pgm_read_word(&data[tc]);
            LCD_Write_DATA(col>>8,col & 0xff);
            
        }
    
    } else {
    
        for (ty=0; ty<sy; ty++)
        {
            setXY(x, y+(ty*scale), x+((sx*scale)-1), y+(ty*scale)+scale);
            for (tsy=0; tsy<scale; tsy++)
                for (tx=0; tx<sx; tx++)
                {
                    col=pgm_read_word(&data[(ty*sx)+tx]);
                    for (tsx=0; tsx<scale; tsx++) LCD_Write_DATA(col>>8,col & 0xff);
                }
        }
        
    }
    
    clrXY();
    
}

void PiScreen::drawBitmap(int x, int y, int sx, int sy, bitmapdatatype data, int deg, int rox, int roy) {

    unsigned int col;
    int tx, ty, newx, newy;
    double radian;
    radian=deg*0.0175;  

    if (deg==0)
        drawBitmap(x, y, sx, sy, data);
    else
    {
    
        for (ty=0; ty<sy; ty++)
            for (tx=0; tx<sx; tx++)
            {
                col=pgm_read_word(&data[(ty*sx)+tx]);

                newx=x+rox+(((tx-rox)*cos(radian))-((ty-roy)*sin(radian)));
                newy=y+roy+(((ty-roy)*cos(radian))+((tx-rox)*sin(radian)));

                setXY(newx, newy, newx, newy);
                LCD_Write_DATA(col>>8,col & 0xff);
            }
            
    }
    clrXY();
}

// SdCard Bitmap functions

void getExt(char * in,char * out) {

    for(int i=0;;i++) {
        
        if(in[i] == '.') {
            
            i++;
            
            for(int n=i;;n++) {
            
                out[n-i] = in[n];
                
                if(in[n] == 0) return;
            
            }
        
        }
        
    }

}

bool caselessStrMatch(const char * src0,const char * src1) {

    #define lowercase(x) (x < 97 ? x + 32 : x)

    for(int i=0;;i++) {
    
        // Stop at the end of either string
        if(!src0[i] || !src1[i]) return (src0[i] == src1[i]);
        
        // return false if anything doesn't match
        if(lowercase(src0[i]) != lowercase(src1[i])) return false;
        
    }
    
    #undef lowercase(x)

}

void PiScreen::mergeImages(SdFile * newFile,SdFile * backFile,SdFile * frontFile,int x,int y,int frame) {

    int timeA = micros();

    image_info back,front;
    
    back.file = *backFile;
    front.file = *frontFile;
    
    front.x = x;
    front.y = y;

    loadHeader(&back);
    loadHeader(&front);
    
    printImageInfo(&back);
    printImageInfo(&front);
    
    frontFile->seekSet(0);
    
    // for(int i=0;i<50;i++) {
    // 
    //     Serial.printf("%02X ",frontFile->read());
    // 
    // }
    
    // Fill the new file with the same header info as the background fileStart
    // which will serve as our base
    
    newFile->seekSet(0);
    backFile->seekSet(0);

    int len = backFile->read(readBuffer, back.file_start);
    newFile->write(readBuffer, len);
    
    int r=0,g=0,b=0,a=0;
    int rb=0,gb=0,bb=0;
    
    float af;
    
    uint8_t alpha = 0xFF;
    
    int pixel = 0;
    
    int backBytes,frontBytes;
    
    int frontframesize = front.width * front.height * (front.bits/8);
    
    // Uses 33KB of the buffer with a 220 wide source and 220 wide overlay
    int const BUFFER_ROWS = 30;
    
    // Loop through the rows 
    for(int row=0;row<back.height;row+=BUFFER_ROWS) {

        // Back file
        back.file.seekSet(back.file_start + back.width * back.bits/8 * row);
        backBytes = back.file.read(readBuffer, back.width * back.bits/8 * BUFFER_ROWS);
        
        // Front file
        if(row - front.y < front.height && row >= front.y) {
        
            // Serial.printf("%d %d\r\n",front.file_start + front.width * front.bits/8 * (row - front.y),front.width * front.bits/8);
        
            front.file.seekSet(front.file_start + frontframesize * frame + front.width * front.bits/8 * (row - front.y));
            frontBytes = front.file.read(readBuffer + backBytes, front.width * front.bits/8 * BUFFER_ROWS);
        
        } else {
        
            frontBytes = 0;
        
        }
        
        // Serial.printf("frontBytes %d\r\n",frontBytes);
        
        pixel = back.width * row + front.x;
        
        int count = 0;
        
        // Iterate through the data that was buffered
        for(int i=0;i<frontBytes;i+=front.bits/8) {
        
            // if(D) db.printf("px %05d %d",pixel,readBuffer[i + 2]);
            // if(D) db.printf(" to x %d y %d",x + pixel % imageWidth,y + pixel / imageWidth);
        
            alpha = readBuffer[backBytes + i + 2];
        
            int curx = front.x + i;
            int cury = row;
            
            // Opaque
            if(alpha == 0xFF) {
                
                // Pulling data for the front image here
                
                uint8_t hi = readBuffer[backBytes + i];
                uint8_t lo = readBuffer[backBytes + i + 1];
                
                // Put this data into the buffer
                
                readBuffer[front.x * back.bits/8 + count] = hi;
                readBuffer[front.x * back.bits/8 + count + 1] = lo;
                
            // Transparent
            } else if(alpha == 0x00) {
            
            // Somewhere between transparent and opaque
            } else {
                
                uint8_t hi;
                uint8_t lo;
                
                hi = readBuffer[backBytes + i];
                lo = readBuffer[backBytes + i + 1];
                
                r = hi & 248;
                g = ((hi << 5) + (lo >> 3)) & 252;
                b = (lo << 3) & 255;
                
                af = (float)alpha/255;
                
                hi = readBuffer[front.x * back.bits/8 + count];
                lo = readBuffer[front.x * back.bits/8 + count + 1];
                
                rb = hi & 248;
                gb = ((hi << 5) + (lo >> 3)) & 252;
                bb = (lo << 3) & 255;
        
                r = (r*af + rb*1*(1-af)) / (af + (1-af));
                g = (g*af + gb*1*(1-af)) / (af + (1-af));
                b = (b*af + bb*1*(1-af)) / (af + (1-af));
        
                hi = ((r&248)|g>>5);
                lo = ((g&28)<<3|b>>3);
                
                readBuffer[front.x * back.bits/8 + count] = hi;
                readBuffer[front.x * back.bits/8 + count + 1] = lo;
                
            }
            
            count += back.bits/8;
            pixel++;
            
            if(i == front.width * front.bits/8 - 1) pixel += back.width * back.bits/8 - front.width * front.bits/8;
            
            // if(D) db.println();
            
        }

        // Back file
        newFile->seekSet(back.file_start + back.width * back.bits/8 * row);
        newFile->write(readBuffer, back.width * back.bits/8 * BUFFER_ROWS);
        
        
    }
    
    newFile->sync();
    
    int timeB = micros();
    
    Serial.printf("time merge %d\r\n",timeB-timeA);

}

// Background functions

void PiScreen::setBackground(char * filename,int x,int y) {

    clrBackground();

    // Checking image type based on extension

    const char gciExt[] = "gci";
    const char bmpExt[] = "bmp";
    
    char extension[10];
    
    getExt(filename,extension);
    
    if(caselessStrMatch(extension,bmpExt))      backgroundInfo.type = imagetype::BITMAP;
    else if(caselessStrMatch(extension,gciExt)) backgroundInfo.type = imagetype::GCI;
    else                                        backgroundInfo.type = imagetype::AUTO;
    
    // Save filename in the struct
    
    for(int i=0;filename[i] != 0;i++) backgroundInfo.filename[i] = filename[i];
    
    // Open file
    
    #ifdef USE_SDFAT
        if(!backgroundInfo.file.open(filename,O_RDWR)) return;
    #else
        backgroundInfo.file = SD.open(filename);
        if(!backgroundInfo.file) return;
    #endif
    
    printImageInfo(&backgroundInfo);
    
    loadHeader(&backgroundInfo);
    
    printImageInfo(&backgroundInfo);
    
}

void PiScreen::clrBackground() {

    backgroundInfo.file.close();

}

void PiScreen::printBackground() {

    printImage(&backgroundInfo);

}

// Intelligent image handling

void PiScreen::loadHeader(image_info * info) {

    info->file.seekSet(0);

    if(info->file.read() == 'B' && info->file.read() == 'M') {
    
        info->type = imagetype::BITMAP;
        
        // The location where useful image data starts
        
        info->file.seekSet(10);
        
        info->file_start  = ((info->file.read()      ) & 0x000000FF);
        info->file_start += ((info->file.read() <<  8) & 0x0000FF00);
        info->file_start += ((info->file.read() << 16) & 0x00FF0000);
        info->file_start += ((info->file.read() << 24) & 0xFF000000);
        
        info->file.seekSet(18);
        
        // Width
        info->width  = ((info->file.read()      ) & 0x000000FF);
        info->width += ((info->file.read() <<  8) & 0x0000FF00);
        info->width += ((info->file.read() << 16) & 0x00FF0000);
        info->width += ((info->file.read() << 24) & 0xFF000000);
        
        // Height
        info->height  = ((info->file.read()      ) & 0x000000FF);
        info->height += ((info->file.read() <<  8) & 0x0000FF00);
        info->height += ((info->file.read() << 16) & 0x00FF0000);
        info->height += ((info->file.read() << 24) & 0xFF000000);
        
        info->file.seekSet(28);
        
        // Bits of data in a single LCD pixel
        info->bits  = ((info->file.read()      ) & 0x00FF);
        info->bits += ((info->file.read() <<  8) & 0xFF00);

    } else {
    
        info->type = imagetype::GCI;
    
        char rd[10];
        
        if(info->file.read(rd, 8) != 8) {
        
            ERR("load header GCI");
            return;
        
        }

        info->file.seekSet(0);
        
        info->width         = (info->file.read() << 8) + info->file.read(); // Width
        info->height        = (info->file.read() << 8) + info->file.read(); // Height
        info->bits          =  info->file.read();               // dataBits
        info->frame_delay   =  info->file.read();               // delay
        info->frames        = (info->file.read() << 8) + info->file.read(); // Frame Total
        
        info->file_start    = 8;
        
    }
   
}

bool PiScreen::printImage(image_info * info,int frame) {

    if(info->type == imagetype::GCI) {
    
        int fileStart = info->file_start;

        fileStart += frame * info->width * info->height * info->bits/8;
        
        if(info->bits == 16)          
            
            printRawBitmap16(
            info->file,
            info->file_start,
            info->x,info->y,
            info->width,info->height); 
            
        else if(info->bits == 24)
        
            printRawBitmap24(
            info->file,
            info->file_start,
            info->x,info->y,
            info->width,info->height);
    
    } else if(info->type == imagetype::BITMAP) {
    
        if(info->bits == 16) 
        
            printBitmap16(
            info->file,
            info->file_start,
            info->x,info->y,
            info->width,info->height);
            
        else if(info->bits == 32) 
        
            printBitmap32(
            info->file,
            info->file_start,
            info->x,info->y,
            info->width,info->height); 
            
        else if(info->bits == 24) 
        
            printBitmap24(
            info->file,
            info->file_start,
            info->x,info->y,
            info->width,info->height);
    
    }

}

void PiScreen::printImageInfo(image_info * info) {

    Serial.printf("Image Info: %s\r\n",info->filename);
    Serial.printf("x %d\r\n",info->x);
    Serial.printf("y %d\r\n",info->y);
    Serial.printf("width %d\r\n",info->width);
    Serial.printf("height %d\r\n",info->height);
    Serial.printf("file_start %d\r\n",info->file_start);
    Serial.printf("bits %d\r\n",info->bits);
    
    Serial.print("type ");
    switch(info->type) {
    
        case imagetype::AUTO: Serial.println("AUTO"); break;
        case imagetype::NONE: Serial.println("NONE"); break;
        case imagetype::BITMAP: Serial.println("BITMAP"); break;
        case imagetype::GCI: Serial.println("GCI"); break;
        default: Serial.println("ERROR"); break;
        
    }
    

}

// Bitmap

void PiScreen::printBitmap(SdFile tmpFile,int x,int y) {

    printBitmap(tmpFile,x,y,false,-1,-1,-1,-1,false);

}

void PiScreen::printBitmap(SdFile tmpFile,int x,int y,int imageXa,int imageYa,int imageXb,int imageYb) {

    printBitmap(tmpFile,x,y,true,imageXa,imageYa,imageXb,imageYb,false);

}

void PiScreen::printBitmap(SdFile tmpFile,int x,int y,bool partialPrint,int imageXa,int imageYa,int imageXb,int imageYb,bool isbackground) {

#ifdef USE_SDFAT
    if(!tmpFile.isOpen()) {
#else
    if(!tmpFile) {
#endif
    
        printErrorImage(x,y);
        return;

// Yes, this doesn't actually do anything. It is just so notepad++ 
// doesn't mess up the collapse groups because of a missing ending brace
#ifdef USE_SDFAT
    }
#else
    }
#endif
    
    tmpFile.seekSet(10);
    
    int imageStart;
    
    // The location where useful image data starts
    imageStart  = ((tmpFile.read()      ) & 0x000000FF);
    imageStart += ((tmpFile.read() <<  8) & 0x0000FF00);
    imageStart += ((tmpFile.read() << 16) & 0x00FF0000);
    imageStart += ((tmpFile.read() << 24) & 0xFF000000);
    
    tmpFile.seekSet(18);
    
    int imageWidth,imageHeight;
    
    // Width
    imageWidth  = ((tmpFile.read()      ) & 0x000000FF);
    imageWidth += ((tmpFile.read() <<  8) & 0x0000FF00);
    imageWidth += ((tmpFile.read() << 16) & 0x00FF0000);
    imageWidth += ((tmpFile.read() << 24) & 0xFF000000);
    
    // Height
    imageHeight  = ((tmpFile.read()      ) & 0x000000FF);
    imageHeight += ((tmpFile.read() <<  8) & 0x0000FF00);
    imageHeight += ((tmpFile.read() << 16) & 0x00FF0000);
    imageHeight += ((tmpFile.read() << 24) & 0xFF000000);
    
    tmpFile.seekSet(28);
    
    int dataBits;
    
    // Bits of data in a single LCD pixel
    dataBits  = ((tmpFile.read()      ) & 0x00FF);
    dataBits += ((tmpFile.read() <<  8) & 0xFF00);
    
    #ifdef db
        db.printf("printBitmap imageWidth %d imageHeight %d dataBits %d\r\n",imageWidth,imageHeight,dataBits);
    #endif
    
    if(isbackground) {
    
        // backX = x;
        // backY = y;
        x = backX;
        y = backY;
        backImageStart = imageStart;
        backWidth = imageWidth;
        backHeight = imageHeight;
        backBytes = dataBits/8;
    
    }
    
    if(partialPrint) {    
    
        if(dataBits == 16) {         
            printPartialBitmap16(tmpFile,imageStart,x,y,imageWidth,imageHeight,imageXa,imageYa,imageXb,imageYb);
        } 
        // else if(dataBits == 24) {     printBitmap24(imageStart,x,y,imageWidth,imageHeight,imageXa,imageYa,imageXb,imageYb); }

    } else {
    
        if(dataBits == 16) {          printBitmap16(tmpFile,imageStart,x,y,imageWidth,imageHeight); } 
        else if(dataBits == 32) {     printBitmap32(tmpFile,imageStart,x,y,imageWidth,imageHeight); }
        else if(dataBits == 24) {     printBitmap24(tmpFile,imageStart,x,y,imageWidth,imageHeight); }
    
    }
    
}

void PiScreen::printBitmap16(SdFile tempFile,int imageStart,int x,int y,int imageWidth,int imageHeight) {

    #ifdef db
        db.printf("printBitmap16 imageStart %d x %d y %d\r\n",imageStart,x,y);
    #endif
    
    // Bytes of color per pixel this is the 565 + alpha format
    int const dataBytes = 2;

    setEntryMode(BOTTOM_LEFT);
    
    // Set the window that we want to print to [For some reason this needs to be after the above 'cbi' command]
    setXY(x,y,x+imageWidth-1,y+imageHeight-1);
    
    sbi(P_RS, B_RS);
    
    #ifdef USE_SDFAT
        int bytesread;
    #else
        // Arduino Sd Library returns a int16_t so we are doing this to get 
        // the right number back when we are over the limit for a int16_t
        uint16_t bytesread;
    #endif
    
    int toRead = BUFFER_SIZE;
    toRead -= toRead % dataBytes;
    
    int imageBytes = imageWidth * imageHeight * dataBytes; 
    
    // Seek to the start of the image
    tempFile.seekSet(imageStart);
    
    // Loop through the bytes of data in the image
    for(int byteCount=0;byteCount<imageBytes;byteCount += toRead) {
    
        if(imageBytes - byteCount < toRead) toRead = imageBytes - byteCount;
        
        // Pull the data from the sdcard
        bytesread = tempFile.read(readBuffer,toRead);
        
        // Iterate through the data that was buffered
        for(int i=0;i<bytesread;i+=dataBytes) {
            
            GPIOD_PDOR = readBuffer[i + 1];
            pulse_low(P_WR, B_WR);
            
            GPIOD_PDOR = readBuffer[i];
            pulse_low(P_WR, B_WR);
        
        }
        
    }
    
    // Set the entry mode back to normal
    setEntryMode(TOP_LEFT);
    
    clrXY();
    
}

void PiScreen::printPartialBitmap16(SdFile tempFile,int imageStart,int x,int y,int imageWidth,int imageHeight,int imageXa,int imageYa,int imageXb,int imageYb) {

    // Bytes of color per pixel this is the 565 format
    int const dataBytes = 2;

    setEntryMode(TOP_LEFT);
    
    // Set the window that we want to print to
    setXY(x+imageXa,y+imageYa,x+imageXb,y+imageYb);
    
    sbi(P_RS, B_RS);
    
    int widthinbytes = imageWidth*dataBytes;
    int windowWidthBytes = (imageXb-imageXa+1)*dataBytes;
    int tmpHeight = imageHeight - 1;
    int byteCount;
    
    imageStart += (imageWidth - 1 - imageXb) * dataBytes;
    
    // Loop through the rows
    for(int he=imageYa;he<=imageYb;he++) {
        
        // Seek to the start of the line to read 
        tempFile.seekSet(imageStart + (tmpHeight-he)*widthinbytes); 
        
        // Pull the data from the sdcard, 'total' is the number of lines that are being pulled into the buffer
        byteCount = tempFile.read(readBuffer,windowWidthBytes) - 1;
        
        // Print all of the pixels in this row
        while(byteCount>=0) {
            
            GPIOD_PDOR = readBuffer[byteCount--];
            pulse_low(P_WR, B_WR);
            
            GPIOD_PDOR = readBuffer[byteCount--];
            pulse_low(P_WR, B_WR);
            
        }
        
    }
    
    clrXY();
    
}

// Video

int PiScreen::loadVideo(SdFile tmpFile,int x,int y) {
    
    tmpFile.seekSet(0);
    
    videoW          = (tmpFile.read() << 8) + tmpFile.read(); // Width
    videoH          = (tmpFile.read() << 8) + tmpFile.read(); // Height
    int dataBits    =  tmpFile.read(); // dataBits
    int frameDelay  =  tmpFile.read(); // delay
    videoFrames     = (tmpFile.read() << 8) + tmpFile.read(); // Frame Total
    
    videoStart = 8;
    
    // if(D) db.printf("videoFrames %d x %d y %d\r\n",videoFrames,x,y);
    // if(D) db.printf("videoW %d videoH %d dataBits %d\r\n",videoW,videoH,dataBits);
    
    int dataBytes = dataBits / 8;
    
    if(dataBytes != 2) { 
        #ifdef db
            db.printf("ERROR: unrecognized gci byte format %d %d\r\n",dataBits,dataBytes); 
        #endif
        return 0; 
    }
    
    if(dataBits == 16)  {
        
        videoLoaded = true;
    
        video = tmpFile;
        
        videoX = x;
        videoY = y;
        
        return videoFrames;

    }
    
    videoLoaded = false;

    return 0;

}

void PiScreen::videoFrame(int frame,int x,int y) {

    // int timeA = micros();

    // Don't bother if there is no image loaded
    if(!videoLoaded) return;

    // If x and y values weren't entered then use the video position
    if(x == 9999) x = videoX;
    if(y == 9999) y = videoY;
    
    // Don't do anything if the video position wasn't assigned
    if(x == 9999 || y == 9999) return;
    
    // Bytes of color per pixel this is the 565 format
    int const dataBytes = 2;
    
    int bytesread;
    int widthinbytes = videoW * dataBytes;

    // setEntryMode(TOP_LEFT);
    setXY(x,y,x + videoW - 1,y + videoH - 1);
    sbi(P_RS, B_RS);
    
    // int timeA = micros();
    
    int total = BUFFER_SIZE;
    
    // int imageStart = frame * videoW * videoH * dataBytes;
    
    int toRead = BUFFER_SIZE;
    toRead -= toRead % dataBytes;
    
    int frameBytes = videoW * videoH * dataBytes; 
    
    video.seekSet(videoStart + frame * frameBytes);
    
    // Loop through the bytes of data in the image
    for(int byteCount=0;byteCount<frameBytes;byteCount += toRead) {
    
        if(frameBytes - byteCount < toRead) toRead = frameBytes - byteCount;
    
        // Pull the data from the sdcard
        bytesread = video.read(readBuffer,toRead);
        
        // Iterate through the data that was buffered
        for(int i=0;i<bytesread;i+=dataBytes) {
         
            // Not uning LCD_Writ_Bus() here because it causes a 2 fps drop in speed just calling that other piece of code
            
            *(volatile uint8_t *)(&GPIOD_PDOR) = readBuffer[i];
            pulse_low(P_WR, B_WR);
            
            *(volatile uint8_t *)(&GPIOD_PDOR) = readBuffer[i + 1];
            pulse_low(P_WR, B_WR);
        
        }
        
    }

    // int timeB = micros();
    
    // if(D) db.printf("fps %05.3f\r\n",(double)1000000/(timeB-timeA));
    // if(D) db.printf("time %05d\r\n",timeB-timeA);
    
    // clrXY();
    
    /*
    NOTE!!!
    Something in the below code was causing a nasty crash.
    Maybe it was the 50 doubles?? Weird.
    */
    
    // // Timing
    // int timeB = micros();
    // 
    // int const samples = 50;
    // 
    // static double fpsBuf[samples];
    // static int count = 0;
    // 
    // double fps = (double)1000000/(timeB-timeA);
    // 
    // fpsBuf[count++] = fps;
    // count %= samples;
    // 
    // double fpsAve = 0.0;
    // 
    // for(int i=0;i<samples;i++) fpsAve += fpsBuf[i];
    // 
    // fpsAve /= samples;
    // 
    // if(D) db.printf("fps %03.3f %03.3f\r\n",fps,fpsAve);

}

// Raw

void PiScreen::printRaw(SdFile tmpFile,int x,int y) {

    printRaw(tmpFile,x,y,0,false,false,-1,-1,-1,-1);

}

void PiScreen::printRaw(SdFile tmpFile,int x,int y,int frame) {

    printRaw(tmpFile,x,y,frame,false,false,-1,-1,-1,-1);

}

void PiScreen::printRaw(SdFile tmpFile,int x,int y,int frame,int imageXa,int imageYa,int imageXb,int imageYb,bool inverse) {

    printRaw(tmpFile,x,y,frame,true,inverse,imageXa,imageYa,imageXb,imageYb);

}

void PiScreen::printRaw(SdFile tmpFile,int x,int y,int frame,bool partialPrint,bool inverse,int imageXa,int imageYa,int imageXb,int imageYb) {

    setEntryMode(TOP_LEFT);

    tmpFile.seekSet(0);
    
    int imageWidth = (tmpFile.read() << 8) + tmpFile.read(); // Width
    int imageHeight = (tmpFile.read() << 8) + tmpFile.read(); // Height
    int dataBits = tmpFile.read(); // dataBits
    int frameDelay = tmpFile.read(); // delay
    int frameTotal = (tmpFile.read() << 8) + tmpFile.read(); // Frame Total
    
#ifdef USE_SDFAT
    if(!tmpFile.isOpen()) {
#else
    if(!tmpFile) {
#endif
    
        printErrorImage(x,y,frame);
        return;

// Yes, this doesn't actually do anything. It is just so notepad++ 
// doesn't mess up the collapse groups because of a missing ending brace
#ifdef USE_SDFAT
    }
#else
    }
#endif

    
    // if(D) db.printf("frameTotal %d %d\r\n",frameTotal,frame);
    // if(D) db.printf("imageWidth %d imageHeight %d dataBits %d\r\n",imageWidth,imageHeight,dataBits);
    
    int dataBytes = dataBits / 8;
    if(dataBytes != 2 && dataBytes != 3) { 
        #ifdef db
            db.printf("ERROR: unrecognized gci byte format %d %d\r\n",dataBits,dataBytes); 
        #endif
        return;     
    }
    
    int imageStart = 8;
    
    if(frame >= frameTotal || frame < 0) {
        
        #ifdef db
            db.printf("ERROR: frame doesn't exist in this file. Frame %d frameTotal %d\r\n",frame,frameTotal);
        #endif
        printErrorImage(x,y,x + imageWidth,y + imageHeight,frame);
        return;
    
    }
    
    if(frame >= frameTotal) frame = frameTotal - 1;
    if(frame < 0) frame = 0;
    
    imageStart += frame * (imageWidth * imageHeight * dataBytes);
    
    if(partialPrint) {    

        if(dataBits == 16) {         
            
            printRawPartialBitmap16(
            tmpFile,
            imageStart,x,y,
            imageWidth,imageHeight,
            imageXa,imageYa,imageXb,imageYb);
        
        } else if(dataBits == 24) printRawBitmap24partial(tmpFile,imageStart,x,y,imageWidth,imageHeight,imageXa,imageYa,imageXb,imageYb);

    } else {
    
        if(dataBits == 16)          printRawBitmap16(tmpFile,imageStart,x,y,imageWidth,imageHeight); 
        else if(dataBits == 24)     printRawBitmap24(tmpFile,imageStart,x,y,imageWidth,imageHeight);
    
    }
    
}

// Error images

void PiScreen::printErrorImage(int x1,int y1) {

    printErrorImage(x1,y1,x1,y1,-1);

}

void PiScreen::printErrorImage(int x1,int y1,int frame) {

    printErrorImage(x1,y1,x1,y1,frame);

}

void PiScreen::printErrorImage(int x1,int y1,int x2,int y2) {

    printErrorImage(x1,y1,x2,y2,-1);

}

void PiScreen::printErrorImage(int x1,int y1,int x2,int y2,int frame) {

    int const MIN_SIZE = 100;

    // Make sure 1 and 2 have smallest and largest values respectively
    if(x1 > x2) swap(int,x1,x2);
    if(y1 > y2) swap(int,y1,y2);

    // Don't let anything go out of bounds this way
    if(x1 < 0) x1 = 0;
    if(y1 < 0) y1 = 0;
    
    // expand down and to the right if the dimensions are less than MIN_SIZE
    if(abs(x1 - x2) < MIN_SIZE) x2 = x1 + MIN_SIZE;
    if(abs(y1 - y2) < MIN_SIZE) y2 = y1 + MIN_SIZE;
    
    // Make sure nothing goes out this side of the frame either
    if(x2 > disp_x_size) x2 = disp_x_size;
    if(y2 > disp_y_size) y2 = disp_y_size;
    
    // expand up and left if the last operation made the box too small again
    if(abs(x1 - x2) < MIN_SIZE) x1 = x2 - MIN_SIZE;
    if(abs(y1 - y2) < MIN_SIZE) y1 = y2 - MIN_SIZE;

    setColor(0);
    fillRect(x1,y1,x2,y2);

    setColor(VGA_RED);
    drawRect(x1,y1,x2,y2);
    drawLine(x1,y1,x2,y2);
    drawLine(x2,y1,x1,y2);
    
    if(frame != -1 && frame < 1000000) {
    
        char tmp[10];
        itoa(frame,tmp,10);
        print(tmp,(x1 + x2) / 2,(y1 + y2) / 2);
    
    }

}

void PiScreen::printRawTransparent(int transparencyColor,SdFile tmpFile,int x,int y,int frame,SdFile backFile,int xB,int yB,int frameBack) {

    tmpFile.seekSet(0);
    
    int imageWidth  = (tmpFile.read() << 8) + tmpFile.read(); // Width
    int imageHeight = (tmpFile.read() << 8) + tmpFile.read(); // Height
    int dataBits    = tmpFile.read(); // dataBits
    int frameDelay  = tmpFile.read(); // delay
    int frameTotal  = (tmpFile.read() << 8) + tmpFile.read(); // Frame Total
    
    int dataBytes = dataBits / 8;
    if(dataBytes != 2) { 
        #ifdef db
            db.printf("ERROR: unrecognized gci byte format %d %d\r\n",dataBits,dataBytes); 
        #endif
        return; 
    }
    
    if(frame >= frameTotal || frame < 0) {
        
        #ifdef db
            db.printf("ERROR: frame doesn't exist in this file. Frame %d frameTotal %d\r\n",frame,frameTotal);
        #endif
    
    }
    
    backFile.seekSet(0);
    
    int imageWidthBack  = (backFile.read() << 8) + backFile.read(); // Width
    int imageHeightBack = (backFile.read() << 8) + backFile.read(); // Height
    int dataBitsBack    = backFile.read(); // dataBits
    int frameDelayBack  = backFile.read(); // delay
    int frameTotalBack  = (backFile.read() << 8) + backFile.read(); // Frame Total
    
    int dataBytesBack = dataBitsBack / 8;
    if(dataBytesBack != 2) { 
        #ifdef db
            db.printf("ERROR: Back, unrecognized gci byte format %d %d\r\n",dataBitsBack,dataBytesBack); 
        #endif
        return; 
    }
    
    if(frameBack >= frameTotalBack || frameBack < 0) {
        
        #ifdef db
            db.printf("ERROR: frame doesn't exist in this file. frameBack %d frameTotalBack %d\r\n",frameBack,frameTotalBack);
        #endif
        
    }
    
    int imageStart = 8;
    int imageStartBack = 8;
    
    if(frame>=frameTotal) frame = frameTotal-1;
    if(frame<0) frame = 0;
    
    if(frameBack>=frameTotalBack) frameBack = frameTotalBack-1;
    if(frameBack<0) frameBack = 0;
    
    imageStart += frame*(imageWidth*imageHeight*dataBytes);
    imageStartBack += frameBack*(imageWidthBack*imageHeightBack*dataBytesBack);
    
    if(dataBits == 16) {         
        
        printRawPartialBitmap16(
        tmpFile,backFile,
        transparencyColor,
        imageStart,x,y,imageWidth,imageHeight,
        imageStartBack,xB,yB,imageWidthBack,imageHeightBack,
        0,0,imageWidth,imageHeight); 
        
    } 

}

void PiScreen::printRawPartialBitmap16(
    SdFile tempFile,SdFile backFile,
    int transparencyColor,
    int imageStart,int x,int y,int imageWidth,int imageHeight,
    int imageStartBack,int xBack,int yBack,int imageWidthBack,int imageHeightBack,
    int imageXa,int imageYa,int imageXb,int imageYb) {

    // if(D) db.printf("printRawPartialBitmap16 transparencyColor 0x%X\r\n",transparencyColor);

    // if(D) db << pstr("printPartialBitmap16 ") << dec << imageStart << ' ' << x << ' ' << y << ' ' << imageWidth << ' ' << imageHeight << ' ' << imageXa << ' ' << imageYa << ' ' << imageXb << ' ' << imageYb << endl;

    // Bytes of color per pixel this is the 565 format
    int const dataBytes = 2;
    int const dataBytesBack = 2;
    
    // Debug
    // int timeA = micros();
    // int seektime=0,readtime=0,scrntime=0;
    // int tmptimeA;
    
    // tmptimeA = micros();
    
    setEntryMode(TOP_LEFT);
    
    // Set the window that we want to print to
    setXY(x+imageXa,y+imageYa,x+imageXb,y+imageYb);
    
    sbi(P_RS, B_RS);
    
    // scrntime += micros()-tmptimeA;
    
    int widthinbytes = imageWidth * dataBytes;
    int widthinbytesBack = imageWidthBack * dataBytesBack;
    int windowWidthBytes = (imageXb - imageXa + 1) * dataBytes;
    int tmpHeight = imageHeight - 1;
    int byteCount,byteCountBack;
    int yOffsetFromBack = y - yBack;
    int xByteOffsetFromBack = (x - xBack) * dataBytesBack;

    // Allocate the sd buffer
    byte lineBuffer[windowWidthBytes];
    byte lineBufferBack[windowWidthBytes];
    // byte lineBuffer[(imageWidth*dataBytes+1)*BITMAP_LINES_TO_BUFFER];
    
    imageStart += (imageXa)*dataBytes;
    imageStartBack += xByteOffsetFromBack;
    // imageStart += (imageWidth - 1 - imageXb)*dataBytes;
    
    // Loop through the rows
    for(int he=imageYa;he<=imageYb;he++) { // for(int he=imageYa;he<imageYb;he+=BITMAP_LINES_TO_BUFFER) {
        
        // if(D) db << pstr("he ") << dec << he << endl;
        
        // int total = BITMAP_LINES_TO_BUFFER;
        // if(he+(total-1) > imageYb) total = imageYb-he;
        
        // tmptimeA = micros();
    
        // Seek to the start of the line to read 
        tempFile.seekSet(imageStart + he * widthinbytes);
        backFile.seekSet(imageStartBack + (yOffsetFromBack + he) * widthinbytesBack);
        // tempFile.seekSet(imageStart + (tmpHeight-he)*widthinbytes);
        // tempFile.seekSet(imageStart + (imageWidth - 1 - imageXb)*dataBytes + ((imageHeight - 1 - he) * imageWidth*dataBytes));
    
        // seektime += micros()-tmptimeA;
    
        // tmptimeA = micros();
        
        // Pull the data from the sdcard, 'total' is the number of lines that are being pulled into the buffer
        byteCount = tempFile.read(lineBuffer,windowWidthBytes) - 1; // int lineBytesRead = tempFile.read(lineBuffer,(imageWidth*dataBytes)*total);
        byteCountBack = backFile.read(lineBufferBack,windowWidthBytes) - 1; // int lineBytesRead = tempFile.read(lineBuffer,(imageWidth*dataBytes)*total);
        
        // readtime += micros()-tmptimeA;
        
        // tmptimeA = micros();

        // Iterate through the line that were buffered
        // int byteCount = lineBytesRead-1;
        // for(int bufLines=0;bufLines<total;bufLines++) {
            
            // Print all of the pixels in this row
            // while(byteCount>=0) { // for(int we=imageXa;we<=imageXb;we++) { 
            for(int i=0;i<byteCount;i+=2) {
            
                if(transparencyColor == ((lineBuffer[i]<<8) + lineBuffer[i+1])) {
                
                    lineBuffer[i] = lineBufferBack[i];
                    lineBuffer[i + 1] = lineBufferBack[i + 1];
                
                }
            
                GPIOD_PDOR = lineBuffer[i];
                pulse_low(P_WR, B_WR);
                GPIOD_PDOR = lineBuffer[i+1];
                pulse_low(P_WR, B_WR);
            
            }
        
        // }
        
        
        // scrntime += micros()-tmptimeA;
    
    }
    
    // tmptimeA = micros();
    
    clrXY();
    
    // scrntime += micros()-tmptimeA;
    
    // Debug
    // int timeB = micros();
    // if(D) db << pstr(" Time: ") << dec << (timeB-timeA) << endl;
    // if(D) db << pstr(" Time: seek ") << dec << seektime << pstr(" read ") << dec << readtime << pstr(" scrn ") << dec << scrntime << endl;

}

void PiScreen::printRawBitmap16(SdFile tempFile,int imageStart,int x,int y,int imageWidth,int imageHeight) {
    
    int const dataBytes = 2;
    
    setXY(x,y,x+imageWidth-1,y+imageHeight-1);
    
    sbi(P_RS, B_RS);
    
    int bytesread;
    
    int toRead = BUFFER_SIZE;
    toRead -= toRead % dataBytes;
    
    int imageBytes = imageWidth * imageHeight * dataBytes; 
    
    // Seek to the start of the image
    tempFile.seekSet(imageStart);
    
    // Loop through the bytes of data in the image
    for(int byteCount=0;byteCount<imageBytes;byteCount += toRead) {
    
        if(imageBytes - byteCount < toRead) toRead = imageBytes - byteCount;
    
        // Pull the data from the sdcard
        bytesread = tempFile.read(readBuffer,toRead);
        
        // Iterate through the data that was buffered
        for(int i=0;i<bytesread;i+=dataBytes) {
            
            GPIOD_PDOR = readBuffer[i];
            pulse_low(P_WR, B_WR);
            
            GPIOD_PDOR = readBuffer[i + 1];
            pulse_low(P_WR, B_WR);
        
        }
        
    }
    
    clrXY();
    
}

void PiScreen::printRawPartialBitmap16(SdFile tempFile,int imageStart,int x,int y,int imageWidth,int imageHeight,int imageXa,int imageYa,int imageXb,int imageYb) {

    // if(D) db << pstr("printPartialBitmap16 ") << dec << imageStart << ' ' << x << ' ' << y << ' ' << imageWidth << ' ' << imageHeight << ' ' << imageXa << ' ' << imageYa << ' ' << imageXb << ' ' << imageYb << endl;

    // Bytes of color per pixel this is the 565 format
    int const dataBytes = 2;

    // Allocate the sd buffer
    // byte lineBuffer[(imageWidth*dataBytes+1)*BITMAP_LINES_TO_BUFFER];
    
    // Debug
    // int timeA = micros();
    // int seektime=0,readtime=0,scrntime=0;
    // int tmptimeA;
    
    // tmptimeA = micros();
    
    setEntryMode(TOP_LEFT);
    
    // Set the window that we want to print to
    setXY(x+imageXa,y+imageYa,x+imageXb,y+imageYb);
    
    sbi(P_RS, B_RS);
    
    // scrntime += micros()-tmptimeA;
    
    int widthinbytes = imageWidth * dataBytes;
    int windowWidthBytes = (imageXb-imageXa+1)*dataBytes;
    int tmpHeight = imageHeight - 1;
    int byteCount;
    
    imageStart += (imageXa)*dataBytes;
    // imageStart += (imageWidth - 1 - imageXb)*dataBytes;
    
    // Loop through the rows
    for(int he=imageYa;he<=imageYb;he++) { // for(int he=imageYa;he<imageYb;he+=BITMAP_LINES_TO_BUFFER) {
        
        // if(D) db << pstr("he ") << dec << he << endl;
        
        // int total = BITMAP_LINES_TO_BUFFER;
        // if(he+(total-1) > imageYb) total = imageYb-he;
        
        // tmptimeA = micros();
    
        // Seek to the start of the line to read 
        tempFile.seekSet(imageStart + he*widthinbytes);
        // tempFile.seekSet(imageStart + (tmpHeight-he)*widthinbytes);
        // tempFile.seekSet(imageStart + (imageWidth - 1 - imageXb)*dataBytes + ((imageHeight - 1 - he) * imageWidth*dataBytes));
    
        // seektime += micros()-tmptimeA;
    
        // tmptimeA = micros();
        
        // Pull the data from the sdcard, 'total' is the number of lines that are being pulled into the buffer
        byteCount = tempFile.read(readBuffer,windowWidthBytes) - 1; // int lineBytesRead = tempFile.read(lineBuffer,(imageWidth*dataBytes)*total);
        
        // readtime += micros()-tmptimeA;
        
        // tmptimeA = micros();

        // Iterate through the line that were buffered
        // int byteCount = lineBytesRead-1;
        // for(int bufLines=0;bufLines<total;bufLines++) {
            
            // Print all of the pixels in this row
            // while(byteCount>=0) { // for(int we=imageXa;we<=imageXb;we++) { 
            for(int i=0;i<byteCount;i+=2) {
            
                GPIOD_PDOR = readBuffer[i];
                pulse_low(P_WR, B_WR);
                GPIOD_PDOR = readBuffer[i+1];
                pulse_low(P_WR, B_WR);
            
            }
        
        // }
        
        
        // scrntime += micros()-tmptimeA;
    
    }
    
    // tmptimeA = micros();
    
    clrXY();
    
    // scrntime += micros()-tmptimeA;
    
    // Debug
    // int timeB = micros();
    // if(D) db << pstr(" Time: ") << dec << (timeB-timeA) << endl;
    // if(D) db << pstr(" Time: seek ") << dec << seektime << pstr(" read ") << dec << readtime << pstr(" scrn ") << dec << scrntime << endl;

}

void PiScreen::printRawBitmap24(SdFile tempFile,int imageStart,int x,int y,int imageWidth,int imageHeight) {
    
    // if(D) db.println("printRawBitmap24");
    
    // Bytes of color per pixel, this is the 565 + alpha format
    int const dataBytes = 3;

    setEntryMode(TOP_LEFT);
    
    // Set the window that we want to print to [For some reason this needs to be after the above 'cbi' command]
    setXY(x,y,x+imageWidth-1,y+imageHeight-1);
    
    sbi(P_RS, B_RS);
    
    int bytesread,pixel;
    
    int toRead = BUFFER_SIZE;
    toRead -= toRead % 3;
    
    int imageBytes = imageWidth * imageHeight * dataBytes; 
    
    // The flag that says we are skipping transparent stuff at the moment
    bool transparentSkipMode = false;
    
    // Seek to the start of the image
    tempFile.seekSet(imageStart);
    
    int r=0,g=0,b=0,a=0;
    int rb=0,gb=0,bb=0;
    
    float af;
    
    // Loop through the rows
    for(int byteCount=0;byteCount<imageBytes;byteCount += toRead) {
    
        if(imageBytes - byteCount < toRead) toRead = imageBytes - byteCount;
    
        // Pull the data from the sdcard
        bytesread = tempFile.read(readBuffer,toRead);
        
        pixel = byteCount/3;
    
        // Iterate through the data that was buffered
        for(int i=0;i<bytesread;i+=dataBytes) {
        
            // if(D) db.printf("px %05d %d",pixel,readBuffer[i + 2]);
            // if(D) db.printf(" to x %d y %d",x + pixel % imageWidth,y + pixel / imageWidth);
        
            // Opaque
            if(readBuffer[i + 2] == 0xFF) {
            
                // if(D) db.printf(" opaque %d",transparentSkipMode);
                
                if(transparentSkipMode) {
                    
                    transparentSkipMode = false;
                
                    // Go to the new position
                    gotoXY(x + pixel % imageWidth,y + pixel / imageWidth);
                    
                    // Go back into data writing mode
                    LCD_Write_COM(0x22); 
                    sbi(P_RS, B_RS);
                
                }
                
                GPIOD_PDOR = readBuffer[i];
                pulse_low(P_WR, B_WR);
                
                GPIOD_PDOR = readBuffer[i + 1];
                pulse_low(P_WR, B_WR);
                
            
            // Transparent
            } else if(readBuffer[i + 2] == 0x00) {
            
                // if(D) db.printf(" transparent %d",transparentSkipMode);

                transparentSkipMode = true;
            
            // Somewhere between transparent and opaque
            } else {
                
                int curx = x + pixel % imageWidth;
                int cury = y + pixel / imageWidth;
                
                // if(D) db.printf(" opaqish %d",transparentSkipMode);
            
                if(transparentSkipMode) {
                    
                    transparentSkipMode = false;
                
                    // Go to the new position
                    gotoXY(curx,cury);
                    
                    // Go back into data writing mode
                    LCD_Write_COM(0x22); 
                    sbi(P_RS, B_RS);
                
                }
                
                r = readBuffer[i] & 248;
                g = ((readBuffer[i] << 5) + (readBuffer[i + 1] >> 3)) & 252;
                b = (readBuffer[i + 1] << 3) & 255;
                a = readBuffer[i + 2];
                
                // a = 122;
                
                af = (float)a/255;
                
                int backx = curx - backgroundInfo.x;
                int backy = (cury - backgroundInfo.y);
                
                int pixels = backgroundInfo.width * backy * backgroundInfo.bits/8 + backx * backgroundInfo.bits/8;
                
                int pos = backgroundInfo.file_start + pixels;
                
                backgroundInfo.file.seekSet(pos);
                uint8_t hi = backgroundInfo.file.read();
                uint8_t lo = backgroundInfo.file.read();
                
                rb = hi & 248;
                gb = ((hi << 5) + (lo >> 3)) & 252;
                bb = (lo << 3) & 255;
                
                r = (r*af + rb*1*(1-af)) / (af + (1-af));
                g = (g*af + gb*1*(1-af)) / (af + (1-af));
                b = (b*af + bb*1*(1-af)) / (af + (1-af));
                
                hi = ((r&248)|g>>5);
                lo = ((g&28)<<3|b>>3);
                
                GPIOD_PDOR = hi;
                pulse_low(P_WR, B_WR);
                
                GPIOD_PDOR = lo;
                pulse_low(P_WR, B_WR);
                
            }
            
            pixel++;
            
            // if(D) db.println();
            
        }
        
    }
    
    clrXY();
    
}

void PiScreen::printRawBitmap24partial(SdFile tempFile,int imageStart,int x,int y,int imageWidth,int imageHeight,int imageXa,int imageYa,int imageXb,int imageYb) {
    
    // if(D) db.printf("printRawBitmap24partial x %d y %d\r\n",x,y);
    
    // Bytes of color per pixel this is the 565 + alpha format
    int const dataBytes = 3;

    setEntryMode(TOP_LEFT);
    
    // Set the window that we want to print to [For some reason this needs to be after the above 'cbi' command]
    setXY(x+imageXa,y+imageYa,x+imageXb,y+imageYb);
    
    sbi(P_RS, B_RS);
    
    int bytesread,pixel = 0;
    
    int toRead = BUFFER_SIZE;
    toRead -= toRead % 3;
    
    int imageBytes = imageWidth * imageHeight * dataBytes;

    int widthinbytes = imageWidth * dataBytes;
    int windowWidthBytes = (imageXb-imageXa+1)*dataBytes;
    int byteCount;
    
    imageStart += (imageXa)*dataBytes;    
    
    // The flag that says we are skipping transparent stuff at the moment
    bool transparentSkipMode = false;
    
    // Loop through the rows
    for(int he=imageYa;he<=imageYb;he++) {
        
        // Pull the data from the sdcard
        tempFile.seekSet(imageStart + he*widthinbytes);
        bytesread = tempFile.read(readBuffer,windowWidthBytes);
        
        // pixel;
    
        // Iterate through the data that was buffered
        for(int i=0;i<bytesread;i+=dataBytes) {
        
            // Opaque
            if(readBuffer[i + 2] == 0xFF) {
            
                // if(D) db.printf(" opaque %d",transparentSkipMode);
                
                if(transparentSkipMode) {
                    
                    transparentSkipMode = false;
                
                    // Go to the new position
                    gotoXY(x + imageXa + pixel % ((imageXb - imageXa) + 1),y + imageYa + pixel / ((imageXb - imageXa) + 1));
                    
                    // if(D) db.printf("gotoXY(x %d,y %d); %d %d\r\n",
                    // x + pixel % ((imageXb - imageXa) + 1),y + pixel / ((imageXb - imageXa) + 1),
                    // pixel % ((imageXb - imageXa) + 1),pixel / ((imageXb - imageXa) + 1));
                    
                    // Go back into data writing mode
                    LCD_Write_COM(0x22); 
                    sbi(P_RS, B_RS);
                
                }
                
                GPIOD_PDOR = readBuffer[i];
                pulse_low(P_WR, B_WR);
                
                GPIOD_PDOR = readBuffer[i + 1];
                pulse_low(P_WR, B_WR);
                
            
            // Transparent
            } else if(readBuffer[i + 2] == 0x00) {
            
                // if(D) db.printf(" transparent %d",transparentSkipMode);

                transparentSkipMode = true;
            
            // Somewhere between transparent and opaque
            } else {
            
                // if(D) db.printf(" opaqish %d",transparentSkipMode);
            
                if(transparentSkipMode) {
                    
                    transparentSkipMode = false;
                
                    // Go to the new position
                    gotoXY(x + imageXa + pixel % ((imageXb - imageXa) + 1),y + imageYa + pixel / ((imageXb - imageXa) + 1));
                    
                    // if(D) db.printf("gotoXY(x %d,y %d); %d %d\r\n",
                    // x + pixel % ((imageXb - imageXa) + 1),y + pixel / ((imageXb - imageXa) + 1),
                    // pixel % ((imageXb - imageXa) + 1),pixel / ((imageXb - imageXa) + 1));
                    
                    // Go back into data writing mode
                    LCD_Write_COM(0x22); 
                    sbi(P_RS, B_RS);
                
                }
                
                // Just ignoring the opacity at the moment
                GPIOD_PDOR = readBuffer[i];
                pulse_low(P_WR, B_WR);
                
                GPIOD_PDOR = readBuffer[i + 1];
                pulse_low(P_WR, B_WR);
                
            }
            
            pixel++;
            
            // if(D) db.println();
            
        }
        
    }
    
    clrXY();
    
}

/*


void PiScreen::printBitmap16(SdFile tempFile,int imageStart,int x,int y,int imageWidth,int imageHeight) {

    #define TIMING_DEBUG_A
    #undef TIMING_DEBUG_B
    
    if(D) {
        
        db.printf("printBitmap16 %d %d %d %d %d\r\n",
        imageStart,x,y,imageWidth,imageHeight); 
    
    }
    
    // Bytes of color per pixel this is the 565 format
    int const dataBytes = 2;
    
    // Allocate the sd buffer
    // byte lineBuffer[(imageWidth*dataBytes+1)*BITMAP_LINES_TO_BUFFER];
    
    #if defined TIMING_DEBUG_A
        int timeA = micros();
    #elif defined TIMING_DEBUG_B
        int tmptimeA;
        int seektime=0,readtime=0,scrntime=0,scrncntrltime=0;
        if(D) tmptimeA = micros();
    #endif
    
    // Set some sort of pin states that indicade that data be flowing
    
#endif
    
    setEntryMode(BOTTOM_RIGHT);
    
    // Set the window that we want to print to [this needs to be after the above 'cbi' command]
    setXY(x,y,x+imageWidth-1,y+imageHeight-1);
    
    sbi(P_RS, B_RS);
    
    #if defined TIMING_DEBUG_B
        if(D) scrncntrltime += micros()-tmptimeA;
    #endif
    
    int total = BITMAP_LINES_TO_BUFFER;
    int byteCount;
    int widthinbytes = imageWidth*dataBytes;
    int tmp = imageHeight - 1;
    // int rowCountTmp = 0;
    
    // if(D) { db.println("about to loop"); delay(100); }
    
    // Loop through the rows
    for(int he=0;he<imageHeight;he+=BITMAP_LINES_TO_BUFFER) {
    
        // if(D) { db.printf("he %d\r\n",he); delay(100); }
    
        #if defined TIMING_DEBUG_B
            if(D) tmptimeA = micros();
        #endif
        
        // total = BITMAP_LINES_TO_BUFFER;
        if(he+(BITMAP_LINES_TO_BUFFER-1) > imageHeight) total = imageHeight-he;
        
        // if(D) { db.println("A"); delay(100); }
        
        // Seek to the start of the line to read 
        tempFile.seekSet(imageStart + ((tmp - he - (total-1)) * widthinbytes));
    
        // if(D) { db.println("B"); delay(100); }
        
        #if defined TIMING_DEBUG_B
            if(D) seektime += micros()-tmptimeA;
            if(D) tmptimeA = micros();
        #endif
    
        // if(D) { db.println("C"); delay(100); }
        
        // Pull the data from the sdcard, 'total' is the number of lines that are being pulled into the buffer
        byteCount = tempFile.read(readBuffer,widthinbytes * total) - 1;
    
        // if(D) { db.println("D"); delay(100); }
        
        #if defined TIMING_DEBUG_B
            if(D) readtime += micros()-tmptimeA;
            if(D) tmptimeA = micros();
        #endif
        
        // Serial.printf("total %d byteCount %d widthinbytes %d\r\n",total,byteCount,widthinbytes); delay(10);

        // Iterate through the line that were buffered
        // while(byteCount>=0) {
        // rowCountTmp = 0;
        for(int i=0;i<total;i++) {
        
            // if(D) db.printf("i %d\r\n",i);
        
            int a = byteCount - widthinbytes * i;
        
            // Serial.printf("i %d a %d\r\n",i,a);
            
            for(int n = byteCount - widthinbytes * (i + 1) + 1; n <= a; n+=2) {
        
                // if(D) db.printf("n %d\r\n",n);
    
                GPIOD_PDOR = readBuffer[n + 1];
                // GPIOD_PDOR = lineBuffer[byteCount--];
                pulse_low(P_WR, B_WR);
                
                GPIOD_PDOR = readBuffer[n];
                // GPIOD_PDOR = lineBuffer[byteCount--];
                pulse_low(P_WR, B_WR);
                
            }
        
        }
    
        #if defined TIMING_DEBUG_B
            if(D) scrntime += micros()-tmptimeA;
        #endif
    
    }
    
    // if(D) db.println("Loop done");
    
    #if defined TIMING_DEBUG_B
        if(D) tmptimeA = micros();
    #endif
    
    // Doing something with the pins that says data is done
    #endif
    
    clrXY();
    
    #if defined TIMING_DEBUG_A
        timeA = micros()-timeA;
        if(D) db.printf(" Time: %d\r\n",timeA);
    #elif defined TIMING_DEBUG_B
        if(D) scrncntrltime += micros()-tmptimeA;
        if(D) db.printf(" Time: seek %d read %d scrn %d scrncntrltime %d\r\n",seektime,readtime,scrntime,scrncntrltime);
    #endif

}



*/

void PiScreen::printBitmap24(SdFile tempFile,int imageStart,int x,int y,int imageWidth,int imageHeight) {
    
    #ifdef db
        db.printf("printBitmap24 imageStart %d x %d y %d\r\n",imageStart,x,y);
    #endif
    
    // Bytes of color per pixel this is the R8G8B8 format
    int const dataBytes = 3;

    setEntryMode(BOTTOM_LEFT);
    
    // Set the window that we want to print to [For some reason this needs to be after the above 'cbi' command]
    setXY(x,y,x+imageWidth-1,y+imageHeight-1);
    
    sbi(P_RS, B_RS);
    
    #ifdef USE_SDFAT
        int bytesread;
    #else
        // Arduino Sd Library returns a int16_t so we are doing this to get 
        // the right number back when we are over the limit for a int16_t
        uint16_t bytesread;
    #endif
    
    int toRead = BUFFER_SIZE;
    toRead -= toRead % dataBytes;
    
    // #ifdef db
    //     db.printf("toRead %d\r\n",toRead);
    // #endif
    
    int imageBytes = imageWidth * imageHeight * dataBytes; 
    
    // Seek to the start of the image
    tempFile.seekSet(imageStart);
    
    // Loop through the bytes of data in the image
    for(int byteCount=0;byteCount<imageBytes;byteCount += toRead) {
    
        if(imageBytes - byteCount < toRead) toRead = imageBytes - byteCount;
        
        // Pull the data from the sdcard
        bytesread = tempFile.read(readBuffer,toRead);
        
        // Iterate through the data that was buffered
        for(int i=0;i<bytesread;i+=dataBytes) {
            
            GPIOD_PDOR = ((readBuffer[i+2] & 248) | readBuffer[i+1] >> 5);
            pulse_low(P_WR, B_WR);
            
            GPIOD_PDOR = ((readBuffer[i+1] & 28) << 3 | readBuffer[i] >> 3);
            pulse_low(P_WR, B_WR);
        
        }
        
    }
    
    // Set the entry mode back to normal
    setEntryMode(TOP_LEFT);
    
    clrXY();
    
}

void PiScreen::printBitmap32(SdFile tempFile,int imageStart,int x,int y,int imageWidth,int imageHeight) {

    // if(D) db << pstr("printBitmap16 ") << dec << imageStart << ' ' << x << ' ' << y << ' ' << imageWidth << ' ' << imageHeight << endl;
    
    // Bytes of color per pixel this is the 565 format
    int const dataBytes = 4;
    #if defined db
        int seektime=0,readtime=0,scrntime=0,scrncntrltime=0;
        int tmptimeA;
        tmptimeA = micros();
    #endif
    
    setEntryMode(TOP_LEFT);
    
    // Set the window that we want to print to [For some reason this needs to be after the above 'cbi' command]
    setXY(x,y,x+imageWidth-1,y+imageHeight-1);
    
    sbi(P_RS, B_RS);
    
    #if defined db
        scrncntrltime += micros()-tmptimeA;
    #endif
    
    int total = BITMAP_LINES_TO_BUFFER;
    int byteCount,backByteCount;
    int widthinbytes = imageWidth*dataBytes;
    int backwidthinbytes = backWidth*backBytes;
    int tmpHeight = imageHeight - 1;
    byte thi,tlo;
    int backXoffset = x-backX;
    int backYoffset = y-backY;
    

    // Allocate the sd buffer
    byte lineBuffer[widthinbytes];
    byte lineBufferBack[backwidthinbytes];
    
    int r=0,g=0,b=0,a=0;
    int rb=0,gb=0,bb=0;
    
    float af;
    float tmp;
    
    // Loop through the rows
    for(int he=0;he<imageHeight;he++) {
    
        #if defined db
            tmptimeA = micros();
        #endif
        
        // total = BITMAP_LINES_TO_BUFFER;
        // if(he+(BITMAP_LINES_TO_BUFFER-1) > imageHeight) total = imageHeight-he;
        
        // Seek to the start of the line to read 
        backgroundImageFile.seekSet(backImageStart + (backHeight - 1 - he - backYoffset) * backwidthinbytes - backXoffset*backBytes); 
        tempFile.seekSet(imageStart + (tmpHeight-he)*widthinbytes); 
        // tempFile.seekSet(imageStart + ((tmp - he - (total-1)) * widthinbytes));
    
        #if defined db
            seektime += micros()-tmptimeA;
            tmptimeA = micros();
        #endif
    
        // Pull the data from the sdcard, 'total' is the number of lines that are being pulled into the buffer
        backByteCount = backgroundImageFile.read(lineBufferBack,backwidthinbytes) - 1;
        byteCount = tempFile.read(lineBuffer,widthinbytes) - 1;
        // byteCount = tempFile.read(lineBuffer,(imageWidth*dataBytes)*total) - 1;
    
        #if defined db
            readtime += micros()-tmptimeA;
            tmptimeA = micros();
        #endif

        // Iterate through the line that were buffered
        while(byteCount >= 0) {
        
            r = lineBuffer[byteCount--];
            g = lineBuffer[byteCount--];
            b = lineBuffer[byteCount--];
            a = lineBuffer[byteCount--];
            
            // a = 122;
            
            af = (float)a/255;
            
            rb = lineBufferBack[backByteCount] & 248;
            gb = ((lineBufferBack[backByteCount] << 5) + (lineBufferBack[backByteCount-1] >> 3)) & 252;
            bb = (lineBufferBack[backByteCount-1] << 3) & 255;
            backByteCount-=2;
            
            // if(a==0) {
            
                // r = rb;
                // g = gb;
                // b = bb;
            
            // } else if(a!=255) {
                
                r = (r*af + rb*1*(1-af)) / (af + (1-af));
                g = (g*af + gb*1*(1-af)) / (af + (1-af));
                b = (b*af + bb*1*(1-af)) / (af + (1-af));
            
            // }
            
            // db << dec << rb << ' ' << dec << gb << ' ' << dec << bb << endl;
            
            // r = (r+rb)/2;
            // g = (g+gb)/2;
            // b = (b+bb)/2;
            
            // db << dec << r << ' ' << dec << g << ' ' << dec << b << endl << endl;
            
            // r = abs(r-rb)*af;
            // g = abs(g-gb)*af;
            // b = abs(b-bb)*af;
            
            // r += rb;
            // g += gb;
            // b += bb;
            
            // r = (rb + (r*a/255)); // if(r>255) r=255;
            // g = (gb + (g*a/255)); // if(g>255) g=255;
            // b = (bb + (b*a/255)); // if(b>255) b=255;
            
            // if(a) {
            //     
            //     // Not transparent
                // thi = ((rb&248)|gb>>5);
                // tlo = ((gb&28)<<3|bb>>3);
                thi = ((r&248)|g>>5);
                tlo = ((g&28)<<3|b>>3);
            //     
            //     backByteCount-=2;
            // 
            // } else {
            // 
            //     // Completely transparent
            //     thi = lineBufferBack[backByteCount--];
            //     tlo = lineBufferBack[backByteCount--];
            // 
            // }
            
        
            GPIOD_PDOR = thi;
            pulse_low(P_WR, B_WR);
            
            GPIOD_PDOR = tlo;
            pulse_low(P_WR, B_WR);
        
        }
        
        #ifdef db

            scrntime += micros()-tmptimeA;
        
        #endif
    
    }
    
    #ifdef db
        tmptimeA = micros();
    #endif
    
    clrXY();
    
    #ifdef db
    
        scrncntrltime += micros()-tmptimeA;
        db.printf(" Time: seek %d read %d scrn %d scrncntrltime %d\r\n",seektime,readtime,scrntime,scrncntrltime);
        
    #endif

}





int PiScreen::strMatch(char* mystring,char* searchstring){
  
  //Serial3.println(F("StrMatch:"));delay(15);
  
  int mystringLen = strLength(mystring);
  int searchstringLen = strLength(searchstring);
  
  for(int i=0;i<mystringLen;i++){
    //Serial3.print(mystring[i]);delay(2);
    if(mystring[i]==searchstring[0]){
      for(int n=1;n<searchstringLen;n++){
        if(mystring[i+n]!=searchstring[n])
          break;
        else if(n==searchstringLen-1)
          return i;
      }
    }
  }
  //Serial3.println();
  
  return -1;

}

int PiScreen::strLength(char* string){
  
  
  int count = 0;
  int z=0;
  while(z<150) {
    
    //Serial1.println((String)string[z]);
      
    if(string[z++]==0)
      break;
    else    
      count++;
  
  }
  
  //Serial1.println("count "+(String)count);
  
  return count;

}





int PiScreen::getDisplayXSize() {

    if (orient==PORTRAIT)   return disp_x_size + 1;
    else                    return disp_y_size + 1;
   
}

int PiScreen::getDisplayYSize() {

    if (orient==PORTRAIT)   return disp_y_size + 1;
    else                    return disp_x_size + 1;
    
}

void PiScreen::LCD_Writ_Bus(char VH,char VL) {

    *(volatile uint8_t *)(&GPIOD_PDOR) = VH;
    pulse_low(P_WR, B_WR);
    
    *(volatile uint8_t *)(&GPIOD_PDOR) = VL;
    pulse_low(P_WR, B_WR);
  
}

void PiScreen::_set_direction_registers() {

    GPIOD_PDDR |= 0xFF;
    PORTD_PCR0  = PORT_PCR_SRE | PORT_PCR_DSE | PORT_PCR_MUX(1);
    PORTD_PCR1  = PORT_PCR_SRE | PORT_PCR_DSE | PORT_PCR_MUX(1);
    PORTD_PCR2  = PORT_PCR_SRE | PORT_PCR_DSE | PORT_PCR_MUX(1);
    PORTD_PCR3  = PORT_PCR_SRE | PORT_PCR_DSE | PORT_PCR_MUX(1);
    PORTD_PCR4  = PORT_PCR_SRE | PORT_PCR_DSE | PORT_PCR_MUX(1);
    PORTD_PCR5  = PORT_PCR_SRE | PORT_PCR_DSE | PORT_PCR_MUX(1);
    PORTD_PCR6  = PORT_PCR_SRE | PORT_PCR_DSE | PORT_PCR_MUX(1);
    PORTD_PCR7  = PORT_PCR_SRE | PORT_PCR_DSE | PORT_PCR_MUX(1);

}

void PiScreen::_fast_fill_16(int ch, int cl, long pix) {

    long blocks;

    *(volatile uint8_t *)(&GPIOD_PDOR) = ch;
      GPIOB_PCOR = 0x000F000F;                        // clear data lines B0-3,B16-19
    GPIOB_PSOR = (0x0F & cl) | ((cl >> 4) << 16);      // set data lines 0-3,16-19 if set in cl

    blocks = pix/16;
    for (int i=0; i<blocks; i++) {
    
        pulse_low(P_WR, B_WR);
        pulse_low(P_WR, B_WR);
        pulse_low(P_WR, B_WR);
        pulse_low(P_WR, B_WR);
        pulse_low(P_WR, B_WR);
        pulse_low(P_WR, B_WR);
        pulse_low(P_WR, B_WR);
        pulse_low(P_WR, B_WR);
        pulse_low(P_WR, B_WR);
        pulse_low(P_WR, B_WR);
        pulse_low(P_WR, B_WR);
        pulse_low(P_WR, B_WR);
        pulse_low(P_WR, B_WR);
        pulse_low(P_WR, B_WR);
        pulse_low(P_WR, B_WR);
        pulse_low(P_WR, B_WR);
        
    }
    
    if ((pix % 16) != 0) for (int i=0; i<(pix % 16); i++) pulse_low(P_WR, B_WR);
    
}

void PiScreen::_fast_fill_8(int ch, long pix) {

    long blocks;

    *(volatile uint8_t *)(&GPIOD_PDOR) = ch;

    blocks = pix/16;
    for (int i=0; i<blocks; i++) {
    
        pulse_low(P_WR, B_WR);pulse_low(P_WR, B_WR);
        pulse_low(P_WR, B_WR);pulse_low(P_WR, B_WR);
        pulse_low(P_WR, B_WR);pulse_low(P_WR, B_WR);
        pulse_low(P_WR, B_WR);pulse_low(P_WR, B_WR);
        pulse_low(P_WR, B_WR);pulse_low(P_WR, B_WR);
        pulse_low(P_WR, B_WR);pulse_low(P_WR, B_WR);
        pulse_low(P_WR, B_WR);pulse_low(P_WR, B_WR);
        pulse_low(P_WR, B_WR);pulse_low(P_WR, B_WR);
        pulse_low(P_WR, B_WR);pulse_low(P_WR, B_WR);
        pulse_low(P_WR, B_WR);pulse_low(P_WR, B_WR);
        pulse_low(P_WR, B_WR);pulse_low(P_WR, B_WR);
        pulse_low(P_WR, B_WR);pulse_low(P_WR, B_WR);
        pulse_low(P_WR, B_WR);pulse_low(P_WR, B_WR);
        pulse_low(P_WR, B_WR);pulse_low(P_WR, B_WR);
        pulse_low(P_WR, B_WR);pulse_low(P_WR, B_WR);
        pulse_low(P_WR, B_WR);pulse_low(P_WR, B_WR);
        
    }
    
    if ((pix % 16) != 0) { for(int i=0; i<(pix % 16); i++) { pulse_low(P_WR, B_WR); pulse_low(P_WR, B_WR); } }
    
}

#undef db
