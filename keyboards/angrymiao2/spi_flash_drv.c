#include "spi_flash_drv.h"
#include "wait.h"

/* Copyright 2020 Nick Brassel (tzarc)
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "spi_master.h"

#include "timer.h"

#include "SEGGER_RTT.h"
#include "string.h"
#define FLASH_LOG(...) SEGGER_RTT_printf(0, __VA_ARGS__)


static pin_t currentSlavePin = NO_PIN;
static MUTEX_DECL(flashMutex);

#if defined(K20x) || defined(KL2x) || defined(RP2040)
static SPIConfig spiConfig = {NULL, 0, 0, 0};
#else
static SPIConfig spiConfig = {false, NULL, 0, 0, 0, 0};
#endif

void flash_spi_init(void) {
    static bool is_initialised = false;
    if (!is_initialised) {
        is_initialised = true;

        // Try releasing special pins for a short time
        setPinInput(FLASH_SPI_SCK_PIN);
        setPinInput(FLASH_SPI_MOSI_PIN);
        setPinInput(FLASH_SPI_MISO_PIN);

        chThdSleepMilliseconds(10);
#if defined(USE_GPIOV1)
        palSetPadMode(PAL_PORT(FLASH_SPI_SCK_PIN), PAL_PAD(FLASH_SPI_SCK_PIN), SPI_SCK_PAL_MODE);
        palSetPadMode(PAL_PORT(FLASH_SPI_MOSI_PIN), PAL_PAD(FLASH_SPI_MOSI_PIN), SPI_MOSI_PAL_MODE);
        palSetPadMode(PAL_PORT(FLASH_SPI_MISO_PIN), PAL_PAD(FLASH_SPI_MISO_PIN), SPI_MISO_PAL_MODE);
#else
        palSetPadMode(PAL_PORT(FLASH_SPI_SCK_PIN), PAL_PAD(FLASH_SPI_SCK_PIN), SPI_SCK_FLAGS);
        palSetPadMode(PAL_PORT(FLASH_SPI_MOSI_PIN), PAL_PAD(FLASH_SPI_MOSI_PIN), SPI_MOSI_FLAGS);
        palSetPadMode(PAL_PORT(FLASH_SPI_MISO_PIN), PAL_PAD(FLASH_SPI_MISO_PIN), SPI_MISO_FLAGS);
#endif
        spiStop(&FLASH_SPI_DRIVER);
        currentSlavePin = NO_PIN;
    }
}

bool flash_spi_start(pin_t slavePin, bool lsbFirst, uint8_t mode, uint16_t divisor) {
    if (currentSlavePin != NO_PIN || slavePin == NO_PIN) {
        return false;
    }

#if !(defined(WB32F3G71xx) || defined(WB32FQ95xx))
    uint16_t roundedDivisor = 2;
    while (roundedDivisor < divisor) {
        roundedDivisor <<= 1;
    }

    if (roundedDivisor < 2 || roundedDivisor > 256) {
        return false;
    }
#endif

#if defined(K20x) || defined(KL2x)
    spiConfig.tar0 = SPIx_CTARn_FMSZ(7) | SPIx_CTARn_ASC(1);

    if (lsbFirst) {
        spiConfig.tar0 |= SPIx_CTARn_LSBFE;
    }

    switch (mode) {
        case 0:
            break;
        case 1:
            spiConfig.tar0 |= SPIx_CTARn_CPHA;
            break;
        case 2:
            spiConfig.tar0 |= SPIx_CTARn_CPOL;
            break;
        case 3:
            spiConfig.tar0 |= SPIx_CTARn_CPHA | SPIx_CTARn_CPOL;
            break;
    }

    switch (roundedDivisor) {
        case 2:
            spiConfig.tar0 |= SPIx_CTARn_BR(0);
            break;
        case 4:
            spiConfig.tar0 |= SPIx_CTARn_BR(1);
            break;
        case 8:
            spiConfig.tar0 |= SPIx_CTARn_BR(3);
            break;
        case 16:
            spiConfig.tar0 |= SPIx_CTARn_BR(4);
            break;
        case 32:
            spiConfig.tar0 |= SPIx_CTARn_BR(5);
            break;
        case 64:
            spiConfig.tar0 |= SPIx_CTARn_BR(6);
            break;
        case 128:
            spiConfig.tar0 |= SPIx_CTARn_BR(7);
            break;
        case 256:
            spiConfig.tar0 |= SPIx_CTARn_BR(8);
            break;
    }

#elif defined(HT32)
    spiConfig.cr0 = SPI_CR0_SELOEN;
    spiConfig.cr1 = SPI_CR1_MODE | 8; // 8 bits and in master mode

    if (lsbFirst) {
        spiConfig.cr1 |= SPI_CR1_FIRSTBIT;
    }

    switch (mode) {
        case 0:
            spiConfig.cr1 |= SPI_CR1_FORMAT_MODE0;
            break;
        case 1:
            spiConfig.cr1 |= SPI_CR1_FORMAT_MODE1;
            break;
        case 2:
            spiConfig.cr1 |= SPI_CR1_FORMAT_MODE2;
            break;
        case 3:
            spiConfig.cr1 |= SPI_CR1_FORMAT_MODE3;
            break;
    }

    spiConfig.cpr = (roundedDivisor - 1) >> 1;

#elif defined(WB32F3G71xx) || defined(WB32FQ95xx)
    if (!lsbFirst) {
        osalDbgAssert(lsbFirst != FALSE, "unsupported lsbFirst");
    }

    if (divisor < 1) {
        return false;
    }

    spiConfig.SPI_BaudRatePrescaler = (divisor << 2);

    switch (mode) {
        case 0:
            spiConfig.SPI_CPHA = SPI_CPHA_1Edge;
            spiConfig.SPI_CPOL = SPI_CPOL_Low;
            break;
        case 1:
            spiConfig.SPI_CPHA = SPI_CPHA_2Edge;
            spiConfig.SPI_CPOL = SPI_CPOL_Low;
            break;
        case 2:
            spiConfig.SPI_CPHA = SPI_CPHA_1Edge;
            spiConfig.SPI_CPOL = SPI_CPOL_High;
            break;
        case 3:
            spiConfig.SPI_CPHA = SPI_CPHA_2Edge;
            spiConfig.SPI_CPOL = SPI_CPOL_High;
            break;
    }
#elif defined(MCU_RP)
    if (lsbFirst) {
        osalDbgAssert(lsbFirst == false, "RP2040s PrimeCell SPI implementation does not support sending LSB first.");
    }

    // Motorola frame format and 8bit transfer data size.
    spiConfig.SSPCR0 = SPI_SSPCR0_FRF_MOTOROLA | SPI_SSPCR0_DSS_8BIT;
    // Serial output clock = (ck_sys or ck_peri) / (SSPCPSR->CPSDVSR * (1 +
    // SSPCR0->SCR)). SCR is always set to zero, as QMK SPI API expects the
    // passed divisor to be the only value to divide the input clock by.
    spiConfig.SSPCPSR = roundedDivisor; // Even number from 2 to 254

    switch (mode) {
        case 0:
            spiConfig.SSPCR0 &= ~SPI_SSPCR0_SPO; // Clock polarity: low
            spiConfig.SSPCR0 &= ~SPI_SSPCR0_SPH; // Clock phase: sample on first edge
            break;
        case 1:
            spiConfig.SSPCR0 &= ~SPI_SSPCR0_SPO; // Clock polarity: low
            spiConfig.SSPCR0 |= SPI_SSPCR0_SPH;  // Clock phase: sample on second edge transition
            break;
        case 2:
            spiConfig.SSPCR0 |= SPI_SSPCR0_SPO;  // Clock polarity: high
            spiConfig.SSPCR0 &= ~SPI_SSPCR0_SPH; // Clock phase: sample on first edge
            break;
        case 3:
            spiConfig.SSPCR0 |= SPI_SSPCR0_SPO; // Clock polarity: high
            spiConfig.SSPCR0 |= SPI_SSPCR0_SPH; // Clock phase: sample on second edge transition
            break;
    }
#else
    spiConfig.cr1 = 0;

    if (lsbFirst) {
        spiConfig.cr1 |= SPI_CR1_LSBFIRST;
    }

    switch (mode) {
        case 0:
            break;
        case 1:
            spiConfig.cr1 |= SPI_CR1_CPHA;
            break;
        case 2:
            spiConfig.cr1 |= SPI_CR1_CPOL;
            break;
        case 3:
            spiConfig.cr1 |= SPI_CR1_CPHA | SPI_CR1_CPOL;
            break;
    }

    switch (roundedDivisor) {
        case 2:
            break;
        case 4:
            spiConfig.cr1 |= SPI_CR1_BR_0;
            break;
        case 8:
            spiConfig.cr1 |= SPI_CR1_BR_1;
            break;
        case 16:
            spiConfig.cr1 |= SPI_CR1_BR_1 | SPI_CR1_BR_0;
            break;
        case 32:
            spiConfig.cr1 |= SPI_CR1_BR_2;
            break;
        case 64:
            spiConfig.cr1 |= SPI_CR1_BR_2 | SPI_CR1_BR_0;
            break;
        case 128:
            spiConfig.cr1 |= SPI_CR1_BR_2 | SPI_CR1_BR_1;
            break;
        case 256:
            spiConfig.cr1 |= SPI_CR1_BR_2 | SPI_CR1_BR_1 | SPI_CR1_BR_0;
            break;
    }
#endif

    currentSlavePin  = slavePin;
    spiConfig.ssport = PAL_PORT(slavePin);
    spiConfig.sspad  = PAL_PAD(slavePin);

    setPinOutput(slavePin);
    spiStart(&FLASH_SPI_DRIVER, &spiConfig);
    spiSelect(&FLASH_SPI_DRIVER);

    return true;
}

spi_status_t flash_spi_write(uint8_t data) {
    uint8_t rxData;
    spiExchange(&FLASH_SPI_DRIVER, 1, &data, &rxData);

    return rxData;
}

uint8_t flash_spi_read(void) {
    uint8_t data = 0;
    spiReceive(&FLASH_SPI_DRIVER, 1, &data);

    return data;
}

spi_status_t flash_spi_transmit(const uint8_t *data, uint16_t length) {
    spiSend(&FLASH_SPI_DRIVER, length, data);
    return SPI_STATUS_SUCCESS;
}

spi_status_t flash_spi_receive(uint8_t *data, uint16_t length) {
    spiReceive(&FLASH_SPI_DRIVER, length, data);
    return SPI_STATUS_SUCCESS;
}

void flash_spi_stop(void) {
    if (currentSlavePin != NO_PIN) {
        spiUnselect(&FLASH_SPI_DRIVER);
        spiStop(&FLASH_SPI_DRIVER);
        currentSlavePin = NO_PIN;
    }
}





/*******************************************************************************************************************************************/
/*******************************************************************************************************************************************/
/*******************************************************************************************************************************************/
/*******************************************************************************************************************************************/
/*******************************************************************************************************************************************/
/*******************************************************************************************************************************************/


uint8_t spi_size_type=0;//flash型号
uint8_t cmd_data[5];






/*****************************************************************************
函数：static uint8_t SpiFlash_ReadSR(void)
参数：无
返回：状态寄存器值
功能：读寄存器
备注：
书写人：
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
函数:static uint8_t SpiFlash_Wait_Busy(void)
参数:无
返回：忙状态
功能：查询芯片当前是否忙
备注：无
书写人：
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
函数:static uint8_t SpiFlash_Wait_Busy(void)
参数:无
返回：忙状态
功能：查询芯片当前是否忙
备注：主控异步查询
书写人：
******************************************************************************/

uint8_t SpiFlash_Busy_Suspend(void)
{



    if((SpiFlash_ReadSR()&SPIFLASH_WRITE_BUSYBIT) == 0X01)
    {

				return 1;
    }
		else
		{

			return 0;
		}

}





/*****************************************************************************
函数:static  FlashPowerDown(void)
参数:无
返回:无
功能：芯片进入低功耗模式
备注：无
书写人：
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
函数:static  FlashRelease_PowerDown(void)
参数:无
返回:无
功能：芯片跳出低功耗模式
备注：无
书写人：
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
函数:static void SpiFlash_Write_Enable(void)
参数:无
返回:无
功能：写使能
备注：无
书写人：
******************************************************************************/
static void SpiFlash_Write_Enable(void)
{
    if (!flash_spi_start(FLASH_SPI_CS_PIN, false, 0, FLASH_SPI_DIVISOR)) {
        flash_spi_stop();

    }
	cmd_data[0]=SPIFlash_WriteEnable_CMD;
    flash_spi_transmit(cmd_data,1);

    flash_spi_stop();
}



//读取芯片id
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
/*****************************************************************************
函数:uint8_t SpiFlash_Read(uint8_t *pBuffer,uint32_t ReadAddr,uint32_t ReadBytesNum)
参数:预装数据buff,读地址，读数量
返回:
功能：读取flash中的数据
备注：无
书写人：
******************************************************************************/
uint8_t SpiFlash_ReadData(uint8_t *pBuffer,uint32_t ReadAddr,uint16_t ReadBytesNum)
{
    chMtxLock(&flashMutex);
    cmd_data[0]=SPIFlash_ReadData_CMD;
    cmd_data[0+1] = (ReadAddr>>16)&0xff;
    cmd_data[1+1] = (ReadAddr>>8)&0xff;
    cmd_data[2+1] = ReadAddr&0xff;


    if (!flash_spi_start(FLASH_SPI_CS_PIN, false, 0, FLASH_SPI_DIVISOR)) {
        flash_spi_stop();

    }

    if (flash_spi_transmit(cmd_data, 4) != SPI_STATUS_SUCCESS) {
        flash_spi_stop();
        return false;
    }

    if (flash_spi_receive(pBuffer, ReadBytesNum) != SPI_STATUS_SUCCESS) {
        flash_spi_stop();
        return false;
    }


    flash_spi_stop();

    FLASH_LOG("R_addr %x: ",ReadAddr);
	for(uint16_t i=0;i<5;i++)
	{
		FLASH_LOG("%02X",pBuffer[i]);
	}
    FLASH_LOG("\n");
    chMtxUnlock(&flashMutex);
   return RET_SUCCESS;
}


/*****************************************************************************
函数:void SpiFlash_WriteData(uint8_t *pBuffer, uint32_t Addr, uint32_t WriteBytesNum)
参数:要写的buff,地址，数量
返回:
功能：将数据写到flash中去
备注：无
书写人：
******************************************************************************/
void SpiFlash_WriteData(uint8_t *pBuffer, uint32_t Addr, uint32_t WriteBytesNum)
{
    chMtxLock(&flashMutex);
		FLASH_LOG("w_addr %x:",Addr);
        for(uint16_t i=0;i<WriteBytesNum;i++)
        {
            FLASH_LOG("%02X ",pBuffer[i]);
        }
        FLASH_LOG("\n");

		SpiFlash_Write_Enable();



	cmd_data[0] = SPIFlash_PageProgram_CMD;
    cmd_data[1] = (Addr>>16)&0xff;
    cmd_data[2] = (Addr>>8)&0xff;
    cmd_data[3] = Addr&0xff;



  	if (!flash_spi_start(FLASH_SPI_CS_PIN, false, 0, FLASH_SPI_DIVISOR)) {
        flash_spi_stop();

    }
    if (flash_spi_transmit(cmd_data, 4) != SPI_STATUS_SUCCESS) {
        flash_spi_stop();
        return ;
    }

	if (flash_spi_transmit(pBuffer, WriteBytesNum) != SPI_STATUS_SUCCESS) {
        flash_spi_stop();
        return ;
    }

		flash_spi_stop();
		SpiFlash_Wait_Busy();
	chMtxUnlock(&flashMutex);
    return ;
}



/*****************************************************************************
函数:void SpiFlash_Erase_Sector(uint8_t Block_Num,uint8_t Sector_Number)
参数:块位置
返回:页位置
功能：擦除页
备注：无
书写人：
******************************************************************************/
void SpiFlash_Erase_Sector(uint8_t Block_Num,uint8_t Sector_Number)
{
    chMtxLock(&flashMutex);
    SpiFlash_Write_Enable();

	if (!flash_spi_start(FLASH_SPI_CS_PIN, false, 0, FLASH_SPI_DIVISOR)) {
        flash_spi_stop();

    }

	cmd_data[0] = SPIFlash_SectorErase_CMD;
    cmd_data[0+1] = Block_Num;
    cmd_data[1+1] = Sector_Number<<4;
    cmd_data[2+1] = 0;

    if (flash_spi_transmit(cmd_data, 4) != SPI_STATUS_SUCCESS) {
        flash_spi_stop();
        return ;
    }

    flash_spi_stop();
    SpiFlash_Wait_Busy();    //

    chMtxUnlock(&flashMutex);
    return ;
}

//按地址删除扇区
void SpiFlash_EraseSector(uint32_t Addr)
{

  //  uint8_t pcmd[3+1] = {0};
    chMtxLock(&flashMutex);
    SpiFlash_Write_Enable();
    if (!flash_spi_start(FLASH_SPI_CS_PIN, false, 0, FLASH_SPI_DIVISOR)) {
        flash_spi_stop();

    }

	cmd_data[0] =SPIFlash_SectorErase_CMD;
    cmd_data[1] = (Addr>>16)&0xff;
    cmd_data[2] = (Addr>>8)&0xff;
    cmd_data[3] = Addr&0xff;

    if (flash_spi_transmit(cmd_data, 4) != SPI_STATUS_SUCCESS) {
        flash_spi_stop();
        return ;
    }

    flash_spi_stop();
    SpiFlash_Wait_Busy();    //等待写结束
    chMtxUnlock(&flashMutex);
    return ;
}











/*****************************************
函数：uint8_t SpiFlash_Init(void)
参数：无
返回: 1,spi能通讯；0，spi通讯异常
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

	flash_spi_init();

	SpiFlash_WakeUp();

	SpiFlash_Identification(device_id);

//  SpiFlash_Erase_Chip();

//	SpiFlash_Erase_Block(0);

		//SEGGER_RTT_printf(0,"\r\n======flash device_id :0x%x  0x%x  0x%x======\r\n", device_id[0],device_id[1],device_id[2]);

//	SpiFlash_Wait_Busy();

 // flash_test();
    chMtxObjectInit(&flashMutex);
	SpiFlash_PowerDown();//进入低电
	FLASH_LOG("\r\n======flash device_id :0x%x  0x%x  0x%x======\r\n", device_id[0],device_id[1],device_id[2]);
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

	uint8_t data[1024]={0};

	for(uint16_t i=0;i<256;i++)
	{
		data[i]=i&0xff;
	}
	SpiFlash_EraseSector(4096 );
	SpiFlash_WriteData( data, 4096 ,  256);

	for(uint16_t i=0;i<256;i++)
	{
		data[i]=1;
	}
    SpiFlash_WriteData( data, 4096+256 ,  256);

    for(uint16_t i=0;i<256;i++)
	{
		data[i]=2;
	}
    SpiFlash_WriteData( data, 4096+512 ,  256);

    for(uint16_t i=0;i<256;i++)
	{
		data[i]=3;
	}
    SpiFlash_WriteData( data, 4096+768 ,  256);
	/*
	SpiFlash_WriteData( data, 0x10000 ,  64);
	port_delay_us(100);
	SpiFlash_Wait_Busy();


	SpiFlash_ReadData( data, 0x10000, 64);
	//SpiFlash_ReadData( data, 0x12A174, 64);
	*/
	/*
	SpiFlash_WriteData( data, 0x10000 ,  64);
	port_delay_us(100);
	SpiFlash_Wait_Busy();

	memset(data,0,64);
	*/
	memset(data,0,1024);
	SpiFlash_ReadData( data, 4096, 1024);

	//cdc_send_log( data)
  FLASH_LOG("flash_test data[0]=%x  data[1]=%x data[2]=%x data[3]=%x\n", data[0], data[1], data[2], data[3]);
	FLASH_LOG("flash_test data[8]=%x  data[9]=%x data[10]=%x data[11]=%x\n", data[8], data[9], data[10], data[11]);
	FLASH_LOG("flash_test data[60]=%x  data[61]=%x data[62]=%x data[63]=%x\n", data[60], data[61], data[62], data[63]);
    FLASH_LOG("flash_test data[254]=%x  data[255]=%x data[256]=%x data[257]=%x\n", data[254], data[255], data[256], data[257]);
    FLASH_LOG("flash_test data[511]=%x  data[512]=%x data[513]=%x data[514]=%x\n", data[511], data[512], data[513], data[514]);
    FLASH_LOG("flash_test data[1020]=%x  data[1021]=%x data[1022]=%x data[1023]=%x\n", data[1020], data[1021], data[1022], data[1023]);
}




void test_read_id(void)
{
	uint8_t device_id[4];

//	flash_index = DEVICE_SPI_INDEX_0;
	SpiFlash_WakeUp();
	SpiFlash_Identification(device_id);

	FLASH_LOG("\r\n000 ======flash device_id :0x%x  0x%x  0x%x======\r\n", device_id[0],device_id[1],device_id[2]);


	/*
	flash_index = DEVICE_SPI_INDEX_1;
	SpiFlash_WakeUp();
	SpiFlash_Identification(device_id);
	NRF_LOG_INFO("\r\n111 ======flash device_id :0x%x  0x%x  0x%x======\r\n", device_id[0],device_id[1],device_id[2]);
	*/

}







