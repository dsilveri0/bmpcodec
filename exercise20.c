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
	int red_bitmask;
	int green_bitmask;
	int blue_bitmask;
	int alpha_bitmask;
	int colour_space;
	int redX;
	int redY;
	int redZ;
	int greenX;
	int greenY;
	int greenZ;
	int blueX;
	int blueY;
	int blueZ;
	int red_gamma;
	int green_gamma;
	int blue_gamma;
	int v5_intent;
	int v5_profile_data;
	int v5_profile_size;
	int v5_reserved;
} bmp_header, bmp_header_t;

int calc_rowsize(int bpp, int image_width) {
	// calculating row size based on docs formula
	int row_size = (bpp*image_width)/8;
	
	// aligning row to a multiple of 4 if its not aligned 
	// by adding the necessary bytes as padding
	if(row_size%4!=0){
		int align = 4-row_size%4;
		row_size += align;
	}
	
	return row_size;
}

int calc_arraysize(int row_size, int image_height) {
	// calculating array size based on the row size and height
	return row_size*abs(image_height);
}

/* Read bytes function reads X amount of bytes from a file
 * it receives a file a starting byte and an ending byte.
 * */
BYTE *read_bytes(FILE *ptr, int start_byte, int end_byte) {
	int i,j;
	unsigned int ch;
	// calculating the difference between the start and end bytes and 
	// allocating it as memory for the bytes buffer
	int size = abs(end_byte-start_byte);
	BYTE *bytes = malloc(size);
	
	if(!bytes)
		return NULL;
	
	// reading the first char of the file and then iterating until end 
	// byte has been reached or EOF. If start byte is reached, that byte
	// is stored in the bytes buffer.
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

/* bmp decoder function grabs a .bmp file, and decodes it, by storing the bmp and dib
 * headers in a structure, and saving the pixel array into a seperate file.
 * */
void bmp_decoder(FILE *src_ptr, FILE *dest_ptr, bmp_header_t **pBmp) {	
	
	BYTE *bytes = read_bytes(src_ptr, 0, 138);
	if(bytes) {
		// cast bytes into a struct
		*pBmp = (bmp_header_t*)bytes;
	}
	
	// setting pointer back to the beginning of the file, then read pixel array bytes using the read bytes function
	fseek(src_ptr, 0, SEEK_SET);
	BYTE *pixel_arr_bytes = read_bytes(src_ptr, (*pBmp)->pixel_array_off, (*pBmp)->raw_data_size);
	if(pixel_arr_bytes) {
		// calc the row size and array size using row size and array size functions 
		int row = calc_rowsize((*pBmp)->bpp, (*pBmp)->width);
		int array_size = calc_arraysize(row, (*pBmp)->height);
		
		// writing the pixel array into the dest file
		fwrite(pixel_arr_bytes, array_size, 1, dest_ptr);
	}
	
	// freeing memory allocation for the pixel array size.
	free(pixel_arr_bytes);
}

int bmp_encoder(FILE *src_ptr, bmp_header_t *pBmp) {
	//TODO: Add struct data to a dest file, next add raw pixel data.
	
	FILE* dest_ptr;
	
	// opening file
	dest_ptr = fopen("bmpencoded", "wb");
	if(NULL == dest_ptr)
		return 1;
	
	int row = calc_rowsize(pBmp->bpp, pBmp->width);
	int array_size = calc_arraysize(row, pBmp->height);
		
	pBmp->pixel_array_off = sizeof(bmp_header);
		
	// write header to dest file
	fwrite(pBmp, sizeof(bmp_header), 1, dest_ptr);
	
	// write pixel array to dest file
	int n_read, n_written;
	char buff[array_size];
	do {
		n_read = fread(buff, 1, array_size, src_ptr);
		if(n_read)
			n_written = fwrite(buff, 1, n_read, dest_ptr);
		else
			n_written = 0;
	} while ((n_read > 0) && (n_read == n_written));
	
	//fwrite(src_ptr, array_size, 1, dest_ptr);
	
	// closing file
	fclose(dest_ptr);
	
	return 0;
}


int main(int argc, char **argv)
{
	FILE* ptr_bmp;
	FILE* ptr_qoi;
	FILE* ptr_rawpixeldata;
	FILE* ptr_rawpixeldata_r;
	
	bmp_header_t* pBmp;
	
	// Opening files
	ptr_bmp = fopen(FILEBMP, "rb");
	ptr_qoi = fopen(FILEQOI, "rb");
	ptr_rawpixeldata = fopen(RAWPIXELDATA, "wb");
	ptr_rawpixeldata_r = fopen(RAWPIXELDATA, "rb");
	if(NULL == ptr_bmp || NULL == ptr_rawpixeldata || NULL == ptr_qoi || NULL == ptr_rawpixeldata_r)
		return 1;
	
	// Encoding/Decoding operations for the bmp format
	bmp_decoder(ptr_bmp, ptr_rawpixeldata, &pBmp);
	bmp_encoder(ptr_rawpixeldata_r, pBmp);
	
	// Encoding/Decoding operations for the qoi format	
	/* TODO
	 * Create the qoi encoder and decoder
	 * Create a function bmp->qoi converter that receives a bmp file and converts it to a qoi file.
	 * Create a function qoi->bmp converter that receives a qoi file and converts it to a bmp file.
	*/ 
	
	// Closing files.
	fclose(ptr_rawpixeldata_r);
	fclose(ptr_rawpixeldata);
	fclose(ptr_qoi);
	fclose(ptr_bmp);
	
	// Free memory allocation
	free(pBmp);
	
	return 0;
}
