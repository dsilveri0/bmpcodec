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

typedef unsigned char BYTE;

typedef struct {
	int id;
	int bmp_size;
	int pixel_array_off;
	int dib_header_bytes;
	int width;
	int height;
	int colorplanes;
	int bpp;
	int compression;
	int raw_data_size;
	int image_print_res;
	int ncolors_palette;
	int important_colors;
	int row_size;
	int array_size;
} bmp,  *bmp_format_t;

int calc_rowsize(int bpp, int image_width, int image_height) {	
	int row_size = (bpp*image_width)/8;
	int array_size = row_size*abs(image_height);
	
	printf("%d\n%d\n", row_size, array_size);
	
	if(row_size%4!=0){
		int align = 4-row_size%4;
		row_size += align;
	}
	
	return row_size;
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

int main(int argc, char **argv)
{
	FILE* ptr;
	
	/*
	int ch, chr;
	int inc, inc2 = 0;
	
	
	int bpp = 24;
	int image_width = 326;
	int image_height = 324;
	
	int row_size;
	int array_size;*/
	
	ptr = fopen(FILEBMP, "r");
	if(NULL == ptr)
		return 1;
	
	printf("file contents: \n");	
	BYTE *bytes = read_bytes(ptr, 0, 54);
	if(bytes) {
		
		bmp_format_t pBmp = (bmp_format_t)bytes;
		printf("TESTING: %p ======\n\n",&pBmp[1].id);
		
		// print bytes to verify if its working
		for(int i = 0; i<54; i++) {
			printf("%03x ",i);
			printf("%02x ",bytes[i]);
			printf("%d ",bytes[i]);
			printf("%c\n",bytes[i]);
		}
		// free bytes from memory allocation
		free(bytes);
	}
	
	/*
	do {
		ch = fgetc(ptr);
		
		// Byte 0 to 137 -> Headers
		// Byte 138+ -> Pixel Array
		
		if(inc >= 0 && inc < 138){
			// Begin of print - formatting so its easier to view
			printf("[%d]", inc);
			printf("%02x->", inc);
			printf("%02x", ch);
			if(inc%16==15)
				printf("\n");
			else if(inc%1==0)
				printf(" ");
			//End of print
		}
		//54 -> ... 317658
		if(inc == 0){
			row_size = ((bpp*image_width)/32)*4+1;
			array_size = row_size*abs(image_height);
			
			printf("%d\n%d\n", row_size, array_size);
			
			if(row_size%4!=0){
				int align = 4-row_size%4;
				row_size += align;
			}
			
		}
		int ite = 1;
		if(inc >= 138 && inc <= row_size){
			printf("%02x", ch);
			
			for(int i=0;i<=row_size;i++){
				if(inc==row_size*ite){
					printf("\n\n\n");
				}
				else if(inc%2==1)
				printf(" ");
			ite++;
			}
			
			
			printf("%02x", ch);
			if(inc%8==7)
				printf("\n");
			else if(inc%1==0)
				printf(" ");
				
		}
		inc++;
	} while (ch != EOF);*/
	
	fclose(ptr);
	return 0;
}



































