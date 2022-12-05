#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

typedef uint32_t DWORD;
typedef uint16_t WORD;
typedef uint8_t BYTE;

#pragma pack(push, 1)

struct BITMAPFILEHEADER {
    WORD bfType;
    DWORD bfSize;
    WORD bfReserved1;
    WORD bfReserved2;
    DWORD bfOffBits;
};

#pragma pack(pop)

#pragma pack(push, 1)

struct BITMAPINFO {
    DWORD biSize;
    long biWidth;
    long biHeight;
    WORD biPlanes;
    WORD biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    long biXPelsPerMeter;
    long biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
};

#pragma pack(pop)

struct PIXELSDATA {
    struct PIXEL * data;
};

#pragma pack(push, 1)

struct PIXEL {
    BYTE b;
    BYTE g;
    BYTE r;
};

struct PIXEL pixel(BYTE r, BYTE g, BYTE b) {
    struct PIXEL pixel = {b, g, r};
    return pixel;
}

#pragma pack(pop)

struct BMP {
    struct BITMAPFILEHEADER bitmapfileheader;
    struct BITMAPINFO bitmapinfo;
    struct PIXELSDATA pixelsdata;
};

void write_pixelsdata(struct BMP * image, FILE * file) {
    fwrite(image->pixelsdata.data, 3, image->bitmapinfo.biHeight * image->bitmapinfo.biWidth, file);
}

void write_bmp(struct BMP * image, FILE * outfile) {
    fwrite(&image->bitmapfileheader, sizeof(image->bitmapfileheader), 1, outfile);
    fwrite(&image->bitmapinfo, sizeof(image->bitmapinfo), 1, outfile);
    write_pixelsdata(image, outfile);
    fflush(outfile);
    fclose(outfile);
}

struct BMP create_bmp(unsigned int width, unsigned int height, struct PIXEL * pixels) {
    unsigned int start_of_pixels = 14 + 40;
    unsigned int size = start_of_pixels + 3 * height * width;
    struct BMP image = {
            {0x4D42, size, 0, 0, start_of_pixels},
            {40, (long) width, (long) height, 1, 24, 0, size - start_of_pixels, 0, 0, 0, 0},
            {pixels}
    };
    return image;
}

struct BMP read_bmp(FILE * file) {
    struct BITMAPFILEHEADER * bitmapfileheader = (struct BITMAPFILEHEADER *) calloc(1, 14);
    struct BITMAPINFO * bitmapinfo = (struct BITMAPINFO *) calloc(1, 40);
    fread(bitmapfileheader, 14, 1, file);
    fread(bitmapinfo, 40, 1, file);
    struct PIXEL * pixels = (struct PIXEL *) calloc(bitmapinfo->biHeight * bitmapinfo->biWidth, 3);
    fread(pixels, 3, bitmapinfo->biHeight * bitmapinfo->biWidth, file);
    struct PIXELSDATA pixelsdata = {pixels};
    struct BMP bmp = {*bitmapfileheader, *bitmapinfo, pixelsdata};
    return bmp;
}

int main() {
    FILE * infile = fopen("input.bmp", "r");
    struct BMP bmp = read_bmp(infile);
    fclose(infile);

    fclose(fopen("output.bmp", "w"));
    FILE * outfile = fopen("output.bmp", "w");
    write_bmp(&bmp, outfile);
    return 0;
}