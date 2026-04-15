#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "led_drv.h"
#define KEY_LED_PAGE_SIZE 		(512)
#define HEAD_LED_PAGE_SIZE 		(1024)
#define SIDE_LED_PAGE_SIZE      (256)


#define FLASH_SECTOR_SIZE 		4096
#define FLASH_DATA_SIZE 		(sizeof(user_data_t)) // 增加一个字节用于标记
#define FLAG_VALID_DATA 		0xef
#define FLAG_INVALID_DATA 		0x00

#define USER_FRAME_MAX          (256)

#define USER_DATA_START			(0)
#define USER_DATA_SIZE			(4096)                          /*循环存储区，用于存放设置数据*/

#define KEY_LED1_START			(USER_DATA_START+USER_DATA_SIZE)
#define KEY_LED1_SIZE			(KEY_LED_PAGE_SIZE*USER_FRAME_MAX)/*每帧用512个字节存放，固定地址，共可存放256帧，32个扇区*/
#define KEY_LED2_START			(KEY_LED1_START+KEY_LED1_SIZE)
#define KEY_LED2_SIZE			(KEY_LED_PAGE_SIZE*USER_FRAME_MAX)/*每帧用512个字节存放，固定地址，共可存放256帧，32个扇区*/
#define KEY_LED3_START			(KEY_LED2_START+KEY_LED2_SIZE)
#define KEY_LED3_SIZE			(KEY_LED_PAGE_SIZE*USER_FRAME_MAX)/*每帧用512个字节存放，固定地址，共可存放256帧，32个扇区*/


#define	HEAD_LED1_START			(KEY_LED3_START+KEY_LED3_SIZE)
#define HEAD_LED1_SIZE			(HEAD_LED_PAGE_SIZE*USER_FRAME_MAX)/*每帧用1024个字节存放，固定地址，共可存放256帧，64个扇区*/
#define	HEAD_LED2_START			(HEAD_LED1_START+HEAD_LED1_SIZE)
#define HEAD_LED2_SIZE			(HEAD_LED_PAGE_SIZE*USER_FRAME_MAX)/*每帧用1024个字节存放，固定地址，共可存放256帧，64个扇区*/
#define	HEAD_LED3_START			(HEAD_LED2_START+HEAD_LED2_SIZE)
#define HEAD_LED3_SIZE			(HEAD_LED_PAGE_SIZE*USER_FRAME_MAX)/*每帧用1024个字节存放，固定地址，共可存放256帧，64个扇区*/

#define SIDE_LED1_START	        (HEAD_LED3_START+HEAD_LED3_SIZE)
#define SIDE_LED1_SIZE	        (SIDE_LED_PAGE_SIZE*USER_FRAME_MAX)/*每帧用256个字节存放，固定地址，共可存放256帧，16个扇区*/
#define SIDE_LED2_START	        (SIDE_LED1_START+SIDE_LED1_SIZE)
#define SIDE_LED2_SIZE	        (SIDE_LED_PAGE_SIZE*USER_FRAME_MAX)/*每帧用1024个字节存放，固定地址，共可存放256帧，16个扇区*/
#define SIDE_LED3_START	        (SIDE_LED2_START+SIDE_LED2_SIZE)
#define SIDE_LED3_SIZE	        (SIDE_LED_PAGE_SIZE*USER_FRAME_MAX)/*每帧用1024个字节存放，固定地址，共可存放256帧，16个扇区*/

#define HEAD_DEFAULT1_START	    (SIDE_LED3_START+SIDE_LED3_SIZE)
#define HEAD_DEFAULT1_SIZE      (HEAD_LED_PAGE_SIZE*USER_FRAME_MAX)/*每帧用1024个字节存放，固定地址，共可存放256帧，64个扇区*/
#define HEAD_DEFAULT2_START	    (HEAD_DEFAULT1_START+HEAD_DEFAULT1_SIZE)
#define HEAD_DEFAULT2_SIZE      (HEAD_LED_PAGE_SIZE*USER_FRAME_MAX)/*每帧用1024个字节存放，固定地址，共可存放256帧，64个扇区*/

#define SIDE_DEFAULT1_START	    (HEAD_DEFAULT2_START+HEAD_DEFAULT2_SIZE)
#define SIDE_DEFAULT1_SIZE	    (SIDE_LED_PAGE_SIZE*USER_FRAME_MAX)/*每帧用256个字节存放，固定地址，共可存放256帧，16个扇区*/
#define SIDE_DEFAULT2_START	    (SIDE_DEFAULT1_START+SIDE_DEFAULT1_SIZE)
#define SIDE_DEFAULT2_SIZE	    (SIDE_LED_PAGE_SIZE*USER_FRAME_MAX)/*每帧用256个字节存放，固定地址，共可存放256帧，16个扇区*/


/*已用了497个扇区，总存储2M，512个扇区*/

typedef struct key_led_set
{
	uint8_t frame_num;
	uint8_t lightness;
	uint8_t time_interval;
    uint8_t reserved;
}led_set_t;

typedef enum
{
	CMD_NULL,
	CMD_KEY_LED1,
	CMD_KEY_LED2,
	CMD_KEY_LED3,
	CMD_HEAD_LED1,
	CMD_HEAD_LED2,
	CMD_HEAD_LED3,
    CMD_SIDE_LED1,
    CMD_SIDE_LED2,
    CMD_SIDE_LED3,
    CMD_DEFAT_HEAD1,
    CMD_DEFAT_HEAD2,
    CMD_DEFAT_SIDE1,
    CMD_DEFAT_SIDE2,
    CMD_OTHER_CMD,
	CMD_END,
}cmd_enum_t;

typedef struct
{
	uint8_t used;
	uint8_t head;		//0x81
	uint8_t length;		//总结构体大小:sizeof(user_data_t)
	uint8_t check;		//check之后的所有数据大小
	uint8_t unused;		//NULL
	led_set_t key_led[3];
	led_set_t head_led[3];
    led_set_t side_led[3];
    head_led_mode_t *curr_head_led;
    key_led_mode_t *curr_key_led;
    uint8_t reserved[11];//凑够总大小64个字节
}user_data_t;

typedef enum {
	AM_FLASH_IDLE=0,
	AM_FLASH_WRITE,
	AM_FLASH_READ,
	AM_FALSH_ERASE,
}am_flash_state_t;


typedef struct
{
	uint8_t head;
	uint8_t cmd;		//see cmd_enum_t
	uint8_t page;
	uint8_t pack;
	uint8_t light;
	uint8_t interval;
	uint8_t length;
	uint8_t data[24];
	uint8_t check;
}am_pack_t;

extern user_data_t g_user_data;

void read_led_data(uint8_t led_num,uint8_t *led_data,uint8_t frame);
void save_led_data(am_pack_t * recv_pack);

bool save_user_data(user_data_t *user);
bool read_user_data(user_data_t *user);
