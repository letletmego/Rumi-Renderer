#include "bmp.h"

unsigned char *LoadBmp(char *file_name, unsigned int *width, unsigned int *height, unsigned int *bytes_per_pixel)
{
	FILE* image_src;

	BmpHeader bmp_header;

	if ((image_src = fopen(file_name,"r + b")) == 0x00)
	{
		printf("%s is loaded error!\n\n", file_name);
		system("pause");
		exit(EXIT_FAILURE);
	}
	else 
	{
		printf("%s is loaded success!\n\n", file_name);
	}

	fread(&bmp_header.bmp_ID, sizeof(BmpID), 1, image_src);

	if (bmp_header.bmp_ID.identifier[0] != 'B' && bmp_header.bmp_ID.identifier[1] != 'M')
		printf("The Image is not bitmap!\n\n");

	fread(&bmp_header.file_header, sizeof(FileHeader), 1, image_src);
	fread(&bmp_header.info_header, sizeof(InfoHeader), 1, image_src);

	*width  = bmp_header.info_header.width;
	*height = bmp_header.info_header.height;
	*bytes_per_pixel = bmp_header.info_header.bits_per_pixel / 8;

	unsigned char *buffer = (unsigned char *)malloc((*width) * (*height) * (*bytes_per_pixel) * sizeof(unsigned char));

	// Load image
	fseek(image_src, bmp_header.file_header.bitmap_data_offset, SEEK_SET);

	if ((bmp_header.info_header.width % 4) == 0)
	{
		fread(buffer, (*bytes_per_pixel) * sizeof(unsigned char), (*width) * (*height), image_src);
	}
	else
	{
		int width_byte_count =  (*width) * (*bytes_per_pixel);
		int offset = bmp_header.info_header.width % 4;

		for (unsigned int height_idx = 0; height_idx < *height; ++height_idx)
		{
			// B G R
			fread(buffer + height_idx * width_byte_count, width_byte_count, 1, image_src);
			fseek(image_src, offset, SEEK_CUR);
		}
	}

	fclose(image_src);

	return buffer;
}

void SaveBmp(unsigned char *_buffer, char *_file_name, const unsigned int _width, const unsigned int _height, const unsigned int _byte_per_pixel)
{
	FILE* image;

	unsigned int data_size = _width * _height * sizeof(unsigned char) * _byte_per_pixel;

	BmpHeader bmp_header;

	bmp_header.bmp_ID.identifier[0] = 'B';
	bmp_header.bmp_ID.identifier[1] = 'M';

	bmp_header.file_header.file_size = 54 + data_size;
	bmp_header.file_header.reserved_1 = 0;
	bmp_header.file_header.reserved_2 = 0;
	bmp_header.file_header.bitmap_data_offset = 54;
	
	bmp_header.info_header.bitmap_header_size = 40;
	bmp_header.info_header.width = _width;
	bmp_header.info_header.height = _height;
	bmp_header.info_header.planes = 1;
	bmp_header.info_header.bits_per_pixel = _byte_per_pixel * 8;
	bmp_header.info_header.compression = 0;
	bmp_header.info_header.bitmap_data_size = data_size;
	bmp_header.info_header.H_resolution = 0;
	bmp_header.info_header.V_resolution = 0;
	bmp_header.info_header.used_colors = 0;
	bmp_header.info_header.important_color = 0;

	image = fopen(_file_name, "wb");

	fwrite(&bmp_header.bmp_ID.identifier, 2, 1, image);
	fwrite(&bmp_header.file_header, 12, 1, image);
	fwrite(&bmp_header.info_header, 40, 1, image);
	fwrite(_buffer, data_size, 1, image);

	fclose(image);

	return;
}

void BmpHeaderInfo(BmpHeader bmp_header)
{
	printf("Bitmap Identifier   : %c %c  \n\n",   bmp_header.bmp_ID.identifier[0], bmp_header.bmp_ID.identifier[1]);

	printf("File size           : %d byte\n",     bmp_header.file_header.file_size);
	printf("Reserved 1          : 0x%08x \n",     bmp_header.file_header.reserved_1);
	printf("Reserved 2          : 0x%08x \n",     bmp_header.file_header.reserved_2);
	printf("Bitmap_data_offset  : 0x%08x \n\n",   bmp_header.file_header.bitmap_data_offset);

	printf("Bitmap_Header_Size  : %d byte \n",    bmp_header.info_header.bitmap_header_size);
	printf("Width               : %d pixel\n",    bmp_header.info_header.width);
	printf("Height              : %d pixel\n",    bmp_header.info_header.height);
	printf("Planes              : %d      \n",    bmp_header.info_header.planes);
	printf("Bit_Per_Pixel       : %d bit  \n",    bmp_header.info_header.bits_per_pixel);
	printf("Compression         : %d      \n",    bmp_header.info_header.compression);
	printf("Bitmap_Data_Size    : %d byte \n",    bmp_header.info_header.bitmap_data_size);
	printf("H_Resolution        : %d pixel/m\n",  bmp_header.info_header.H_resolution);
	printf("V_Resolution        : %d pixel/m\n",  bmp_header.info_header.V_resolution);
	printf("Used_Colored        : %d       \n",   bmp_header.info_header.used_colors);
	printf("Importamt_Colors    : %d       \n\n", bmp_header.info_header.important_color);

	return;
}