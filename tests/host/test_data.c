/* Zero Zero UFO for GFOS: host test of the game's datafiles through
 * gfopenallegro's allegro4 core, at 8-bit as the game runs. Every BIN/*.DAT
 * loads, and every object name the sources ask for with
 * Datafile::getObject("NAME") is in one of them with a sane type (bitmaps
 * 8-bit, palettes, samples with data). With ZU_ICON set it writes the
 * player's UFO (UFO_0 on EGA_PAL) there as a PPM, the icon art.
 *   test_data <BIN directory> NAME...
 * Copyright (c) 2026 AmiCradle Team. MIT, as Zero Zero UFO. */
#include <allegro.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int fails;
#define CHECK(c, ...) do { if (!(c)) { fails++; printf("FAIL: " __VA_ARGS__); printf("\n"); } } while (0)

#define MAXDAT 64
static DATAFILE *dat[MAXDAT];
static char datname[MAXDAT][64];
static int ndat;

static int add_dat(const char *file, int attrib, void *param)
{
    (void)attrib; (void)param;
    if (ndat >= MAXDAT) return 0;
    dat[ndat] = load_datafile(file);
    CHECK(dat[ndat] != NULL, "load_datafile %s", file);
    if (dat[ndat]) {
        snprintf(datname[ndat], sizeof(datname[0]), "%s", get_filename(file));
        ndat++;
    }
    return 0;
}

int main(int argc, char **argv)
{
    char pattern[1024];
    int i, d, objects = 0, found = 0;

    if (argc < 2) { printf("usage: test_data BIN NAME...\n"); return 2; }
    CHECK(allegro_init() == 0, "allegro_init");
    set_color_depth(8);
    CHECK(set_gfx_mode(GFX_AUTODETECT, 320, 200, 0, 0) == 0, "set_gfx_mode 320x200x8");
    snprintf(pattern, sizeof(pattern), "%s/*.dat", argv[1]);
    for_each_file_ex(pattern, 0, FA_DIREC, add_dat, NULL);
    snprintf(pattern, sizeof(pattern), "%s/*.DAT", argv[1]);
    if (!ndat) for_each_file_ex(pattern, 0, FA_DIREC, add_dat, NULL);
    CHECK(ndat >= 17, "%d datafiles, not 17 or more", ndat);
    for (d = 0; d < ndat; d++)
        for (i = 0; dat[d][i].type != DAT_END; i++) objects++;

    for (i = 2; i < argc; i++) {
        DATAFILE *o = NULL;
        for (d = 0; d < ndat && !o; d++)
            o = find_datafile_object(dat[d], argv[i]);
        CHECK(o != NULL, "%s is in no datafile", argv[i]);
        if (!o) continue;
        found++;
        if (o->type == DAT_BITMAP)
            CHECK(((BITMAP *)o->dat)->w > 0 && bitmap_color_depth((BITMAP *)o->dat) == 8,
                  "%s: bitmap %dx%d depth %d", argv[i], ((BITMAP *)o->dat)->w,
                  ((BITMAP *)o->dat)->h, bitmap_color_depth((BITMAP *)o->dat));
        else if (o->type == DAT_SAMPLE)
            CHECK(((SAMPLE *)o->dat)->len > 0, "%s: empty sample", argv[i]);
        else
            CHECK(o->type == DAT_PALETTE || o->type == DAT_FONT || o->type == DAT_MIDI
                  || o->type == DAT_DATA || o->type == DAT_RLE_SPRITE,
                  "%s: type %c%c%c%c", argv[i], (o->type >> 24) & 255, (o->type >> 16) & 255,
                  (o->type >> 8) & 255, o->type & 255);
    }
    if (getenv("ZU_ICON")) {
        DATAFILE *b = NULL, *p = NULL;
        for (d = 0; d < ndat; d++) {
            if (!b) b = find_datafile_object(dat[d], "UFO_0");
            if (!p) p = find_datafile_object(dat[d], "EGA_PAL");
        }
        if (b && p) {
            BITMAP *bm = (BITMAP *)b->dat;
            RGB *pal = (RGB *)p->dat;
            FILE *o = fopen(getenv("ZU_ICON"), "wb");
            int x, y;
            if (o) {
                fprintf(o, "P6\n%d %d\n255\n", bm->w, bm->h);
                for (y = 0; y < bm->h; y++)
                    for (x = 0; x < bm->w; x++) {
                        int c = getpixel(bm, x, y);
                        fputc(pal[c].r * 255 / 63, o); fputc(pal[c].g * 255 / 63, o); fputc(pal[c].b * 255 / 63, o);
                    }
                fclose(o);
            }
        }
    }
    printf("datafiles: %d loaded, %d objects; %d of %d names the sources ask for found\n",
           ndat, objects, found, argc - 2);
    for (d = 0; d < ndat; d++) unload_datafile(dat[d]);
    if (!fails) printf("data: all checks passed\n");
    return fails != 0;
}
