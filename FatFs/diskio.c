/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"		/* Obtains integer types */
#include "diskio.h" /* Declarations of disk functions */
#include "./spi/bsp_spi.h"

/* Definitions of physical drive number for each drive */
#define SPI_NOR_FLASH 0 /* Example: SPI_FLISH to physical drive 0 */

/*-----------------------------------------------------------------------*/
/* 获取设备状态                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status(
	BYTE pdrv /* 标识驱动器的物理驱动器号 */
)
{

	DSTATUS status = STA_NOINIT;

	switch (pdrv)
	{

	case SPI_NOR_FLASH:
		if (SPI_Flash_ReadID() == 0XEF4019)
		{
			status &= ~STA_NOINIT;
		}
		else
		{
			status = STA_NOINIT;
		}
		break;

	default:
		status = STA_NOINIT;
	}
	return status;
}

/*-----------------------------------------------------------------------*/
/* 安装驱动器                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize(
	BYTE pdrv /* 标识驱动器的物理驱动器号 */
)
{
	DSTATUS status = STA_NOINIT;

	switch (pdrv)
	{

	case SPI_NOR_FLASH:

		SPIx_Init();

		for (uint16_t i = 0xFFFF; i > 0; i--)
		{
			/* 延时 */
		}

		Flash_WakeUp();
		status = disk_status(SPI_NOR_FLASH);
		break;

	default:
		status = STA_NOINIT;
	}
	return status;
}

/*-----------------------------------------------------------------------*/
/* 读取扇区                                                       */
/*-----------------------------------------------------------------------*/

DRESULT disk_read(
	BYTE pdrv,	  /* 标识驱动器的物理驱动器号 */
	BYTE *buff,	  /* 数据缓冲区，用于存储读取的数据 */
	LBA_t sector, /* LBA中的起始扇区 */
	UINT count	  /* 要读取的扇区数目 */
)
{
	DRESULT status = RES_PARERR;
	switch (pdrv)
	{

	case SPI_NOR_FLASH:

		sector += 4096;
		Read_Buffer(sector << 12, buff, count << 12);
		status = RES_OK;
		break;

	default:
		status = RES_PARERR;
	}

	return status;
}

/*-----------------------------------------------------------------------*/
/* 写入扇区                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write(
	BYTE pdrv,		  /* Physical drive nmuber to identify the drive */
	const BYTE *buff, /* Data to be written */
	LBA_t sector,	  /* Start sector in LBA */
	UINT count		  /* Number of sectors to write */
)
{
	uint32_t write_addr;
	DRESULT status = RES_PARERR;
	if (!count)
	{
		return RES_PARERR; /* Check parameter */
	}

	switch (pdrv)
	{

	case SPI_NOR_FLASH:

		sector += 4096;
		write_addr = sector << 12;
		SectorErase(write_addr);
		Write_Buffer(write_addr, (uint8_t *)buff, count << 12);
		status = RES_OK;
		break;

	default:
		status = RES_PARERR;
	}

	return status;
}

#endif

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl(
	BYTE pdrv, /* Physical drive nmuber (0..) */
	BYTE cmd,  /* 控制指令 */
	void *buff /* Buffer to send/receive control data */
)
{
	DRESULT status = RES_PARERR;

	switch (pdrv)
	{

	case SPI_NOR_FLASH:
		switch (cmd)
		{
		/* 扇区数量：4096 */
		case GET_SECTOR_COUNT:
			*(DWORD *)buff = 4096;
			break;
		/* 扇区大小  */
		case GET_SECTOR_SIZE:
			*(WORD *)buff = 4096;
			break;
		/* 同时擦除扇区个数 */
		case GET_BLOCK_SIZE:
			*(DWORD *)buff = 1;
			break;
		}
		status = RES_OK;
		break;

	default:
		status = RES_PARERR;
	}
	return status;
}
