/* make_diskin.c
 *
 * Usage:
 *      ./make_diskin <a> <b> <c> <d>                → writes diskin.txt
 *      ./make_diskin 1 2 3 4                        → 128×1, 128×2, …
 *
 * Each input may be given in decimal (e.g. 255) or C-style hex (e.g. 0xFF).
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SECTOR_WORDS 128
#define OUTFILE      "./disktest/diskin.txt"

static void write_diskin(uint32_t a, uint32_t b,
                         uint32_t c, uint32_t d,
                         const char *fname)
{
    FILE *fp = fopen(fname, "w");
    if (!fp) {
        perror("diskin fopen");
        exit(EXIT_FAILURE);
    }

    const uint32_t vals[4] = { a, b, c, d };

    for (int blk = 0; blk < 4; ++blk) {
        for (int i = 0; i < SECTOR_WORDS; ++i) {
            fprintf(fp, "%08X\n", vals[blk]);
        }
    }

    fclose(fp);
}

int main(int argc, char *argv[])
{
    if (argc != 5) {
        fprintf(stderr,
            "Usage: %s <a> <b> <c> <d>   (decimal or 0xHEX)\n", argv[0]);
        return EXIT_FAILURE;
    }

    /* strtoul handles both decimal and 0x… hex inputs */
    uint32_t a = (uint32_t)strtoul(argv[1], NULL, 0);
    uint32_t b = (uint32_t)strtoul(argv[2], NULL, 0);
    uint32_t c = (uint32_t)strtoul(argv[3], NULL, 0);
    uint32_t d = (uint32_t)strtoul(argv[4], NULL, 0);

    write_diskin(a, b, c, d, OUTFILE);
    printf("diskin.txt written with 512 words (%08X, %08X, %08X, %08X).\n",
           a, b, c, d);
    return EXIT_SUCCESS;
}