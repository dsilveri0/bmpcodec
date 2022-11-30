/*
 * exercise20.c
 * 
 * Copyright 2022 David <dsilverio@endeavour>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */

#include <stdio.h>
#include <stdlib.h>

#define FILEBMP "insanecat.bmp"
#define FILEQOI "insanecat.qoi"
#define RAWPIXELDATA "rawpixeldata"

typedef unsigned char BYTE;

#pragma pack(1)
typedef struct {
	short id;
	int bmp_size;
	short app_spec;
	short app_spec2;
	int pixel_array_off;
	int dib_header_bytes;
	int width;
	int height;
	short colorplanes;
	short bpp;
	int compression;
	int raw_data_size;
	int image_res_hori;
	int image_res_vert;
	int ncolors_palette;
	int important_colors;
} bmp_header, bmp_header_t;

int calc_rowsize(int bpp, int image_width, int image_height) {	
	int row_size = (bpp*image_width)/8;
	
	if(row_size%4!=0){
		int align = 4-row_size%4;
		row_size += align;
	}
	
	return row_size;
}

int calc_arraysize(int row_size, int image_height) {
	return row_size*abs(image_height);
}

BYTE *read_bytes(FILE *ptr, int start_byte, int end_byte) {
	int i,j;
	unsigned int ch;
	int size = abs(end_byte-start_byte);
	BYTE *bytes = malloc(size);
	
	if(!bytes)
		return NULL;
	
	ch = fgetc(ptr);
	for(i=0, j=0;i<end_byte && ch < EOF;i++) {
		if(i >= start_byte) {
			bytes[j] = ch;
			j++;
		}			
		ch = fgetc(ptr);
	}
	
	return bytes;
}

// Refactor into a generic decoder that works with both formats.
void bmp_decoder(FILE *src_ptr, FILE *dest_ptr, bmp_header_t **pBmp) {	
	
	BYTE *bytes = read_bytes(src_ptr, 0, 54);
	if(bytes) {
		// cast bytes into a struct
		*pBmp = (bmp_header_t*)bytes;
	}
	
	// read pixel array bytes
	BYTE *pixel_arr_bytes = read_bytes(src_ptr, (*pBmp)->pixel_array_off, (*pBmp)->raw_data_size);
	if(pixel_arr_bytes) {
		// calc the row size 
		int row = calc_rowsize((*pBmp)->bpp, (*pBmp)->width, (*pBmp)->height);
		
		// use the row size to cal array size
		int array_size = calc_arraysize(row, (*pBmp)->height);
		
		// store pixel array bytes into dest file
		fwrite(pixel_arr_bytes, array_size, 1, dest_ptr);
	}
	
	// free the memory allocation
	free(pixel_arr_bytes);
}

int bmp_encoder(FILE *src_ptr, bmp_header_t *pBmp) {
	// Add struct data to a dest file, next add raw pixel data.
	
	FILE* dest_ptr;
	
	// opening file
	dest_ptr = fopen("bmpencoded", "wb");
	if(NULL == dest_ptr)
		return 1;
	
	// print data from the struct
	printf("%02x\n", pBmp->id);
	printf("%02x\n", pBmp->bmp_size);
	printf("%02x\n", pBmp->app_spec);
	printf("%02x\n", pBmp->app_spec2);
	printf("%02x\n", pBmp->pixel_array_off);
	printf("%02x\n", pBmp->dib_header_bytes);
	printf("%02x\n", pBmp->width);
	printf("%02x\n", pBmp->height);
	printf("%02x\n", pBmp->colorplanes);
	printf("%02x\n", pBmp->bpp);
	printf("%02x\n", pBmp->compression);
	printf("%02x\n", pBmp->raw_data_size);
	printf("%02x\n", pBmp->image_res_hori);
	printf("%02x\n", pBmp->image_res_vert);
	printf("%02x\n", pBmp->ncolors_palette);
	printf("%02x\n", pBmp->important_colors);
	
	int row = calc_rowsize(pBmp->bpp, pBmp->width, pBmp->height);
	int array_size = calc_arraysize(row, pBmp->height);
		
	pBmp->pixel_array_off = sizeof(bmp_header);
		
	// write header to dest file
	fwrite(pBmp, sizeof(bmp_header), 1, dest_ptr);
	fwrite(src_ptr, array_size, 1, dest_ptr);
	
	// closing file
	fclose(dest_ptr);
	
	return 0;
}


int main(int argc, char **argv)
{
	FILE* ptr_bmp;
	FILE* ptr_qoi;
	FILE* ptr_rawpixeldata;
	
	bmp_header_t* pBmp;
	
	// Opening files
	ptr_bmp = fopen(FILEBMP, "r");
	ptr_qoi = fopen(FILEQOI, "r");
	ptr_rawpixeldata = fopen(RAWPIXELDATA, "w+");
	if(NULL == ptr_bmp || NULL == ptr_rawpixeldata || NULL == ptr_qoi)
		return 1;	
	
	// Encoding/Decoding operations
	bmp_decoder(ptr_bmp, ptr_rawpixeldata, &pBmp);	
	bmp_encoder(ptr_rawpixeldata, pBmp);
	
	// Closing files.
	fclose(ptr_rawpixeldata);
	fclose(ptr_qoi);
	fclose(ptr_bmp);
	
	// Free memory allocation
	free(pBmp);
	
	return 0;
}
