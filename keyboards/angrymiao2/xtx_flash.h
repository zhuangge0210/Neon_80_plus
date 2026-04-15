/*
文件：txt_flash.h
说明：实现对xtx系列flash以及25系列flash的逻辑控制层在此实现，25与xtx指令通用
      移植平台需要替换的地方：
void write_one_bytle(uint8_t data)
void write_bytles(uint8_t *data,int length)
void read_bytles(uint8_t *data,int length)
void read_bytles(uint8_t *data,int length)
*/
#ifndef _XTX_FLASH_H_
#define _XTX_FLASH_H_

#include "stdint.h"
#include "stdbool.h"
//#include "nrf_delay.h"

#define FLASH_SPI_DRIVER SPID1
#define FLASH_SPI_SCK_PIN 	0xA5
#define FLASH_SPI_MOSI_PIN 	0xA7
#define FLASH_SPI_MISO_PIN 	0xA6
#define FLASH_SPI_CS_PIN    0XA4
#define FLASH_SPI_DIVISOR	4
void flash_spi_init(void);
typedef int16_t spi_status_t;
typedef uint8_t pin_t;
bool flash_spi_start(pin_t slavePin, bool lsbFirst, uint8_t mode, uint16_t divisor);

spi_status_t flash_spi_write(uint8_t data);

uint8_t flash_spi_read(void);

spi_status_t flash_spi_transmit(const uint8_t *data, uint16_t length);

spi_status_t flash_spi_receive(uint8_t *data, uint16_t length);

void flash_spi_stop(void);


typedef enum
{
	ID_XT25F08B,
  ID_XT25F16B
}flash_chip_t;


//指令表，和25系列一毛一样
#define    SPIFLASH_CMD_LENGTH        		  0x03
#define    SPIFLASH_WRITE_BUSYBIT      		  0X01
#define    SPIFlash_ReadData_CMD        		0x03
#define    SPIFlash_WriteEnable_CMD     		0x06
#define    SPIFlash_WriteDisable_CMD    		0x04
#define    SPIFlash_PageProgram_CMD     		0x02
#define    SPIFlash_WriteSR_CMD         		0x01
#define    SPIFlash_ReadSR_CMD          		0x05
#define    SPIFlash_SectorErase_CMD     		0x20
#define    SPIFlash_ChipErase_CMD     			0x60
#define    SPIFlash_32KBlockErase_CMD       0x52
#define    SPIFlash_BlockErase_CMD     		  0xD8
#define    SPIFlash_Identification_CMD  		0x9F
#define    SPIFlash_PowerDown_CMD       		0xB9
#define    SPIFlash_PowerDownRelease_CMD    0xAB 
#define    SPIFlash_PAGEBYTE_LENGTH     		256
#define    SPIFlash_SECBYTE_LENGTH      	  (1024*4)

#define   FLASH_SPI_ENABLE_CS         port_pin_clear(FLASH_CS_PIN)
#define   FLASH_SPI_DISENABLE_CS      port_pin_set(FLASH_CS_PIN)

#define   FLASH_DELAY(us)			        port_delay_us(us);//delay_us(us) 
//1s忙检测超时
#define   TIMEOUT_NUMBER_OF_MS        7000
#define   RET_SUCCESS                 1
#define   RET_WriteCmd_ERROR					0
#define   RET_WaitBusy_ERROR          0



extern uint8_t SpiFlash_ReadData(uint8_t *pBuffer,uint32_t ReadAddr,uint16_t ReadBytesNum);
extern void SpiFlash_WriteData(uint8_t *pBuffer, uint32_t Addr, uint32_t WriteBytesNum);
extern uint8_t SpiFlash_Init(void);
extern void SpiFlash_Identification(uint8_t *pBuffer);
extern void SpiFlash_WakeUp(void);
extern void SpiFlash_PowerDown(void);
extern uint8_t Get_Flash_Typel(void);
extern void SpiFlash_EraseSector(uint32_t Addr);
extern void SpiFlash_Erase_Block(uint8_t BlockNum);

extern uint8_t SpiFlash_Busy_Suspend(void);
extern void SpiFlash_EraseSector_suspend(uint32_t Addr);

extern void SpiFlash_Erase_Block32K_suspend(uint32_t Addr);

//擦除整块芯片
extern void SpiFlash_Erase_Chip(void);

extern void SpiFlash_Erase_Chip_Suspend(void );


extern void test_read_id(void);



#endif
