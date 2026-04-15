
#include "stdint.h"
#include "SEGGER_RTT.h"
#include "xtx_flash.h"
//#include "port_transfer.h"


uint8_t spi_size_type=0;//flash閿熼叺鐚存嫹
uint8_t cmd_data[5];


#define FLASH_LOG(...) SEGGER_RTT_printf(0, __VA_ARGS__)


void write_one_bytle(uint8_t data)
{
    flash_spi_write(data);
}

uint8_t  read_one_byte(void)
{
	return flash_spi_read();
}
void write_bytles(uint8_t *data,int length)
{
    flash_spi_transmit(data,length);
}
void read_bytles(uint8_t *data,int length)
{
    flash_spi_receive(data,length);
}


static uint8_t SpiFlash_Write_CMD(uint8_t *CMD)
{
    flash_spi_transmit(CMD,SPIFLASH_CMD_LENGTH);
    return RET_SUCCESS;
}


/*****************************************************************************
閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷穝tatic uint8_t SpiFlash_ReadSR(void)
閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹
閿熸枻鎷烽敓鎴綇鎷风姸鎬侀敓渚ヨ揪鎷烽敓鏂ゆ嫹鍊�
閿熸枻鎷烽敓鏉帮綇鎷烽敓鏂ゆ嫹閿熶茎杈炬嫹閿熸枻鎷�
閿熸枻鎷锋敞閿熸枻鎷�
閿熸枻鎷峰啓閿熷壙锝忔嫹
******************************************************************************/

static uint8_t SpiFlash_ReadSR(void)
{
      uint8_t retValue[2] = {0};
    if (!flash_spi_start(FLASH_SPI_CS_PIN, false, 0, FLASH_SPI_DIVISOR)) {
        flash_spi_stop();
        return false;
    }


	cmd_data[0]=SPIFlash_ReadSR_CMD;

    if (flash_spi_transmit(cmd_data, 1) != SPI_STATUS_SUCCESS) {
        flash_spi_stop();
        return false;
    }

    if (flash_spi_receive(retValue, 2) != SPI_STATUS_SUCCESS) {
        flash_spi_stop();
        return false;
    }

    flash_spi_stop();
    return retValue[1];


}


/*****************************************************************************
閿熸枻鎷烽敓鏂ゆ嫹:static uint8_t SpiFlash_Wait_Busy(void)
閿熸枻鎷烽敓鏂ゆ嫹:閿熸枻鎷�
閿熸枻鎷烽敓鎴綇鎷峰繖鐘舵€�
閿熸枻鎷烽敓鏉帮綇鎷烽敓鏂ゆ嫹璇㈣姱鐗囬敓鏂ゆ嫹鍓嶉敓瑙掑嚖鎷峰繖
閿熸枻鎷锋敞閿熸枻鎷烽敓鏂ゆ嫹
閿熸枻鎷峰啓閿熷壙锝忔嫹
******************************************************************************/

static uint8_t SpiFlash_Wait_Busy(void)
{
    uint32_t sDelayCount = 0;
    while((SpiFlash_ReadSR()&SPIFLASH_WRITE_BUSYBIT) == 0X01)
    {
       // SpiFlash_Delay(10);
         wait_us(10);
        if(++sDelayCount > 100*TIMEOUT_NUMBER_OF_MS)
        {
            break;
        }
    }
    return RET_SUCCESS;
}


/*****************************************************************************
閿熸枻鎷烽敓鏂ゆ嫹:static uint8_t SpiFlash_Wait_Busy(void)
閿熸枻鎷烽敓鏂ゆ嫹:閿熸枻鎷�
閿熸枻鎷烽敓鎴綇鎷峰繖鐘舵€�
閿熸枻鎷烽敓鏉帮綇鎷烽敓鏂ゆ嫹璇㈣姱鐗囬敓鏂ゆ嫹鍓嶉敓瑙掑嚖鎷峰繖
閿熸枻鎷锋敞閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓灞婃閿熸枻鎷疯
閿熸枻鎷峰啓閿熷壙锝忔嫹
******************************************************************************/

uint8_t SpiFlash_Busy_Suspend(void)
{
    uint32_t sDelayCount = 0;

//	   SpiFlash_WakeUp();
    if((SpiFlash_ReadSR()&SPIFLASH_WRITE_BUSYBIT) == 0X01)
    {
	//		  SpiFlash_PowerDown();//閿熸枻鎷烽敓鏂ゆ嫹鍋烽敓锟�
				return 1;
    }
		else
		{
	//	  SpiFlash_PowerDown();//閿熸枻鎷烽敓鏂ゆ嫹鍋烽敓锟�
			return 0;
		}

}





/*****************************************************************************
閿熸枻鎷烽敓鏂ゆ嫹:static  FlashPowerDown(void)
閿熸枻鎷烽敓鏂ゆ嫹:閿熸枻鎷�
閿熸枻鎷烽敓鏂ゆ嫹:閿熸枻鎷�
閿熸枻鎷烽敓鏉帮綇鎷疯姱鐗囬敓鏂ゆ嫹閿熸枻鎷峰嚫閿熸枻鎷烽敓渚ワ吉锟�
閿熸枻鎷锋敞閿熸枻鎷烽敓鏂ゆ嫹
閿熸枻鎷峰啓閿熷壙锝忔嫹
******************************************************************************/
void SpiFlash_PowerDown(void)
{
	if (!flash_spi_start(FLASH_SPI_CS_PIN, false, 0, FLASH_SPI_DIVISOR)) {
        flash_spi_stop();

    }

	flash_spi_transmit(cmd_data,1);
	flash_spi_stop();
	wait_us(10);
}

/*****************************************************************************
閿熸枻鎷烽敓鏂ゆ嫹:static  FlashRelease_PowerDown(void)
閿熸枻鎷烽敓鏂ゆ嫹:閿熸枻鎷�
閿熸枻鎷烽敓鏂ゆ嫹:閿熸枻鎷�
閿熸枻鎷烽敓鏉帮綇鎷疯姱鐗囬敓鏂ゆ嫹閿熸枻鎷烽敓閰电櫢鎷烽敓鏂ゆ嫹妯″紡
閿熸枻鎷锋敞閿熸枻鎷烽敓鏂ゆ嫹
閿熸枻鎷峰啓閿熷壙锝忔嫹
******************************************************************************/
void SpiFlash_WakeUp(void)
{
	if (!flash_spi_start(FLASH_SPI_CS_PIN, false, 0, FLASH_SPI_DIVISOR)) {
        flash_spi_stop();

    }


	flash_spi_transmit(cmd_data,1);
	flash_spi_stop();
	wait_us(10);

}

/*****************************************************************************
閿熸枻鎷烽敓鏂ゆ嫹:static void SpiFlash_Write_Enable(void)
閿熸枻鎷烽敓鏂ゆ嫹:閿熸枻鎷�
閿熸枻鎷烽敓鏂ゆ嫹:閿熸枻鎷�
閿熸枻鎷烽敓鏉帮綇鎷峰啓浣块敓鏂ゆ嫹
閿熸枻鎷锋敞閿熸枻鎷烽敓鏂ゆ嫹
閿熸枻鎷峰啓閿熷壙锝忔嫹
******************************************************************************/
static void SpiFlash_Write_Enable(void)
{
    if (!flash_spi_start(FLASH_SPI_CS_PIN, false, 0, FLASH_SPI_DIVISOR)) {
        flash_spi_stop();

    }
    flash_spi_transmit(cmd_data,1);

    flash_spi_stop();
}



//閿熸枻鎷峰彇鑺墖id
void SpiFlash_Identification(uint8_t *pBuffer)
{
    if (!flash_spi_start(FLASH_SPI_CS_PIN, false, 0, FLASH_SPI_DIVISOR)) {
        flash_spi_stop();

    }


	cmd_data[0]=SPIFlash_Identification_CMD;

    if (flash_spi_transmit(cmd_data, 1) != SPI_STATUS_SUCCESS) {
        flash_spi_stop();
        return ;
    }

    if (flash_spi_receive(pBuffer, 3) != SPI_STATUS_SUCCESS) {
        flash_spi_stop();
        return ;
    }



    flash_spi_stop();
    return ;
}


//閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷�
void SpiFlash_Erase_Block32K_suspend(uint32_t Addr )
{
//    uint8_t pcmd[3] = {0};

    SpiFlash_Write_Enable();
    if (!flash_spi_start(FLASH_SPI_CS_PIN, false, 0, FLASH_SPI_DIVISOR)) {
        flash_spi_stop();

    }

	cmd_data[0]=SPIFlash_32KBlockErase_CMD;
  cmd_data[1] = (Addr>>16)&0xff;
  cmd_data[2] = (Addr>>8)&0xff;
  cmd_data[3] = Addr&0xff;

    if (flash_spi_transmit(cmd_data, 4) != SPI_STATUS_SUCCESS) {
        flash_spi_stop();
        return ;
    }
    flash_spi_stop();
  //  SpiFlash_Wait_Busy();

    return ;
}



//閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹鑺墖
void SpiFlash_Erase_Chip(void )
{
		SpiFlash_Write_Enable();
    if (!flash_spi_start(FLASH_SPI_CS_PIN, false, 0, FLASH_SPI_DIVISOR)) {
        flash_spi_stop();

    }
    cmd_data[0] = SPIFlash_ChipErase_CMD;

    if (flash_spi_transmit(cmd_data, 1) != SPI_STATUS_SUCCESS) {
        flash_spi_stop();
        return ;
    }
    flash_spi_stop();
   	SpiFlash_Wait_Busy();
}



//閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹鑺墖,閿熷眾姝�
void SpiFlash_Erase_Chip_Suspend(void )
{
		SpiFlash_Write_Enable();
    if (!flash_spi_start(FLASH_SPI_CS_PIN, false, 0, FLASH_SPI_DIVISOR)) {
        flash_spi_stop();

    }
    cmd_data[0] = SPIFlash_ChipErase_CMD;

    if (flash_spi_transmit(cmd_data, 1) != SPI_STATUS_SUCCESS) {
        flash_spi_stop();
        return ;
    }
    flash_spi_stop();
   	SpiFlash_Busy_Suspend();
}










/*****************************************
閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷穟int8_t SpiFlash_Init(void)
閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹
閿熸枻鎷烽敓鏂ゆ嫹: 1,spi閿熸枻鎷烽€氳閿熸枻鎷�0閿熸枻鎷穝pi閫氳閿熷眾甯�
*****************************************/
/*
#define   SPIFLASH_ID_1 	  0x0b  //MID7-MID0
#define   SPIFLASH_ID_2 	  0x40  //JDID15-JDID8
#define   SPIFLASH_ID_3_1M 	0x14  //JDID7-JDID0 1M=0X14 2M=0X15
#define   SPIFLASH_ID_3_2M 	0x15
*/


#define   SPIFLASH_ID_1 	  0xc8  //MID7-MID0
#define   SPIFLASH_ID_2 	  0x40  //JDID15-JDID8
#define   SPIFLASH_ID_3_1M 	0x14  //JDID7-JDID0 1M=0X14 2M=0X15
#define   SPIFLASH_ID_3_2M 	0x15


uint8_t Get_Flash_Typel(void)
{
 return spi_size_type;
}

void flash_test(void);

uint8_t SpiFlash_Init(void)
{
	uint8_t device_id[4];



	SpiFlash_WakeUp();

	SpiFlash_Identification(device_id);

//  SpiFlash_Erase_Chip();

//	SpiFlash_Erase_Block(0);

		//SEGGER_RTT_printf(0,"\r\n======flash device_id :0x%x  0x%x  0x%x======\r\n", device_id[0],device_id[1],device_id[2]);

//	SpiFlash_Wait_Busy();

 // flash_test();
	SpiFlash_PowerDown();//閿熸枻鎷烽敓鏂ゆ嫹鍋烽敓锟�
	NRF_LOG_INFO("\r\n======flash device_id :0x%x  0x%x  0x%x======\r\n", device_id[0],device_id[1],device_id[2]);
	if((device_id[0] == SPIFLASH_ID_1) && (device_id[1] == SPIFLASH_ID_2))
	{
			if(device_id[2] == SPIFLASH_ID_3_1M)
			{
				spi_size_type = ID_XT25F08B;
			  return 1;
			}
			else if(device_id[2] == SPIFLASH_ID_3_2M)
			{
				spi_size_type = ID_XT25F16B;
			  return 1;
			}
			else
			{
			  return 0;
			}

	}
	else
	{
		return 0;
	}

}


void flash_test(void)
{

	uint8_t data[64]={0};

	for(uint8_t i=0;i<64;i++)
	{
		if(i%2==0)
		{
			data[i]=0x55;
		}
	}
	SpiFlash_EraseSector(0x10000 );
	SpiFlash_Wait_Busy();
	/*
	SpiFlash_WriteData( data, 0x7F2FC ,  64);
	port_delay_us(100);
	SpiFlash_Wait_Busy();


	SpiFlash_ReadData( data, 0x7F2FC, 64);
	//SpiFlash_ReadData( data, 0x12A174, 64);
	*/
	/*
	SpiFlash_WriteData( data, 0x10000 ,  64);
	port_delay_us(100);
	SpiFlash_Wait_Busy();

	memset(data,0,64);
	*/
	SpiFlash_ReadData( data, 0x10000, 64);

	//cdc_send_log( data)
  NRF_LOG_INFO("flash_test data[0]=%x  data[1]=%x data[2]=%x data[3]=%x", data[0], data[1], data[2], data[3]);
	NRF_LOG_INFO("flash_test data[8]=%x  data[9]=%x data[10]=%x data[11]=%x", data[8], data[9], data[10], data[11]);
	NRF_LOG_INFO("flash_test data[60]=%x  data[61]=%x data[62]=%x data[63]=%x", data[60], data[61], data[62], data[63]);
}




void test_read_id(void)
{
	uint8_t device_id[4];

//	flash_index = DEVICE_SPI_INDEX_0;
	SpiFlash_WakeUp();
	SpiFlash_Identification(device_id);

	NRF_LOG_INFO("\r\n000 ======flash device_id :0x%x  0x%x  0x%x======\r\n", device_id[0],device_id[1],device_id[2]);


	/*
	flash_index = DEVICE_SPI_INDEX_1;
	SpiFlash_WakeUp();
	SpiFlash_Identification(device_id);
	NRF_LOG_INFO("\r\n111 ======flash device_id :0x%x  0x%x  0x%x======\r\n", device_id[0],device_id[1],device_id[2]);
	*/

}


