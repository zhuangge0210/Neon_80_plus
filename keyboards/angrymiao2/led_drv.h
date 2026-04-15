#ifndef __LED_DRV_
#define __LED_DRV_
/******************************key_led_drv**********************************/
#define KEY_RGB_LED_X_NUM   15
#define KEY_RGB_LED_Y_NUM   6

#define HEAD_RGB_LED_X      46
#define HEAD_RGB_LED_Y      5

#define AM_LED_NUM			89

#define SIDE_LED_NUM	    70
#define SIGNALRGB_KEY_LED_COUNT   AM_LED_NUM
#define SIGNALRGB_HEAD_LED_COUNT  (HEAD_RGB_LED_X * HEAD_RGB_LED_Y)
#define SIGNALRGB_SIDE_LED_COUNT  SIDE_LED_NUM
#define SIGNALRGB_TOTAL_LED_COUNT (SIGNALRGB_KEY_LED_COUNT + SIGNALRGB_HEAD_LED_COUNT + SIGNALRGB_SIDE_LED_COUNT)

#define CAPSLOCK_COLOR_ON   0xFF,0xFF,0xFF
#define CAPSLOCK_COLOR_OFF  0x00,0x00,0x00
#define CAPSLOCK_COL        7
#define CAPSLOCK_ROW        3
#define CAPSLOCK_LIGHTNESS  200
//帧数据与实际芯片rgb像素点映射表
typedef struct
{
	uint8_t chip_index;		//芯片索引
	uint8_t x;						//在确定芯片上布局排布的x坐标
	uint8_t y;						//在确定芯片上布局排布的y坐标
}rgb_map_t ;


typedef enum
{
    AW_CHIP_INDEX_0,
    AW_CHIP_INDEX_1,
    AW_CHIP_INDEX_2,
    AW_CHIP_INDEX_3,
    AW_CHIP_INDEX_4,
    AW_CHIP_INDEX_5,
    AW_CHIP_INDEX_6
}chip_t;

typedef enum {
    SIGNALRGB_ZONE_KEY = 0,
    SIGNALRGB_ZONE_HEAD,
    SIGNALRGB_ZONE_SIDE,
} signalrgb_zone_t;
/***************************************************************************/
/*****************************key_led_app***********************************/

#define MAX_RECORD_QUEUE_SIZE		5
typedef struct
{
  uint32_t time_count;
    uint16_t key_frame_num;
}mode_para_t;


typedef struct
{
	uint8_t 	start_index;
	uint8_t 	end_index;
	uint32_t time[MAX_RECORD_QUEUE_SIZE];
}key_record_t;


  //触发动作
typedef void(*delay_action_t)(void);
typedef void (*key_led_prase_t)(mode_para_t *mode_para,uint8_t (*data)[KEY_RGB_LED_X_NUM][3],key_record_t (*press_time)[KEY_RGB_LED_X_NUM],key_record_t (*release_time)[KEY_RGB_LED_X_NUM]);
typedef void (*head_led_prase_t)(mode_para_t *mode_para,uint8_t (*data)[HEAD_RGB_LED_X][3],key_record_t (*press_time)[KEY_RGB_LED_X_NUM],key_record_t (*release_time)[KEY_RGB_LED_X_NUM]);

//模式内容结构体
typedef struct key_led_mode
{
	mode_para_t 					 mode_para;			//数据解析可能会用到的参数
	uint16_t 							 scan_base_ms;	//基础扫描时间
	uint16_t 						   scan_step_ms;	//速度步进值
	uint8_t								 pause;
	uint32_t							 delay_ms;
	uint8_t							lightness_index;
	delay_action_t				 action;				//到点动作
	bool 						   		 key_event;			//按键联动，1：支持按键联动，0:不支持
    bool                                speed_en;           //速度调节使能
    bool                            color_en;
	key_led_prase_t 			 key_led_prase;	//数据解析函数指针
	struct key_led_mode 	*next_mode;			//下一个模式指针

} key_led_mode_t;

//模式内容结构体
typedef struct head_led_mode
{
	mode_para_t 					 mode_para;			//数据解析可能会用到的参数
	uint16_t 							 scan_base_ms;	//基础扫描时间
	uint16_t 						   scan_step_ms;	//速度步进值
	uint8_t							lightness_index;
	delay_action_t				 action;				//到点动作
    bool                                speed_en;           //速度调节使能
    bool                            color_en;
	head_led_prase_t 			 head_led_prase;	//数据解析函数指针
	struct head_led_mode_t 	*next_mode;			//下一个模式指针

} head_led_mode_t;

void     signalrgb_mode_set(bool enabled);
bool     signalrgb_mode_enabled(void);
uint16_t signalrgb_total_led_count(void);
bool     signalrgb_apply_zone_stream(signalrgb_zone_t zone, uint8_t start, uint8_t count, const uint8_t *rgb_data);
void     signalrgb_commit_frame(void);



/***************************************************************************/


#endif
