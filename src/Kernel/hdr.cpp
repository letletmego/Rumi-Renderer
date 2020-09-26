#include "hdr.h"


void LoadHDR(char *file_name, float **data_buffer, int *width, int *height)
{
	FILE *file_ptr = fopen(file_name, "rb");

	if (file_ptr == 0x00)
	{
		printf("File is not exist\n");
		exit(EXIT_FAILURE);
	}

	char buffer[256] = { 0x00 };

	fgets(buffer, sizeof(buffer), file_ptr);
	if (strcmp("#?RADIANCE\n", buffer) != 0)
	{
		printf("RADIANCE Error\n");
		fclose(file_ptr);
		return;
	}

	while (file_ptr)
	{
		memset(buffer, 0x00, sizeof(buffer));

		fgets(buffer, sizeof(buffer), file_ptr);

		if (buffer[0] != '#')
			break;
	}

	if (strcmp("FORMAT=32-bit_rle_rgbe\n", buffer) != 0)
	{
		printf("RADIANCE Error\n");
		fclose(file_ptr);
		return;
	}

	while (true)
	{
		fgets(buffer, sizeof(buffer), file_ptr);

		if (memcmp("EXPOSURE", buffer, 8) == 0)
			continue;

		if (memcmp("GAMMA", buffer, 5) == 0)
			continue;

		if (memcmp("\n", buffer, 1) == 0)
			break;
	}

	fgets(buffer, sizeof(buffer), file_ptr);
	sscanf(buffer, "-Y %d + X  %d", height, width);

	*data_buffer = (float *)malloc(sizeof(float) * 3 * (*width) * (*height));
	float *input_data = *data_buffer;
	float f;

	unsigned char *scan_buffer = (unsigned char *)malloc(sizeof(unsigned char) * 4 * (*width));
	unsigned char rgbe[4] = { 0x00 };

	for (int h_idx = 0; h_idx < (*height); ++h_idx)
	{
		if (ScanLine(file_ptr, *width, scan_buffer) == false)
		{
			printf("ScanLine False!\n");
			break;
		}

		for (int idx = 0; idx < *width; ++idx)
		{
			rgbe[0] = *(scan_buffer + idx);
			rgbe[1] = *(scan_buffer + idx + (*width));
			rgbe[2] = *(scan_buffer + idx + (*width) * 2);
			rgbe[3] = *(scan_buffer + idx + (*width) * 3);

			if (rgbe[3])
			{
				f = ldexp(1.0, rgbe[3] - (int)(128 + 8));
				*input_data     = (float)rgbe[0] * f;
				*(++input_data) = (float)rgbe[1] * f;
				*(++input_data) = (float)rgbe[2] * f;
			}
			else
			{
				*input_data     = 0.0f;
				*(++input_data) = 0.0f;
				*(++input_data) = 0.0f;
			}

			++input_data;
		}
	}

	printf("Loading %s Success!\n\n", file_name);

	free(scan_buffer);
	scan_buffer = 0x00;

	fclose(file_ptr);

	return;
}

bool ScanLine(FILE *file_ptr, int scan_width, unsigned char *scan_buffer)
{
	if (scan_width < 8 || 0x7fff < scan_width)
		return false;

	char rgbe[4] = { 0x00 };

	if (fread(rgbe, sizeof(char), 4, file_ptr) < 1)
	{
		printf("fread rgbe Error\n");
		return false;
	}

	if (rgbe[0] != 2 || rgbe[1] != 2 || rgbe[2] & 0x80)
		return false;
	
	if (((int)rgbe[2] << 8 | rgbe[3]) != scan_width)
		return false;

	unsigned char *scan_ptr = scan_buffer;
	unsigned char *scan_ptr_end = 0x00;
	unsigned char temp[2] = { 0x00 };
	int count = 0;

	for (int idx = 0; idx < 4; ++idx)
	{
		scan_ptr_end = (scan_buffer + (idx + 1) * scan_width);

		while (scan_ptr < scan_ptr_end)
		{
			if (fread(temp, sizeof(unsigned char), 2, file_ptr) < 1)
				return false;

			if (temp[0] > 128)
			{
				count = temp[0] - 128;

				if (count == 0 || count > scan_ptr_end - scan_ptr)
					return false;

				while (count > 0)
				{
					*scan_ptr = temp[1];
					++scan_ptr;
					--count;
				}
			}
			else
			{
				count = temp[0];

				if (count == 0 || count > scan_ptr_end - scan_ptr)
					return false;

				*scan_ptr++ = temp[1];

				if (--count > 0)
				{
					if (fread(scan_ptr, sizeof(unsigned char), count, file_ptr) < 1)
						return false;

					scan_ptr += count;
				}
			}
		}
	}

	return true;
}