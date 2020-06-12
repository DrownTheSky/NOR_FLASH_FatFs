#include "./spi/bsp_spi.h"

void SPIx_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    SPI_InitTypeDef SPI_InitStruct;

    RCC_AHB1PeriphClockCmd(SPI_FLASH_CE_GPIO_CLK | SPI_FLASH_SCK_GPIO_CLK |
                               SPI_FLASH_MISO_GPIO_CLK | SPI_FLASH_MOSI_GPIO_CLK,
                           ENABLE);
    RCC_APB2PeriphClockCmd(SPIx_FLASH_CLK, ENABLE);

    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStruct.GPIO_Speed = GPIO_Fast_Speed;

    GPIO_InitStruct.GPIO_Pin = SPI_FLASH_CE_GPIO_PIN;
    GPIO_Init(SPI_FLASH_CE_GPIO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStruct.GPIO_Speed = GPIO_Fast_Speed;

    GPIO_PinAFConfig(SPI_FLASH_SCK_GPIO_PORT, SPI_FLASH_SCK_GPIO_PINSOURCE, SPI_FLASH_SCK_GPIO_AF);
    GPIO_InitStruct.GPIO_Pin = SPI_FLASH_SCK_GPIO_PIN;
    GPIO_Init(SPI_FLASH_SCK_GPIO_PORT, &GPIO_InitStruct);

    GPIO_PinAFConfig(SPI_FLASH_MISO_GPIO_PORT, SPI_FLASH_MISO_GPIO_PINSOURCE, SPI_FLASH_MISO_GPIO_AF);
    GPIO_InitStruct.GPIO_Pin = SPI_FLASH_MISO_GPIO_PIN;
    GPIO_Init(SPI_FLASH_MISO_GPIO_PORT, &GPIO_InitStruct);

    GPIO_PinAFConfig(SPI_FLASH_MOSI_GPIO_PORT, SPI_FLASH_MOSI_GPIO_PINSOURCE, SPI_FLASH_MOSI_GPIO_AF);
    GPIO_InitStruct.GPIO_Pin = SPI_FLASH_MOSI_GPIO_PIN;
    GPIO_Init(SPI_FLASH_MOSI_GPIO_PORT, &GPIO_InitStruct);

    SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
    SPI_InitStruct.SPI_CPHA = SPI_CPHA_2Edge; //第二个边沿
    SPI_InitStruct.SPI_CPOL = SPI_CPOL_High;  //空闲高电平
    SPI_InitStruct.SPI_CRCPolynomial = 0;     //不用校验
    SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStruct.SPI_Mode = SPI_Mode_Master;
    SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;

    SPI_Init(SPIx_FLASH, &SPI_InitStruct);
    SPI_Cmd(SPIx_FLASH, ENABLE);
		
		SPI_FLASH_Mode();
}

static uint8_t SPI_Read_Write(uint8_t Date)
{
    while (!SPI_I2S_GetFlagStatus(SPIx_FLASH, SPI_I2S_FLAG_TXE))
        ;

    SPI_I2S_SendData(SPIx_FLASH, Date);

    while (!SPI_I2S_GetFlagStatus(SPIx_FLASH, SPI_I2S_FLAG_RXNE))
        ;

    return SPI_I2S_ReceiveData(SPIx_FLASH);
}

static void Wait_For_Ready(void)
{
  u8 FLASH_Status = 0;

  /* 选择 FLASH: CS 低 */
  SPI_FLASH_CE_ENABLE;

  /* 发送 读状态寄存器 命令 */
  SPI_Read_Write(READ_STATUS);

  /* 若FLASH忙碌，则等待 */
  do
  {
    /* 读取FLASH芯片的状态寄存器 */
    FLASH_Status = SPI_Read_Write(DUMMY);	 
  }
  while ((FLASH_Status & 0x01) == SET); /* 正在写入标志 */

  /* 停止信号  FLASH: CS 高 */
  SPI_FLASH_CE_DISBALE;
}

static void Write_Enable(void)
{
    SPI_FLASH_CE_ENABLE;
    SPI_Read_Write(WRITE_ENABLE);
    SPI_FLASH_CE_DISBALE;
}

void Flash_WakeUp(void)
{
    SPI_FLASH_CE_ENABLE;
    SPI_Read_Write(ReleasePowerDown);
    SPI_FLASH_CE_DISBALE;
}

void SPI_FLASH_Mode(void)
{
	uint8_t Temp;
	
	/*选择 FLASH: CS 低 */
	SPI_FLASH_CE_ENABLE;
	
	/* 发送状态寄存器 3 命令 */
	SPI_Read_Write(ReadStatusRegister3); 
	
	Temp = SPI_Read_Write(DUMMY);
	
	/* 停止信号 FLASH: CS 高 */
	SPI_FLASH_CE_DISBALE;
	
	if((Temp&0x01) == 0)
	{
		/*选择 FLASH: CS 低 */
		SPI_FLASH_CE_ENABLE;
		
		/* 进入4字节模式 */
		SPI_Read_Write(Enter4ByteMode);
		
		/* 停止信号 FLASH: CS 高 */
		SPI_FLASH_CE_DISBALE;
	}
}

void SectorErase(uint32_t Addr)
{
    Wait_For_Ready();
    Write_Enable();
    SPI_FLASH_CE_ENABLE;

    SPI_Read_Write(SECTOR_ERASE);

    SPI_Read_Write((Addr & 0xFF000000) >> 24);
    SPI_Read_Write((Addr & 0x00FF0000) >> 16);
    SPI_Read_Write((Addr & 0x0000FF00) >> 8);
    SPI_Read_Write((Addr & 0x000000FF) >> 0);

    SPI_FLASH_CE_DISBALE;
		Wait_For_Ready();
}

uint32_t SPI_Flash_ReadID(void)
{
    uint8_t Temp[3];

    Wait_For_Ready();
    SPI_FLASH_CE_ENABLE;

    SPI_Read_Write(JEDEC_ID);
		Temp[0] = SPI_Read_Write(DUMMY);
    Temp[1] = SPI_Read_Write(DUMMY);
    Temp[2] = SPI_Read_Write(DUMMY);

    SPI_FLASH_CE_DISBALE;
		Wait_For_Ready();
	
    return Temp[0] << 16 | Temp[1] << 8 | Temp[2];
}

void Read_Buffer(uint32_t Addr, uint8_t *pDate, uint16_t num)
{
    Wait_For_Ready();
    SPI_FLASH_CE_ENABLE;

    SPI_Read_Write(READ_DATE);
    SPI_Read_Write((Addr & 0xFF000000) >> 24);
    SPI_Read_Write((Addr & 0x00FF0000) >> 16);
    SPI_Read_Write((Addr & 0x0000FF00) >> 8);
    SPI_Read_Write((Addr & 0x000000FF) >> 0);

    while (num--)
    {
        *pDate = SPI_Read_Write(DUMMY);
        pDate++;
    }
		
		
		SPI_FLASH_CE_DISBALE;
		Wait_For_Ready();
}

static void Write_Page(uint32_t Addr, uint8_t *pDate, uint16_t num)
{
    Wait_For_Ready();
    Write_Enable();
		SPI_FLASH_CE_ENABLE;
	
    SPI_Read_Write(PAGE_PROGRAM);
    SPI_Read_Write((Addr & 0xFF000000) >> 24);
    SPI_Read_Write((Addr & 0x00FF0000) >> 16);
    SPI_Read_Write((Addr & 0x0000FF00) >> 8);
    SPI_Read_Write((Addr & 0x000000FF) >> 0);
    while (num--)
    {
        SPI_Read_Write(*pDate);
        pDate++;
    }
		
	  SPI_FLASH_CE_DISBALE;	
		Wait_For_Ready();
}

void Write_Buffer(uint32_t WriteAddr,uint8_t* pBuffer,uint32_t NumByteToWrite)
{
  u8 NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;
	
	/*mod运算求余，若writeAddr是SPI_FLASH_PageSize整数倍，运算结果Addr值为0*/
  Addr = WriteAddr % SPI_FLASH_PageSize;
	
	/*差count个数据值，刚好可以对齐到页地址*/
  count = SPI_FLASH_PageSize - Addr;	
	/*计算出要写多少整数页*/
  NumOfPage =  NumByteToWrite / SPI_FLASH_PageSize;
	/*mod运算求余，计算出剩余不满一页的字节数*/
  NumOfSingle = NumByteToWrite % SPI_FLASH_PageSize;

	 /* Addr=0,则WriteAddr 刚好按页对齐 aligned  */
  if (Addr == 0) 
  {
		/* NumByteToWrite < SPI_FLASH_PageSize */
    if (NumOfPage == 0) 
    {
      Write_Page(WriteAddr, pBuffer, NumByteToWrite);
    }
    else /* NumByteToWrite > SPI_FLASH_PageSize */
    {
			/*先把整数页都写了*/
      while (NumOfPage--)
      {
        Write_Page(WriteAddr, pBuffer, SPI_FLASH_PageSize);
        WriteAddr +=  SPI_FLASH_PageSize;
        pBuffer += SPI_FLASH_PageSize;
      }
			
			/*若有多余的不满一页的数据，把它写完*/
      Write_Page(WriteAddr, pBuffer, NumOfSingle);
    }
  }
	/* 若地址与 SPI_FLASH_PageSize 不对齐  */
  else 
  {
		/* NumByteToWrite < SPI_FLASH_PageSize */
    if (NumOfPage == 0) 
    {
			/*当前页剩余的count个位置比NumOfSingle小，写不完*/
      if (NumOfSingle > count) 
      {
        temp = NumOfSingle - count;
				
				/*先写满当前页*/
        Write_Page(WriteAddr, pBuffer, count);
        WriteAddr +=  count;
        pBuffer += count;
				
				/*再写剩余的数据*/
        Write_Page(WriteAddr, pBuffer, temp);
      }
      else /*当前页剩余的count个位置能写完NumOfSingle个数据*/
      {				
        Write_Page(WriteAddr, pBuffer, NumByteToWrite);
      }
    }
    else /* NumByteToWrite > SPI_FLASH_PageSize */
    {
			/*地址不对齐多出的count分开处理，不加入这个运算*/
      NumByteToWrite -= count;
      NumOfPage =  NumByteToWrite / SPI_FLASH_PageSize;
      NumOfSingle = NumByteToWrite % SPI_FLASH_PageSize;

      Write_Page(WriteAddr, pBuffer, count);
      WriteAddr +=  count;
      pBuffer += count;
			
			/*把整数页都写了*/
      while (NumOfPage--)
      {
        Write_Page(WriteAddr, pBuffer, SPI_FLASH_PageSize);
        WriteAddr +=  SPI_FLASH_PageSize;
        pBuffer += SPI_FLASH_PageSize;
      }
			/*若有多余的不满一页的数据，把它写完*/
      if (NumOfSingle != 0)
      {
        Write_Page(WriteAddr, pBuffer, NumOfSingle);
      }
    }
  }
}

