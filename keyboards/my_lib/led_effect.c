#include "led_effect.h"
#include "app_flash.h"
static const uint8_t gamma_brightness[] =
{
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
	109, 104,  99, 94,  89,  85,  81,  77,
	73,   69,  65, 61,  57,  53,  50,  47,
	44,   41,  38,  35,  32,  29,  26,  24,
	22,   20,  18,  16,  14,  12,  10,  8,
	7,    6,    5,  4,   3,   2,   1,   0,
};

const uint8_t keyboard_x_num[6] = {17,17,17,13,13,10};
const uint8_t key_addr[KEY_RGB_LED_Y_NUM][17][2] = \
{/*x,y对应硬件map的坐标*/
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
    {/*L_SHFT    Z    X                       ......            ?   R_SHIFT    ↑*/
        {4,0},{4,1},{4,2},{4,3},{4,4},{4,5},{4,6},{4,7},{4,8},{4,9},{4,10},{4,11},{5,12},K_NULL,K_NULL,K_NULL,K_NULL,
    },
    {/*ctrl    code   alt                       ......       ↓     ↑ */
        {5,0},{5,1},{5,2},{5,3},{5,10},{5,11},{5,13},{5,8},{5,7},{5,6},K_NULL,K_NULL,K_NULL,K_NULL,K_NULL,K_NULL,K_NULL,
    },
};

const uint8_t keyboard_x_num_2[6] = {17,17,17,17,17,17};

const uint8_t key_addr_2[KEY_RGB_LED_Y_NUM][17][2] = \
{/*x,y对应硬件map的坐标*/
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
    {/*L_SHFT    Z    X                       ......            ?   R_SHIFT    ↑*/
        {4,0},{4,1},{4,2},{4,3},{4,4},{4,5},{4,6},{4,7},{4,8},{4,9},{4,10},{4,11},K_NULL,K_NULL,K_NULL,{5,12},K_NULL,
    },
    {/*ctrl    code   alt                       ......       ↓     ↑ */
        {5,0},{5,1},{5,2},K_NULL,K_NULL,K_NULL,{5,3},K_NULL,K_NULL,K_NULL,K_NULL,{5,10},{5,11},{5,13},{5,8},{5,7},{5,6},
    },
};

static uint8_t brightness_num1=sizeof(gamma_brightness)/2;
static uint8_t brightness_num2=sizeof(gamma_brightness);
static uint16_t color_num=sizeof(color_table)/sizeof(color_table[0]);

static uint8_t index_table[KEY_RGB_LED_Y_NUM][KEY_RGB_LED_X_NUM];
uint8_t side_led_data[SIDE_LED_NUM][3];
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

	memset(data,0xff,KEY_RGB_LED_Y_NUM*KEY_RGB_LED_X_NUM*3);
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
key_record_t key_press_count[KEY_RGB_LED_Y_NUM][KEY_RGB_LED_X_NUM];

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
			else//不相邻，start不需要后移
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

