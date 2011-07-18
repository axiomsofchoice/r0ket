#include <sysinit.h>

#include "basic/basic.h"

#include "lcd/lcd.h"
#include "lcd/print.h"
#include "lcd/allfonts.h"

#include "filesystem/ff.h"
#include "funk/nrf24l01p.h"
#include "usb/usbmsc.h"

#include <string.h>

/**************************************************************************/

uint32_t const testkey[4] = {
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
};

#define MAXNICK 20
char nickname[MAXNICK];

void f_init(void){
    FIL file[2];            /* File objects */
    UINT readbytes;
    int res;

    nrf_init();

    struct NRF_CFG config = {
        .channel= 81,
        .txmac= "REMOT",
        .nrmacs=1,
        .mac0=  "REMOT",
        .maclen ="\x10",
    };

    nrf_config_set(&config);

    res=f_open(&file[0], "nick.cfg", FA_OPEN_EXISTING|FA_READ);
    lcdPrint("open:");
    lcdPrintln(f_get_rc_string(res));
    if(res){
        lcdPrintln("no nick.cfg...");
        return;
    };

    res = f_read(&file[0], nickname, MAXNICK-1, &readbytes);
    lcdPrint("read:");
    lcdPrintln(f_get_rc_string(res));
    if(res){
        return;
    };

    nickname[MAXNICK-1]=0;

    // truncate at newline (or other control chars)
    for(int i=0;i<MAXNICK;i++){
        if(nickname[i]<32)nickname[i]=0;
    };

    lcdPrint("<");
    lcdPrint(nickname);
    lcdPrint(">");
    lcdNl();

    res=f_close(&file[0]);
    lcdPrint("close:");
    lcdPrintln(f_get_rc_string(res));
    if(res){
        return;
    };

    lcdPrintln("Done.");
};


static FONT fonts[] = {
 &Font_7x8,
 &Font_Ubuntu18pt,
 &Font_Ubuntu29pt,
 &Font_Ubuntu36pt,
 &Font_Orbitron14pt,
 &Font_3x6,
 &Font_5x8,
 &Font_8x8,
 &Font_8x8Thin,
 &Font_Invaders
};


void f_nick(void){
    static char ctr=0;
    char key;
    signed char x=10;
    signed char y=10;
    while (1) {
        lcdClear();
        lcdFill(255);

        font=fonts[ctr%10];
        DoString(x,y,nickname);
//        lcdSafeSetPixel(x,y,1);

        font=&Font_7x8;
        lcdSetCrsr(50,50);
        lcdPrintInt(x);
        lcdPrint("x");
        lcdPrintInt(y);

        lcdDisplay(0);
        delayms(40);

        key= getInputRaw();
        if(key==BTN_UP){
            --y;//if(--y<0) y=0;
        }else if (key ==BTN_DOWN){
            ++y;//if(++y>=RESY) y=RESY-1;
        }else if (key ==BTN_LEFT){
            --x;//if(--x<0) x=0;
        }else if (key ==BTN_RIGHT){
            ++x;//if(++x>=RESX) x=RESX-1;
        }else if (key ==BTN_ENTER){
            lcdClear();
            lcdPrintln("Done.");
            lcdDisplay(0);
            ctr++;
            break;
        };
    };
};

/***********************************************************************/

void gotoISP(void) {
    DoString(0,0,"Enter ISP!");
    lcdDisplay(0);
    ISPandReset(5);
}

void lcd_mirror(void) {
    lcdToggleFlag(LCD_MIRRORX);
};

void lcd_invert(void) {
    lcdToggleFlag(LCD_INVERTED);
};

void adc_check(void) {
    int dx=0;
    int dy=8;
    // Print Voltage
    dx=DoString(0,dy,"Voltage:");
    while ((getInputRaw())==BTN_NONE){
        DoInt(dx,dy,GetVoltage());
       lcdDisplay(0);
    };
    dy+=8;
    dx=DoString(0,dy,"Done.");
};

void msc_menu(void){
    DoString(0,8,"MSC Enabled.");
    lcdDisplay(0);
    usbMSCInit();
    while(!getInputRaw())delayms(10);
    DoString(0,16,"MSC Disabled.");
    usbMSCOff();
};

/**************************************************************************/

const struct MENU_DEF menu_ISP =    {"Invoke ISP",  &gotoISP};
const struct MENU_DEF menu_init =   {"F Init",   &f_init};
const struct MENU_DEF menu_nick =    {"F Nick",   &f_nick};
//const struct MENU_DEF menu_snd =    {"F Send",   &f_send};
const struct MENU_DEF menu_mirror = {"Mirror",   &lcd_mirror};
const struct MENU_DEF menu_invert = {"Invert",   &lcd_invert};
const struct MENU_DEF menu_volt =   {"Akku",   &adc_check};
const struct MENU_DEF menu_msc =   {"MSC",   &msc_menu};
const struct MENU_DEF menu_nop =    {"---",   NULL};

static menuentry menu[] = {
    &menu_init,
    &menu_nick,
//    &menu_snd,
    &menu_nop,
    &menu_mirror,
    &menu_invert,
    &menu_volt,
    &menu_msc,
    &menu_nop,
    &menu_ISP,
    NULL,
};

static const struct MENU mainmenu = {"Mainmenu", menu};

void main_font(void) {

    strcpy(nickname,"Äg€"); 

    font=&Font_7x8;

    while (1) {
        lcdFill(0); // clear display buffer
        lcdDisplay(0);
        handleMenu(&mainmenu);
        gotoISP();
    }
};

void tick_font(void){
    static int foo=0;
    static int toggle=0;
	if(foo++>80){
        toggle=1-toggle;
		foo=0;
        gpioSetValue (RB_LED0, toggle); 
	};
    return;
};

