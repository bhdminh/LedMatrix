/*--------------------------------------------------------------------------------------
 Demo for RGB panels

 DMD_STM32a example code for STM32F103xxx board
 ------------------------------------------------------------------------------------- */
#include <Arduino.h>
#include "DMD_RGB.h"

#include "main.h"

#if (CONFIG_APP_CC_SK)
 // Fonts includes
#include "st_fonts/Arial14.h"
#pragma GCC diagnostic warning "-Wnarrowing"
#pragma GCC diagnostic warning "-Woverflow" 

//Number of panels in x and y axis
#define DISPLAYS_ACROSS 1
#define DISPLAYS_DOWN 1

// Enable of output buffering
// if true, changes only outputs to matrix after
// swapBuffers(true) command
// If dual buffer not enabled, all output draw at matrix directly
// and swapBuffers(true) cimmand do nothing
#define ENABLE_DUAL_BUFFER false

// ==== DMD_RGB pins ====
// mux pins - A, B, C... all mux pins must be selected from same port!
#define DMD_PIN_A PB4
#define DMD_PIN_B PB5
#define DMD_PIN_C PB6
#define DMD_PIN_D PB7
#define DMD_PIN_E PB15
// put all mux pins at list
uint8_t mux_list[] = { DMD_PIN_A , DMD_PIN_B , DMD_PIN_C , DMD_PIN_D , DMD_PIN_E };

// 1bit color = pin OE must be one of PB0 PB1 PA6 PA7
// 4bit color = pin OE any gpio
#define DMD_PIN_nOE PB12
#define DMD_PIN_SCLK PB9

// pins for R0, G0, B0, R1, G1, B1 channels and for clock
// all this pins also must be selected from same port!
uint8_t custom_rgbpins[] = { PB8, PB0,PB1,PB13,PB2,PB3,PB14 }; // CLK, R0, G0, B0, R1, G1, B1

//Fire up the DMD object as dmd<MATRIX_TYPE, COLOR_DEPTH>
// We use 64x32 matrix with 16 scans and 1bit color:
DMD_RGB <RGB64x32plainS16, COLOR_4BITS> dmd(mux_list, DMD_PIN_nOE, DMD_PIN_SCLK, custom_rgbpins, DISPLAYS_ACROSS, DISPLAYS_DOWN, ENABLE_DUAL_BUFFER);
// other options are:
// <RGB32x16plainS8> -  32x16 matrix with 8scans
// <RGB80x40plainS20> - 80x40 matrix with 20scans
// <RGB64x64plainS32> - 64x64 matrix with 32scans
// Color depth - <COLOR_4BITS> or <COLOR_1BITS>


// --- Define fonts ----
DMD_Standard_Font Arial_F(Arial_14);
static void draw_scrolling_edge(void); 
static void draw_scrolling_edge_3(void); 
/*--------------------------------------------------------------------------------------
  UTF8 char recoding
 
--------------------------------------------------------------------------------------*/
int utf8_vni_char(char* dest, const unsigned char* src) {

    uint16_t i, j;
    for (i = 0, j = 0; src[i]; i++) {
        if ((src[i] == 0xD0) && src[i + 1]) { dest[j++] = src[++i] - 0x10; }
        else if ((src[i] == 0xD1) && src[i + 1]) { dest[j++] = src[++i] + 0x30; }
        else dest[j++] = src[i];
    }
    dest[j] = '\0';
    return j;
}

/*--------------------------------------------------------------------------------------
  setup
  Called by the Arduino architecture before the main loop begins
--------------------------------------------------------------------------------------*/

void setup(void)
{

    enableDebugPorts(); 
    // initialize DMD objects
	dmd.init(700); 
    
}


/*--------------------------------------------------------------------------------------
  loop
  Arduino architecture main loop
--------------------------------------------------------------------------------------*/
typedef enum {
    EFFECT_TEXT_STATIC, 
    EFFECT_TEXT_SCROLL_UP, 
    EFFECT_MAX, 
}; 

#define EFFECT_STATIC_TIME      200 // 200*30 
#define EFFECT_TIME_STEP        30  // ms
#if 1
#define TOTAL_MSG           4
char *msg[TOTAL_MSG] = 
{
    "AAA BBBB", 
    "CCC DDDD", 
    "EEE FFFF", 
    "GGG HHHH"
};
#else 
#define TOTAL_MSG           4
char *msg[TOTAL_MSG] = 
{
    "CAT CHIA", 
    "SUA KHOA", 
    "LAY NGAY", 
    "0909073358"
};
#endif
uint16_t txt_color = dmd.Color888(255,0, 0); // red; 
void loop(void)
{
    uint16_t bg = 0;  // background - black

    // text

    dmd.selectFont(&Arial_F);
    
    // set text foreground and background colors
    dmd.setTextColor(txt_color, bg);

    // running text shift interval
    uint16_t interval = EFFECT_TIME_STEP;

    long prev_step = millis();
    uint8_t i = 0, b = 0;
    uint16_t eff_state_cnt = EFFECT_STATIC_TIME; 
    uint16_t eff_state = 0; 
    uint8_t msg_cnt = 2; 
    char *m = 0; 
    bool swap_buffer = 0; 
    dmd.setBrightness(200);
    
    dmd.clearScreen(true);
    dmd.swapBuffers(true);
    // Cycle for tests:
    // -- running texts moving at x and y axis with single and double speed
    // -- vertical scrolling message
    prev_step = millis();
    i = 0; 
    eff_state = EFFECT_MAX; 
    msg_cnt = TOTAL_MSG; 
    while (1) {
        if ((millis() - prev_step) > interval) {
            swap_buffer = 0; 
            if (eff_state >= EFFECT_MAX) {
                if(++msg_cnt >= TOTAL_MSG) {
                    msg_cnt = 0;
                }
                eff_state = 0; 
                eff_state_cnt = EFFECT_STATIC_TIME;
                // draw message
                m = msg[msg_cnt]; 
                
                dmd.clearScreen(true);
                dmd.drawString(1, 2, m, dmd.stringWidth(m), txt_color); 
                // dmd.swapBuffers(true);
                swap_buffer = 1; 
            }

            switch (eff_state) {
                // moving text at x axis
            case EFFECT_TEXT_STATIC:
                eff_state_cnt--; 
                if(eff_state_cnt == 0) {
                    eff_state++; 
                    // dmd.clearScreen(true);
                    if(msg_cnt + 1 >= TOTAL_MSG) {
                        m = msg[0]; 
                    }else {
                        m = msg[msg_cnt + 1]; 
                    }
                    dmd.drawMarqueeX(m, 1, dmd.height());
                    // output mem buffer to matrix
                    // dmd.swapBuffers(true);
                    swap_buffer = 1; 
                }
                break;
                // moving text at y axis
            case EFFECT_TEXT_SCROLL_UP:
                if (dmd.stepMarquee(0, -1) & 8) {  // if text is reached screen bounds
                    eff_state++; 
                    eff_state_cnt = EFFECT_STATIC_TIME; 
                }
                // output mem buffer to matrix
                // dmd.swapBuffers(true);
                swap_buffer = 1; 
                break;
            }

            // draw_scrolling_edge(); 
            draw_scrolling_edge_3(); 
            if(swap_buffer) {
                dmd.swapBuffers(true);
            }
            prev_step = millis();

        }
    }
}

static void draw_scrolling_edge(void) {
    static int16_t offset_x = 0; 
    static int16_t offset_y = 0; 

    dmd.drawFastHLine(0, 0, dmd.width(), 0); 
    dmd.drawFastHLine(0, dmd.height() / 2 - 1, dmd.width(), 0); 
    dmd.drawFastVLine(0, 0, dmd.height(), 0); 
    dmd.drawFastVLine(dmd.width() - 1, 0, dmd.height(), 0); 
    dmd.drawPixel(offset_x, offset_y, txt_color); 

    if(offset_y == 0) {
        offset_x++; 
        if(offset_x >= dmd.width()) {
            offset_x = dmd.width() - 1; 
            offset_y++; 
        }
    }else if(offset_x == dmd.width() - 1) {
        offset_y++; 
        if(offset_y >= dmd.height() / 2) {
            offset_y = dmd.height() / 2 - 1; 
            offset_x--; 
        }
    }
    else if((offset_y == dmd.height() / 2 - 1)) {
        offset_x--; 
        if(offset_x < 0) {
            offset_x = 0; 
            offset_y--; 
        }
    }
    else {
        offset_y--; 
        if(offset_y < 0) {
            offset_y = 0; 
        }
    }
}

void conver_pos_offset_to_edge_pos(int16_t pos, int16_t *x, int16_t *y)
{
    // Postion / (width+height)== 0 => Postion / width <= 0 : (x = Postion, y = 0 ? (x = width - 1, y = Postion % width)
    //                         else 1 => Postion = Postion % (width + height); 
    
    const int16_t totalWH = dmd.width() + dmd.height(); 
    int16_t screenWidth = dmd.width(); 
    int16_t screenHeigth = dmd.height(); 
    if(pos / totalWH == 0) {
        if(pos / screenWidth == 0) {
            *x = pos; 
            *y = 0;
        }else {
            *x = screenWidth - 1; 
            *y = pos % screenWidth; 
        }
    }else {
        pos = pos % totalWH; 
        if(pos / screenWidth == 0) {
            *x = screenWidth - pos; 
            *y = screenHeigth - 1;
        }else {
            *x = 0; 
            *y = screenHeigth - pos % screenWidth; 
        }
    }


}

static void draw_scrolling_edge_3(void) {
    static int16_t pos_offset = 0; 

    dmd.drawFastHLine(0, 0, dmd.width(), 0); 
    dmd.drawFastHLine(0, dmd.height() / 2 - 1, dmd.width(), 0); 
    dmd.drawFastVLine(0, 0, dmd.height(), 0); 
    dmd.drawFastVLine(dmd.width() - 1, 0, dmd.height(), 0); 
    int16_t totalWH = dmd.width() + dmd.height(); 
    int16_t offset_x = 0; 
    int16_t offset_y = 0; 
    int16_t tmp_pos = 0; 
    for(int pos = pos_offset - 1; pos >= 0; pos--) {
        conver_pos_offset_to_edge_pos(pos, &offset_x, &offset_y);
        tmp_pos = pos_offset - pos; 
        if(((tmp_pos / 8) & 0x01) == 1) {
            dmd.drawPixel(offset_x, offset_y, txt_color); 
        }else {

        }
    }
    for(int pos = pos_offset; pos < 2* totalWH; pos++) {
        conver_pos_offset_to_edge_pos(pos, &offset_x, &offset_y);
        tmp_pos = pos - pos_offset; 
        if(((tmp_pos / 8) & 0x01) == 0) {
            dmd.drawPixel(offset_x, offset_y, txt_color); 
        }else {

        }
    }
    pos_offset++; 
    if(pos_offset >= 2*totalWH) {
        pos_offset = 0; 
    }
}

#endif // CONFIG_APP_TEST_MODULE
