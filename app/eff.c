#include <kernel.h>
#include <t_syslog.h>
#include <t_stdlib.h>
#include <sil.h>
#include "syssvc/serial.h"
#include "syssvc/syslog.h"

#include "eff.h"

#define DELAY_TIME          (100)
#define REV_DELAY_REPEAT    (2)
#define DELAY_BUF_SIZE      (48000 * DELAY_TIME / 1000)

static int delay_l[DELAY_BUF_SIZE], delay_r[DELAY_BUF_SIZE];
static int delay_idx = 0;

void eff_through(uint32_t *pinbuf_l, uint32_t *pinbuf_r,
        uint32_t *poutbuf_l, uint32_t *poutbuf_r,
        uint32_t *poutbuf_ls, uint32_t *poutbuf_rs);
void eff_rearreb(uint32_t *pinbuf_l, uint32_t *pinbuf_r,
        uint32_t *poutbuf_l, uint32_t *poutbuf_r,
        uint32_t *poutbuf_ls, uint32_t *poutbuf_rs);

void eff(uint32_t *pinbuf_l, uint32_t *pinbuf_r,
        uint32_t *poutbuf_l, uint32_t *poutbuf_r,
        uint32_t *poutbuf_ls, uint32_t *poutbuf_rs)
{
//    eff_through(pinbuf_l, pinbuf_r,poutbuf_l, poutbuf_r,
//            poutbuf_ls, poutbuf_rs);
    eff_rearreb(pinbuf_l, pinbuf_r,poutbuf_l, poutbuf_r,
            poutbuf_ls, poutbuf_rs);
}

void eff_init(void)
{
    int i;

    delay_idx = 0;
    for (i = 0; i < DELAY_BUF_SIZE; i++) {
        delay_l[i] = 0;
        delay_r[i] = 0;
    }
}

void eff_through(uint32_t *pinbuf_l, uint32_t *pinbuf_r,
        uint32_t *poutbuf_l, uint32_t *poutbuf_r,
        uint32_t *poutbuf_ls, uint32_t *poutbuf_rs)
{
    int i;

    for (i = 0; i < 128; i++) {
        poutbuf_l[i] = pinbuf_l[i];
        poutbuf_r[i] = pinbuf_r[i];
        poutbuf_ls[i] = 0;
        poutbuf_rs[i] = 0;
    }
}

void eff_rearreb(uint32_t *pinbuf_l, uint32_t *pinbuf_r,
        uint32_t *poutbuf_l, uint32_t *poutbuf_r,
        uint32_t *poutbuf_ls, uint32_t *poutbuf_rs)
{
    int delay;
    int i;

    for (i = 0; i < 128; i++) {
        poutbuf_l[i] = pinbuf_l[i];
        poutbuf_r[i] = pinbuf_r[i];

        delay = (int)pinbuf_l[i] / 2 +
            (int)(delay_l[delay_idx] / REV_DELAY_REPEAT);
        delay_l[delay_idx] = delay;
        poutbuf_ls[i] = (uint32_t)delay;

        delay = (int)pinbuf_r[i] / 2 +
            (int)(delay_r[delay_idx] / REV_DELAY_REPEAT);
        delay_r[delay_idx] = delay;
        poutbuf_rs[i] = (uint32_t)delay;

        delay_idx = (delay_idx + 1) % DELAY_BUF_SIZE;
    }
}
