#ifndef LED_EFFECT_H
#define LED_EFFECT_H

#include "led_drv.h"
#include "ui_data.h"


void key_time_queue_put(key_record_t (*time_queue)[KEY_RGB_LED_X_NUM],uint32_t time,uint8_t x,uint8_t y);
    

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



#endif
