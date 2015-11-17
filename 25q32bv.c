//在树莓派上面读写winbone 25q32bv flash 芯片的程序
//参数1为输入的文件名，参数2为输出的文件名

#include<stdio.h>
#include<stdlib.h>
#include<wiringPi.h>
#include<wiringPiSPI.h>
#include "25q32bv.h"

typedef unsigned char uchar;
uchar buffer[512];
int channel = 0, clock = 16000000;

int printJEDECID();
int printUNIID();
int waitForDone();
int writeEnable();
int chipErase();
int pageRead(int pageaddr, uchar output[], int lenght);
int pageProgram(int pageaddr, uchar input[], int lenght);

int main(int argc, char *args[])
{
	printf("Starting\n");
	wiringPiSetup();		//初始化spi接口
	int spiinit = 	wiringPiSPISetup(channel, clock);
	if(spiinit == -1)
	{
		printf("Can't init spi");
		return(-1);
	}
	printJEDECID();		//测试输出芯片的一些信息
	printUNIID();

	unsigned char filebuffer[512];
	FILE *infile, *outfile;
	if(argc <= 2)
	{
		return(-1);
	}
	infile = fopen(args[1], "rb");		//打开文件
	outfile = fopen(args[2], "wb");
	if(infile == NULL || outfile == NULL)                                                                                 
	{                                                                                                                     
		printf("files wrong\n");                                                                                          
		return(-1);                                                                                                       
	}       
	fseek(infile, 0, SEEK_END);                                                                                           
	unsigned long filelenght = ftell(infile);                                                                             
	printf("file leng %ld\n", filelenght);                                                                                
	fseek(infile, 0, SEEK_SET);       

	int pageToWrite = (filelenght / 256) + 1;
	printf("PTW: %d\n",pageToWrite);
	chipErase();		//写入数据前先全片擦除
	int readbyte = 0;
	int pagecount, bfcount;
	for(pagecount = 0 ; pagecount <= pageToWrite ; pagecount++)
	{
	readbyte = fread(filebuffer, sizeof(uchar), 256, infile);
	pageProgram(pagecount, filebuffer, readbyte);		//开始写入数据
	for(bfcount = 0 ; bfcount < 256 ; bfcount++)
	{
		filebuffer[bfcount] = 0;
	}
	pageRead(pagecount, filebuffer, readbyte);
	fwrite(filebuffer, sizeof(uchar), readbyte, outfile);
	}
	//	chipErase();
	//	uchar wribuf[256];
	//	int offset = 0;
	//	for(offset = 0; offset < 256 ; offset++)
	//	{
	//		wribuf[offset] = offset;
	//	}
	//	pageProgram(0xa0,wribuf,256);
	//	pageProgram(0xa1,wribuf,256);
	//	uchar readbuf[256];
	//	pageRead(0xa0, readbuf, 256);
	//	for(offset = 0; offset < 256 ; offset++)
	//	{
	//		printf("%x\n",readbuf[offset]);
	//	}
	//	pageRead(0xa1, readbuf, 256);
	//	for(offset = 0; offset < 256 ; offset++)
	//	{
	//		printf("%x\n",readbuf[offset]);
	//	}
	//
	fclose(infile);
	fclose(outfile);
	return(1);
}

int printJEDECID()
{
	buffer[0] = JEDEC_ID;
	wiringPiSPIDataRW(0, buffer, 4); 
	printf("Manufacturer: %x\n",buffer[1]);
	printf("Memory Type: %x\n",buffer[2]);
	printf("Capacity: %x\n",buffer[3]);
	waitForDone();
	return(1);
}

int printUNIID()
{
	buffer[0] = READ_UNI_ID;
	wiringPiSPIDataRW(0, buffer, 6);
	printf("Unique ID: %x\n",buffer[5]);
	waitForDone();
	return(1);
}

int chipErase()
{
	printf("Chip erase\n");
	writeEnable();	//擦除前先向芯片发出写指令
	buffer[0] = CHIP_ERASE;
	wiringPiSPIDataRW(0, buffer, 1);
	waitForDone();
	printf("Done\n");
	return(1);
}

int waitForDone()
{
	uchar operaDone = 1;
	do{
		buffer[0] = READ_STATUS_REG1;
		wiringPiSPIDataRW(0, buffer, 2);
		operaDone = (buffer[1]) & 0x01;
	}while(operaDone == 1);
	return(1);
}

int writeEnable()
{
	buffer[0] = WRITE_ENABLE;
	wiringPiSPIDataRW(0, buffer, 1);
	waitForDone();
	printf("Write Enabled\n");
	return(1);
}

int pageRead(int pageaddr, uchar output[], int lenght)
{
	int count = 0;
	buffer[0] = READ_DATE;
	buffer[1] = (pageaddr >> 8) & 0xff;
	buffer[2] = (pageaddr)	& 0xff;
	buffer[3] = 0;
	wiringPiSPIDataRW(0, buffer, lenght + 4 );
	for(count = 0 ; count < lenght ; count++ )
	{
		output[count] = buffer[count + 4];
	}
	waitForDone();
	return(1);
}

int pageProgram(int pageaddr, uchar input[256], int lenght)
{
	int count = 0;	
	writeEnable();	//读取芯片的指令有很多个，但是写入芯片的指令只有一个pagePrigram页编程指令，并且每次写入一个页之前都要先发送写使能指令
	printf("Program Begin\n");
	buffer[0] = PAGE_PROGRAM;
	buffer[1] = (pageaddr >> 8) & 0xff;
	buffer[2] = (pageaddr)	& 0xff;
	buffer[3] = 0;
	for(count = 0 ; count < lenght ; count++ )
	{
		buffer[count + 4] = input[count];
	}
	wiringPiSPIDataRW(0, buffer, lenght + 4);
	waitForDone();
	printf("Program Done\n");
	return(1);

}
