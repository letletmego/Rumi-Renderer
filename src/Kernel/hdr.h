#ifndef _HDR_H
#define _HDR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


void LoadHDR(char *file_name, float **data_buffer, int *width, int *height);

bool ScanLine(FILE *file_ptr, int scan_width, unsigned char *scan_buffer);

#endif