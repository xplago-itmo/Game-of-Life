#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

typedef uint32_t DWORD;
typedef uint16_t WORD;
typedef uint8_t BYTE;

struct BITMAPFILEHEADER {
    WORD bfType;
    DWORD bfSize;
    WORD bfReserved1;
    WORD bfReserved2;
    DWORD bfOffBits;
};

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

struct PIXELSDATA {
    struct PIXEL * data;
};

struct PIXEL {
    BYTE r;
    BYTE g;
    BYTE b;
};

struct BMP {
    struct BITMAPFILEHEADER bitmapfileheader;
    struct BITMAPINFO bitmapinfo;
    struct PIXELSDATA pixelsdata;
};

void reverse_fwrite(char * ptr, unsigned int size, unsigned int times, FILE * file) {
    for (int time = 0; time < times; time++) {
        char * time_ptr = ptr + time * size;
        for (int i = 0; i < size / 8; i++) {
            char * byte_ptr_left = time_ptr + i * 8;
            char * byte_ptr_right = time_ptr + size - i * 8;
            char tmp = *byte_ptr_left;
            *byte_ptr_left = *byte_ptr_right;
            *byte_ptr_right = tmp;
        }
    }
    fwrite(ptr, size, times, file);
}

void print_image(struct BMP * image) {
    printf("%d %d %d %d %d\n",
           image->bitmapfileheader.bfType,
           image->bitmapfileheader.bfSize,
           image->bitmapfileheader.bfReserved1,
           image->bitmapfileheader.bfReserved2,
           image->bitmapfileheader.bfOffBits
           );
    printf("%d %ld %ld %d %d %d %d %ld %ld %d %d\n",
           image->bitmapinfo.biSize,
           image->bitmapinfo.biWidth,
           image->bitmapinfo.biHeight,
           image->bitmapinfo.biPlanes,
           image->bitmapinfo.biBitCount,
           image->bitmapinfo.biCompression,
           image->bitmapinfo.biSizeImage,
           image->bitmapinfo.biXPelsPerMeter,
           image->bitmapinfo.biYPelsPerMeter,
           image->bitmapinfo.biClrUsed,
           image->bitmapinfo.biClrImportant
    );
}

void write_bitmapfileheader(struct BMP * image, FILE * file) {
    reverse_fwrite(&image->bitmapfileheader.bfType, sizeof(image->bitmapfileheader.bfType), (unsigned int) 1, file);
    reverse_fwrite(&image->bitmapfileheader.bfSize, sizeof(image->bitmapfileheader.bfSize), (unsigned int) 1, file);
    reverse_fwrite(&image->bitmapfileheader.bfReserved1, sizeof(image->bitmapfileheader.bfReserved1), (unsigned int) 1, file);
    reverse_fwrite(&image->bitmapfileheader.bfReserved2, sizeof(image->bitmapfileheader.bfReserved2), (unsigned int) 1, file);
    reverse_fwrite(&image->bitmapfileheader.bfOffBits, sizeof(image->bitmapfileheader.bfOffBits), (unsigned int) 1, file);
}

void write_bitmapinfo(struct BMP * image, FILE * file) {
    reverse_fwrite(&image->bitmapinfo.biSize, sizeof(image->bitmapinfo.biSize), (unsigned int) 1, file);
    reverse_fwrite(&image->bitmapinfo.biWidth, sizeof(image->bitmapinfo.biWidth), (unsigned int) 1, file);
    reverse_fwrite(&image->bitmapinfo.biHeight, sizeof(image->bitmapinfo.biHeight), (unsigned int) 1, file);
    reverse_fwrite(&image->bitmapinfo.biPlanes, sizeof(image->bitmapinfo.biPlanes), (unsigned int) 1, file);
    reverse_fwrite(&image->bitmapinfo.biBitCount, sizeof(image->bitmapinfo.biBitCount), (unsigned int) 1, file);
    reverse_fwrite(&image->bitmapinfo.biCompression, sizeof(image->bitmapinfo.biCompression), (unsigned int) 1, file);
    reverse_fwrite(&image->bitmapinfo.biSizeImage, sizeof(image->bitmapinfo.biSizeImage), (unsigned int) 1, file);
    reverse_fwrite(&image->bitmapinfo.biXPelsPerMeter, sizeof(image->bitmapinfo.biXPelsPerMeter), (unsigned int) 1, file);
    reverse_fwrite(&image->bitmapinfo.biYPelsPerMeter, sizeof(image->bitmapinfo.biYPelsPerMeter), (unsigned int) 1, file);
    reverse_fwrite(&image->bitmapinfo.biClrUsed, sizeof(image->bitmapinfo.biClrUsed), (unsigned int) 1, file);
    reverse_fwrite(&image->bitmapinfo.biClrImportant, sizeof(image->bitmapinfo.biClrImportant), (unsigned int) 1, file);
}

void write_pixel(struct PIXEL * pixel, FILE * file) {
    fwrite(&pixel->b, 1, 1, file);
    fwrite(&pixel->g, 1, 1, file);
    fwrite(&pixel->r, 1, 1, file);
}

void write_pixelsdata(struct BMP * image, FILE * file) {
    char * ptr = image->pixelsdata.data;
    for (int i = 0; i < image->bitmapinfo.biHeight * image->bitmapinfo.biWidth; i++) {
        write_pixel(ptr, file);
        ptr += 3;
    }
}

void write_bmp(struct BMP * image, FILE * outfile) {
    write_bitmapfileheader(image, outfile);
    write_bitmapinfo(image, outfile);
    write_pixelsdata(image, outfile);
    fflush(outfile);
    if (fclose(outfile)) perror("fclose error");
    else printf("File mylib/myfile closed successfully.\n");
}

struct BMP create_bmp(unsigned int width, unsigned int height, struct PIXEL * pixels) {
    unsigned int start_of_pixels = 14 + 40;
    unsigned int size = start_of_pixels + 3 * height * width;
    struct BMP image = {
            {0x4D42, size, 0, 0, start_of_pixels},
            {40, (long) width, (long) height, 1, 24, 0, size - start_of_pixels, 0, 0, 0, 0},
            {pixels}
    };
    print_image(&image);
    return image;
}

int main() {
    unsigned int height = 1000;
    unsigned int width = 1000;
    struct PIXEL * pixels = (struct PIXEL *) calloc(height * width, 3 * 24);
    for (int i = 0; i < height * width; i++) {
        struct PIXEL red = {255, 0, 0};
        struct PIXEL green = {0, 255, 0};
        struct PIXEL blue = {0, 0, 255};
        if (i % 3 == 0) {
            pixels[i] = red;
        } else if (i % 3 == 1) {
            pixels[i] = green;
        } else {
            pixels[i] = blue;
        }
    }
    struct BMP bmp = create_bmp(width, height, pixels);
    fclose(fopen("test.bmp", "w"));
    FILE * outfile = fopen("test.bmp", "w");
    write_bmp(&bmp, outfile);
    return 0;
}