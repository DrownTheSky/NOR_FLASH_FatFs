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
    SPI_InitStruct.SPI_CPHA = SPI_CPHA_2Edge; //�ڶ�������
    SPI_InitStruct.SPI_CPOL = SPI_CPOL_High;  //���иߵ�ƽ
    SPI_InitStruct.SPI_CRCPolynomial = 0;     //����У��
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

  /* ѡ�� FLASH: CS �� */
  SPI_FLASH_CE_ENABLE;

  /* ���� ��״̬�Ĵ��� ���� */
  SPI_Read_Write(READ_STATUS);

  /* ��FLASHæµ����ȴ� */
  do
  {
    /* ��ȡFLASHоƬ��״̬�Ĵ��� */
    FLASH_Status = SPI_Read_Write(DUMMY);	 
  }
  while ((FLASH_Status & 0x01) == SET); /* ����д���־ */

  /* ֹͣ�ź�  FLASH: CS �� */
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
	
	/*ѡ�� FLASH: CS �� */
	SPI_FLASH_CE_ENABLE;
	
	/* ����״̬�Ĵ��� 3 ���� */
	SPI_Read_Write(ReadStatusRegister3); 
	
	Temp = SPI_Read_Write(DUMMY);
	
	/* ֹͣ�ź� FLASH: CS �� */
	SPI_FLASH_CE_DISBALE;
	
	if((Temp&0x01) == 0)
	{
		/*ѡ�� FLASH: CS �� */
		SPI_FLASH_CE_ENABLE;
		
		/* ����4�ֽ�ģʽ */
		SPI_Read_Write(Enter4ByteMode);
		
		/* ֹͣ�ź� FLASH: CS �� */
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
	
	/*mod�������࣬��writeAddr��SPI_FLASH_PageSize��������������AddrֵΪ0*/
  Addr = WriteAddr % SPI_FLASH_PageSize;
	
	/*��count������ֵ���պÿ��Զ��뵽ҳ��ַ*/
  count = SPI_FLASH_PageSize - Addr;	
	/*�����Ҫд��������ҳ*/
  NumOfPage =  NumByteToWrite / SPI_FLASH_PageSize;
	/*mod�������࣬�����ʣ�಻��һҳ���ֽ���*/
  NumOfSingle = NumByteToWrite % SPI_FLASH_PageSize;

	 /* Addr=0,��WriteAddr �պð�ҳ���� aligned  */
  if (Addr == 0) 
  {
		/* NumByteToWrite < SPI_FLASH_PageSize */
    if (NumOfPage == 0) 
    {
      Write_Page(WriteAddr, pBuffer, NumByteToWrite);
    }
    else /* NumByteToWrite > SPI_FLASH_PageSize */
    {
			/*�Ȱ�����ҳ��д��*/
      while (NumOfPage--)
      {
        Write_Page(WriteAddr, pBuffer, SPI_FLASH_PageSize);
        WriteAddr +=  SPI_FLASH_PageSize;
        pBuffer += SPI_FLASH_PageSize;
      }
			
			/*���ж���Ĳ���һҳ�����ݣ�����д��*/
      Write_Page(WriteAddr, pBuffer, NumOfSingle);
    }
  }
	/* ����ַ�� SPI_FLASH_PageSize ������  */
  else 
  {
		/* NumByteToWrite < SPI_FLASH_PageSize */
    if (NumOfPage == 0) 
    {
			/*��ǰҳʣ���count��λ�ñ�NumOfSingleС��д����*/
      if (NumOfSingle > count) 
      {
        temp = NumOfSingle - count;
				
				/*��д����ǰҳ*/
        Write_Page(WriteAddr, pBuffer, count);
        WriteAddr +=  count;
        pBuffer += count;
				
				/*��дʣ�������*/
        Write_Page(WriteAddr, pBuffer, temp);
      }
      else /*��ǰҳʣ���count��λ����д��NumOfSingle������*/
      {				
        Write_Page(WriteAddr, pBuffer, NumByteToWrite);
      }
    }
    else /* NumByteToWrite > SPI_FLASH_PageSize */
    {
			/*��ַ����������count�ֿ������������������*/
      NumByteToWrite -= count;
      NumOfPage =  NumByteToWrite / SPI_FLASH_PageSize;
      NumOfSingle = NumByteToWrite % SPI_FLASH_PageSize;

      Write_Page(WriteAddr, pBuffer, count);
      WriteAddr +=  count;
      pBuffer += count;
			
			/*������ҳ��д��*/
      while (NumOfPage--)
      {
        Write_Page(WriteAddr, pBuffer, SPI_FLASH_PageSize);
        WriteAddr +=  SPI_FLASH_PageSize;
        pBuffer += SPI_FLASH_PageSize;
      }
			/*���ж���Ĳ���һҳ�����ݣ�����д��*/
      if (NumOfSingle != 0)
      {
        Write_Page(WriteAddr, pBuffer, NumOfSingle);
      }
    }
  }
}

