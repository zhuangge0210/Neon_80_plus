#include "xtx_flash.h"
#include "app_flash.h"
#include "SEGGER_RTT.h"
#include "string.h"
#include "led_drv.h"
#include "wait.h"
#include "config.h"
#define FLASH_LOG(...) SEGGER_RTT_printf(0, __VA_ARGS__)


user_data_t g_user_data;

enum signalrgb_hid_command_t {
    AM_SIGNALRGB_GET_INFO    = 0x60,
    AM_SIGNALRGB_SET_MODE    = 0x61,
    AM_SIGNALRGB_STREAM_ZONE = 0x62,
    AM_SIGNALRGB_COMMIT_FRAME = 0x63,
};

enum signalrgb_hid_constants_t {
    AM_SIGNALRGB_PROTOCOL_MAJOR = 1,
    AM_SIGNALRGB_PROTOCOL_MINOR = 2,
    AM_SIGNALRGB_PROTOCOL_PATCH = 0,
    AM_SIGNALRGB_UID_1          = 'N',
    AM_SIGNALRGB_UID_2          = '8',
    AM_SIGNALRGB_UID_3          = '0',
};

// static volatile uint8_t g_flash_state = FLASH_IDLE;

void user_date_pack(user_data_t * user_data)
{
	uint16_t sum = 0;
	user_data->used = FLAG_VALID_DATA;
	user_data->head = 0x81;
	user_data->length = sizeof(user_data_t);
	for(uint16_t i=2;i<(sizeof(user_data_t)-2);i++)
	{
		sum+=*((uint8_t* )(user_data)+i);
	}
	user_data->check = sum&0xff;
}

bool user_data_check(user_data_t * user_data)
{
	uint16_t sum = 0;
	if(user_data->head != 0x81)
	{
		return false;
	}
	if(user_data->length!=sizeof(user_data_t))
	{
		return false;
	}
	for(uint16_t i=2;i<(sizeof(user_data_t)-2);i++)
	{
		sum+=*((uint8_t* )(user_data)+i);
	}
	if(user_data->check!=(sum&0xff))
	{
		return false;
	}
	return true;
}


bool pack_check(am_pack_t * pack)
{
	uint16_t sum = 0;
	if(pack->head != 0xf0)
	{
		return false;
	}
	for(uint8_t i=0;i<(sizeof(am_pack_t)-1);i++)
	{
		sum+=*((uint8_t* )(pack)+i);
	}
	if((sum&0xff)!=pack->check)
	{
		FLASH_LOG("check err %02X\n",(sum&0xff));
		return false;
	}
	return true;
}

bool hid_pack(am_pack_t * pack)
{
    uint16_t sum = 0;
    for(uint8_t i=0;i<(sizeof(am_pack_t)-1);i++)
	{
		sum+=*((uint8_t* )(pack)+i);
	}
    pack->check = sum&0xff;
    return true;
}



void save_led_data(am_pack_t * recv_pack)
{
	int write_addr = 0;
	uint8_t write_size = 0;
	int erase_size = 0;
	uint8_t data[32] = {0};
	uint32_t addr =0 ;
	// g_flash_state = FLASH_WRITE;
	if((recv_pack->cmd <CMD_KEY_LED1) || (recv_pack->cmd>CMD_END))
	{
		return;//error para;
	}

	if(recv_pack->pack == 0xff)
	{
		if(CMD_NULL< recv_pack->cmd &&  recv_pack->cmd<CMD_HEAD_LED1)
		{
			g_user_data.key_led[recv_pack->cmd-1].frame_num = (recv_pack->page+1)&0xff;
            g_user_data.key_led[recv_pack->cmd-1].lightness =recv_pack->light;
            g_user_data.key_led[recv_pack->cmd-1].time_interval =recv_pack->interval;
			recv_pack->pack = 0x0b;
			FLASH_LOG("key_led[%d].frame=%d",recv_pack->cmd-1,g_user_data.key_led[recv_pack->cmd-1].frame_num);
		}
		else if(CMD_KEY_LED3< recv_pack->cmd && recv_pack->cmd < CMD_SIDE_LED1)
		{
			g_user_data.head_led[recv_pack->cmd-4].frame_num = (recv_pack->page+1)&0xff;
            g_user_data.head_led[recv_pack->cmd-4].lightness = recv_pack->light;
            g_user_data.head_led[recv_pack->cmd-4].time_interval = recv_pack->interval;
			recv_pack->pack = 0x1c;
			FLASH_LOG("head_led[%d].frame=%d",recv_pack->cmd-1,g_user_data.head_led[recv_pack->cmd-4].frame_num);
		}
        else if(CMD_HEAD_LED3< recv_pack->cmd && recv_pack->cmd < CMD_DEFAT_HEAD1)
		{
			g_user_data.side_led[recv_pack->cmd-7].frame_num = (recv_pack->page+1)&0xff;
            g_user_data.side_led[recv_pack->cmd-7].lightness = recv_pack->light;
            g_user_data.side_led[recv_pack->cmd-7].time_interval = recv_pack->interval;
			recv_pack->pack = 8;
			FLASH_LOG("side_led[%d].frame=%d",recv_pack->cmd-1,g_user_data.side_led[recv_pack->cmd-7].frame_num);
		}
		else
		{

		}
		user_date_pack(&g_user_data);
		save_user_data(&g_user_data);
	}
	FLASH_LOG("cmd:%d  ",recv_pack->cmd);
	switch(recv_pack->cmd)
	{
		case CMD_KEY_LED1:
			write_addr = KEY_LED1_START + recv_pack->page*KEY_LED_PAGE_SIZE + recv_pack->pack*24;
			erase_size = KEY_LED1_SIZE;
			break;
		case CMD_KEY_LED2:
			write_addr = KEY_LED2_START + recv_pack->page*KEY_LED_PAGE_SIZE + recv_pack->pack*24;
			erase_size = KEY_LED2_SIZE;
			break;
		case CMD_KEY_LED3:
			write_addr = KEY_LED3_START + recv_pack->page*KEY_LED_PAGE_SIZE + recv_pack->pack*24;
			erase_size = KEY_LED3_SIZE;
			break;
		case CMD_HEAD_LED1:
			write_addr = HEAD_LED1_START + recv_pack->page*HEAD_LED_PAGE_SIZE + recv_pack->pack*24;
			erase_size = HEAD_LED1_SIZE;
			break;
		case CMD_HEAD_LED2:
			write_addr = HEAD_LED2_START + recv_pack->page*HEAD_LED_PAGE_SIZE + recv_pack->pack*24;
			erase_size = HEAD_LED2_SIZE;
			break;
		case CMD_HEAD_LED3:
			write_addr = HEAD_LED3_START + recv_pack->page*HEAD_LED_PAGE_SIZE + recv_pack->pack*24;
			erase_size = HEAD_LED3_SIZE;
			break;
        case CMD_SIDE_LED1:
            write_addr = SIDE_LED1_START + recv_pack->page*SIDE_LED_PAGE_SIZE + recv_pack->pack*24;
			erase_size = SIDE_LED1_SIZE;
            break;
        case CMD_SIDE_LED2:
            write_addr = SIDE_LED2_START + recv_pack->page*SIDE_LED_PAGE_SIZE + recv_pack->pack*24;
			erase_size = SIDE_LED2_SIZE;
            break;
        case CMD_SIDE_LED3:
            write_addr = SIDE_LED3_START + recv_pack->page*SIDE_LED_PAGE_SIZE + recv_pack->pack*24;
			erase_size = SIDE_LED3_SIZE;
            break;
        case CMD_DEFAT_HEAD1:
            write_addr = HEAD_DEFAULT1_START + recv_pack->page*HEAD_LED_PAGE_SIZE + recv_pack->pack*24;
			erase_size = SIDE_LED3_SIZE;
            break;
        case CMD_DEFAT_HEAD2:
            write_addr = HEAD_DEFAULT2_START + recv_pack->page*HEAD_LED_PAGE_SIZE + recv_pack->pack*24;
			erase_size = SIDE_LED3_SIZE;
            break;
        case CMD_DEFAT_SIDE1:
            write_addr = SIDE_DEFAULT1_START + recv_pack->page*SIDE_LED_PAGE_SIZE + recv_pack->pack*24;
			erase_size = SIDE_LED3_SIZE;
            break;
        case CMD_DEFAT_SIDE2:
            write_addr = SIDE_DEFAULT2_START + recv_pack->page*SIDE_LED_PAGE_SIZE + recv_pack->pack*24;
			erase_size = SIDE_LED3_SIZE;
            break;
		default:
			break;
	}
	if(recv_pack->page == 0 && recv_pack->pack == 0)
	{
		for(int i=0;i<(erase_size);i+=4096)
		{
			FLASH_LOG("%d :",write_addr+i);
			SpiFlash_EraseSector(write_addr+i);

		}
	}

	write_size = recv_pack->length;
	memcpy(data,recv_pack->data,write_size);
	addr = write_addr;

	for(uint8_t i=0;i<6;i++)
	{
		SpiFlash_WriteData(&data[i*4], addr,4);
		addr+=4;
	}

}



uint8_t read_buffer[HEAD_LED_PAGE_SIZE];
void read_led_data(uint8_t led_num,uint8_t *led_data,uint8_t frame)
{
	int read_addr = 0;
	uint16_t read_size = 0;

	uint16_t copy_size = 0;
	switch(led_num)
	{
		case CMD_KEY_LED1:
			read_addr = KEY_LED1_START + frame*KEY_LED_PAGE_SIZE;
			read_size = KEY_LED_PAGE_SIZE;
			copy_size = AM_LED_NUM*3;
			break;
		case CMD_KEY_LED2:
			read_addr = KEY_LED2_START + frame*KEY_LED_PAGE_SIZE;
			read_size = KEY_LED_PAGE_SIZE;
			copy_size = AM_LED_NUM*3;
			break;
		case CMD_KEY_LED3:
			read_addr = KEY_LED3_START + frame*KEY_LED_PAGE_SIZE;
			read_size = KEY_LED_PAGE_SIZE;
			copy_size = AM_LED_NUM*3;
			break;
		case CMD_HEAD_LED1:
			read_addr = HEAD_LED1_START + frame*HEAD_LED_PAGE_SIZE;
			read_size = HEAD_LED_PAGE_SIZE;
			copy_size = HEAD_RGB_LED_X*HEAD_RGB_LED_Y*3;
			break;
		case CMD_HEAD_LED2:
			read_addr = HEAD_LED2_START + frame*HEAD_LED_PAGE_SIZE;
			read_size = HEAD_LED_PAGE_SIZE;
			copy_size = HEAD_RGB_LED_X*HEAD_RGB_LED_Y*3;
			break;
		case CMD_HEAD_LED3:
			read_addr = HEAD_LED3_START + frame*HEAD_LED_PAGE_SIZE;
			read_size = HEAD_LED_PAGE_SIZE;
			copy_size = HEAD_RGB_LED_X*HEAD_RGB_LED_Y*3;
			break;
        case CMD_SIDE_LED1:
			read_addr = SIDE_LED1_START + frame*SIDE_LED_PAGE_SIZE;
			read_size = SIDE_LED_PAGE_SIZE;
			copy_size = SIDE_LED_NUM*3;       /*70个灯*/
			break;
        case CMD_SIDE_LED2:
			read_addr = SIDE_LED2_START + frame*SIDE_LED_PAGE_SIZE;
			read_size = SIDE_LED_PAGE_SIZE;
			copy_size = SIDE_LED_NUM*3;       /*70个灯*/
			break;
        case CMD_SIDE_LED3:
			read_addr = SIDE_LED3_START + frame*SIDE_LED_PAGE_SIZE;
			read_size = SIDE_LED_PAGE_SIZE;
			copy_size = SIDE_LED_NUM*3;       /*70个灯*/
			break;
        case CMD_DEFAT_HEAD1:
			read_addr = HEAD_DEFAULT1_START + frame*HEAD_LED_PAGE_SIZE;
			read_size = HEAD_LED_PAGE_SIZE;
			copy_size = HEAD_RGB_LED_X*HEAD_RGB_LED_Y*3;
			break;
		case CMD_DEFAT_HEAD2:
			read_addr = HEAD_LED2_START + frame*HEAD_LED_PAGE_SIZE;
			read_size = HEAD_LED_PAGE_SIZE;
			copy_size = HEAD_RGB_LED_X*HEAD_RGB_LED_Y*3;
			break;
        case CMD_DEFAT_SIDE1:
			read_addr = SIDE_DEFAULT1_START + frame*SIDE_LED_PAGE_SIZE;
			read_size = SIDE_LED_PAGE_SIZE;
			copy_size = SIDE_LED_NUM*3;       /*70个灯*/
			break;
        case CMD_DEFAT_SIDE2:
			read_addr = SIDE_DEFAULT2_START + frame*SIDE_LED_PAGE_SIZE;
			read_size = SIDE_LED_PAGE_SIZE;
			copy_size = SIDE_LED_NUM*3;       /*70个灯*/
			break;
		default:
			break;
	}
	SpiFlash_ReadData(read_buffer,read_addr,read_size);
	// FLASH_LOG("R_addr %x: ",ReadAddr);
    if(led_num == CMD_SIDE_LED1)
    {
        for(uint16_t i=0;i<copy_size;i++)
        {
            FLASH_LOG("%02X ",read_buffer[i]);
        }
        FLASH_LOG("\n");
    }

	memcpy(led_data,read_buffer,copy_size);
	// FLASH_LOG("frame:%d addr:%x:",frame,read_addr);

	FLASH_LOG("\n");
}

static bool signalrgb_raw_hid_receive_kb(uint8_t *data, uint8_t length)
{
    bool handled = true;

    if (length != 32) {
        return false;
    }

    switch (data[0]) {
        case AM_SIGNALRGB_GET_INFO:
            memset(data, 0, length);
            data[0]  = AM_SIGNALRGB_GET_INFO;
            data[1]  = AM_SIGNALRGB_PROTOCOL_MAJOR;
            data[2]  = AM_SIGNALRGB_PROTOCOL_MINOR;
            data[3]  = AM_SIGNALRGB_PROTOCOL_PATCH;
            data[4]  = AM_SIGNALRGB_UID_1;
            data[5]  = AM_SIGNALRGB_UID_2;
            data[6]  = AM_SIGNALRGB_UID_3;
            data[7]  = SIGNALRGB_KEY_LED_COUNT;
            data[8]  = SIGNALRGB_HEAD_LED_COUNT;
            data[9]  = SIGNALRGB_SIDE_LED_COUNT;
            data[10] = signalrgb_mode_enabled() ? 1 : 0;
            data[11] = SOFT_VER_0;
            data[12] = SOFT_VER_1;
            data[13] = SOFT_VER_2;
            break;

        case AM_SIGNALRGB_SET_MODE:
            signalrgb_mode_set(data[1] != 0);
            memset(data, 0, length);
            data[0] = AM_SIGNALRGB_SET_MODE;
            data[1] = signalrgb_mode_enabled() ? 1 : 0;
            break;

        case AM_SIGNALRGB_STREAM_ZONE: {
            const uint8_t zone      = data[1];
            const uint8_t start_led = data[2];
            const uint8_t led_count = data[3];
            bool          ok        = false;

            if (led_count <= 14) {
                ok = signalrgb_apply_zone_stream((signalrgb_zone_t)zone, start_led, led_count, &data[4]);
            }

            memset(data, 0, length);
            data[0] = AM_SIGNALRGB_STREAM_ZONE;
            data[1] = zone;
            data[2] = start_led;
            data[3] = led_count;
            data[4] = ok ? 1 : 0;
            break;
        }

        case AM_SIGNALRGB_COMMIT_FRAME:
            signalrgb_commit_frame();
            memset(data, 0, length);
            data[0] = AM_SIGNALRGB_COMMIT_FRAME;
            data[1] = 1;
            break;

        default:
            handled = false;
            break;
    }

    return handled;
}


void raw_hid_receive_kb(uint8_t *data, uint8_t length)
{
	am_pack_t pack;
	for(uint8_t i=0;i<length;i++)
	{
		FLASH_LOG("%02X ",*(data+i));
	}
	FLASH_LOG("\n");
    if (signalrgb_raw_hid_receive_kb(data, length))
    {
        return;
    }
	if(length != 32)
	{
		return ;
	}
	memcpy(&pack,data,32);

	if (false == pack_check(&pack))
	{
		return ;
	}
	FLASH_LOG("check_ok\n");
    if((pack.cmd <CMD_OTHER_CMD) && (pack.cmd>CMD_NULL))
    {
        save_led_data(&pack);
    }
	else
    {
        // FLASH_LOG("check_1\n");
        if(pack.cmd == CMD_OTHER_CMD)
        {
            if(pack.page == 1)
            {
                // FLASH_LOG("check_2\n");
                pack.pack = SOFT_VER_0;
                pack.light = SOFT_VER_1;
                pack.interval = SOFT_VER_2;
            }
        }
    }
    hid_pack(&pack);
    memcpy(data,&pack,32);
}



bool read_user_data(user_data_t *user) {
    uint8_t data[FLASH_DATA_SIZE];
    bool found = false;
    uint32_t offset = 0;
    for ( offset = 0; offset < FLASH_SECTOR_SIZE; offset += FLASH_DATA_SIZE) {
        if (!SpiFlash_ReadData(data, offset , FLASH_DATA_SIZE)) {
            return false; // 读取失败
        }

        if (data[0] == FLAG_VALID_DATA) {
            memcpy(user, data, sizeof(user_data_t));
            found = true;
            break;
        }
    }
    FLASH_LOG("addr:%x ",offset);
    for(uint16_t i=0;i<FLASH_DATA_SIZE;i++)
	{
		FLASH_LOG("%02X ",data[i]);
	}

    return found;
}


bool save_user_data(user_data_t *user) {
    uint8_t data[FLASH_DATA_SIZE];
    memcpy(data, user, sizeof(user_data_t));
    data[0] = FLAG_VALID_DATA; // 标记数据为最新
    uint32_t addr=0;
    uint32_t available_offset = 0xffffffff;
    uint32_t offset = 0;
    uint8_t i=0;
    for (offset = USER_DATA_START; offset < (FLASH_SECTOR_SIZE-FLASH_DATA_SIZE-1); offset += FLASH_DATA_SIZE) {
        uint8_t current_data[FLASH_DATA_SIZE];
        if (!SpiFlash_ReadData(current_data,offset , FLASH_DATA_SIZE)) {
            FLASH_LOG("read failed 1\n");
            return false; // 读取失败
        }
        // wait_ms(5);
        if (current_data[0] == FLAG_VALID_DATA ) {
			current_data[0] = FLAG_INVALID_DATA;
			available_offset = offset+FLASH_DATA_SIZE;
			// SpiFlash_WriteData(data, available_offset, FLASH_DATA_SIZE);		//写新数据
            if (!SpiFlash_ReadData(current_data,available_offset , FLASH_DATA_SIZE)) {
                FLASH_LOG("read failed 2\n");
                return false; // 读取失败
            }
            for( i=0;i<FLASH_DATA_SIZE;i++)
            {
                if(current_data[i] != 0xff)
                {//数据无效，继续搜索
                    break;
                }
            }
            if(i != FLASH_DATA_SIZE)
            {
                current_data[0] = FLAG_INVALID_DATA;
                addr = offset;
                for( i=0;i<(FLASH_DATA_SIZE/4);i++)
                {
                    SpiFlash_WriteData(&current_data[i*4], addr,4);
                    addr+=4;
                }
                if(FLASH_DATA_SIZE%4)
                {
                    SpiFlash_WriteData(&current_data[i*4],addr,FLASH_DATA_SIZE%4);
                }
                continue; ////数据非全ff，继续搜索
            }
            addr = available_offset;
            for( i=0;i<(FLASH_DATA_SIZE/4);i++)
            {
                SpiFlash_WriteData(&data[i*4], addr,4);
                addr+=4;
            }
            if(FLASH_DATA_SIZE%4)
            {
                SpiFlash_WriteData(&data[i*4],addr,FLASH_DATA_SIZE%4);
            }
			// SpiFlash_WriteData(current_data, offset, FLASH_DATA_SIZE);			//旧数据写成已读
            addr = offset;
            current_data[0] = FLAG_INVALID_DATA;
            for( i=0;i<(FLASH_DATA_SIZE/4);i++)
            {
                SpiFlash_WriteData(&current_data[i*4], addr,4);
                addr+=4;
            }
            if(FLASH_DATA_SIZE%4)
            {
                SpiFlash_WriteData(&current_data[i*4],addr,FLASH_DATA_SIZE%4);
            }
            FLASH_LOG("write success 1\n");
            return true;
        }
		if(current_data[0] == 0xff)
		{
			uint8_t i=0;
			for( i=0;i<FLASH_DATA_SIZE;i++)
			{
				if(current_data[i] != 0xff)
				{//数据无效，继续搜索
					break;
				}

			}


			if(i == FLASH_DATA_SIZE)
			{
				available_offset = offset;
				// SpiFlash_WriteData(data, available_offset, FLASH_DATA_SIZE);		//写新数据
                addr = available_offset;
                for( i=0;i<(FLASH_DATA_SIZE/4);i++)
                {
                    SpiFlash_WriteData(&data[i*4], addr,4);
                    addr+=4;
                }
                if(FLASH_DATA_SIZE%4)
                {
                    SpiFlash_WriteData(&data[i*4],addr,FLASH_DATA_SIZE%4);
                }
			}
            else
            {
                current_data[0] = FLAG_INVALID_DATA;//把该区域标记为已用
                addr = offset;
                for( i=0;i<(FLASH_DATA_SIZE/4);i++)
                {
                    SpiFlash_WriteData(&current_data[i*4], addr,4);
                    addr+=4;
                }
                if(FLASH_DATA_SIZE%4)
                {
                    SpiFlash_WriteData(&current_data[i*4],addr,FLASH_DATA_SIZE%4);
                }
                continue; ////数据非全ff，继续搜索
            }
            FLASH_LOG("write success 2\n");
            return true;

		}
    }

    if (offset > (FLASH_SECTOR_SIZE-USER_DATA_START-FLASH_DATA_SIZE-1)) {
        // Flash 存储已满或无效数据块
        FLASH_LOG("use full! need erase sector 1\n");
		SpiFlash_EraseSector(USER_DATA_START);
		available_offset = 0;
		// SpiFlash_WriteData(data, available_offset, FLASH_DATA_SIZE);
        addr = available_offset;
        for( i=0;i<(FLASH_DATA_SIZE/4);i++)
        {
            SpiFlash_WriteData(&data[i*4], addr,4);
            addr+=4;
        }
        if(FLASH_DATA_SIZE%4)
        {
            SpiFlash_WriteData(&data[i*4],addr,FLASH_DATA_SIZE%4);
        }

        return true;
    }

    FLASH_LOG("write failed 3\n");
    return true;
}
