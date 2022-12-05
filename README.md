
# The Game of Life

The Game of Life is a cyclical algorithm that simulates the development of digital life. The algorithm takes an image with black and white pixels as input and updates the image according to certain rules with each iteration.
- The place of action of the game is a plane marked into cells, which can be unlimited, limited or closed.
- Each cell on this surface has eight neighbors surrounding it, and can be in two states: to be *alive* (filled) or *dead* (empty).
- The distribution of living cells at the beginning of the game is called the first generation. Each next generation is calculated based on the previous one according to the following rules:
    - in an *empty* (dead) cell, which is adjacent to three *living* cells, life is born;
    - if a *living* cell has two or three *living* neighbors, then this cell continues to live; otherwise (if there are less than two or more than three living neighbors), the cell dies (“from loneliness” or “from overcrowding”).
- The game ends if not a single *living* cell will remain on the field;
    - the configuration at the next step will exactly (without shifts and rotations) repeat itself at one of the earlier steps (a periodic configuration is added)
    - at the next step, none of the cells changes its state (the previous rule applies one step back, a stable configuration is formed).

## Authors

- [@xplago-itmo](https://www.github.com/xplago-itmo)


## Used libraries:

```C
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
```

## Used custom types:

New types are used to simplify unsigned numbers:

- `DWORD` - `4` byte;
- `WORD` - `2` byte;
- `BYTE` - `1` byte;

```C
typedef uint32_t DWORD;
typedef uint16_t WORD;
typedef uint8_t BYTE;
```

## BMP realization

### BMP struct

The BMP structure, like the .bmp file, consists of several parts:
- `BITMAPFILEHEADER` - `12` bytes;
- `BITMAPINFO` - `40` bytes;
  - `PIXELSDATA` - `width * height * 3` bytes;

```C
struct BMP {
    struct BITMAPFILEHEADER bitmapfileheader;
    struct BITMAPINFO bitmapinfo;
    struct PIXELSDATA pixelsdata;
};
```
### BITMAPFILEHEADER struct

The structure written to the beginning of the file contains the following fields:

- `bfType: WORD` - `0x4D42`;
- `bfSize: DWORD` - size of file in bytes;
- `bfReserved1: WORD` - always `0`;
- `bfReserved2: WORD` - always `0`;
- `bfOffBits: DWORD` - offset of pixels data;

To sequentially order fields in memory, use `pragma`.

```C
#pragma pack(push, 1)

struct BITMAPFILEHEADER {
    WORD bfType;
    DWORD bfSize;
    WORD bfReserved1;
    WORD bfReserved2;
    DWORD bfOffBits;
};

#pragma pack(pop)
```
### BITMAPINFO struct

The BITMAPINFO block consists of three parts:

- Structure with information fields;
- Bit masks for extracting color channel values (not always present);
- Color table (not always present);

This 24-bit implementation lacks bit masks and a color table.

The structure written to the beginning of the file contains the following fields:

- `biSize: DWORD` - size of `BITMAPINFO` block in bytes - `40` ;
- `biWidth: long` - width of image;
- `biHeight: long` - height of image;
- `biPlanes: WORD` - only `1` for `.bmp` files;
- `biBitCount: WORD` - size of pixel in bits - `24` in this realization;
- `biCompression: DWORD` - specifies how pixels are stored - `0` - `BI_RGB`;
- `biSizeImage: DWORD` - size of pixel data in bytes;
- `biXPelsPerMeter: long` - the number of pixels per meter horizontally;
- `biYPelsPerMeter: long` - the number of pixels per meter vertically;
- `biClrUsed: DWORD` - color table size in cells - `0`;
- `biClrImportant: DWORD` - number of cells from the beginning of the color table to the last used - `0`;

To sequentially order fields in memory, use `pragma`.

```C
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
```
### PIXELSDATA struct

Contains only one data field - a pointer to an array of pixels. For this, a special PIXEL structure is used, the sequence of the fields r, g, b is determined by the use of pragma.

```C
#pragma pack(push, 1)

struct PIXEL {
    BYTE b;
    BYTE g;
    BYTE r;
};

#pragma pack(pop)

struct PIXELSDATA {
    struct PIXEL * data;
};
```

### PIXEL functions

There are two functions for the PIXEL structure:
- `pixel(r: BYTE, g: BYTE, b: BYTE): struct PIXEL` - creates a new pixel;
- `eq_pixel(f: struct PIXEL, s: struct PIXEL): int` - checking pixels for equivalence;

```C
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
```

### Write functions

To write the bmp structure to a file, function `write_bmp` is used, which internally uses function `write_pixelsdata`.
- `write_pixelsdata(image: * struct BMP, file: * FILE): void` - writes the given pixel to a file;
- `write_bmp(image: * struct BMP, outfile: * FILE): void` - writes the given `BMP` structure to a file;

```C
void write_pixelsdata(struct BMP * image, FILE * file) {
    fwrite(image->pixelsdata.data, 3, image->bitmapinfo.biHeight * image->bitmapinfo.biWidth, file);
}

void write_bmp(struct BMP * image, FILE * outfile) {
    fwrite(&image->bitmapfileheader, sizeof(image->bitmapfileheader), 1, outfile);
    fwrite(&image->bitmapinfo, sizeof(image->bitmapinfo), 1, outfile);
    write_pixelsdata(image, outfile);
    fflush(outfile);
}
```

### Empty BMP create function

Creates a new image of the given size filled with black pixels.
- `create_bmp(width: unsigned int, height: unsigned int, pixels: * struct PIXEL): struct BMP`;

```C
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
```

### Read BMP struct

Returns a structure based on a file.
- `read_bmp(file: * FILE): struct BMP`;

```C
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
```

### Tool functions

Functions that serve as tools for working with `BMP` files and structures:
- `ends_with_bmp(string: * char): int` - checks the string for the ending `.bmp`;
- `get_index(row: unsigned int, column: unsigned int, bmp: * struct BMP): unsigned int` - get pixel position in array by row and column number;

```C
int ends_with_bmp(char * string) {
    string = strrchr(string, '.');
    if( string != NULL ) return(strcmp(string, ".bmp"));
    return(-1);
}

unsigned int get_index(unsigned int row, unsigned int column, struct BMP * bmp) {
    return row * ((unsigned int) bmp->bitmapinfo.biWidth) + column;
}
```

## The Game of Life realization

The algorithm of The Game of Life is implemented in the main function.
The program receives several arguments as input:

- `--input <filename>` (required) - name of input `.bmp` file;
- `--output <filename>` (required) - name of output `.bmp` file;
- `--max_iter <num>` (required) - max value of game iteration;
- `--dump_freq <num>` - time of one iteration step in seconds;

The program gets the original image from the file whose name is passed in the parameter.
Next, the algorithm cycles through each image pixel, counting the number of its "live" neighbors, and updates the pixel's status by updating the output image.

The program terminates prematurely in case of an error:

- the required parameter for launching the program was not passed, or it was passed in the wrong format;
- pixels other than white or black are used;

Also, the program will terminate prematurely:

- if a stable image shape is formed ;
- if there are no *live* pixels;

```C
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
```