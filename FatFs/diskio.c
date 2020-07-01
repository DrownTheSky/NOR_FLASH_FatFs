/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2014        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "diskio.h"		/* FatFs lower layer API */
#include "./spi/bsp_spi.h"

/* Definitions of physical drive number for each drive */
#define SPI_NOR_FLASH		0	/* Example: SPI_FLISH to physical drive 0 */


/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
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
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
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
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address in LBA */
	UINT count		/* Number of sectors to read */
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
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _USE_WRITE
DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address in LBA */
	UINT count			/* Number of sectors to write */
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

#if _USE_IOCTL
DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT status = RES_PARERR;

	switch (pdrv)
	{

	case SPI_NOR_FLASH:
		switch (cmd)
		{
		/* 扇区数量 */
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
#endif
