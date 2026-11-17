/* Zero Zero UFO for GFOS: host smoke probe. Linked into the host build of
 * the whole game; after AT_SMOKE_SECS seconds it checks that the game has
 * opened gfopenallegro's (fake) acgame screen and shown frames, writes the
 * last frame shown to AT_SMOKE_PPM and ends the run. On GFOS
 * allegro4.library calls the game's timers from a task of its own; the
 * host core calls them only at its poll points, which this game's loop
 * (speed_counter, SRC/GAME.CPP) never reaches, so a second thread here
 * calls them as that task does.
 * Copyright (c) 2026 AmiCradle Team. MIT, as Zero Zero UFO. */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "fake_acgame.h"

void a4_timer_dispatch(void);

static void *timer_task(void *arg)
{
    (void)arg;
    for (;;) {
        a4_timer_dispatch();
        usleep(1000);
    }
    return NULL;
}

static void *probe(void *arg)
{
    const char *secs = getenv("AT_SMOKE_SECS");
    const char *ppm = getenv("AT_SMOKE_PPM");
    int w, h, x, y, lit = 0;
    (void)arg;
    sleep(secs ? (unsigned)atoi(secs) : 8);
    w = fake.cfg.width;
    h = fake.cfg.height;
    printf("smoke: screen %s %dx%d, %d frames shown\n",
           fake.open ? "open" : "closed", w, h, fake.presents);
    if (!fake.open || fake.presents < 1 || w < 1 || h < 1) {
        printf("smoke: FAIL\n");
        fflush(stdout);
        _exit(1);
    }
    for (y = 0; y < h; y++)
        for (x = 0; x < w; x++)
            if (fake.shown[y * w + x]) lit++;
    printf("smoke: %d of %d pixels not colour 0\n", lit, w * h);
    if (ppm) {
        FILE *f = fopen(ppm, "wb");
        if (f) {
            fprintf(f, "P6\n%d %d\n255\n", w, h);
            for (y = 0; y < h * w; y++) {
                const acgame_rgb8 *c = &fake.shown_pal[fake.shown[y]];
                fputc(c->r, f); fputc(c->g, f); fputc(c->b, f);
            }
            fclose(f);
        }
    }
    printf(lit > w * h / 20 ? "smoke: PASS\n" : "smoke: FAIL (blank screen)\n");
    fflush(stdout);
    _exit(lit > w * h / 20 ? 0 : 1);
    return NULL;
}

static void __attribute__((constructor)) start_probe(void)
{
    pthread_t t, tt;
    if (pthread_create(&t, NULL, probe, NULL) == 0)
        pthread_detach(t);
    if (pthread_create(&tt, NULL, timer_task, NULL) == 0)
        pthread_detach(tt);
}
