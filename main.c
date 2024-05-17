
#include <stdio.h>
#include <stdlib.h>

#define FILEBMP "insanecat.bmp"
#define FILEQOI "insanecat.qoi"
#define RAWPIXELDATA "rawpixeldata"
#define BMPENCODED "bmpencoded.bmp"
#define QOIENCODED "qoi.bmp"
#define QOI_COLOR_HASH(C) (C.rgba.r*3 + C.rgba.g*5 + C.rgba.b*7 + C.rgba.a*11)

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


#pragma pack(1)
typedef struct {
	int id;
	int width;
	int height;
	char channels;
	char colour_space;
} qoi_header, qoi_header_t;

// Struct to store RGBA pixel data.
typedef union {
    struct { BYTE r, g, b, a;} rgba;
    unsigned int v;
} qoi_rgba_t;

int calc_rowsize(int bpp, int image_width) {
	// calculating row size based on docs formula
	int row_size = (bpp*image_width)/8;

	// aligning row to a multiple of 4 if it's not aligned
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
 * returns NULL if it errors out reading
 * */

BYTE *read_bytes(FILE *ptr, int start_byte, int end_byte, int *readbytes) {
	int i,k;
	int ch;
	// calculating the difference between the start and end bytes and
	// allocating it as memory for the bytes buffer
	int size = abs(end_byte-start_byte);
	BYTE *bytes = malloc(size);

	if(bytes == NULL)
		return NULL;

	// seeking start_byte on the file then iterating from start byte until end
	// byte has been reached or EOF. Storing the bytes in the bytes buffer.
	fseek(ptr, start_byte, SEEK_SET);
	ch = fgetc(ptr);

	for(i=start_byte, k=0;i<end_byte && ch != EOF;i++) {
		bytes[k] = ch;
		k++;
		ch = fgetc(ptr);
	}

	*readbytes = k;
	bytes = realloc(bytes, k);

    // if there's an error with realloc, return null
    if(bytes == NULL)
        return NULL;

	// if there was an error reading, printing error and freeing bytes' allocation.
	if(ch == EOF){
		if(ferror(ptr)) {
			perror("Error reading.");
			free(bytes);
			return NULL;
		}
	}

	// if bytes requested weren't all read, printing error informing that
	if(size != k)
		perror("Couldn't read all requested bytes");

	return bytes;
}

/* bmp decoder function grabs a .bmp file, and decodes it, by storing the bmp and dib
 * headers in a structure, and saving the pixel array into a separate file.
 * */
void bmp_decoder(FILE *src_ptr, FILE *dest_ptr, bmp_header_t **pBmp) {
	int bytesread = 0;

	// Reading the BMP header's first 14 bytes.
	BYTE *header = read_bytes(src_ptr, 0, 14, &bytesread);
	if(header) {
		// Verifying the size of the header based on the pixel array offset value and
		// casting the header bytes into a struct
		*pBmp = (bmp_header_t*)header;
		BYTE *bytes = read_bytes(src_ptr, 0, (*pBmp)->pixel_array_off, &bytesread);
		if(bytes)
			*pBmp = (bmp_header_t*)bytes;
	}

	// setting pointer back to the beginning of the file, then read pixel array bytes using the read bytes function
	fseek(src_ptr, 0, SEEK_SET);
	BYTE *pixel_arr_bytes = read_bytes(src_ptr, (*pBmp)->pixel_array_off, (*pBmp)->bmp_size, &bytesread);
	if(pixel_arr_bytes) {
		// calc the row size and array size using row size and array size functions
		int row = calc_rowsize((*pBmp)->bpp, (*pBmp)->width);
		int array_size = calc_arraysize(row, (*pBmp)->height);

		// writing the pixel array into the dest file
		int count = 1;
		int n_obj = fwrite(pixel_arr_bytes, array_size, count, dest_ptr);
		if(n_obj != count) {
			//error occurred writing to file
			perror("Error occurred decoding file");
		}
	}

	// freeing memory allocation for the pixel array size.
	if(pixel_arr_bytes != NULL)
		free(pixel_arr_bytes);
}

int bmp_encoder(FILE *src_ptr, bmp_header_t *pBmp) {
	FILE* dest_ptr;

	// opening file in write mode
	dest_ptr = fopen(BMPENCODED, "wb");
	if(NULL == dest_ptr)
		return 1;

	// calculating row and array sizes
	int row = calc_rowsize(pBmp->bpp, pBmp->width);
	int array_size = calc_arraysize(row, pBmp->height);

	pBmp->pixel_array_off = sizeof(bmp_header);

	// writing header to dest file
		int count = 1;
		int n_obj = fwrite(pBmp, sizeof(bmp_header), count, dest_ptr);
		if(n_obj != count) {
			//error occurred writing to file
			perror("Error occurred encoding file");
			return 1;
		}

	// write pixel array to dest file
	int n_read, n_written;
	char buff[array_size];

	// seeking beginning of file before reading
	// then reading the contents until array_size is reached
	fseek(src_ptr, 0, SEEK_SET);
	n_read = fread(buff, 1, array_size, src_ptr);

	// if number of bytes read is equal to the array size, write the buffer to dest file
	if(n_read == array_size){
		n_written = fwrite(buff, 1, n_read, dest_ptr);
		// error handling in case number of bytes read and written are different
		if(n_written != n_read)
			perror("Error occurred encoding file");
	} else {
		// error handling in case number of bytes read is different from the array size
		if (feof(src_ptr))
			printf("Unexpected EOF.");
		else if (ferror(src_ptr))
			printf("Error occurred.");
	}

	// closing file
	fclose(dest_ptr);

	return 0;
}


void qoi_decoder(FILE *src_ptr, FILE *dest_ptr, qoi_header_t **pQoi) {
	int bytesread = 0;
    int run_length = 0;
	int rgb_header_m = 0b11111110;
    int rgba_header_m = 0b11111111;
	int two_header_m = 0b11000000;
    int index_header_m = 0b01000000;
    int diff_header_m = 0b10000000;
    int run_header_m = 0b11000000;
    int run_length_m = 0b00111111;
    qoi_rgba_t index[64];

    // Header doesn't provide info on where the pixel array starts
	BYTE *header = read_bytes(src_ptr, 0, 14, &bytesread);
	if(header) {
        *pQoi = (qoi_header_t*)header;
    }

	fseek(src_ptr, 0, SEEK_SET);
    int qoi_size = (*pQoi)->width * (*pQoi)->height * (*pQoi)->channels; //todo swap end_byte with this value to read the entire file

	BYTE *pixel_arr_bytes = read_bytes(src_ptr, 14, 50000, &bytesread); //todo change so it goes till end byte of file.
    qoi_rgba_t RGBA;
    BYTE *final_arr_bytes;

	for(int i = 0; i<bytesread; i++) {
        // verify if rgb or rgba (from the header info) and if its 1 or the other, store the pixels
        if(pixel_arr_bytes[i] == rgb_header_m) {
            i++;
            RGBA.rgba.r = pixel_arr_bytes[i++];
            RGBA.rgba.g = pixel_arr_bytes[i++];
            RGBA.rgba.b = pixel_arr_bytes[i++];

        } else if(pixel_arr_bytes[i] == rgba_header_m) {
            i++;
            RGBA.rgba.r = pixel_arr_bytes[i++];
            RGBA.rgba.g = pixel_arr_bytes[i++];
            RGBA.rgba.b = pixel_arr_bytes[i++];
            RGBA.rgba.a = pixel_arr_bytes[i++];

        } else if((pixel_arr_bytes[i] & two_header_m) == index_header_m) {
            // If current byte matches mask 11000000 and is equal to the index mask, add byte to index array.
            RGBA = index[pixel_arr_bytes[i]];

        } else if ((pixel_arr_bytes[i] & two_header_m) == diff_header_m) {
            // todo - if current byte matches mask 11000000 and is equal to the diff mask, do index stuff
            printf("do diff stuff");

        //todo also need to do luma mask check on its own else if.
        } else if ((pixel_arr_bytes[i] & two_header_m) == run_header_m) {
            // If current byte matches mask 11000000 and is equal to the op_run mask, get 00111111 bit and store it in run_length
            run_length = (pixel_arr_bytes[i]&run_length_m);
        }
        index[QOI_COLOR_HASH(RGBA) % 64] = RGBA;

        // todo check what/how exactly im gonna store the pixel array into the destination file. (probably similar to bmp decoder)
        /* probably going to make a BYTE *pixels_arr and store all pixels in order into it.
        */

        // continue here...
	}

	if(pixel_arr_bytes) { // todo swap this for the arr_bytes array, or the array with the pixel data whenever i get it
		int count = 1;
		int n_obj = fwrite(pixel_arr_bytes, 50000, count, dest_ptr); //change so it goes till end byte of file.
		if(n_obj != count) {
			//error occurred writing to file
			perror("Error occurred decoding file");
		}
	}

}

int qoi_encoder(FILE *src_ptr, qoi_header_t *pQoi) {
	// todo make the qoi encoder - it receives raw pixel file.
    //  Encoder stuff goes here.
	return 0;
}


int main(int argc, char **argv) {

	FILE* ptr_bmp;
	FILE* ptr_qoi;
	FILE* ptr_rawpixeldata;

	bmp_header_t* pBmp;
	qoi_header_t* pQoi;

	// Opening files
	ptr_bmp = fopen(FILEBMP, "rb");
	ptr_qoi = fopen(FILEQOI, "rb");
	ptr_rawpixeldata = fopen(RAWPIXELDATA, "wb+");
	if(NULL == ptr_bmp || NULL == ptr_rawpixeldata || NULL == ptr_qoi)
		return 1;

	// Encoding/Decoding operations for the bmp format
	//bmp_decoder(ptr_bmp, ptr_rawpixeldata, &pBmp);
	//bmp_encoder(ptr_rawpixeldata, pBmp);

	// Encoding/Decoding operations for the qoi format
	qoi_decoder(ptr_qoi, ptr_rawpixeldata, &pQoi);
	//qoi_encoder(ptr_rawpixeldata, pQoi);

	/* TODO
	 * Create an all purpose function that verifies what file is being given and either converts it to qoi or bmp.
	 * Create a function bmp->qoi converter that receives a bmp file and converts it to a qoi file.
	 * Create a function qoi->bmp converter that receives a qoi file and converts it to a bmp file.
	 * Separate code into different files, and create a seperate .h file to declare functions
	*/

	// Closing files.
	fclose(ptr_rawpixeldata);
	fclose(ptr_qoi);
	fclose(ptr_bmp);

	// Free memory allocation
	//free(pBmp);
	free(pQoi);

	return 0;
}
