

#ifndef _UI_DATA_H
#define _UI_DATA_H

#include <stdint.h>
#include <string.h>


typedef struct 
{
	uint8_t 	x_width;//字体宽度
	uint8_t 	y_high;//字体长度
	uint16_t 	unicode;//unicode
	uint8_t * ui_p;   //ui数据
}word_map_t;



//冒号
extern const uint8_t colon_1x5[1][5][3];
//5x5数字
extern const uint8_t num_5x5[10][5][5][3];
//5x5大写字母
extern const uint8_t char_5x5[26][5][5][3];
//ble ui界面
extern const uint8_t ble_16x5[3][5][16][3];
//切换键层界面
extern const uint8_t key_layer_16x5[7][5][32][3];
//颜色表
//extern const uint8_t color_table[188][3];
extern const uint8_t color_table[188][3];
//随机表
extern const uint8_t random_table[256];

extern const uint8_t random_table1[5][46];

extern const uint8_t random_table2[6][15];


//7x3电池插入电量
extern const uint8_t charge_batt_7x3[4][3][7][3];

//7x3电池电量
extern const uint8_t batt_7x3[5][3][7][3];




//7x3电池低电报警
extern const uint8_t batt_warn_7x3[1][3][7][3];

//7x3充电图形
extern const uint8_t batt_charge_7x3[3][7][3];


extern const uint8_t cyberboard[5][49][3];


extern  uint8_t batt_charge_3x3[][3][3][3];


extern const uint8_t char_4x5[][5][4][3];


extern const uint8_t char_W_5x5[][5][5][3];
extern const uint8_t char_i_1x5[][5][1][3];
extern const uint8_t char_l_1x5[][5][1][3];
extern const uint8_t char_T_5x5[][5][5][3];
extern const uint8_t char_M_5x5[][5][5][3];
extern const uint8_t char_I_3x5[][5][3][3];


//3x5数字
extern const uint8_t num_3x5[10][5][3][3];

extern const word_map_t default_map[];
//extern word_map_t default_map[];

#endif 


