#ifndef _BMP_H
#define _BMP_H

#include <stdio.h>
#include <stdlib.h>

// 2 Byte
typedef struct BmpID
{
	unsigned char identifier[2];
}
BmpID;

// 12 Byte
typedef struct FileHeader
{
	unsigned long file_size;
	unsigned __int16 reserved_1;
	unsigned __int16 reserved_2;
	unsigned long bitmap_data_offset;
}
FileHeader;

// 40 Byte
typedef struct InfoHeader
{
	unsigned long  bitmap_header_size;
	unsigned long  width;
	unsigned long  height;
	unsigned short planes;
	unsigned short bits_per_pixel;
	unsigned long  compression;
	unsigned long  bitmap_data_size;
	unsigned long  H_resolution;
	unsigned long  V_resolution;
	unsigned long  used_colors;
	unsigned long  important_color;
}
InfoHeader;

typedef struct BmpHeader
{
	BmpID bmp_ID;
	FileHeader file_header;
	InfoHeader info_header;
}
BmpHeader;

unsigned char *LoadBmp(char *_file_name, unsigned int *_width, unsigned int *_height, unsigned int *_bytes_per_pixel);

void SaveBmp(unsigned char *_buffer, char *_file_name, const unsigned int _width, const unsigned int _height, const unsigned int _byte_per_pixel);

void BmpHeaderInfo(BmpHeader bmp_header);

#endif //_BMP_H