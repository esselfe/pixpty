#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <GL/gl.h>
#include <setjmp.h>
#include <png.h>

GLubyte *ImageFromPNGFile(unsigned int width, unsigned int height, char *filename) {
	FILE *fp = fopen(filename, "rb");
	if (fp == NULL) {
		printf("pixpty::ImageFromPNGFile() error: Cannot open %s: %s\n", filename, strerror(errno));
		return NULL;
	}
	
	unsigned char sig[8];
	fread(sig, 1, 8, fp);
	if (!png_check_sig(sig, 8)) {
		printf("pixpty::ImageFromPNGFile() error: PNG signature check failed: %s\n", filename);
		fclose(fp);
		return NULL;
	}
	
	png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	png_infop info = png_create_info_struct(png);
	if (setjmp(png_jmpbuf(png))) {
		png_destroy_read_struct(&png, &info, NULL);
		fclose(fp);
		return NULL;
	}
	png_set_sig_bytes(png, 8);
	png_init_io(png, fp);
	png_read_png(png, info, 0, 0);
	
	unsigned int components = 0, color_type = png_get_color_type(png, info);
	switch(color_type) {
	case PNG_COLOR_TYPE_GRAY:
		components = 1;
		break;
	case PNG_COLOR_TYPE_PALETTE:
		printf("pixpty::ImageFromPNGFile() error: PNG color type palette/indexed is not supported yet\n");
		png_destroy_read_struct(&png, &info, NULL);
		fclose(fp);
		return NULL;
	case PNG_COLOR_TYPE_RGB:
		components = 3;
		break;
	case PNG_COLOR_TYPE_RGB_ALPHA:
		components = 4;
		break;
	case PNG_COLOR_TYPE_GRAY_ALPHA:
		components = 2;
		break;
	default:
		printf("pixpty::ImageFromPNGFile() error: Unknown png color type: %u\n", color_type);
		png_destroy_read_struct(&png, &info, NULL);
		fclose(fp);
		return NULL;
	}
	
	GLubyte *buf;
	//if (components == 1 || components == 3)
	//	buf = malloc(width*height*3);
	//else
	buf = malloc(width*height*4);
	png_bytepp rows = png_get_rows(png, info);
	png_bytep row;
	int x, y, cnt = 0;
	for (y = 0; y < height; y++) {
		row = rows[y];
		if (components == 1) {
			for (x=0; x < width; x++, cnt += 4) {
				buf[cnt] = row[x];
				buf[cnt+1] = row[x];
				buf[cnt+2] = row[x];
				buf[cnt+3] = 0xff;
			}
		}
		else if (components == 2) {
			for (x=0; x < width*2; x += 2, cnt += 4) {
				buf[cnt] = row[x];
				buf[cnt+1] = row[x];
				buf[cnt+2] = row[x];
				buf[cnt+3] = row[x+1];
			}
		}
		else if (components == 3) {
			for (x=0; x < width*3; x += 3, cnt += 4) {
				buf[cnt] = row[x];
				buf[cnt+1] = row[x+1];
				buf[cnt+2] = row[x+2];
				buf[cnt+3] = 0xff;
			}
		}
		else if (components == 4) {
			for (x=0; x < width*4; x += 4, cnt += 4) {
				buf[cnt] = row[x];
				buf[cnt+1] = row[x+1];
				buf[cnt+2] = row[x+2];
				buf[cnt+3] = row[x+3];
			}
		}
	}
	
	png_destroy_read_struct(&png, &info, NULL);
	fclose(fp);
	
	return buf;
}

