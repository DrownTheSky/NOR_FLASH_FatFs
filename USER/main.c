#include "stm32f4xx.h"
#include "./usart/bsp_usart.h"
#include "ff.h"
#include <string.h>

void Fatfs_Mkfs(void);
void Get_FatFs_Infor(void);
void FatFs_Read(void);
void FatFs_Write(void);
void FatFs_Read_Write(void);

FATFS fs;
FRESULT fr;
FIL fil;

int main(void)
{
	USART_init();

	fr = f_mount(&fs, "", 1);
	if (fr == 0)
		printf("\nFatFs���سɹ�\n");
	else
		printf("\nFatFs����ʧ��,�������(%d)\n", fr);

	Fatfs_Mkfs();
	Get_FatFs_Infor();
	FatFs_Read_Write();
	Get_FatFs_Infor();
	FatFs_Write();
	FatFs_Read();

	while (1)
	{
	}
}

void Fatfs_Mkfs(void)
{
	fr = f_mkfs("",0,0);

	if (fr == 0)
		printf("\n��ʽ���ɹ�\n");
	else
		printf("\n��ʽ��ʧ��,�������(%d)\n", fr);
}

void Get_FatFs_Infor(void)
{
	FATFS *pfs;
	DWORD fr_clust, fr_sect, tot_sect;

	/* ��ȡ�豸��Ϣ�Ϳմش�С */
	fr = f_getfree("", &fr_clust, &pfs);

	/* ����õ��ܵ����������Ϳ��������� */
	tot_sect = (pfs->n_fatent - 2) * pfs->csize;
	fr_sect = fr_clust * pfs->csize;

	printf("\n�豸�ܿռ䣺%10lu KB��\n���ÿռ䣺  %10lu KB��\n",
		   (unsigned long)tot_sect * 4, (unsigned long)fr_sect * 4);
}

void FatFs_Read_Write(void)
{
	uint8_t Temp = 0;
	UINT bw;
	uint8_t str[100] = "FATFS��д�ļ�����";

	fr = f_open(&fil, "0:Test.txt", FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
	if (fr == 0)
		printf("\n�ļ��򿪳ɹ�\n");
	else
		printf("\n�ļ���ʧ��,�������(%d)\n", fr);

	fr = f_write(&fil, &str, strlen((char *)str), &bw);
	if (fr == 0)
		printf("\n�ļ�д��ɹ�,д���ֽ��� %d \n", bw);
	else
		printf("\n�ļ�д��ʧ��,�������(%d)\n", fr);

	fr = f_lseek(&fil, f_size(&fil));
	if (fr == 0)
		printf("\nƫ�ƹ��ɹ�,ƫ���� %d \n", (int)f_size(&fil));
	else
		printf("\nƫ�ƹ��ʧ��\n");

	Temp += f_printf(&fil, "\n���������\n");
	Temp += f_printf(&fil, "��ǰ�ַ��� ��%d\n", f_size(&fil));
	if (Temp > 0)
		printf("\n��������ݳɹ�,�ֽ���Ϊ ��%d\n", Temp);
	else
		printf("\n���������ʧ��\n");

	fr = f_lseek(&fil, 0);
	if (fr == 0)
		printf("\nƫ�ƹ��ɹ�,ƫ���� %d \n", 0);
	else
		printf("\nƫ�ƹ��ʧ��\n");

	fr = f_read(&fil, &str, f_size(&fil), &bw);
	if (fr == 0)
		printf("\n�ļ���ȡ�ɹ�,��ȡ�ֽ��� %d \n��ȡ���� ��\n%s\n", bw, str);
	else
		printf("\n�ļ�д��ʧ��,�������(%d)\n", fr);

	fr = f_close(&fil);
	if (fr == 0)
		printf("\n�ļ��رճɹ�\n");
	else
		printf("\n�ļ��ر�ʧ��,�������(%d)\n", fr);
}

void FatFs_Read(void)
{
	UINT bw;
	uint8_t str[100];
	
	fr = f_open(&fil, "0:Test.txt", FA_READ);
	if (fr == 0)
		printf("\n�ļ��򿪳ɹ�\n");
	else
		printf("\n�ļ���ʧ��,�������(%d)\n", fr);
	
	fr = f_lseek(&fil, 0);
	if (fr == 0)
		printf("\nƫ�ƹ��ɹ�,ƫ���� %d \n", 0);
	else
		printf("\nƫ�ƹ��ʧ��\n");
	
	fr = f_read(&fil, &str, sizeof(str), &bw);
	if (fr == 0)
		printf("\n�ļ���ȡ�ɹ�,��ȡ�ֽ��� %d \n��ȡ���� ��\n%s\n", bw, str);
	else
		printf("\n�ļ�д��ʧ��,�������(%d)\n", fr);

	fr = f_close(&fil);
	if (fr == 0)
		printf("\n�ļ��رճɹ�\n");
	else
		printf("\n�ļ��ر�ʧ��,�������(%d)\n", fr);
}

void FatFs_Write(void)
{
	UINT bw;
	uint8_t str[] = "123һ����abc";
	
	fr = f_open(&fil, "0:Test.txt", FA_WRITE);
	if (fr == 0)
		printf("\n�ļ��򿪳ɹ�\n");
	else
		printf("\n�ļ���ʧ��,�������(%d)\n", fr);
	
	fr = f_lseek(&fil, f_size(&fil));
	if (fr == 0)
		printf("\nƫ�ƹ��ɹ�,ƫ���� %d \n", (int)f_size(&fil));
	else
		printf("\nƫ�ƹ��ʧ��\n");
	
	fr = f_write(&fil, &str, strlen((char *)str), &bw);
	if (fr == 0)
		printf("\n�ļ�д��ɹ�,д���ֽ��� %d \n", bw);
	else
		printf("\n�ļ�д��ʧ��,�������(%d)\n", fr);

	fr = f_close(&fil);
	if (fr == 0)
		printf("\n�ļ��رճɹ�\n");
	else
		printf("\n�ļ��ر�ʧ��,�������(%d)\n", fr);
}

