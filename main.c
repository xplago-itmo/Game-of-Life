#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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

struct PIXEL {
    BYTE b;
    BYTE g;
    BYTE r;
};

#pragma pack(push, 1)

struct PIXELSDATA {
    struct PIXEL * data;
};

#pragma pack(pop)

struct PIXEL pixel(BYTE r, BYTE g, BYTE b) {
    struct PIXEL pixel = {b, g, r};
    return pixel;
}

int eq_pixel(struct PIXEL f, struct PIXEL s) {
    if (f.r != s.r) return 0;
    if (f.g != s.g) return 0;
    if (f.b != s.b) return 0;
    return 1;
}

struct BMP {
    struct BITMAPFILEHEADER bitmapfileheader;
    struct BITMAPINFO bitmapinfo;
    struct PIXELSDATA pixelsdata;
};

void write_pixelsdata(struct BMP * image, FILE * file) {
    long mul = 3 * image->bitmapinfo.biWidth;
    long dif = 0;
    if (mul % 4 != 0) dif = 4 - mul % 4;

    for (long i = 0; i < image->bitmapinfo.biHeight; i++) {
        fwrite(image->pixelsdata.data + (image->bitmapinfo.biWidth * i), image->bitmapinfo.biWidth, 3, file);
        fseek(file, dif, SEEK_CUR);
    }
}

void write_bmp(struct BMP * image, FILE * outfile) {
    fwrite(&image->bitmapfileheader, sizeof(image->bitmapfileheader), 1, outfile);
    fwrite(&image->bitmapinfo, sizeof(image->bitmapinfo), 1, outfile);
    write_pixelsdata(image, outfile);
    fflush(outfile);
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

    long mul = 3 * bitmapinfo->biWidth;
    long dif = 0;
    if (mul % 4 != 0) dif = 4 - mul % 4;

    struct PIXEL * pixels = (struct PIXEL *) calloc(bitmapinfo->biHeight * bitmapinfo->biWidth, 3);

    for (long i = 0; i < bitmapinfo->biHeight; i++) {
        fread(pixels + (bitmapinfo->biWidth * i), 3, bitmapinfo->biWidth, file);
        fseek(file, dif, SEEK_CUR);
    }

    struct PIXELSDATA pixelsdata = {pixels};
    struct BMP bmp = {*bitmapfileheader, *bitmapinfo, pixelsdata};
    return bmp;
}

int ends_with_bmp(char * string) {
    string = strrchr(string, '.');
    if( string != NULL ) return(strcmp(string, ".bmp"));
    return(-1);
}

unsigned int get_index(unsigned int row, unsigned int column, struct BMP * bmp) {
    return row * ((unsigned int) bmp->bitmapinfo.biWidth) + column;
}

int main(int argc, char *argv[]) {
    char * input_filename = "";
    char * output_filename = "";
    int max_iter = -1;
    int dump_freq = 1;

    char has_error = 0;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--input") == 0) {
            input_filename = argv[++i];
            if (ends_with_bmp(input_filename) != 0) {
                fprintf(stderr, "Error: Invalid input file name \"%s\"\n", input_filename);
                has_error = 1;
            }
        } else if (strcmp(argv[i], "--output") == 0) {
            output_filename = argv[++i];
            if (ends_with_bmp(output_filename) != 0) {
                fprintf(stderr, "Error: Invalid output file name \"%s\"\n", output_filename);
                has_error = 1;
            }
        } else if (strcmp(argv[i], "--max_iter") == 0) {
            char * max_iter_str = argv[++i];
            max_iter = atoi(max_iter_str);
            if (max_iter < 1) {
                fprintf(stderr, "Error: --max-iter parameter value must be positive\n");
                has_error = 1;
            }
        } else if (strcmp(argv[i], "--dump_freq") == 0) {
            char * dump_freq_str = argv[++i];
            dump_freq = atoi(dump_freq_str);
            if (dump_freq < 1) {
                fprintf(stderr, "Error: --dump_freq parameter value must be positive\n");
                has_error = 1;
            }
        }
    }

    if (strcmp(input_filename, "") == 0) {
        fprintf(stderr, "Error: Missing required parameter --input\n");
        has_error = 1;
    }
    if (strcmp(output_filename, "") == 0) {
        fprintf(stderr, "Error: Missing required parameter --output\n");
        has_error = 1;
    }
    if (max_iter == -1) {
        fprintf(stderr, "Error: Missing required parameter --max_iter\n");
        has_error = 1;
    }

    if (has_error) return -1;

    FILE * infile = fopen(input_filename, "r");
    struct BMP bmp = read_bmp(infile);
    fclose(infile);

    unsigned int height = (unsigned int) bmp.bitmapinfo.biHeight;
    unsigned int width = (unsigned int) bmp.bitmapinfo.biWidth;

    struct PIXEL * pixels = bmp.pixelsdata.data;
    struct PIXEL * start_pixels = bmp.pixelsdata.data;
    struct PIXEL black = pixel(0, 0, 0);
    struct PIXEL white = pixel(255, 255, 255);

    fclose(fopen(output_filename, "w"));

    int stable_flag = 1;
    int empty_flag = 1;

    for (unsigned int time = 0; time < max_iter; time++) {

        sleep(dump_freq);
        printf("time: %d ", time);

        struct PIXEL * new_pixels = (struct PIXEL *) calloc(width * height, 24);
        stable_flag = 1;
        empty_flag = 1;

        for (unsigned int i = 0; i < height; i++) {
            for (unsigned int j = 0; j < width; j++) {
                unsigned int count = 0;

                unsigned int index = get_index((height + i) % height, (width + j) % width, &bmp);

                if (eq_pixel(pixels[get_index((height + i - 1) % height, (width + j - 1) % width, &bmp)], black) == 1) count++;
                if (eq_pixel(pixels[get_index((height + i - 1) % height, (width + j) % width, &bmp)], black) == 1) count++;
                if (eq_pixel(pixels[get_index((height + i - 1) % height, (width + j + 1) % width, &bmp)], black) == 1) count++;
                if (eq_pixel(pixels[get_index((height + i) % height, (width + j - 1) % width, &bmp)], black) == 1) count++;
                if (eq_pixel(pixels[get_index((height + i) % height, (width + j + 1) % width, &bmp)], black) == 1) count++;
                if (eq_pixel(pixels[get_index((height + i + 1) % height, (width + j - 1) % width, &bmp)], black) == 1) count++;
                if (eq_pixel(pixels[get_index((height + i + 1) % height, (width + j) % width, &bmp)], black) == 1) count++;
                if (eq_pixel(pixels[get_index((height + i + 1) % height, (width + j + 1) % width, &bmp)], black) == 1) count++;

                if (eq_pixel(pixels[index], black) == 1) {
                    if (count == 2 || count == 3) {
                        new_pixels[index] = black;
                        empty_flag = 0;
                    } else {
                        new_pixels[index] = white;
                    }
                } else if (eq_pixel(pixels[index], white) == 1) {
                    if (count == 3) {
                        new_pixels[index] = black;
                        empty_flag = 0;
                    } else {
                        new_pixels[index] = white;
                    }
                } else {
                    struct PIXEL p = pixels[index];
                    fprintf(stderr, "Error: Unsupported color {r: %d, g: %d, b: %d}\n", p.r, p.g, p.b);
                }

                if (eq_pixel(pixels[index], new_pixels[index]) == 0) {
                    stable_flag = 0;
                }
            }
        }

        if (bmp.pixelsdata.data != start_pixels) free(bmp.pixelsdata.data);

        bmp.pixelsdata.data = new_pixels;
        pixels = new_pixels;

        FILE * outfile = fopen(output_filename, "w");
        write_bmp(&bmp, outfile);
        printf("written\n");

        if (stable_flag == 1) {
            printf("The Game of Life is stable");
            return 0;
        }
        if (empty_flag == 1) {
            printf("The Game of Life is dead");
            return 0;
        }
    }
    return 0;
}