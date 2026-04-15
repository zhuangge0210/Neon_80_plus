#include <string.h>
#include <stdio.h>
#include "config.h"
#include "wait.h"
#include "debug.h"
#include "timer.h"
#include "spi_master.h"
#include "hal.h"
#include "SEGGER_RTT.h"
#include "led.h"
#include "aw20216.h"
#include "string.h"
#include "ch.h"
#include "chthreads.h"
#include "chvt.h"
#include "led_drv.h"
#include "action.h"
#include "app_flash.h"
#define QMK_LOG(...) SEGGER_RTT_printf(0, __VA_ARGS__)



#define MAX_HEAD_LED_INTERVAL_TIME      200
#define MIN_HEAD_LED_INTERVAL_TIME      0
#define MAX_KEY_LED_INTERVAL_TIME       200
#define MIN_KEY_LED_INTERVAL_TIME       0
#define SIGNALRGB_REFRESH_INTERVAL_MS   4
#define SIGNALRGB_TIMEOUT_MS            2000U


#define LED_CS1			C9
#define LED_CS2			A8

#define LED_CS3			D5
#define LED_CS4			D6
#define LED_CS5			D7
#define LED_CS6			D3

#define LED_CS7			E2

#define LED_CS8			D2
#define LED_CS9			E6


typedef enum custom_keycodes {
    TEST_KEY=0x5DB1,//0x5DB1  not use
    KEY_LED_NEXT,
    KEY_LED_PREV,
    KEY_LED_SPEED_INC,
    KEY_LED_SPEED_DEC,
    KEY_LED_LIGHT_INS,
    KEY_LED_LIGHT_DEC,
	HEAD_LED_NEXT,
	HEAD_LED_PREV,
    HEAD_LED_SPEED_INC = 0x5DBA,
    HEAD_LED_SPEED_DEC,
    HEAD_LED_LIGHT_INS,
    HEAD_LED_LIGHT_DEC,
    HEAD_LED_POWER,
    KEY_LED_POWER,
}custom_keycodes_t;



#define	DCDC_EN			C10
#define DCDC_MODE	    E1

#define LED_EN1         C8
#define LED_EN2         D4
#define LED_EN3         E3





#define AW_PAGE_FUNCTION 0x00 << 1   // PG0, Function registers
#define AW_PAGE_PWM 0x01 << 1        // PG1, LED PWM control
#define AW_PAGE_SCALING 0x02 << 1    // PG2, LED current scaling control
#define AW_PAGE_PATCHOICE 0x03 << 1  // PG3, Pattern choice?
#define AW_PAGE_PWMSCALING 0x04 << 1 // PG4, LED PWM + Scaling control?

#define K_NULL    {5,9}/*KEY NULL*/

static THD_WORKING_AREA(led_thread_area, 2048);
static THD_WORKING_AREA(head_led_thread_area, 4096);
static MUTEX_DECL(myMutex);

static uint16_t sleep_tick=0;


static key_led_mode_t *key_led_play;
static head_led_mode_t *head_led_play;
//virtual_timer_t key_led_scan_timer;

bool lock_led_state_g[3] = {0}; //capslock,scroll_lock_numlock
bool led_switch_g[2]   = {1,1}; //led 开关，[0]:head_led;[1]:key_lad
bool led_sleep[2]      ={0,0};  //led 开关，[0]:head_led;[1]:key_lad
uint8_t g_head_lightness_index = 7 ,g_key_lightness_index = 7;
static volatile bool signalrgb_mode_active = false;
/************************************************led_driver**********************************************************/
/*****************************************************************************************************************/
#define REG_NUM         216

#define AW_PAGE_PWM 0x01 << 1        // PG1, LED PWM control
#define AW_PAGE_SCALING 0x02 << 1    // PG2, LED current scaling control
#define AW_PAGE_PATCHOICE 0x03 << 1  // PG3, Pattern choice?
#define AW_PAGE_PWMSCALING 0x04 << 1 // PG4, LED PWM + Scaling control?

#define RGB_PAGE        AW_PAGE_SCALING
#define LIGHT_PAGE      AW_PAGE_PWM



static uint8_t chip0_data[256]={0};
static uint8_t chip1_data[256]={0};

uint8_t key_led_data[KEY_RGB_LED_Y_NUM][KEY_RGB_LED_X_NUM][3]={0};//key_led显示数据

uint8_t head_lightness = 0;

//需要根据原理图和位号图修改，贼坑
static const rgb_map_t map[KEY_RGB_LED_Y_NUM][KEY_RGB_LED_X_NUM]=
{
	//第一行	            芯片3															芯片4
	{   /*ESC      F1      F2      F3      F4     F5      F6       		F7     F8      F9       F10    F11     F12     USER   print*/
		{0,0,2},{0,0,1},{0,0,0},{0,0,5},{0,5,5},{0,0,4},{0,0,3},	{0,5,3},{1,0,2},{1,0,1},{1,0,0},{1,0,5},{1,5,5},{1,4,5},{1,7,5},
	},
	//第二行
	{ /*~        1       2        3       4      5       6     		7      8      9        O      -      +     bakcspace insert   */
        {0,1,2},{0,1,1},{0,1,0},{0,7,0},{0,1,5},{0,7,5},{0,1,4},	{0,1,3},{0,6,3},{1,1,2},{1,1,1},{1,1,0},{1,1,5},{1,6,5},{1,3,5},
	},
	//第三行
	{/*~  TAB     Q       W        E       R      T       Y       	 	U      I      O         P      {       }       |   	DEL*/
		{0,2,2},{0,2,1},{0,2,0},{0,6,0},{0,2,5},{0,6,5},{0,2,4},	{0,2,3},{1,7,2},{1,2,2},{1,2,1},{1,2,0},{1,6,0},{1,2,5},{1,2,4},
	},
	//第四行 /*{0,3,2}*/
	{/*CAPS{0,3,2} A       S       D       F       T       Y      	   J       K      L      ;        '      ENTER*/
		{0,4,2},{0,3,1},{0,3,0},{0,4,0},{0,3,5},{0,6,4},{0,3,4},	{0,3,3},{1,6,2},{1,3,2},{1,3,1},{1,3,0},{1,5,0},{1,4,4},{1,0,4},
	},
	//第五行
	{/*   SHIFT     Z       X       C       V     */
		{0,5,2},{0,4,1},{0,5,1},{0,5,0},{0,4,5},{0,5,4},{0,4,4},	{0,4,3},{1,5,2},{1,4,2},{1,4,1},{1,4,0},{1,5,4},{1,6,4},{1,1,4},
	},
    //第六行
	{ /*								 NULL    NULL								 NULL*/
		{0,6,2},{0,7,2},{0,7,1},{0,7,4},{1,7,4},{1,7,4},{1,5,3},	{1,6,3},{1,7,3},{1,7,4},{1,6,1},{1,7,1},{1,0,3},{1,7,0},{1,3,4},
	},
};//{1,5,1},{0,6,1}

//真实按键从左到右，从上到下的顺序顺序，一维数组
const rgb_map_t real_map[AM_LED_NUM] =\
{
	//第一行	            芯片3															芯片4
	/*ESC      F1      F2      F3      F4     F5      F6       F7     F8      F9       F10    F11     F12     USER   print   lock    pause*/
	{0,0,2},{0,0,1},{0,0,0},{0,0,5},{0,5,5},{0,0,4},{0,0,3},{0,5,3},{1,0,2},{1,0,1},{1,0,0},{1,0,5},{1,5,5},{1,4,5},{1,7,5},{1,0,4},{1,4,4},\

	//第二行
 	/*~      1       2        3       4      5       6        7        8      9        O      -      +     bakcspace insert  home    pgup*/
 	{0,1,2},{0,1,1},{0,1,0},{0,7,0},{0,1,5},{0,7,5},{0,1,4},{0,1,3},{0,6,3},{1,1,2},{1,1,1},{1,1,0},{1,1,5},{1,6,5},{1,3,5},{1,1,4},{1,6,4},\

	//第三行
	/* TAB     Q       W        E       R      T       Y       	U      I      O         P      {       }       |   	  DEL     end     pgdn*/
	{0,2,2},{0,2,1},{0,2,0},{0,6,0},{0,2,5},{0,6,5},{0,2,4},{0,2,3},{1,7,2},{1,2,2},{1,2,1},{1,2,0},{1,6,0},{1,2,5},{1,2,4},{1,3,4},{1,5,4},\

	//第四行 /*{0,3,2}*/
	/*CAPS{0,3,2} A     S       D       F       T      Y       J       K      L      ;        '      ENTER*/
	{0,4,2},{0,3,1},{0,3,0},{0,4,0},{0,3,5},{0,6,4},{0,3,4},{0,3,3},{1,6,2},{1,3,2},{1,3,1},{1,3,0},{1,5,0},\

	//第五行
	/*SHIFT     Z       X       C       V     B       N       M       <       >       ?     SHIFT     RIGHT*/
	{0,5,2},{0,4,1},{0,5,1},{0,5,0},{0,4,5},{0,5,4},{0,4,4},{0,4,3},{1,5,2},{1,4,2},{1,4,1},{1,4,0},{1,0,3},\

    //第六行
	/*l_ctrl l_code  l_alt  space_l  space  space_r  Fn  code  r_ctrl  left  down  right*/
	{0,6,2},{0,7,2},{0,7,1},{0,6,1},{0,7,4},{1,5,1},{1,6,1},{1,7,1},{1,7,0},{1,7,3},{1,6,3},{1,5,3},

};

const uint8_t keyboard_x_num[6] = {17,17,17,13,13,10};
const uint8_t key_addr[KEY_RGB_LED_Y_NUM][17][2] = \
{/* x,y map coordinates */
    {  /*esc    F1    F2                      ......                                                   lock   pause*/
        {0,0},{0,1},{0,2},{0,3},{0,4},{0,5},{0,6},{0,7},{0,8},{0,9},{0,10},{0,11},{0,12},{0,13},{0,14},{3,14},{3,13},
    },
    {  /*~      1     2                       ......                                                    Home    PGUP*/
        {1,0},{1,1},{1,2},{1,3},{1,4},{1,5},{1,6},{1,7},{1,8},{1,9},{1,10},{1,11},{1,12},{1,13},{1,14},{4,14},{4,13},
    },
    {  /*TAB	Q	  W						  ......													End		PGDN*/
        {2,0},{2,1},{2,2},{2,3},{2,4},{2,5},{2,6},{2,7},{2,8},{2,9},{2,10},{2,11},{2,12},{2,13},{2,14},{5,14},{4,12},
    },
    {  /*CAPLK  A     B                       ......                         "    ENTER   NULL   NULL   NULL   NULL */
        {3,0},{3,1},{3,2},{3,3},{3,4},{3,5},{3,6},{3,7},{3,8},{3,9},{3,10},{3,11},{3,12},K_NULL,K_NULL,K_NULL,K_NULL,
    },
    {/*L_SHFT    Z    X                       ......            ?   R_SHIFT    RIGHT*/
        {4,0},{4,1},{4,2},{4,3},{4,4},{4,5},{4,6},{4,7},{4,8},{4,9},{4,10},{4,11},{5,12},K_NULL,K_NULL,K_NULL,K_NULL,
    },
    {/*ctrl    code   alt                       ......       ↓     ↑ */
        {5,0},{5,1},{5,2},{5,3},{5,10},{5,11},{5,13},{5,8},{5,7},{5,6},K_NULL,K_NULL,K_NULL,K_NULL,K_NULL,K_NULL,K_NULL,
    },
};

const uint8_t keyboard_x_num_2[6] = {17,17,17,17,17,17};

const uint8_t key_addr_2[KEY_RGB_LED_Y_NUM][17][2] = \
{/* x,y map coordinates */
    {  /*esc    F1    F2                      ......                                                   lock   pause*/
        {0,0},{0,1},{0,2},{0,3},{0,4},{0,5},{0,6},{0,7},{0,8},{0,9},{0,10},{0,11},{0,12},{0,13},{0,14},{3,14},{3,13},
    },
    {  /*~      1     2                       ......                                                    Home    PGUP*/
        {1,0},{1,1},{1,2},{1,3},{1,4},{1,5},{1,6},{1,7},{1,8},{1,9},{1,10},{1,11},{1,12},{1,13},{1,14},{4,14},{4,13},
    },
    {  /*TAB	Q	  W						  ......													End		PGDN*/
        {2,0},{2,1},{2,2},{2,3},{2,4},{2,5},{2,6},{2,7},{2,8},{2,9},{2,10},{2,11},{2,12},{2,13},{2,14},{5,14},{4,12},
    },
    {  /*CAPLK  A     B                       ......                         "    ENTER   NULL   NULL   NULL   NULL */
        {3,0},{3,1},{3,2},{3,3},{3,4},{3,5},{3,6},{3,7},{3,8},{3,9},{3,10},{3,11},{3,12},K_NULL,K_NULL,K_NULL,K_NULL,
    },
    {/*L_SHFT    Z    X                       ......            ?   R_SHIFT    RIGHT*/
        {4,0},{4,1},{4,2},{4,3},{4,4},{4,5},{4,6},{4,7},{4,8},{4,9},{4,10},{4,11},K_NULL,K_NULL,K_NULL,{5,12},K_NULL,
    },
    {/*ctrl    code   alt                       ......       ↓     ↑ */
        {5,0},{5,1},{5,2},K_NULL,K_NULL,K_NULL,{5,3},K_NULL,K_NULL,K_NULL,K_NULL,{5,10},{5,11},{5,13},{5,8},{5,7},{5,6},
    },
};



//Set one frame RGB data
void set_led_pwm_brightness(pin_t chip_CS,uint8_t *pwm_data)
{//RGB
    AW20216_write(chip_CS,RGB_PAGE,0,pwm_data,REG_NUM);
}


//Set one frame brightness
void set_per_led_brightness(pin_t chip_CS,uint8_t *bright_data)
{//LIGHT_NESS
    AW20216_write(chip_CS,LIGHT_PAGE,0,bright_data,REG_NUM);
}

void set_chip_led_brightness(pin_t chip_CS,uint8_t brightness)
{
    uint8_t page_data[256]={0};
	memset(page_data,brightness,256);


    if(LED_CS1 == chip_CS)
    {//capslock灯亮度固定
        page_data[(CAPSLOCK_COL)*6*3+3*(CAPSLOCK_ROW)+0]       = CAPSLOCK_LIGHTNESS;//G
        page_data[(CAPSLOCK_COL)*6*3+3*(CAPSLOCK_ROW)+1]       = CAPSLOCK_LIGHTNESS;//R
        page_data[(CAPSLOCK_COL)*6*3+3*(CAPSLOCK_ROW)+2]       = CAPSLOCK_LIGHTNESS;//B
    }
    set_per_led_brightness( chip_CS, page_data);
}
static uint8_t key_state[KEY_RGB_LED_Y_NUM][KEY_RGB_LED_X_NUM];

//key_led
static uint8_t chip0_last_brightness = 0;
static uint8_t chip1_last_brightness = 0;

//head_led
static uint8_t chip2_last_brightness = 0;
static uint8_t chip3_last_brightness = 0;
static uint8_t chip4_last_brightness = 0;
static uint8_t chip5_last_brightness = 0;
static uint8_t chip6_last_brightness = 0;

uint8_t key_board_spi_test(void)
{
    chMtxLock(&myMutex);
    uint8_t reg_value = 0;
    AW20216_read(LED_CS1,AW_PAGE_FUNCTION,0x2f,&reg_value,1);
    chMtxUnlock(&myMutex);
    if(0x70 != reg_value)
    {
        return false;
    }


    chMtxLock(&myMutex);
    AW20216_read(LED_CS1,AW_PAGE_FUNCTION,1,&reg_value,1);
    QMK_LOG("0x01:%02X \n",reg_value);
    if(0 == (reg_value & 0x01))
    {
        reg_value = 0xB1;
        AW20216_write(LED_CS1,AW_PAGE_FUNCTION,0,&reg_value,1);
    }
    AW20216_read(LED_CS2,AW_PAGE_FUNCTION,1,&reg_value,1);
    QMK_LOG("0x01:%02X \n",reg_value);
    if(0 == (reg_value & 0x01))
    {
        reg_value = 0xB1;
        AW20216_write(LED_CS2,AW_PAGE_FUNCTION,0,&reg_value,1);
    }
    chMtxUnlock(&myMutex);
    return true;
}

uint8_t head_board_spi_test(void)
{
    uint8_t reg_value = 0;
    chMtxLock(&myMutex);
    AW20216_read(LED_CS3,AW_PAGE_FUNCTION,0x2f,&reg_value,1);
    chMtxUnlock(&myMutex);
    if(0x70 != reg_value)
    {
        return false;
    }

    chMtxLock(&myMutex);
    AW20216_read(LED_CS3,AW_PAGE_FUNCTION,1,&reg_value,1);
    QMK_LOG("0x01:%02X \n",reg_value);
    if(0 == (reg_value & 0x01))
    {
        reg_value = 0xB1;
        AW20216_write(LED_CS3,AW_PAGE_FUNCTION,0,&reg_value,1);
    }
    AW20216_read(LED_CS4,AW_PAGE_FUNCTION,1,&reg_value,1);
    QMK_LOG("0x01:%02X \n",reg_value);
    if(0 == (reg_value & 0x01))
    {
        reg_value = 0xB1;
        AW20216_write(LED_CS4,AW_PAGE_FUNCTION,0,&reg_value,1);
    }
    AW20216_read(LED_CS5,AW_PAGE_FUNCTION,1,&reg_value,1);
    QMK_LOG("0x01:%02X \n",reg_value);
    if(0 == (reg_value & 0x01))
    {
        reg_value = 0xB1;
        AW20216_write(LED_CS5,AW_PAGE_FUNCTION,0,&reg_value,1);
    }
    AW20216_read(LED_CS6,AW_PAGE_FUNCTION,1,&reg_value,1);
    QMK_LOG("0x01:%02X \n",reg_value);
    if(0 == (reg_value & 0x01))
    {
        reg_value = 0xB1;
        AW20216_write(LED_CS6,AW_PAGE_FUNCTION,0,&reg_value,1);
    }
    AW20216_read(LED_CS7,AW_PAGE_FUNCTION,1,&reg_value,1);
    QMK_LOG("0x01:%02X \n",reg_value);
    if(0 == (reg_value & 0x01))
    {
        reg_value = 0xB1;
        AW20216_write(LED_CS7,AW_PAGE_FUNCTION,0,&reg_value,1);
    }
    chMtxUnlock(&myMutex);
    return true;
}

bool aw20216_lightness_get(pin_t chip_cs,uint8_t * reg_value)
{
    bool ret = false;
    chMtxLock(&myMutex);
    ret = AW20216_read(chip_cs,LIGHT_PAGE,0,reg_value,1);
    chMtxUnlock(&myMutex);
    QMK_LOG("light:%d ret:%d\n",*reg_value,ret);
    return ret;
}

bool aw20216_check_en(pin_t chip_cs)
{
    bool ret = false;
    uint8_t reg_value;
    chMtxLock(&myMutex);
    ret = AW20216_read(chip_cs,AW_PAGE_FUNCTION,0,&reg_value,1);
    chMtxUnlock(&myMutex);
    QMK_LOG("en:%d ret:%d\n",reg_value,ret);
    if(reg_value & (1<<0))
    {
        return true;
    }
    return false;
}

void aw_disp_one_chip_data(uint8_t device_index,uint8_t *pwm_data,uint8_t brightness)
{
	chMtxLock(&myMutex);
    switch(device_index)
    {
        case AW_CHIP_INDEX_0:
            set_led_pwm_brightness(LED_CS1, pwm_data);
            if(chip0_last_brightness != brightness)
            {
                set_chip_led_brightness( LED_CS1, brightness);
                chip0_last_brightness = brightness;
            }
            break;
        case AW_CHIP_INDEX_1:
            set_led_pwm_brightness(LED_CS2, pwm_data);
            if(chip1_last_brightness != brightness)
            {
                set_chip_led_brightness(LED_CS2, brightness);
                chip1_last_brightness = brightness;
            }
            break;
        case AW_CHIP_INDEX_2:
            set_led_pwm_brightness(LED_CS3, pwm_data);
            if(chip2_last_brightness != brightness)
            {
                set_chip_led_brightness(LED_CS3, brightness);
                chip2_last_brightness = brightness;
            }
            break;
        case AW_CHIP_INDEX_3:
            set_led_pwm_brightness(LED_CS4, pwm_data);
            if(chip3_last_brightness != brightness)
            {
                set_chip_led_brightness(LED_CS4, brightness);
                chip3_last_brightness = brightness;
            }
            break;
        case AW_CHIP_INDEX_4:
            set_led_pwm_brightness(LED_CS5, pwm_data);
            if(chip4_last_brightness != brightness)
            {
                set_chip_led_brightness(LED_CS5, brightness);
                chip4_last_brightness = brightness;
            }
            break;
        case AW_CHIP_INDEX_5:
            set_led_pwm_brightness(LED_CS6, pwm_data);
            if(chip5_last_brightness != brightness)
            {
                set_chip_led_brightness(LED_CS6, brightness);
                chip5_last_brightness = brightness;
            }
            break;
        case AW_CHIP_INDEX_6:
            set_led_pwm_brightness(LED_CS7, pwm_data);
            if(chip6_last_brightness != brightness)
            {
                set_chip_led_brightness(LED_CS7, brightness);
                chip6_last_brightness = brightness;
            }
            break;
        default:
            break;
    }
	chMtxUnlock(&myMutex);

}

//调用了原理图按键map排列
static void frame_data_to_chip(uint8_t (*led_data)[KEY_RGB_LED_X_NUM][3])
{

	for(uint8_t i=0;i<KEY_RGB_LED_Y_NUM;i++)
	{
		for(uint8_t j=0;j<KEY_RGB_LED_X_NUM;j++)
		{
			switch (map[i][j].chip_index)
			{
				case AW_CHIP_INDEX_0:
				{
			/*G*/	chip0_data[(map[i][j].x)*6*3+3*map[i][j].y+0]       = led_data[i][j][1];//G   与硬件排布有关系
					chip0_data[(map[i][j].x)*6*3+3*map[i][j].y+1]     = led_data[i][j][0];//B
					chip0_data[(map[i][j].x)*6*3+3*map[i][j].y+2]     = led_data[i][j][2];//R

				}break;

				case AW_CHIP_INDEX_1:
				{
					chip1_data[(map[i][j].x)*6*3+3*map[i][j].y+0]       = led_data[i][j][1];//G
					chip1_data[(map[i][j].x)*6*3+3*map[i][j].y+1]     = led_data[i][j][0];//B
					chip1_data[(map[i][j].x)*6*3+3*map[i][j].y+2]     = led_data[i][j][2];//R
				}break;

			}
		}
	}

}

//调用了真实按键map排列
static void frame_real_data_to_chip(uint8_t (*led_data)[3])
{

	for(uint8_t i=0;i<AM_LED_NUM;i++)
	{

        switch (real_map[i].chip_index)
        {
            case AW_CHIP_INDEX_0:
            {
        /*G*/	chip0_data[(real_map[i].x)*6*3+3*real_map[i].y+0]     = led_data[i][1];//G   与硬件排布有关系
                chip0_data[(real_map[i].x)*6*3+3*real_map[i].y+1]     = led_data[i][0];//B
                chip0_data[(real_map[i].x)*6*3+3*real_map[i].y+2]     = led_data[i][2];//R

            }break;
            case AW_CHIP_INDEX_1:
            {
                chip1_data[(real_map[i].x)*6*3+3*real_map[i].y+0]     = led_data[i][1];//G
                chip1_data[(real_map[i].x)*6*3+3*real_map[i].y+1]     = led_data[i][0];//B
                chip1_data[(real_map[i].x)*6*3+3*real_map[i].y+2]     = led_data[i][2];//R
            }break;

        }

	}

}

void Aw20216_DispUnderKeyFrame_data(uint8_t (*led_data)[KEY_RGB_LED_X_NUM][3],uint8_t lightness_per)
{
	//更新屏亮度

	frame_data_to_chip( led_data);

    chip0_data[(3)*6*3+3*(2)+0]       = chip0_data[(4)*6*3+3*(2)+0];//capslock下两个灯保持一致
    chip0_data[(3)*6*3+3*(2)+1]       = chip0_data[(4)*6*3+3*(2)+1];
    chip0_data[(3)*6*3+3*(2)+2]       = chip0_data[(4)*6*3+3*(2)+2];

	aw_disp_one_chip_data( AW_CHIP_INDEX_0, chip0_data,lightness_per);
	aw_disp_one_chip_data( AW_CHIP_INDEX_1, chip1_data,lightness_per);
}

void Aw20216_DispRealUnderKeyFrame_data(uint8_t (*led_data)[3],uint8_t lightness_per)
{
	//更新屏亮度
	frame_real_data_to_chip( led_data);

    chip0_data[(3)*6*3+3*(2)+0]       = chip0_data[(4)*6*3+3*(2)+0];//capslock下两个灯保持一致
    chip0_data[(3)*6*3+3*(2)+1]       = chip0_data[(4)*6*3+3*(2)+1];
    chip0_data[(3)*6*3+3*(2)+2]       = chip0_data[(4)*6*3+3*(2)+2];

	aw_disp_one_chip_data( AW_CHIP_INDEX_0, chip0_data,lightness_per);
	aw_disp_one_chip_data( AW_CHIP_INDEX_1, chip1_data,lightness_per);
}


void Aw20216_SpaceLeft(uint8_t red,uint8_t green,uint8_t blue)
{//
    chip1_data[(5)*6*3+3*1+0]       = green;//G
    chip1_data[(5)*6*3+3*1+1]       = red;//R
    chip1_data[(5)*6*3+3*1+2]       = blue;//B
}

void Aw20216_SpaceRight(uint8_t red,uint8_t green,uint8_t blue)
{
    chip0_data[(6)*6*3+3*1+0]       = green;//G
    chip0_data[(6)*6*3+3*1+1]       = red;//R
    chip0_data[(6)*6*3+3*1+2]       = blue;//B
}

void Aw20216_CapsLock(uint8_t red,uint8_t green,uint8_t blue)
{

    // uint8_t led_data[3];
    // led_data[0] = green;
    // led_data[1] = red;
    // led_data[2] = blue;
    // uint8_t addr = (3)*6*3+3*(2);
    // chMtxLock(&myMutex);
    // AW20216_write(LED_CS1,RGB_PAGE,addr,led_data,3);
    // addr = (7)*6*3+3*(3);
    // AW20216_write(LED_CS1,RGB_PAGE,addr,led_data,3);
    // chMtxUnlock(&myMutex);


    chip0_data[(7)*6*3+3*(3)+0]       = green;//G
    chip0_data[(7)*6*3+3*(3)+1]       = red;//R
    chip0_data[(7)*6*3+3*(3)+2]       = blue;//B
}

/*****************************************************head_led_driver*************************************************/
/*********************************************************************************************************************/
uint8_t head_led_data[HEAD_RGB_LED_Y][HEAD_RGB_LED_X][3]={0};//led显示数据
static uint8_t chip2_data[256]={0};
static uint8_t chip3_data[256]={0};
static uint8_t chip4_data[256]={0};
static uint8_t chip5_data[256]={0};

//需要根据原理图和位号图修改，贼坑
static const rgb_map_t h_map[HEAD_RGB_LED_Y][HEAD_RGB_LED_X]=
{
	//第一行	           					 芯片3		CS3																					 芯片4		CS4																						芯片5		CS5																					芯片6		CS6
	{
		{2,0,0},{2,1,0},{2,2,0},{2,3,0},{2,4,0},{2,5,0},{2,6,0},{2,7,0},{2,8,0},{2,9,0},{2,10,0},{2,11,0},	{3,0,0},{3,1,0},{3,2,0},{3,3,0},{3,4,0},{3,5,0},{3,6,0},{3,7,0},{3,8,0},{3,9,0},{3,10,0},{3,11,0},  {4,0,0},{4,1,0},{4,2,0},{4,3,0},{4,4,0},{4,5,0},{4,6,0},{4,7,0},{4,8,0},{4,9,0},{4,10,0},{4,11,0},   {5,0,0},{5,1,0},{5,2,0},{5,3,0},{5,4,0},{5,5,0},{5,6,0},{5,7,0},{5,8,0},{5,9,0}
	},
	//第二行
	{
        {2,0,1},{2,1,1},{2,2,1},{2,3,1},{2,4,1},{2,5,1},{2,6,1},{2,7,1},{2,8,1},{2,9,1},{2,10,1},{2,11,1},  {3,0,1},{3,1,1},{3,2,1},{3,3,1},{3,4,1},{3,5,1},{3,6,1},{3,7,1},{3,8,1},{3,9,1},{3,10,1},{3,11,1},  {4,0,1},{4,1,1},{4,2,1},{4,3,1},{4,4,1},{4,5,1},{4,6,1},{4,7,1},{4,8,1},{4,9,1},{4,10,1},{4,11,1},   {5,0,1},{5,1,1},{5,2,1},{5,3,1},{5,4,1},{5,5,1},{5,6,1},{5,7,1},{5,8,1},{5,9,1}
	},
	//第三行
	{
		{2,0,2},{2,1,2},{2,2,2},{2,3,2},{2,4,2},{2,5,2},{2,6,2},{2,7,2},{2,8,2},{2,9,2},{2,10,2},{2,11,2},  {3,0,2},{3,1,2},{3,2,2},{3,3,2},{3,4,2},{3,5,2},{3,6,2},{3,7,2},{3,8,2},{3,9,2},{3,10,2},{3,11,2},  {4,0,2},{4,1,2},{4,2,2},{4,3,2},{4,4,2},{4,5,2},{4,6,2},{4,7,2},{4,8,2},{4,9,2},{4,10,2},{4,11,2},   {5,0,2},{5,1,2},{5,2,2},{5,3,2},{5,4,2},{5,5,2},{5,6,2},{5,7,2},{5,8,2},{5,9,2}
	},
	//第四行
	{
		{2,0,3},{2,1,3},{2,2,3},{2,3,3},{2,4,3},{2,5,3},{2,6,3},{2,7,3},{2,8,3},{2,9,3},{2,10,3},{2,11,3},  {3,0,3},{3,1,3},{3,2,3},{3,3,3},{3,4,3},{3,5,3},{3,6,3},{3,7,3},{3,8,3},{3,9,3},{3,10,3},{3,11,3},  {4,0,3},{4,1,3},{4,2,3},{4,3,3},{4,4,3},{4,5,3},{4,6,3},{4,7,3},{4,8,3},{4,9,3},{4,10,3},{4,11,3},   {5,0,3},{5,1,3},{5,2,3},{5,3,3},{5,4,3},{5,5,3},{5,6,3},{5,7,3},{5,8,3},{5,9,3}
	},
	//第五行
	{/*   SHIFT     Z       X       C       V     */
		{2,0,4},{2,1,4},{2,2,4},{2,3,4},{2,4,4},{2,5,4},{2,6,4},{2,7,4},{2,8,4},{2,9,4},{2,10,4},{2,11,4},  {3,0,4},{3,1,4},{3,2,4},{3,3,4},{3,4,4},{3,5,4},{3,6,4},{3,7,4},{3,8,4},{3,9,4},{3,10,4},{3,11,4},  {4,0,4},{4,1,4},{4,2,4},{4,3,4},{4,4,4},{4,5,4},{4,6,4},{4,7,4},{4,8,4},{4,9,4},{4,10,4},{4,11,4},   {5,0,4},{5,1,4},{5,2,4},{5,3,4},{5,4,4},{5,5,4},{5,6,4},{5,7,4},{5,8,4},{5,9,4}
	},
};



static void head_frame_data_to_chip(uint8_t (*led_data)[HEAD_RGB_LED_X][3])
{

	for(uint8_t i=0;i<HEAD_RGB_LED_Y;i++)
	{
		for(uint8_t j=0;j<HEAD_RGB_LED_X;j++)
		{
			switch (h_map[i][j].chip_index)
			{
				case AW_CHIP_INDEX_2:
				{
					chip2_data[(h_map[i][j].x)*6*3+3*h_map[i][j].y+2]       = led_data[i][j][2];//B   与硬件排布有关系
					chip2_data[(h_map[i][j].x)*6*3+3*h_map[i][j].y+1]     = led_data[i][j][1];//G
					chip2_data[(h_map[i][j].x)*6*3+3*h_map[i][j].y+0]     = led_data[i][j][0];//R

				}break;

				case AW_CHIP_INDEX_3:
				{
					chip3_data[(h_map[i][j].x)*6*3+3*h_map[i][j].y+2]       = led_data[i][j][2];
					chip3_data[(h_map[i][j].x)*6*3+3*h_map[i][j].y+1]     = led_data[i][j][1];
					chip3_data[(h_map[i][j].x)*6*3+3*h_map[i][j].y+0]     = led_data[i][j][0];
				}break;
				case AW_CHIP_INDEX_4:
				{
					chip4_data[(h_map[i][j].x)*6*3+3*h_map[i][j].y+2]       = led_data[i][j][2];
					chip4_data[(h_map[i][j].x)*6*3+3*h_map[i][j].y+1]     = led_data[i][j][1];
					chip4_data[(h_map[i][j].x)*6*3+3*h_map[i][j].y+0]     = led_data[i][j][0];

				}break;

				case AW_CHIP_INDEX_5:
				{
					chip5_data[(h_map[i][j].x)*6*3+3*h_map[i][j].y+2]       = led_data[i][j][2];//B
					chip5_data[(h_map[i][j].x)*6*3+3*h_map[i][j].y+1]     = led_data[i][j][1];//G
					chip5_data[(h_map[i][j].x)*6*3+3*h_map[i][j].y+0]     = led_data[i][j][0];//R
				}break;
			}

		}
	}

}




void Aw20216_DispHeadLedFrame_data(uint8_t (*led_data)[HEAD_RGB_LED_X][3],uint8_t lightness_per)
{
	//更新屏亮度

	head_frame_data_to_chip( led_data);
	aw_disp_one_chip_data( AW_CHIP_INDEX_2, chip2_data,lightness_per);
	aw_disp_one_chip_data( AW_CHIP_INDEX_3, chip3_data,lightness_per);
    aw_disp_one_chip_data( AW_CHIP_INDEX_4, chip4_data,lightness_per);
	aw_disp_one_chip_data( AW_CHIP_INDEX_5, chip5_data,lightness_per);

}

/*****************************************************side_led_driver*************************************************/
/*********************************************************************************************************************/
uint8_t side_led_data[SIDE_LED_NUM][3];
static uint8_t signalrgb_key_led_frames[2][SIGNALRGB_KEY_LED_COUNT][3]         = {0};
static uint8_t signalrgb_head_led_frames[2][HEAD_RGB_LED_Y][HEAD_RGB_LED_X][3] = {0};
static uint8_t signalrgb_side_led_frames[2][SIGNALRGB_SIDE_LED_COUNT][3]       = {0};
static volatile uint8_t signalrgb_active_frame_index                           = 0;
static uint8_t          signalrgb_staging_frame_index                          = 1;
static volatile uint32_t signalrgb_last_packet_ms                              = 0;
static uint8_t chip6_data[256]={0};

//需要根据原理图和位号图修改，贼坑
static const rgb_map_t s_map[SIDE_LED_NUM]=
{
//第一行	           					 芯片3		CS3

    {6,0,3},{6,1,3},{6,2,3},{6,3,3},{6,4,3},{6,5,3},{6,6,3},{6,7,3},{6,8,3},{6,9,3},\

//第二行

    {6,0,2},{6,1,2},{6,2,2},{6,3,2},{6,4,2},{6,5,2},{6,6,2},{6,7,2},{6,8,2},{6,9,2},{6,10,2},{6,0,4},{6,1,4},{6,2,4},{6,3,4},{6,4,4},{6,5,4},{6,11,3},{6,10,3},\

//第三行
    {6,0,1},{6,1,1},{6,2,1},{6,3,1},{6,4,1},{6,5,1},{6,6,1},{6,7,1},{6,8,1},{6,9,1},{6,10,1},{6,11,1},{6,11,2},{6,6,4},{6,7,4},{6,8,4},{6,9,4},{6,10,4},{6,11,4},{6,9,5},{6,8,5},\

//第四行
    {6,0,0},{6,1,0},{6,2,0},{6,3,0},{6,4,0},{6,5,0},{6,6,0},{6,7,0},{6,8,0},{6,9,0},{6,10,0},{6,11,0},{6,0,5},{6,1,5},{6,2,5},{6,3,5},{6,4,5},{6,5,5},{6,6,5},{6,7,5}

};

static void side_frame_data_to_chip(uint8_t (*led_data)[3])
{
	for(uint8_t i=0;i<SIDE_LED_NUM;i++)
	{
        switch (s_map[i].chip_index)
        {
            case AW_CHIP_INDEX_6:
            {
                chip6_data[(s_map[i].x)*6*3+3*s_map[i].y+2]     = led_data[i][2];//B   与硬件排布有关系
                chip6_data[(s_map[i].x)*6*3+3*s_map[i].y+1]     = led_data[i][1];//G
                chip6_data[(s_map[i].x)*6*3+3*s_map[i].y+0]     = led_data[i][0];//R

                break;
            }
            default:
                break;
        }
	}

}

void Aw20216_DispSideLedFrame_data(uint8_t (*led_data)[3],uint8_t lightness_per)
{
	//更新屏亮度
	side_frame_data_to_chip( led_data);

	aw_disp_one_chip_data( AW_CHIP_INDEX_6, chip6_data,lightness_per);
}

static inline void signalrgb_mark_packet_received(void)
{
    signalrgb_last_packet_ms = timer_read32();
}

static void signalrgb_check_timeout(void)
{
    if (!signalrgb_mode_active) {
        return;
    }

    const uint32_t now = timer_read32();
    if ((uint32_t)(now - signalrgb_last_packet_ms) >= SIGNALRGB_TIMEOUT_MS) {
        signalrgb_mode_active = false;
    }
}

void signalrgb_mode_set(bool enabled)
{
    signalrgb_mode_active = enabled;
    if (enabled) {
        memset(signalrgb_key_led_frames, 0, sizeof(signalrgb_key_led_frames));
        memset(signalrgb_head_led_frames, 0, sizeof(signalrgb_head_led_frames));
        memset(signalrgb_side_led_frames, 0, sizeof(signalrgb_side_led_frames));
        signalrgb_active_frame_index  = 0;
        signalrgb_staging_frame_index = 1;
        signalrgb_mark_packet_received();
    }
}

bool signalrgb_mode_enabled(void)
{
    signalrgb_check_timeout();
    return signalrgb_mode_active;
}

uint16_t signalrgb_total_led_count(void)
{
    return SIGNALRGB_TOTAL_LED_COUNT;
}

bool signalrgb_apply_zone_stream(signalrgb_zone_t zone, uint8_t start, uint8_t count, const uint8_t *rgb_data)
{
    const uint8_t staging_index = signalrgb_staging_frame_index;

    if (rgb_data == NULL) {
        return false;
    }

    for (uint8_t i = 0; i < count; i++) {
        const uint8_t offset = i * 2;
        const uint16_t packed_rgb565 = ((uint16_t)rgb_data[offset + 0] << 8) | rgb_data[offset + 1];
        const uint8_t red   = ((packed_rgb565 >> 11) & 0x1F);
        const uint8_t green = ((packed_rgb565 >> 5) & 0x3F);
        const uint8_t blue  = (packed_rgb565 & 0x1F);
        const uint8_t red8   = (red << 3) | (red >> 2);
        const uint8_t green8 = (green << 2) | (green >> 4);
        const uint8_t blue8  = (blue << 3) | (blue >> 2);

        switch (zone) {
            case SIGNALRGB_ZONE_KEY:
                if ((uint16_t)start + i >= SIGNALRGB_KEY_LED_COUNT) {
                    return false;
                }
                signalrgb_key_led_frames[staging_index][start + i][0] = red8;
                signalrgb_key_led_frames[staging_index][start + i][1] = green8;
                signalrgb_key_led_frames[staging_index][start + i][2] = blue8;
                break;

            case SIGNALRGB_ZONE_HEAD: {
                const uint16_t led_index = (uint16_t)start + i;
                if (led_index >= SIGNALRGB_HEAD_LED_COUNT) {
                    return false;
                }

                const uint8_t y = led_index / HEAD_RGB_LED_X;
                const uint8_t x = led_index % HEAD_RGB_LED_X;
                signalrgb_head_led_frames[staging_index][y][x][0] = red8;
                signalrgb_head_led_frames[staging_index][y][x][1] = green8;
                signalrgb_head_led_frames[staging_index][y][x][2] = blue8;
                break;
            }

            case SIGNALRGB_ZONE_SIDE:
                if ((uint16_t)start + i >= SIGNALRGB_SIDE_LED_COUNT) {
                    return false;
                }
                signalrgb_side_led_frames[staging_index][start + i][0] = red8;
                signalrgb_side_led_frames[staging_index][start + i][1] = green8;
                signalrgb_side_led_frames[staging_index][start + i][2] = blue8;
                break;

            default:
                return false;
        }
    }

    signalrgb_mark_packet_received();
    return true;
}

void signalrgb_commit_frame(void)
{
    const uint8_t committed_index = signalrgb_staging_frame_index;
    const uint8_t next_staging    = committed_index ^ 1;

    signalrgb_active_frame_index  = committed_index;
    signalrgb_staging_frame_index = next_staging;

    memcpy(signalrgb_key_led_frames[next_staging], signalrgb_key_led_frames[committed_index], sizeof(signalrgb_key_led_frames[0]));
    memcpy(signalrgb_head_led_frames[next_staging], signalrgb_head_led_frames[committed_index], sizeof(signalrgb_head_led_frames[0]));
    memcpy(signalrgb_side_led_frames[next_staging], signalrgb_side_led_frames[committed_index], sizeof(signalrgb_side_led_frames[0]));
    signalrgb_mark_packet_received();
}


/*****************************************************led_app*********************************************************/
/*********************************************************************************************************************/
#include "ui_data.h"

static const uint8_t gamma_brightness[] = {
    0, 1, 2, 3, 4, 5, 6, 7,
    8, 10, 12, 14, 16, 18, 20, 22,
    24, 26, 29, 32, 35, 38, 41, 44,
    47, 50, 53, 57, 61, 65, 69, 73,
    77, 81, 85, 89, 94, 99, 104, 109,
    114, 119, 124, 129, 134, 140, 146, 152,
    158, 164, 170, 176, 182, 188, 195, 202,
    209, 216, 223, 230, 237, 244, 251, 255,

    255, 251, 244, 237, 230, 223, 216, 209,
    202, 195, 188, 182, 176, 170, 164, 158,
    152, 146, 140, 134, 129, 124, 119, 114,
    109, 104, 99, 94, 89, 85, 81, 77,
    73, 69, 65, 61, 57, 53, 50, 47,
    44, 41, 38, 35, 32, 29, 26, 24,
    22, 20, 18, 16, 14, 12, 10, 8,
    7, 6, 5, 4, 3, 2, 1, 0,
};

static uint8_t brightness_num1 = sizeof(gamma_brightness) / 2;
static uint8_t brightness_num2 = sizeof(gamma_brightness);
static uint16_t color_num = sizeof(color_table) / sizeof(color_table[0]);




const uint8_t key_led_lightness_code[] = {0,5,10,15,25,34,45,75,125,195,0xff};
key_record_t key_press_count[KEY_RGB_LED_Y_NUM][KEY_RGB_LED_X_NUM];
key_record_t key_release_count[KEY_RGB_LED_Y_NUM][KEY_RGB_LED_X_NUM];


/*key led*/
void Mosaic_flow_prase(mode_para_t *mode_para,uint8_t (*data)[KEY_RGB_LED_X_NUM][3],key_record_t (*press_time)[KEY_RGB_LED_X_NUM],key_record_t (*release_time)[KEY_RGB_LED_X_NUM]);
void BlueBreath_mode_prase(mode_para_t *mode_para,uint8_t (*data)[KEY_RGB_LED_X_NUM][3],key_record_t (*press_time)[KEY_RGB_LED_X_NUM],key_record_t (*release_time)[KEY_RGB_LED_X_NUM]);
void ColorTrans_mode_prase(mode_para_t *mode_para,uint8_t (*data)[KEY_RGB_LED_X_NUM][3],key_record_t (*press_time)[KEY_RGB_LED_X_NUM],key_record_t (*release_time)[KEY_RGB_LED_X_NUM]);
void CenterSpread_mode_prase(mode_para_t *mode_para,uint8_t (*data)[KEY_RGB_LED_X_NUM][3],key_record_t (*press_time)[KEY_RGB_LED_X_NUM],key_record_t (*release_time)[KEY_RGB_LED_X_NUM]);
void RowTrans_mode_prase(mode_para_t *mode_para,uint8_t (*data)[KEY_RGB_LED_X_NUM][3],key_record_t (*press_time)[KEY_RGB_LED_X_NUM],key_record_t (*release_time)[KEY_RGB_LED_X_NUM]);
void KeyRowFlash_mode_prase(mode_para_t *mode_para,uint8_t (*data)[KEY_RGB_LED_X_NUM][3],key_record_t (*press_time)[KEY_RGB_LED_X_NUM],key_record_t (*release_time)[KEY_RGB_LED_X_NUM]);
void KeyColumnPush_mode_prase(mode_para_t *mode_para,uint8_t (*data)[KEY_RGB_LED_X_NUM][3],key_record_t (*press_time)[KEY_RGB_LED_X_NUM],key_record_t (*release_time)[KEY_RGB_LED_X_NUM]);
void KeyFlashSlake_mode_prase(mode_para_t *mode_para,uint8_t (*data)[KEY_RGB_LED_X_NUM][3],key_record_t (*press_time)[KEY_RGB_LED_X_NUM],key_record_t (*release_time)[KEY_RGB_LED_X_NUM]);
void All_white_mode_prase(mode_para_t *mode_para,uint8_t (*data)[KEY_RGB_LED_X_NUM][3],key_record_t (*press_time)[KEY_RGB_LED_X_NUM],key_record_t (*release_time)[KEY_RGB_LED_X_NUM]);
void All_red_mode_prase(mode_para_t *mode_para,uint8_t (*data)[KEY_RGB_LED_X_NUM][3],key_record_t (*press_time)[KEY_RGB_LED_X_NUM],key_record_t (*release_time)[KEY_RGB_LED_X_NUM]);
void All_green_mode_prase(mode_para_t *mode_para,uint8_t (*data)[KEY_RGB_LED_X_NUM][3],key_record_t (*press_time)[KEY_RGB_LED_X_NUM],key_record_t (*release_time)[KEY_RGB_LED_X_NUM]);
void All_blue_mode_prase(mode_para_t *mode_para,uint8_t (*data)[KEY_RGB_LED_X_NUM][3],key_record_t (*press_time)[KEY_RGB_LED_X_NUM],key_record_t (*release_time)[KEY_RGB_LED_X_NUM]);
void User_mode1_prase(mode_para_t *mode_para,uint8_t (*data)[KEY_RGB_LED_X_NUM][3],key_record_t (*press_time)[KEY_RGB_LED_X_NUM],key_record_t (*release_time)[KEY_RGB_LED_X_NUM]);
void User_mode2_prase(mode_para_t *mode_para,uint8_t (*data)[KEY_RGB_LED_X_NUM][3],key_record_t (*press_time)[KEY_RGB_LED_X_NUM],key_record_t (*release_time)[KEY_RGB_LED_X_NUM]);
void User_mode3_prase(mode_para_t *mode_para,uint8_t (*data)[KEY_RGB_LED_X_NUM][3],key_record_t (*press_time)[KEY_RGB_LED_X_NUM],key_record_t (*release_time)[KEY_RGB_LED_X_NUM]);





/*head led*/
void Head_Mosaic_flow_prase(mode_para_t *mode_para,uint8_t (*data)[HEAD_RGB_LED_X][3],key_record_t (*press_time)[KEY_RGB_LED_X_NUM],key_record_t (*release_time)[KEY_RGB_LED_X_NUM]);
void Head_mode1_prase(mode_para_t *mode_para,uint8_t (*data)[HEAD_RGB_LED_X][3],key_record_t (*press_time)[KEY_RGB_LED_X_NUM],key_record_t (*release_time)[KEY_RGB_LED_X_NUM]);
void Head_mode2_prase(mode_para_t *mode_para,uint8_t (*data)[HEAD_RGB_LED_X][3],key_record_t (*press_time)[KEY_RGB_LED_X_NUM],key_record_t (*release_time)[KEY_RGB_LED_X_NUM]);
void Head_mode3_prase(mode_para_t *mode_para,uint8_t (*data)[HEAD_RGB_LED_X][3],key_record_t (*press_time)[KEY_RGB_LED_X_NUM],key_record_t (*release_time)[KEY_RGB_LED_X_NUM]);

void Head_all_white_prase(mode_para_t *mode_para,uint8_t (*data)[HEAD_RGB_LED_X][3],key_record_t (*press_time)[KEY_RGB_LED_X_NUM],key_record_t (*release_time)[KEY_RGB_LED_X_NUM]);



//0.0 马赛克流动
static key_led_mode_t mosaic_flow_mode=
{
	.mode_para =
	{
		.time_count=0
	},
	.pause=0,
	.scan_base_ms = 10,
	.scan_step_ms = 7,
	.lightness_index = 3,
	.delay_ms=0,
	.action=NULL,
	.key_event=false,
    .speed_en=true,
    .color_en=false,
	.next_mode = NULL,
	.key_led_prase = Mosaic_flow_prase,
};




//1.蓝灯呼吸
static key_led_mode_t BlueBreath_mode=
{
	.mode_para =
	{
		.time_count=0
	},
	.pause=0,
	.scan_base_ms = 10,
	.scan_step_ms = 7,
	.lightness_index = 3,
	.delay_ms=0,
	.action=NULL,
	.key_event=false,
    .speed_en=true,
    .color_en=true,
	.next_mode = NULL,
	.key_led_prase = BlueBreath_mode_prase,
};

//2.整屏颜色渐变
static key_led_mode_t ColorTrans_mode=
{
	.mode_para =
	{
		.time_count=0
	},
	.pause=0,
    .scan_base_ms = 10,
	.scan_step_ms = 7,
	.lightness_index = 3,
	.delay_ms=0,
	.action=NULL,
	.key_event=false,
    .speed_en=true,
    .color_en=false,
	.next_mode = NULL,
	.key_led_prase = ColorTrans_mode_prase,
};

//3.中心扩散
static key_led_mode_t CenterSpread_mode=
{
	.mode_para =
	{
		.time_count=0
	},
	.pause=0,
	.scan_base_ms = 10,
	.scan_step_ms = 5,
	.lightness_index = 3,
	.delay_ms=0,
	.action=NULL,
	.key_event=false,
    .speed_en=true,
    .color_en=false,
	.next_mode = NULL,
	.key_led_prase = CenterSpread_mode_prase,
};


//4.逐行渐变
static key_led_mode_t RowTrans_mode=
{
	.mode_para =
	{
		.time_count=0
	},
	.pause=0,
	.scan_base_ms = 10,
	.scan_step_ms = 7,
	.lightness_index = 3,
	.delay_ms=0,
	.key_event=false,
    .speed_en=true,
    .color_en=false,
	.next_mode = NULL,
	.key_led_prase = RowTrans_mode_prase,
};

////5.流光
//static key_led_mode_t Streamer_mode=
//{
//	.mode_para =
//	{
//		.time_count=0
//	},
//	.scan_base_ms = 2,
//	.scan_step_ms = 1,
//	.speed=95,
//	.delay_ms=0,
//	.action=NULL,
//	.key_event=false,
//    .speed_en=true,
//    .color_en=false,
//	.next_mode = NULL,
//	.key_led_prase = Streamer_mode_prase,
////};

//6.按键一行闪熄
static key_led_mode_t KeyRowFlash_mode=
{
	.mode_para =
	{
		.time_count=0
	},
	.pause=0,
	.scan_base_ms =10,
	.scan_step_ms = 7,
	.lightness_index = 7,
	.delay_ms=0,
	.action=NULL,
	.key_event=true,
    .speed_en=false,
    .color_en=false,
	.next_mode = NULL,
	.key_led_prase = KeyRowFlash_mode_prase,
};

//7.按键侧推
// static key_led_mode_t KeyColumnPush_mode=
// {
// 	.mode_para =
// 	{
// 		.time_count=0
// 	},
// 	.pause=0,
// 	.scan_base_ms = 10,
// 	.scan_step_ms = 10,
// 	.lightness_index = 7,
// 	.delay_ms=0,
// 	.key_event=true,
//     .speed_en=false,
//     .color_en=false,
// 	.next_mode = NULL,
// 	.key_led_prase = KeyColumnPush_mode_prase,
// };

//8.全白
// static key_led_mode_t All_white_mode=
// {
// 	.mode_para =
// 	{
// 		.time_count=0
// 	},
// 	.pause=0,
// 	.scan_base_ms = 10,
// 	.scan_step_ms = 7,
// 	.lightness_index = 7,
// 	.delay_ms=0,
// 	.action=NULL,
// 	.key_event=false,
//     .color_en=false,
// 	.next_mode = NULL,
// 	.key_led_prase = All_white_mode_prase,
// };

// 9.按键亮熄，亮一下，逐步熄灭
static key_led_mode_t KeyFlashSlake_mode=
{
	.mode_para =
	{
		.time_count=0
	},
	.pause=0,
	.scan_base_ms = 10,
	.scan_step_ms = 7,
	.lightness_index = 7,
	.delay_ms=0,
	.action=NULL,
	.key_event=true,
    .speed_en=false,
    .color_en=false,
	.next_mode = NULL,
	.key_led_prase = KeyFlashSlake_mode_prase,
};

//10.全红
// static key_led_mode_t All_red_mode=
// {
// 	.mode_para =
// 	{
// 		.time_count=0
// 	},
// 	.pause=0,
// 	.scan_base_ms = 10,
// 	.scan_step_ms = 7,
// 	.lightness_index = 7,
// 	.delay_ms=0,
// 	.action=NULL,
// 	.key_event=false,
//     .color_en=false,
// 	.next_mode = NULL,
// 	.key_led_prase = All_red_mode_prase,
// };

// //11.全绿
// static key_led_mode_t All_green_mode=
// {
// 	.mode_para =
// 	{
// 		.time_count=0
// 	},
// 	.pause=0,
// 	.scan_base_ms = 10,
// 	.scan_step_ms = 7,
// 	.lightness_index = 7,
// 	.delay_ms=0,
// 	.action=NULL,
// 	.key_event=false,
//     .color_en=false,
// 	.next_mode = NULL,
// 	.key_led_prase = All_green_mode_prase,
// };

// //12.全蓝
// static key_led_mode_t All_blue_mode=
// {
// 	.mode_para =
// 	{
// 		.time_count=0
// 	},
// 	.pause=0,
// 	.scan_base_ms = 10,
// 	.scan_step_ms = 7,
// 	.lightness_index = 7,
// 	.delay_ms=0,
// 	.action=NULL,
// 	.key_event=false,
//     .color_en=false,
// 	.next_mode = NULL,
// 	.key_led_prase = All_blue_mode_prase,
// };

//12.自定义1
static key_led_mode_t key_mode_1=
{
	.mode_para =
	{
		.time_count=0
	},
	.pause=0,
	.scan_base_ms = 10,
	.scan_step_ms = 7,
	.lightness_index = 3,
	.delay_ms=0,
	.action=NULL,
	.key_event=false,
    .color_en=false,
	.next_mode = NULL,
	.key_led_prase = User_mode1_prase,
};

//12.自定义1
static key_led_mode_t key_mode_2=
{
	.mode_para =
	{
		.time_count=0
	},
	.pause=0,
	.scan_base_ms = 10,
	.scan_step_ms = 7,
	.lightness_index = 3,
	.delay_ms=0,
	.action=NULL,
	.key_event=false,
    .color_en=false,
	.next_mode = NULL,
	.key_led_prase = User_mode2_prase,
};

//12.自定义1
static key_led_mode_t key_mode_3=
{
	.mode_para =
	{
		.time_count=0
	},
	.pause=0,
	.scan_base_ms = 10,
	.scan_step_ms = 7,
	.lightness_index = 3,
	.delay_ms=0,
	.action=NULL,
	.key_event=false,
    .color_en=false,
	.next_mode = NULL,
	.key_led_prase = User_mode3_prase,
};
static head_led_mode_t head_mosaic_flow_mode=
{
	.mode_para =
	{
		.time_count=0
	},
	.scan_base_ms = 10,
	.scan_step_ms = 7,
	.lightness_index = 3,
	.action=NULL,
    .color_en=false,
	.next_mode = NULL,
	.head_led_prase = Head_Mosaic_flow_prase,
};

static head_led_mode_t head_mode_1=
{
	.mode_para =
	{
		.time_count=0
	},
	.scan_base_ms = 10,
	.scan_step_ms = 7,
	.lightness_index = 3,
	.action=NULL,
    .color_en=false,
	.next_mode = NULL,
	.head_led_prase = Head_mode1_prase,
};

static head_led_mode_t head_mode_2=
{
	.mode_para =
	{
		.time_count=0
	},
	.scan_base_ms = 10,
	.scan_step_ms = 7,
	.lightness_index = 3,
	.action=NULL,
    .color_en=false,
	.next_mode = NULL,
	.head_led_prase = Head_mode2_prase,
};

static head_led_mode_t head_mode_3=
{
	.mode_para =
	{
		.time_count=0
	},
	.scan_base_ms = 10,
	.scan_step_ms = 7,
	.lightness_index = 3,
	.action=NULL,
    .color_en=false,
	.next_mode = NULL,
	.head_led_prase = Head_mode3_prase,
};

// static head_led_mode_t head_all_white=
// {
// 	.mode_para =
// 	{
// 		.time_count=0
// 	},
// 	.scan_base_ms = 10,
// 	.scan_step_ms = 200,
// 	.lightness_index = 7,
// 	.action=NULL,
//     .color_en=false,
// 	.next_mode = NULL,
// 	.head_led_prase = Head_all_white_prase,
// };




static uint8_t index_table[KEY_RGB_LED_Y_NUM][KEY_RGB_LED_X_NUM];

void Mosaic_flow_prase(mode_para_t *mode_para,uint8_t (*data)[KEY_RGB_LED_X_NUM][3],key_record_t (*press_time)[KEY_RGB_LED_X_NUM],key_record_t (*release_time)[KEY_RGB_LED_X_NUM])
{


	static uint8_t index=0;
	if(!index)
	{
		memcpy(index_table,random_table1,sizeof(index_table));

		index = 1;
	}

	for(uint8_t i=0;i<KEY_RGB_LED_X_NUM;i++)
	{
		for(uint8_t j=0;j<KEY_RGB_LED_Y_NUM;j++)
		{
			if(index_table[j][i]>=sizeof(color_table)/sizeof(color_table[0]))
			{
				index_table[j][i] -= sizeof(color_table)/sizeof(color_table[0]);
			}
			memcpy(&data[j][i][0],&color_table[index_table[j][i]],3);
			index_table[j][i]++;
		}
	}

	Aw20216_SpaceLeft(data[0][1][0],data[0][1][1],data[0][1][2]);
	Aw20216_SpaceRight(data[3][7][0],data[3][7][1],data[3][7][2]);


}


//1.蓝灯呼吸
void BlueBreath_mode_prase(mode_para_t *mode_para,uint8_t (*data)[KEY_RGB_LED_X_NUM][3],key_record_t (*press_time)[KEY_RGB_LED_X_NUM],key_record_t (*release_time)[KEY_RGB_LED_X_NUM])
{

	for(uint8_t i=0;i<KEY_RGB_LED_Y_NUM;i++)
	{
		for(uint8_t j=0;j<KEY_RGB_LED_X_NUM;j++)
		{
			data[i][j][0]=0;
			data[i][j][1]=0;
		  data[i][j][2]=gamma_brightness[mode_para->time_count%brightness_num2];
		}
	}

	Aw20216_SpaceLeft(data[0][1][0],data[0][1][1],data[0][1][2]);
	Aw20216_SpaceRight(data[3][7][0],data[3][7][1],data[3][7][2]);

//    if(0 == (mode_para->time_count%2))
//    {
        //随机选取4个灯的位置
//        set_lamp_Led_1(&data[0][0][0]);
//        set_lamp_Led_2(&data[0][0][0]);
//        set_lamp_Led_3(&data[0][0][0]);
//        set_lamp_Led_4(&data[0][0][0]);
//    }
}

//2.整屏颜色渐变
void ColorTrans_mode_prase(mode_para_t *mode_para,uint8_t (*data)[KEY_RGB_LED_X_NUM][3],key_record_t (*press_time)[KEY_RGB_LED_X_NUM],key_record_t (*release_time)[KEY_RGB_LED_X_NUM])
{

	uint8_t color_offset=0;
	for(uint8_t i=0;i<KEY_RGB_LED_Y_NUM;i++)
	{
		for(uint8_t j=0;j<KEY_RGB_LED_X_NUM;j++)
		{
			color_offset = mode_para->time_count%color_num;

			data[i][j][0]=color_table[color_offset][0];
			data[i][j][1]=color_table[color_offset][1];
		  	data[i][j][2]=color_table[color_offset][2];
		}
	}

	Aw20216_SpaceLeft(data[0][1][0],data[0][1][1],data[0][1][2]);
	Aw20216_SpaceRight(data[3][7][0],data[3][7][1],data[3][7][2]);

}

//3.中心扩散

static const uint8_t center_offset[KEY_RGB_LED_Y_NUM][KEY_RGB_LED_X_NUM]=
{
	{9,8,7,6,5,4,3,2 ,3,4,5,6,7,8,9},
	{8,7,6,5,4,3,2,1 ,2,3,4,5,6,7,9},
	{7,6,5,4,3,2,1,0 ,1,2,3,4,5,6,9},
	{7,6,5,4,3,2,1,0 ,1,2,3,4,5,6,9},
	{8,7,6,5,4,3,2,1 ,2,3,4,5,6,7,9},
	{9,8,7,6,5,4,3,2 ,3,4,5,6,7,8,9}
};

void CenterSpread_mode_prase(mode_para_t *mode_para,uint8_t (*data)[KEY_RGB_LED_X_NUM][3],key_record_t (*press_time)[KEY_RGB_LED_X_NUM],key_record_t (*release_time)[KEY_RGB_LED_X_NUM])
{
  uint16_t color_offset = 0;//中心点在颜色表的偏移


	uint8_t color_index=0;



	uint8_t r_offset = 20; //不同半径层之间的颜色偏移值，调试确定值

	color_offset = mode_para->time_count%(color_num);//中心两点的颜色



	for(uint8_t i=0;i<KEY_RGB_LED_X_NUM;i++)
	{
		for(uint8_t j=0;j<KEY_RGB_LED_Y_NUM;j++)
		{

			color_index =r_offset*center_offset[j][i];//起始时距离中心点的偏移量

			if(color_offset>=color_index)
			{
				color_index = color_offset - color_index;
			}
			else
			{
			  color_index = (color_num)-(color_index-color_offset);
			}
			data[j][i][0] = color_table[color_index][0];
			data[j][i][1] = color_table[color_index][1];
			data[j][i][2] = color_table[color_index][2];
		}
	}

    Aw20216_SpaceLeft(data[0][7][0],data[0][7][1],data[0][7][2]);
	Aw20216_SpaceRight(data[0][7][0],data[0][7][1],data[0][7][2]);

}

//4.逐行渐变
/*ALICE键值重新定义的坐标*/\



void RowTrans_mode_prase(mode_para_t *mode_para,uint8_t (*data)[KEY_RGB_LED_X_NUM][3],key_record_t (*press_time)[KEY_RGB_LED_X_NUM],key_record_t (*release_time)[KEY_RGB_LED_X_NUM])
{
	uint8_t color_offset=0;
	uint8_t color_interval =30;


	for(uint8_t i=0;i<KEY_RGB_LED_Y_NUM;i++)
	{
		for(uint8_t j=0;j<keyboard_x_num[i];j++)
		{

			color_offset = mode_para->time_count%(color_num);


			color_offset = (color_offset+i*color_interval)%(color_num);


			uint8_t new_y = key_addr[i][j][0];
            uint8_t new_x = key_addr[i][j][1];

            data[new_y][new_x][0]=color_table[color_offset][0];
			data[new_y][new_x][1]=color_table[color_offset][1];
            data[new_y][new_x][2]=color_table[color_offset][2];

//			data[i][j][0]=color_table[color_offset][0];
//			data[i][j][1]=color_table[color_offset][1];
//            data[i][j][2]=color_table[color_offset][2];
		}


	}

}


void All_white_mode_prase(mode_para_t *mode_para,uint8_t (*data)[KEY_RGB_LED_X_NUM][3],key_record_t (*press_time)[KEY_RGB_LED_X_NUM],key_record_t (*release_time)[KEY_RGB_LED_X_NUM])
{

	memset(data,0xff,sizeof(key_led_data));
	uint8_t chip_data[256];
	memset(chip_data,0,sizeof(chip_data));
	for(uint16_t i=0;i<256;i++)
	{
//		if((i%3) == 0)//blue
		{
			chip_data[i] = 0xff;
		}
	}
	 Aw20216_SpaceLeft(255,255,255);
	Aw20216_SpaceRight(255,255,255);
	// aw_disp_one_chip_data( AW_CHIP_INDEX_2, chip_data,75);
	// aw_disp_one_chip_data( AW_CHIP_INDEX_3, chip_data,75);
	// aw_disp_one_chip_data( AW_CHIP_INDEX_4, chip_data,75);
	// aw_disp_one_chip_data( AW_CHIP_INDEX_5, chip_data,75);
	aw_disp_one_chip_data( AW_CHIP_INDEX_6, chip_data,75);
}

void All_red_mode_prase(mode_para_t *mode_para,uint8_t (*data)[KEY_RGB_LED_X_NUM][3],key_record_t (*press_time)[KEY_RGB_LED_X_NUM],key_record_t (*release_time)[KEY_RGB_LED_X_NUM])
{
	for(uint8_t i=0;i<KEY_RGB_LED_X_NUM;i++)
	{
		for(uint8_t j=0;j<KEY_RGB_LED_Y_NUM;j++)
		{
			data[j][i][0] = 0x0;
			data[j][i][1] = 0xff;//键盘红
			data[j][i][2] = 0;
		}
	}
	uint8_t chip_data[256];
	memset(chip_data,0,sizeof(chip_data));
	for(uint16_t i=0;i<256;i++)
	{
		if((i%3) == 0)//blue
		{
			chip_data[i] = 0xff;
		}

	}
	Aw20216_SpaceLeft(255,0,0);
	Aw20216_SpaceRight(255,0,0);

	// aw_disp_one_chip_data( AW_CHIP_INDEX_2, chip_data,75);
	// aw_disp_one_chip_data( AW_CHIP_INDEX_3, chip_data,75);
	// aw_disp_one_chip_data( AW_CHIP_INDEX_4, chip_data,75);
	// aw_disp_one_chip_data( AW_CHIP_INDEX_5, chip_data,75);
	aw_disp_one_chip_data( AW_CHIP_INDEX_6, chip_data,75);
}

void All_green_mode_prase(mode_para_t *mode_para,uint8_t (*data)[KEY_RGB_LED_X_NUM][3],key_record_t (*press_time)[KEY_RGB_LED_X_NUM],key_record_t (*release_time)[KEY_RGB_LED_X_NUM])
{
	for(uint8_t i=0;i<KEY_RGB_LED_X_NUM;i++)
	{
		for(uint8_t j=0;j<KEY_RGB_LED_Y_NUM;j++)
		{


			data[j][i][0] = 0xff;
			data[j][i][1] = 0;
			data[j][i][2] = 0;
		}
	}

		uint8_t chip_data[256];
	memset(chip_data,0,sizeof(chip_data));
	for(uint16_t i=0;i<256;i++)
	{
		if((i%3) == 1)//green
		{
			chip_data[i] = 0xff;
		}

	}
	 Aw20216_SpaceLeft(0,255,0);
	Aw20216_SpaceRight(0,255,0);
	// aw_disp_one_chip_data( AW_CHIP_INDEX_2, chip_data,75);
	// aw_disp_one_chip_data( AW_CHIP_INDEX_3, chip_data,75);
	// aw_disp_one_chip_data( AW_CHIP_INDEX_4, chip_data,75);
	// aw_disp_one_chip_data( AW_CHIP_INDEX_5, chip_data,75);
	// aw_disp_one_chip_data( AW_CHIP_INDEX_6, chip_data,75);
}

void All_blue_mode_prase(mode_para_t *mode_para,uint8_t (*data)[KEY_RGB_LED_X_NUM][3],key_record_t (*press_time)[KEY_RGB_LED_X_NUM],key_record_t (*release_time)[KEY_RGB_LED_X_NUM])
{

	for(uint8_t i=0;i<KEY_RGB_LED_X_NUM;i++)
	{
		for(uint8_t j=0;j<KEY_RGB_LED_Y_NUM;j++)
		{


			data[j][i][0] = 0;
			data[j][i][1] = 0;
			data[j][i][2] = 0xff;
		}
	}

	uint8_t chip_data[256];
	memset(chip_data,0,sizeof(chip_data));
	for(uint16_t i=0;i<256;i++)
	{
		if((i%3) == 2)//red
		{
			chip_data[i] = 0xff;
		}

	}
	Aw20216_SpaceLeft(0,0,255);
	Aw20216_SpaceRight(0,0,255);

	// aw_disp_one_chip_data( AW_CHIP_INDEX_2, chip_data,75);
	// aw_disp_one_chip_data( AW_CHIP_INDEX_3, chip_data,75);
	// aw_disp_one_chip_data( AW_CHIP_INDEX_4, chip_data,75);
	// aw_disp_one_chip_data( AW_CHIP_INDEX_5, chip_data,75);
	aw_disp_one_chip_data( AW_CHIP_INDEX_6, chip_data,75);
}


// const rgb_map_t key_map[]
static uint8_t page_num = 0;
void User_mode1_prase(mode_para_t *mode_para,uint8_t (*data)[KEY_RGB_LED_X_NUM][3],key_record_t (*press_time)[KEY_RGB_LED_X_NUM],key_record_t (*release_time)[KEY_RGB_LED_X_NUM])
{

	read_led_data(CMD_KEY_LED1,(uint8_t *)data,page_num);
	page_num++;

	if(page_num >= g_user_data.key_led[0].frame_num)
	{
		page_num = 0;
	}
}

// const rgb_map_t key_map[]
void User_mode2_prase(mode_para_t *mode_para,uint8_t (*data)[KEY_RGB_LED_X_NUM][3],key_record_t (*press_time)[KEY_RGB_LED_X_NUM],key_record_t (*release_time)[KEY_RGB_LED_X_NUM])
{

	read_led_data(CMD_KEY_LED2,(uint8_t *)data,page_num);

	page_num++;
	if(page_num >= g_user_data.key_led[1].frame_num)
	{
		page_num = 0;
	}
}

// const rgb_map_t key_map[]
void User_mode3_prase(mode_para_t *mode_para,uint8_t (*data)[KEY_RGB_LED_X_NUM][3],key_record_t (*press_time)[KEY_RGB_LED_X_NUM],key_record_t (*release_time)[KEY_RGB_LED_X_NUM])
{
	read_led_data(CMD_KEY_LED3,(uint8_t *)data,page_num);
	page_num++;
	if(page_num >= g_user_data.key_led[2].frame_num)
	{
		page_num = 0;
	}
}

//6.按键一行闪熄
typedef void (*key_time_clear_timeout_t)(uint32_t time_out, key_record_t (*record)[KEY_RGB_LED_X_NUM],uint8_t x,uint8_t y);


static key_time_clear_timeout_t clear_timeout_cb;
uint8_t brightness_index=0;
#define STEP_INTERVAL   5//间隔时间

void key_time_clear_timeout(uint32_t time_out, key_record_t (*record)[KEY_RGB_LED_X_NUM],uint8_t x,uint8_t y)
{
	uint8_t erro_count=0;//防止record数据乱了陷入死循环
	uint8_t index;





	if(	((record[y][x].start_index==record[y][x].end_index)&&
			(record[y][x].time[record[y][x].end_index]==0))||
			(time_out==0)
		)//空队列，不查了
	{
			//NRF_LOG_INFO("time_out  =%d",time_out);
		return;
	}


	do
	{
		index = record[y][x].start_index;
	//	NRF_LOG_INFO("------------------y =%d x=%d index =%d  time_out=%d ",y,x,index,time_out);
	//	NRF_LOG_INFO(" record[y][x].start_index=%d ",record[y][x].start_index);
	//	NRF_LOG_INFO(" record[y][x].end_index=%d ",record[y][x].end_index);

		if(record[y][x].time[index]<=time_out)
		{
		//	NRF_LOG_INFO("y =%d x=%d index =%d clear index time_out=%d ",y,x,index,time_out);
			record[y][x].time[index]=0;
			if(record[y][x].start_index!=record[y][x].end_index)
			{
				record[y][x].start_index=record[y][x].start_index+1>=MAX_RECORD_QUEUE_SIZE?0:record[y][x].start_index+1;
			}
			else
			{
				//队列内只有一个节点的时候数据清零就可以了
			}
	//		NRF_LOG_INFO("record[y][x].start_index =%d record[y][x].end_index=%d ",record[y][x].start_index,record[y][x].end_index);





		}

		erro_count++;
		if(erro_count>MAX_RECORD_QUEUE_SIZE)
		{
			break;
		}
	}while(index!=record[y][x].end_index);




}
static key_time_clear_timeout_t clear_timeout_cb = key_time_clear_timeout;

void key_time_queue_put(key_record_t (*time_queue)[KEY_RGB_LED_X_NUM],uint32_t time,uint8_t x,uint8_t y)
{

		if(time_queue[y][x].start_index==time_queue[y][x].end_index)
		{
			if(time_queue[y][x].time[time_queue[y][x].end_index])//当前队列中存在一个节点
			{
				time_queue[y][x].end_index = time_queue[y][x].end_index+1>=MAX_RECORD_QUEUE_SIZE?0: time_queue[y][x].end_index+1;
				time_queue[y][x].time[time_queue[y][x].end_index]=time;
			}
			else//当前队列为空节点
			{
				time_queue[y][x].time[time_queue[y][x].end_index]=time;
			}
		}
		else
		{
			time_queue[y][x].end_index = time_queue[y][x].end_index+1>=MAX_RECORD_QUEUE_SIZE?0: time_queue[y][x].end_index+1;
			if(time_queue[y][x].end_index==time_queue[y][x].start_index)//end和start相邻
			{
				time_queue[y][x].start_index = time_queue[y][x].start_index+1>=MAX_RECORD_QUEUE_SIZE?0: time_queue[y][x].start_index+1;
			}
			else//当前队列为空节点
			{

			}
			time_queue[y][x].time[time_queue[y][x].end_index]=time;

		}

//		 NRF_LOG_INFO("start_index  =%d  end_index   =%d ",time_queue[y][x].start_index,time_queue[y][x].end_index);

}


//缓存之前的颜色计算结果
static uint8_t calculate_data[KEY_RGB_LED_Y_NUM][KEY_RGB_LED_X_NUM][3];

//比较两个颜色值，取得颜色值
void get_max_rgb_data(uint8_t (*new_data)[KEY_RGB_LED_X_NUM][3],uint8_t (*max_data)[KEY_RGB_LED_X_NUM][3])
{
	for(uint8_t i=0;i<KEY_RGB_LED_Y_NUM;i++)
	{
		for(uint8_t j=0;j<KEY_RGB_LED_X_NUM;j++)
		{
			if(new_data[i][j][0]+new_data[i][j][1]+new_data[i][j][2]>max_data[i][j][0]+max_data[i][j][1]+max_data[i][j][2])
			{
			  max_data[i][j][0]=new_data[i][j][0];
				max_data[i][j][1]=new_data[i][j][1];
			  max_data[i][j][2]=new_data[i][j][2];
			}
			else
			{

			}
		}
	}
}



void set_new_xy(uint8_t *x,uint8_t *y)
{
    for(uint8_t j=0;j<KEY_RGB_LED_Y_NUM;j++)
    {
        for(uint8_t i=0;i<keyboard_x_num[j];i++)
        {
            if((*x==key_addr[j][i][1]) && (*y==key_addr[j][i][0]))
            {
                *x = i;
                *y = j;
                return;
            }
        }
    }
}

void set_new_xy_2(uint8_t *x,uint8_t *y)
{
    for(uint8_t j=0;j<KEY_RGB_LED_Y_NUM;j++)
    {
        for(uint8_t i=0;i<keyboard_x_num_2[j];i++)
        {
            if((*x==key_addr_2[j][i][1]) && (*y==key_addr_2[j][i][0]))
            {
                *x = i;
                *y = j;
                return;
            }
        }
    }
}


void GetRowData_by_time(uint32_t time,uint8_t (*data)[KEY_RGB_LED_X_NUM][3],uint8_t y,uint8_t x)
{
  uint8_t rgb_interval=20;		//键灯之间相差20个颜色值，可调
	uint8_t color_index = 0;				//缓存其他键的颜色表值

	uint8_t distance=0;




	//port_delay_ms(10);


//	NRF_LOG_INFO("time =%d",time);

	//NRF_LOG_INFO("GetRowData_by_time");
	memset(data,0,KEY_RGB_LED_X_NUM*KEY_RGB_LED_Y_NUM*3);

    set_new_xy(&x,&y);//ALICE需要重新定位按键位置


	for(uint8_t i=0;i<keyboard_x_num[y];i++)
	{
		distance = i>x?i-x:x-i;

	//  NRF_LOG_INFO("brightness_num1 =%d distance =%d",brightness_num1,distance);
		if(time>=distance)//当前位置灯可以开始显示了
		{

			if(time>distance+brightness_num1)
			{
				brightness_index = 0;
			}
			else
			{
				brightness_index = brightness_num1-(time-distance);
			}


			if(distance*rgb_interval>=color_num)
			{
					color_index = distance*rgb_interval -color_num;
			}
			else
			{
					color_index = distance*rgb_interval;
			}
		}
		else
		{
			brightness_index = 0;
			memset(&data[y][i][0],0,3);
			//continue;
		}

		 //分配颜色
		/*
		 calculate_data[y][i][0]=color_table[color_index][0];
		 calculate_data[y][i][1]=color_table[color_index][1];
		 calculate_data[y][i][2]=color_table[color_index][2];
			*/

		 uint8_t r=color_table[color_index][0];
		 uint8_t g=color_table[color_index][1];
		 uint8_t b=color_table[color_index][2];

		/*
		 uint8_t r=0;
		 uint8_t g=255;
		 uint8_t b=0;
		*/
		 uint8_t light=gamma_brightness[brightness_index];

		//brightness_index
		 //根据亮度重新计算颜色
         uint8_t new_y = key_addr[y][i][0];
         uint8_t new_x = key_addr[y][i][1];

//         NRF_LOG_INFO("new_x:%d,new_y:%d",new_x,new_y);
        data[new_y][new_x][0] = gamma_brightness[brightness_index]*data[new_y][new_x][0]/255;
		 data[new_y][new_x][1] = gamma_brightness[brightness_index]*data[new_y][new_x][1]/255;
		 data[new_y][new_x][2] = gamma_brightness[brightness_index]*data[new_y][new_x][2]/255;


//         data[y][i][0] = gamma_brightness[brightness_index]*data[y][i][0]/255;
//         data[y][i][1] = gamma_brightness[brightness_index]*data[y][i][1]/255;
//         data[y][i][2] = gamma_brightness[brightness_index]*data[y][i][2]/255;



		calculate_data[new_y][new_x][0]=	light*r/255;
		calculate_data[new_y][new_x][1] = light*g/255;
		calculate_data[new_y][new_x][2] = light*b/255;



//		 NRF_LOG_INFO("color_index =%d",color_index);



	}



}

void KeyRowFlash_mode_prase(mode_para_t *mode_para,uint8_t (*data)[KEY_RGB_LED_X_NUM][3],key_record_t (*press_time)[KEY_RGB_LED_X_NUM],key_record_t (*release_time)[KEY_RGB_LED_X_NUM])
{
		uint8_t index=0;

	  //清空数据，脏数据会影响数据比较
		memset(data,0,KEY_RGB_LED_X_NUM*KEY_RGB_LED_Y_NUM*3);
	  memset(calculate_data,0,KEY_RGB_LED_X_NUM*KEY_RGB_LED_Y_NUM*3);


	//  NRF_LOG_INFO("mode_para->time_count =%d",mode_para->time_count);

 //  NRF_LOG_INFO("disp_time =%d",disp_time);

		for(uint8_t i=0;i<KEY_RGB_LED_Y_NUM;i++)
		{
			for(uint8_t j=0;j<KEY_RGB_LED_X_NUM;j++)//遍历键盘，查找存在按下显示的键位
			{


				if(	(press_time[i][j].end_index!=press_time[i][j].start_index)||
						((press_time[i][j].end_index==press_time[i][j].start_index)&&(press_time[i][j].time[press_time[i][j].end_index]))
					)//当前队列中存在有效节点
				{
		//			NRF_LOG_INFO("KeyRowFlash_mode_prase");
					index = press_time[i][j].start_index;

					//对某个按键的所有按下历史做出显叠加示
					do
					{
						//遍历按键的所有按下记录，在显示时间内的按下叠加显示

						//	NRF_LOG_INFO("mode_para->time_count-press_time[i][j].time[index] =%d",mode_para->time_count-press_time[i][j].time[index]);


						index = press_time[i][j].end_index;

						GetRowData_by_time( mode_para->time_count-press_time[i][j].time[index],calculate_data, i, j);
						get_max_rgb_data( calculate_data,data);

						if(index ==press_time[i][j].end_index)
						{
							//break;
						}
						else
						{
							if(index<press_time[i][j].end_index)
							{
								index++;
							}
							else
							{
								index = index+1>=MAX_RECORD_QUEUE_SIZE?0:index+1;
							}
					  }

//						NRF_LOG_INFO("----key led data feed---");
					}while(index!=press_time[i][j].end_index);
					//这个循环处理的是连续按下产生的灯效，对多个按键的效果进行叠加，叠加算法：get_max_rgb_data
//					NRF_LOG_INFO("----key led data feed    end");
				}
				else//不需要显示
				{

				}

			}
		}
		Aw20216_SpaceLeft(data[5][3][0],data[5][3][1],data[5][3][2]);
	Aw20216_SpaceRight(data[5][3][0],data[5][3][1],data[5][3][2]);


}

//7.按键侧推

void GetColunmData_by_time(uint32_t time,uint8_t y,uint8_t x)
{
	static uint8_t rgb_index=0;	//用于存储按下点的颜色表值
  uint8_t rgb_interval=20;		//键灯之间相差20个颜色值，可调

	uint8_t left_column=0;
	uint8_t right_column=0;
	 set_new_xy_2(&x,&y);//ALICE需要重新定位按键位置
	 QMK_LOG("x = %d y = %d",x,y);
  left_column = x>=time?x-time:0xff;
	right_column = x+time<keyboard_x_num[y]?x+time:0xff;

//	key_addr_2[i][right_column][0]

	for(uint8_t i=0;i<KEY_RGB_LED_Y_NUM;i++)
	{
		rgb_index=rgb_index+rgb_interval*i;
		rgb_index = rgb_index>color_num?rgb_index-color_num:rgb_index;

		if(left_column!=0xff)
		{
			 calculate_data[key_addr_2[i][left_column][0]][left_column][0]=color_table[rgb_index][0];
			 calculate_data[key_addr_2[i][left_column][0]][left_column][1]=color_table[rgb_index][1];
			 calculate_data[key_addr_2[i][left_column][0]][left_column][2]=color_table[rgb_index][2];
		}

		if(right_column!=0xff)
		{
			 calculate_data[key_addr_2[i][right_column][0]][right_column][0]=color_table[rgb_index][0];
			 calculate_data[key_addr_2[i][right_column][0]][right_column][1]=color_table[rgb_index][1];
			 calculate_data[key_addr_2[i][right_column][0]][right_column][2]=color_table[rgb_index][2];
		}
	}


}

//按键效果叠加
typedef void (*single_key_prase_t)( uint32_t ms, uint8_t y, uint8_t x);

void key_effect_overlay( mode_para_t *mode_para,uint8_t (*data)[KEY_RGB_LED_X_NUM][3],key_record_t (*press_time)[KEY_RGB_LED_X_NUM],
												 key_record_t (*release_time)[KEY_RGB_LED_X_NUM],uint32_t press_show_ms,uint32_t release_show_ms,single_key_prase_t press_prase,single_key_prase_t release_prase)
{
		uint8_t index=0;

 		memset(data,0,KEY_RGB_LED_X_NUM*KEY_RGB_LED_Y_NUM*3);
		memset(calculate_data,0,KEY_RGB_LED_X_NUM*KEY_RGB_LED_Y_NUM*3);


		for(uint8_t i=0;i<KEY_RGB_LED_Y_NUM;i++)
		{
			for(uint8_t j=0;j<KEY_RGB_LED_X_NUM;j++)//遍历键盘，查找存在按下显示的键位
			{
				if(clear_timeout_cb!=NULL)
				{
				  clear_timeout_cb(mode_para->time_count>press_show_ms?mode_para->time_count-press_show_ms:0,press_time,j,i);//清掉过期的按键按下记录
//                  if(mode_para->time_count>press_show_ms)
					clear_timeout_cb(mode_para->time_count>release_show_ms?mode_para->time_count-release_show_ms:0,release_time,j,i);//清掉过期的按键弹起记录
				}

				if(press_prase!=NULL)
				{

						if(	(press_time[i][j].end_index!=press_time[i][j].start_index)||
								((press_time[i][j].end_index==press_time[i][j].start_index)&&(press_time[i][j].time[press_time[i][j].end_index]))
							)//当前队列中存在有效节点
							{
								//按下事件比弹起事件新
								if(((press_time[i][j].time[press_time[i][j].end_index]>release_time[i][j].time[release_time[i][j].end_index])&&(release_time[i][j].time[release_time[i][j].end_index]))||
										(release_time[i][j].time[release_time[i][j].end_index]==0)
									)
								{
										index = press_time[i][j].start_index;

										do//对某个按键的按下历史做出显叠加示
										{
											//遍历按键的所有按下记录，在显示时间内的按下叠加显示
											index = press_time[i][j].end_index;

									//		if(press_prase!=NULL)
											{
												press_prase( mode_para->time_count-press_time[i][j].time[index],i,j);
											}

											get_max_rgb_data( calculate_data,data);

											if(index ==press_time[i][j].end_index)
											{
												//break;
											}
											else
											{
												if(index<press_time[i][j].end_index)
												{
													index++;
												}
												else
												{
													index = index+1>=MAX_RECORD_QUEUE_SIZE?0:index+1;
												}
											}


										}while(index!=press_time[i][j].end_index);//这个循环处理的是连续按下产生的灯效，对多个按键的效果进行叠加，叠加算法：get_max_rgb_data

										if(release_show_ms==0xffffffff)//如果是弹起后显示一种动效，一直等到下一次的按下才结束
										{
											clear_timeout_cb(mode_para->time_count,release_time,j,i);
										}

									}
							}
			}


				if(release_prase!=NULL)
				{

						if(	(release_time[i][j].end_index!=release_time[i][j].start_index)||
								((release_time[i][j].end_index==release_time[i][j].start_index)&&(release_time[i][j].time[release_time[i][j].end_index]))
							)//当前队列中存在有效节点
							{

								//NRF_LOG_INFO("1111111");

								  //弹起事件比按下事件新
									if(((release_time[i][j].time[release_time[i][j].end_index]>press_time[i][j].time[press_time[i][j].end_index])&&(press_time[i][j].time[press_time[i][j].end_index]))||
											(press_time[i][j].time[press_time[i][j].end_index]==0)
										)
									{

										index = release_time[i][j].start_index;

										do//对某个按键的按下历史做出显叠加示
										{
											//遍历按键的所有按下记录，在显示时间内的按下叠加显示
											index = release_time[i][j].end_index;

									//		if(release_prase!=NULL)
											{
												release_prase( mode_para->time_count-release_time[i][j].time[index],i,j);
											}

											get_max_rgb_data( calculate_data,data);

											if(index ==release_time[i][j].end_index)
											{
												//break;
											}
											else
											{
												if(index<release_time[i][j].end_index)
												{
													index++;
												}
												else
												{
													index = index+1>=MAX_RECORD_QUEUE_SIZE?0:index+1;
												}
											}

									}while(index!=release_time[i][j].end_index);//这个循环处理的是连续按下产生的灯效，对多个按键的效果进行叠加，叠加算法：get_max_rgb_data



										if(press_show_ms==0xffffffff)//如果是按下后一直显示 一种动效，直到弹起才结束
										{
											//NRF_LOG_INFO("clear_timeout_cb");
											clear_timeout_cb(mode_para->time_count,press_time,j,i);
										}
								}
							}
				}
		}
	}
}


void KeyColumnPush_mode_prase(mode_para_t *mode_para,uint8_t (*data)[KEY_RGB_LED_X_NUM][3],key_record_t (*press_time)[KEY_RGB_LED_X_NUM],key_record_t (*release_time)[KEY_RGB_LED_X_NUM])
{

// key_effect_overlay( mode_para,data, press_time,release_time, 16,GetColunmData_by_time);//1.5s显示时间可调

 key_effect_overlay( mode_para,data, press_time,release_time,
												 16, 0,GetColunmData_by_time , NULL);

	Aw20216_SpaceLeft(data[5][3][0],data[5][3][1],data[5][3][2]);
	Aw20216_SpaceRight(data[5][3][0],data[5][3][1],data[5][3][2]);
}


//9.按键亮熄，亮一下，逐步熄灭
//按键亮熄效果，弹起解析函数
//查找队列中超时的数据并清空
void key_effect_overlay_2( mode_para_t *mode_para,uint8_t (*data)[KEY_RGB_LED_X_NUM][3],key_record_t (*press_time)[KEY_RGB_LED_X_NUM],
												 key_record_t (*release_time)[KEY_RGB_LED_X_NUM],uint32_t press_show_ms,uint32_t release_show_ms,single_key_prase_t press_prase,single_key_prase_t release_prase)
{
		uint8_t index=0;

 		memset(data,0,KEY_RGB_LED_X_NUM*KEY_RGB_LED_Y_NUM*3);
		memset(calculate_data,0,KEY_RGB_LED_X_NUM*KEY_RGB_LED_Y_NUM*3);


		for(uint8_t i=0;i<KEY_RGB_LED_Y_NUM;i++)
		{
			for(uint8_t j=0;j<KEY_RGB_LED_X_NUM;j++)//遍历键盘，查找存在按下显示的键位
			{
                if(clear_timeout_cb!=NULL)
				{
				  clear_timeout_cb(mode_para->time_count>press_show_ms?mode_para->time_count-press_show_ms:0,press_time,j,i);//清掉过期的按键按下记录
//                  if(mode_para->time_count>press_show_ms)
					clear_timeout_cb(mode_para->time_count>release_show_ms?mode_para->time_count-release_show_ms:0,release_time,j,i);//清掉过期的按键弹起记录
				}
				if(1 == key_state[i][j])
				{
					{
						calculate_data[i][j][0]=color_table[random_table[(press_time[i][j].time[press_time[i][j].end_index])&0xff]][0];
						calculate_data[i][j][1]=color_table[random_table[(press_time[i][j].time[press_time[i][j].end_index])&0xff]][1];
						calculate_data[i][j][2]=color_table[random_table[(press_time[i][j].time[press_time[i][j].end_index])&0xff]][2];

						press_prase( mode_para->time_count-press_time[i][j].time[index],i,j);
					}

					get_max_rgb_data( calculate_data,data);

				}

				if(release_prase!=NULL)
				{

						if(	(release_time[i][j].end_index!=release_time[i][j].start_index)||
								((release_time[i][j].end_index==release_time[i][j].start_index)&&(release_time[i][j].time[release_time[i][j].end_index]))
							)//当前队列中存在有效节点
							{

								  //弹起事件比按下事件新
									if(((release_time[i][j].time[release_time[i][j].end_index]>press_time[i][j].time[press_time[i][j].end_index])&&(press_time[i][j].time[press_time[i][j].end_index]))||
											(press_time[i][j].time[press_time[i][j].end_index]==0)
										)
									{
//                                        NRF_LOG_INFO("i:%d  j:%d   start:%d   end:%d          ",i,j,press_time[i][j].time[0],press_time[i][j].time[0]);
										index = release_time[i][j].start_index;

										do//对某个按键的按下历史做出显叠加示
										{
											//遍历按键的所有按下记录，在显示时间内的按下叠加显示
											index = release_time[i][j].end_index;

									//		if(release_prase!=NULL)

											{
//                                                if(press_time[i][j].time[press_time[i][j].start_index] != 0)
                                                {
                                                    calculate_data[i][j][0]=color_table[random_table[(release_time[i][j].time[release_time[i][j].end_index])&0xff]][0];
                                                    calculate_data[i][j][1]=color_table[random_table[(release_time[i][j].time[release_time[i][j].end_index])&0xff]][1];
                                                    calculate_data[i][j][2]=color_table[random_table[(release_time[i][j].time[release_time[i][j].end_index])&0xff]][2];

//                                                    calculate_data[i][j][0]= 0xff;
//                                                    calculate_data[i][j][1]= 0xff;
//                                                    calculate_data[i][j][2]= 0xff;
                                                }


                                                release_prase( mode_para->time_count-release_time[i][j].time[index],i,j);
											}

											get_max_rgb_data( calculate_data,data);

											if(index ==release_time[i][j].end_index)
											{
												//break;
											}
											else
											{
												if(index<release_time[i][j].end_index)
												{
													index++;
												}
												else
												{
													index = index+1>=MAX_RECORD_QUEUE_SIZE?0:index+1;
												}
											}

									}while(index!=release_time[i][j].end_index);//这个循环处理的是连续按下产生的灯效，对多个按键的效果进行叠加，叠加算法：get_max_rgb_data



										if(press_show_ms==0xffffffff)//如果是按下后一直显示 一种动效，直到弹起才结束
										{
											//NRF_LOG_INFO("clear_timeout_cb");
											clear_timeout_cb(mode_para->time_count,press_time,j,i);
										}
								}
							}
				}
		}
	}
}

static void Press_GetFlashData_by_time(uint32_t time,uint8_t y,uint8_t x)
{
//	NRF_LOG_INFO("Press_GetFlashData_by_time  x:%d  y:%d",x,y);

}




//按键亮熄效果，弹起解析函数
static void Release_GetFlashData_by_time(uint32_t time,uint8_t y,uint8_t x)
{

//	rgb_index = rgb_index+rgb_offset>color_num?rgb_index+rgb_offset-color_num:rgb_index+rgb_offset;
//    calculate_data[y][x][0]=color_table[random_table[random_index]][0];
//    calculate_data[y][x][1]=color_table[random_table[random_index]][1];
//    calculate_data[y][x][2]=color_table[random_table[random_index]][2];

	time=brightness_num1-time;
//	NRF_LOG_INFO("time =%d",time);
  calculate_data[y][x][0] = gamma_brightness[time]*calculate_data[y][x][0]/255;
  calculate_data[y][x][1] = gamma_brightness[time]*calculate_data[y][x][1]/255;
  calculate_data[y][x][2] = gamma_brightness[time]*calculate_data[y][x][2]/255;
}


void KeyFlashSlake_mode_prase(mode_para_t *mode_para,uint8_t (*data)[KEY_RGB_LED_X_NUM][3],key_record_t (*press_time)[KEY_RGB_LED_X_NUM],key_record_t (*release_time)[KEY_RGB_LED_X_NUM])
{
	key_effect_overlay_2( mode_para,data, press_time,release_time,
												 0xffffffff, brightness_num1,Press_GetFlashData_by_time , Release_GetFlashData_by_time);

    Aw20216_SpaceLeft(data[5][3][0],data[5][3][1],data[5][3][2]);
	Aw20216_SpaceRight(data[5][3][0],data[5][3][1],data[5][3][2]);
}


//head 马赛克灯效
static uint8_t head_index_table[HEAD_RGB_LED_Y][HEAD_RGB_LED_X];
void Head_Mosaic_flow_prase(mode_para_t *mode_para,uint8_t (*data)[HEAD_RGB_LED_X][3],key_record_t (*press_time)[KEY_RGB_LED_X_NUM],key_record_t (*release_time)[KEY_RGB_LED_X_NUM])
{

	static uint8_t index=0;
    uint8_t side_lightness=0;
	if(!index)
	{

		memcpy(head_index_table,random_table1,sizeof(head_index_table));

		index = 1;
	}

	for(uint8_t i=0;i<HEAD_RGB_LED_X;i++)
	{
		for(uint8_t j=0;j<HEAD_RGB_LED_Y;j++)
		{

			memcpy(&data[j][i][0],&color_table[head_index_table[j][i]],3);

			head_index_table[j][i]++;

			if(head_index_table[j][i]>=sizeof(color_table)/sizeof(color_table[0]))
			{
				head_index_table[j][i] = 0;
			}

		}
	}

    memcpy(side_led_data,&data[1][4][0],sizeof(side_led_data));
    if(led_switch_g[0])
    {
        side_lightness = head_lightness;
    }
    else
    {
        side_lightness = 0;
    }
    Aw20216_DispSideLedFrame_data(side_led_data,side_lightness);
}


static uint8_t head_page_num = 0;//用于3个自定义灯效帧计数

void Head_mode1_prase(mode_para_t *mode_para,uint8_t (*data)[HEAD_RGB_LED_X][3],key_record_t (*press_time)[KEY_RGB_LED_X_NUM],key_record_t (*release_time)[KEY_RGB_LED_X_NUM])
{
    uint8_t side_lightness = 45;
	read_led_data(CMD_HEAD_LED1,(uint8_t *)data,head_page_num);
    read_led_data(CMD_SIDE_LED1,(uint8_t *)side_led_data,head_page_num);
    if(led_switch_g[0])
    {
        side_lightness =  head_lightness;
;
    }
    else
    {
        side_lightness = 0;
    }
    Aw20216_DispSideLedFrame_data(side_led_data,side_lightness);
		head_page_num++;
	if(head_page_num >= g_user_data.head_led[0].frame_num)
	{
		head_page_num = 0;
	}
}

void Head_mode2_prase(mode_para_t *mode_para,uint8_t (*data)[HEAD_RGB_LED_X][3],key_record_t (*press_time)[KEY_RGB_LED_X_NUM],key_record_t (*release_time)[KEY_RGB_LED_X_NUM])
{
    uint8_t side_lightness = 45;
	read_led_data(CMD_HEAD_LED2,(uint8_t *)data,head_page_num);
    read_led_data(CMD_SIDE_LED2,(uint8_t *)side_led_data,head_page_num);
    if(led_switch_g[0])
    {
        side_lightness =  head_lightness;
    }
    else
    {
        side_lightness = 0;
    }
    Aw20216_DispSideLedFrame_data(side_led_data,side_lightness);

		head_page_num++;
	if(head_page_num >= g_user_data.head_led[1].frame_num)
	{
		head_page_num = 0;
	}
}

void Head_mode3_prase(mode_para_t *mode_para,uint8_t (*data)[HEAD_RGB_LED_X][3],key_record_t (*press_time)[KEY_RGB_LED_X_NUM],key_record_t (*release_time)[KEY_RGB_LED_X_NUM])
{

	read_led_data(CMD_HEAD_LED3,(uint8_t *)data,head_page_num);
    read_led_data(CMD_SIDE_LED3,(uint8_t *)side_led_data,head_page_num);
    uint8_t side_lightness = 45;
    if(led_switch_g[0])
    {
        side_lightness =  head_lightness;
    }
    else
    {
        side_lightness = 0;
    }
    Aw20216_DispSideLedFrame_data(side_led_data,side_lightness);
		head_page_num++;
	if(head_page_num >= g_user_data.head_led[2].frame_num)
	{
		head_page_num = 0;
	}
}


void Head_all_white_prase(mode_para_t *mode_para,uint8_t (*data)[HEAD_RGB_LED_X][3],key_record_t (*press_time)[KEY_RGB_LED_X_NUM],key_record_t (*release_time)[KEY_RGB_LED_X_NUM])
{
	memset(data,0xff,sizeof(head_led_data));
}

/*********************************************************************************************************************/
/*********************************************************************************************************************/


static uint8_t key_led_index = 0;
static uint8_t head_led_index = 0;


//模式排序,模式增，删，改序在这里
const key_led_mode_t *mode_list[]=
{

    &mosaic_flow_mode,
	&BlueBreath_mode,
	&ColorTrans_mode,
	&CenterSpread_mode,
	&RowTrans_mode,
	&KeyRowFlash_mode,
	// &KeyColumnPush_mode,
	&KeyFlashSlake_mode,//按一下亮一下
	// &All_white_mode,
	// &All_red_mode,
	// &All_green_mode,
	// &All_blue_mode,
	&key_mode_1,
    &key_mode_2,
    &key_mode_3,
};

//模式排序,模式增，删，改序在这里
const head_led_mode_t *head_mode_list[]=
{
	&head_mosaic_flow_mode,
    // &head_all_white,
	&head_mode_1,
	&head_mode_2,
	&head_mode_3
};


volatile char head_running,key_running;
static int led_count = 0;
static THD_FUNCTION(led_scan_thread, arg) {
	mode_para_t mode_para;
    uint8_t key_lightness = 45;
    uint16_t frame_delay_ms = 0;
	QMK_LOG("led_thread_start\n");

    chRegSetThreadName("led_thread");

    setPinOutput(LED_EN1);
	writePinHigh(LED_EN1);
    uint8_t light_up = 0;
    uint8_t light_up_complete =0;
    while (true) {
        key_running = 1;
		led_count++;

		mode_para.time_count = led_count;
		if(key_led_play != NULL)
		{
            if(led_switch_g[1])
            {
                if(led_sleep[1] == 0)
                {
                    key_lightness = key_led_lightness_code[g_key_lightness_index];
                }
                else
                {
                    key_lightness = 0;
                }
            }
            else
            {
                key_lightness = 0;
            }
            if(0 == light_up_complete)
            {//用于做上电缓亮
                // if(0 == (led_count % 2))
                {
                    light_up++;
                }
                key_lightness =light_up;
                if(light_up == key_led_lightness_code[g_key_lightness_index])
                {
                    light_up_complete = 1;
                }
            }

            lock_led_state_g[0]?Aw20216_CapsLock(CAPSLOCK_COLOR_ON):Aw20216_CapsLock(CAPSLOCK_COLOR_OFF);

            if (signalrgb_mode_enabled()) {
                const uint8_t frame_index = signalrgb_active_frame_index;
                Aw20216_DispRealUnderKeyFrame_data(signalrgb_key_led_frames[frame_index],key_lightness);
            } else {
			    key_led_play->key_led_prase(&mode_para,key_led_data,key_press_count,key_release_count);
                Aw20216_DispUnderKeyFrame_data(key_led_data,key_lightness);
            }


		}
        if(0 == (led_count % 50))//每50帧进行一次连接检测
        {
            setPinOutput(LED_EN1);
            writePinHigh(LED_EN1);

            if(false == aw20216_check_en(LED_CS1))
            {
                QMK_LOG("RESTART\n");
                chMtxLock(&myMutex);

                AW20216_init(LED_CS1,LED_EN1);
                AW20216_init(LED_CS2,LED_EN1);
                AW20216_init(LED_CS3,LED_EN2);
                AW20216_init(LED_CS4,LED_EN2);
                AW20216_init(LED_CS5,LED_EN2);
                AW20216_init(LED_CS6,LED_EN2);
                AW20216_init(LED_CS7,LED_EN3);

                uint8_t page_data[256]={0};
                memset(page_data,0,256);
                set_per_led_brightness( LED_CS1, page_data);
                // set_chip_led_brightness( LED_CS1, 0);
                set_chip_led_brightness( LED_CS2, 0);
                set_chip_led_brightness( LED_CS3, 0);
                set_chip_led_brightness( LED_CS4, 0);
                set_chip_led_brightness( LED_CS5, 0);
                set_chip_led_brightness( LED_CS6, 0);
                set_chip_led_brightness( LED_CS7, 0);


                g_head_lightness_index = 0;
                g_key_lightness_index = 0;
                chMtxUnlock(&myMutex);
            }

        }
        key_running = 0;
        if (!signalrgb_mode_enabled()) {
            while (1 == head_running)
        {//用于进行帧同步，额头灯和轴底灯某个走的较快
            chThdSleepMilliseconds(1);
        }
        }
        if (signalrgb_mode_enabled()) {
            frame_delay_ms = SIGNALRGB_REFRESH_INTERVAL_MS;
        } else {
		    frame_delay_ms = key_led_play->scan_base_ms + key_led_play->scan_step_ms;
        }
		chThdSleepMilliseconds(frame_delay_ms);
    }
}

static THD_FUNCTION(head_led_scan_thread, arg) {

	static int led_count = 0;
	mode_para_t mode_para;
    uint16_t frame_delay_ms = 0;

	QMK_LOG("head_led_thread_start\n");

    chRegSetThreadName("head_led_thread");
    uint8_t light_up = 0;
    uint8_t light_up_complete =0;
    // chThdSleepMilliseconds(500);
    setPinOutput(LED_EN2);
	writePinHigh(LED_EN2);

    setPinOutput(LED_EN3);
	writePinHigh(LED_EN3);
    while (true) {
        head_running=1;
		led_count++;

		mode_para.time_count = led_count;
		if(head_led_play != NULL)
		{
            if(led_switch_g[0])
            {
                if(led_sleep[0] == 0)
                {
                    head_lightness = key_led_lightness_code[g_head_lightness_index];
                }
                else
                {
                    head_lightness = 0;
                }
            }
            else
            {
                head_lightness = 0;
            }
            if(0 == light_up_complete)
            {//用于做上电缓亮
                // if(0 == (led_count % 2))
                {
                    light_up++;
                }
                head_lightness =light_up;
                if(light_up == key_led_lightness_code[g_head_lightness_index])
                {
                    light_up_complete = 1;
                }
            }

            if (signalrgb_mode_enabled()) {
                const uint8_t frame_index = signalrgb_active_frame_index;
			    Aw20216_DispHeadLedFrame_data(signalrgb_head_led_frames[frame_index],head_lightness);
                Aw20216_DispSideLedFrame_data(signalrgb_side_led_frames[frame_index],head_lightness);
            } else {
                head_led_play->head_led_prase(&mode_para,head_led_data,NULL,NULL);
			    Aw20216_DispHeadLedFrame_data(head_led_data,head_lightness);
            }

		}




        head_running = 0;
        if (!signalrgb_mode_enabled()) {
            while (1 == key_running)
        {//用于进行帧同步，额头灯和轴底灯某个走的较快
            chThdSleepMilliseconds(1);
        }
        }
        if (signalrgb_mode_enabled()) {
            frame_delay_ms = SIGNALRGB_REFRESH_INTERVAL_MS;
        } else {
		    frame_delay_ms = head_led_play->scan_base_ms + head_led_play->scan_step_ms;
        }
		chThdSleepMilliseconds(frame_delay_ms);


    }
}


void key_led_next_func(void)
{
	key_led_index++;

	if(key_led_index>=(sizeof(mode_list)/sizeof(mode_list[0])))
	{
		key_led_index = 0;
	}
	key_led_play = (key_led_mode_t *)mode_list[key_led_index];
    g_user_data.curr_head_led = head_led_play;
    g_user_data.curr_key_led = key_led_play;
    save_user_data(&g_user_data);
}


void key_led_prev_func(void)
{
	if(key_led_index==0)
	{
		key_led_index = (sizeof(mode_list)/sizeof(mode_list[0]))-1;
	}
	else
	{
		key_led_index--;
	}
	key_led_play = (key_led_mode_t *)mode_list[key_led_index];
    g_user_data.curr_head_led = head_led_play;
    g_user_data.curr_key_led = key_led_play;
    save_user_data(&g_user_data);
}

void head_led_next_func(void)
{
	head_led_index++;
    head_page_num = 0;
    page_num = 0;

	if(head_led_index>=(sizeof(head_mode_list)/sizeof(head_mode_list[0])))
	{
		head_led_index = 0;
	}
	head_led_play = (head_led_mode_t *)head_mode_list[head_led_index];

    if (head_led_play == &head_mosaic_flow_mode)
    {
        key_led_play = &mosaic_flow_mode;//按键灯效要跟随同步额头灯的灯效
        key_led_index = 0;
    }

    else if(head_led_play == &head_mode_1)
    {
        key_led_play = &key_mode_1;
        key_led_index = 7;
    }
    else if(head_led_play == &head_mode_2)
    {
        key_led_play = &key_mode_2;
        key_led_index = 8;
    }
    else if(head_led_play == &head_mode_3)
    {
        key_led_play = &key_mode_3;
        key_led_index = 9;
    }

    key_led_play->scan_step_ms = head_led_play->scan_step_ms;
    g_user_data.curr_head_led = head_led_play;
    g_user_data.curr_key_led = key_led_play;
    save_user_data(&g_user_data);
}


void head_led_prev_func(void)
{
	head_page_num = 0;
    page_num = 0;

	if(head_led_index==0)
	{
		head_led_index = (sizeof(head_mode_list)/sizeof(head_mode_list[0]))-1;
	}
	else
	{
		head_led_index--;
	}
	head_led_play = (head_led_mode_t *)head_mode_list[head_led_index];

    if (head_led_play == &head_mosaic_flow_mode)
    {
        key_led_play = &mosaic_flow_mode;//按键灯效要跟随同步额头灯的灯效
        key_led_index = 0;
    }

    else if(head_led_play == &head_mode_1)
    {
        key_led_play = &key_mode_1;
        key_led_index = 7;
    }
    else if(head_led_play == &head_mode_2)
    {
        key_led_play = &key_mode_2;
        key_led_index = 8;
    }
    else if(head_led_play == &head_mode_3)
    {
        key_led_play = &key_mode_3;
        key_led_index = 9;
    }

    key_led_play->scan_step_ms = head_led_play->scan_step_ms;
    g_user_data.curr_head_led = head_led_play;
    g_user_data.curr_key_led = key_led_play;
    save_user_data(&g_user_data);
}

void key_led_lightness_ins(void)
{
	// key_led_play->lightness_index++;
    g_key_lightness_index ++;
	// if(key_led_play->lightness_index >= sizeof(key_led_lightness_code))
	// {
	// 	key_led_play->lightness_index = sizeof(key_led_lightness_code)-1;
	// }
    if(g_key_lightness_index >= sizeof(key_led_lightness_code))
	{
		g_key_lightness_index = sizeof(key_led_lightness_code)-1;
	}
}

void key_led_lightness_dec(void)
{


	// if(key_led_play->lightness_index > 0)
	// {
	// 	key_led_play->lightness_index--;
	// }
    if(g_key_lightness_index > 0)
	{
		g_key_lightness_index--;
	}

}

void head_led_lightness_ins(void)
{
	g_head_lightness_index++;

	if(g_head_lightness_index >= sizeof(key_led_lightness_code))
	{
		g_head_lightness_index = sizeof(key_led_lightness_code)-1;
	}

}

void haed_led_lightness_dec(void)
{

    if(g_head_lightness_index > 0)
	{
		g_head_lightness_index--;
	}

}

void key_led_speed_dec(void)
{

	if(key_led_play->scan_step_ms < MAX_KEY_LED_INTERVAL_TIME)
	{
        if(key_led_play->scan_step_ms<100)
        {
            key_led_play->scan_step_ms+=4;
        }
        else
        {
            key_led_play->scan_step_ms+=10;
        }

	}

}

void key_led_speed_inc(void)
{

	if(key_led_play->scan_step_ms > MIN_KEY_LED_INTERVAL_TIME+4)
	{
        if(key_led_play->scan_step_ms<100)
        {
            key_led_play->scan_step_ms-=4;
        }
        else
        {
            key_led_play->scan_step_ms-=10;
        }

	}
}

void haed_led_speed_dec(void)
{

	if(head_led_play->scan_step_ms < MAX_HEAD_LED_INTERVAL_TIME)
	{
        if(head_led_play->scan_step_ms<100)
        {
            head_led_play->scan_step_ms+=4;
            key_led_play->scan_step_ms+=4;
        }
		else
        {
            head_led_play->scan_step_ms+=10;
            key_led_play->scan_step_ms+=10;
        }

        save_user_data(&g_user_data);

	}


}

void head_led_speed_inc(void)
{

	if(head_led_play->scan_step_ms > MIN_HEAD_LED_INTERVAL_TIME+4)
	{
        if(head_led_play->scan_step_ms<100)
        {
            head_led_play->scan_step_ms-=4;
        key_led_play->scan_step_ms-=4;
        }
        else
        {
            head_led_play->scan_step_ms-=10;
            key_led_play->scan_step_ms-=10;
        }

        save_user_data(&g_user_data);
	}


}

uint8_t reg_value=0;
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
	if (record->event.pressed == 0) {
        sleep_tick =0;
		key_state[record->event.key.row][record->event.key.col] = 0;
		QMK_LOG("keycode:%X ,col:%d row:%d time:%d\n",keycode,record->event.key.col,record->event.key.row,record->event.time);

		switch (keycode) {
			case KEY_LED_NEXT:
				key_led_next_func();
				break;
			case KEY_LED_PREV:
				key_led_prev_func();
				break;
			case KEY_LED_SPEED_INC:
				key_led_speed_inc();
                QMK_LOG("key_speed:%d",key_led_play->scan_base_ms+key_led_play->scan_step_ms);
				break;
			case KEY_LED_SPEED_DEC:
				key_led_speed_dec();
                QMK_LOG("key_speed:%d",key_led_play->scan_base_ms+key_led_play->scan_step_ms);
				break;
			case KEY_LED_LIGHT_INS:
				key_led_lightness_ins();
				break;
			case KEY_LED_LIGHT_DEC:
				key_led_lightness_dec();
				break;
			case HEAD_LED_NEXT:
				head_led_next_func();
				break;
			case HEAD_LED_PREV:
				head_led_prev_func();
				break;
            case HEAD_LED_LIGHT_INS:
                head_led_lightness_ins();
                break;
            case HEAD_LED_LIGHT_DEC:
                haed_led_lightness_dec();
                break;
            case HEAD_LED_SPEED_INC:
				head_led_speed_inc();
                QMK_LOG("head_speed:%d",head_led_play->scan_base_ms+head_led_play->scan_step_ms);
				break;
			case HEAD_LED_SPEED_DEC:
				haed_led_speed_dec();
                QMK_LOG("head_speed:%d",head_led_play->scan_base_ms+head_led_play->scan_step_ms);
				break;
            case HEAD_LED_POWER:
                led_switch_g[0] = !led_switch_g[0];
                break;
            case KEY_LED_POWER:
                led_switch_g[1] = !led_switch_g[1];
                break;
			default:
				break;
		}
	}
	else
	{
		key_state[record->event.key.row][record->event.key.col] = 1;

		QMK_LOG("keycode:%X ,col:%d row:%d time:%d\n",keycode,record->event.key.col,record->event.key.row,record->event.time);
	}
  return true;
}



void led_set_kb(uint8_t usb_led) {

    QMK_LOG("USB_LED:%X\n",usb_led);
    if (usb_led & (1 << 1))
        lock_led_state_g[0] = 1;
    else
        lock_led_state_g[0] = 0;
}

#include "spi_flash_drv.h"

void key_led_init(void)
{
	uint8_t reg_value = 0;


    wait_ms(20);
	setPinOutput(DCDC_EN);
	writePinHigh(DCDC_EN);
    wait_ms(1000);


	spi_init();
    AW20216_init(LED_CS1, LED_EN1);
    AW20216_init(LED_CS2, LED_EN1);

	AW20216_init(LED_CS3, LED_EN2);
	AW20216_init(LED_CS4, LED_EN2);
	AW20216_init(LED_CS5, LED_EN2);
	AW20216_init(LED_CS6, LED_EN2);
	AW20216_init(LED_CS7, LED_EN3);

	AW20216_read(LED_CS1,0,0x2f,&reg_value,1);
	QMK_LOG("reg_rsth:%x\n",reg_value);



    key_led_play = &mosaic_flow_mode;
    head_led_play = &head_mosaic_flow_mode;
    QMK_LOG("curr_head_led:%x\n",g_user_data.curr_head_led);
    for(uint8_t i=0;i<(sizeof(head_mode_list)/sizeof(head_mode_list[0]));i++)
    {
        QMK_LOG("head i:%d %x\n",i,head_mode_list[i]);
        if(g_user_data.curr_head_led == head_mode_list[i])
        {
            head_led_play = g_user_data.curr_head_led;
            break;
        }
        else
        {
            head_led_play = &head_mosaic_flow_mode;
        }
    }

    QMK_LOG("curr_key_led:%x",g_user_data.curr_key_led);
    for(uint8_t i=0;i<(sizeof(mode_list)/sizeof(mode_list[0]));i++)
    {
        QMK_LOG("key i:%d %x\n",i,mode_list[i]);
        if(g_user_data.curr_key_led == mode_list[i])
        {
            key_led_play = g_user_data.curr_key_led;
            break;
        }
        else
        {
            key_led_play = &mosaic_flow_mode;
        }
    }


	chThdCreateStatic(led_thread_area, sizeof(led_thread_area), 200, led_scan_thread, NULL);
	chThdCreateStatic(head_led_thread_area, sizeof(head_led_thread_area), 200, head_led_scan_thread, NULL);
}






void matrix_init_user(void) {

    setPinOutput(LED_EN1);
	writePinLow(LED_EN1);

    setPinOutput(LED_EN2);
	writePinLow(LED_EN2);

    setPinOutput(LED_EN3);
	writePinLow(LED_EN3);

    setPinOutput(DCDC_MODE);
	writePinLow(DCDC_MODE);

    SEGGER_RTT_Init();
	wait_ms(500);
	QMK_LOG("AngryMiao QMK Keyboard\n");

	chMtxObjectInit(&myMutex);
    SpiFlash_Init();

	read_user_data(&g_user_data);
	key_led_init();



}






