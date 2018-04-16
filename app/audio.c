#include <kernel.h>
#include <t_syslog.h>
#include <t_stdlib.h>
#include <sil.h>
#include "syssvc/serial.h"
#include "syssvc/syslog.h"
#include "kernel_cfg.h"
#include "audio.h"

#include "theater.h"
#include "gpio.h"
#include "i2c.h"
#include "i2s.h"
#include "wm8731.h"
#include "pwm.h"

#define QUEUE_SIZE 512
#define queue_next(n) (((n) + 1) % QUEUE_SIZE)

typedef struct buf_queue {
    uint32_t buf_l[QUEUE_SIZE];
    uint32_t buf_r[QUEUE_SIZE];
    int head;
    int tail;
} BUFQ;

static BUFQ inbufq;
static BUFQ outbufq;

void audio_queue_init(BUFQ *q);
bool_t audio_queue_isempty(BUFQ *q);
bool_t audio_queue_isfull(BUFQ *q);
bool_t audio_queue_getnum(BUFQ *q);
bool_t audio_queue_push(BUFQ *q, uint32_t *pval_l, uint32_t *pval_r);
bool_t audio_queue_pop(BUFQ *q, uint32_t *pval_l, uint32_t *pval_r);

void audio_open(void) {
    static bool_t open_flag = false;

    if (open_flag == false) {
        ini_sem(AUDIO_READ_SEM);
        ini_sem(AUDIO_WRITE_SEM);

        gpio_init();
        i2c_init(I2C_MSTR1, 0x1a);
        i2s_init(I2S_SLAV, 48000, 32);
        wm8731_init(WM8731_MSTR, 48000, 32);
        wm8713_active();

        open_flag = true;
    }

    i2s_rcv_int_ena(audio_read_rdy_cb);
    i2s_snd_int_ena(audio_write_rdy_cb);
}

void audio_close(void) {
    i2s_rcv_int_dis();
    i2s_snd_int_dis();
}

void audio_read_data(uint32_t *pbuf_l, uint32_t *pbuf_r)
{
    int i;

    syslog(LOG_NOTICE, "audio_read_data called");

    if (audio_queue_getnum(&inbufq) < 128) {
        wai_sem(AUDIO_READ_SEM);
    }

    for (i = 0; i < 128; i++) {
        audio_queue_pop(&inbufq, pbuf_l, pbuf_r);
        pbuf_l++;
        pbuf_r++;
    }
}

void audio_read_rdy_cb(void)
{
    uint32_t l, r;
    bool_t push_result = false;

    while (i2s_rcv_isrdy() == true) {
        i2s_rcv_data(&l, &r);
        push_result = audio_queue_push(&inbufq, &l, &r);
        if (push_result == false) {
            // overflow
        }
    }

    if (audio_queue_getnum(&inbufq) >= 128) {
        isig_sem(AUDIO_READ_SEM);
    }
}

void audio_write_data(uint32_t *pbuf_l, uint32_t *pbuf_r)
{
    int i;

    syslog(LOG_NOTICE, "audio_read_data called");

    if (audio_queue_getnum(&outbufq) >= (512 - 128)) {
        wai_sem(AUDIO_WRITE_SEM);
    }

    for (i = 0; i < 128; i++) {
        audio_queue_push(&outbufq, pbuf_l, pbuf_r);
        pbuf_l++;
        pbuf_r++;
    }
}

void audio_write_rdy_cb(void)
{
    uint32_t l, r;
    bool_t pop_result = false;

    while (i2s_snd_isrdy() == true) {
        pop_result = audio_queue_pop(&outbufq, &l, &r);
        if (pop_result == false) {
            // underflow
        } else {
            i2s_snd_data(&l, &r);
        }
    }

    if (audio_queue_getnum(&outbufq) < (512 - 128)) {
        isig_sem(AUDIO_WRITE_SEM);
    }
}

void audio_queue_init(BUFQ *q)
{
    q->head = 0;
    q->tail = 0;
}

bool_t audio_queue_isempty(BUFQ *q)
{
    bool_t empty = false;

    if (q->head == q->tail) {
        empty = true;
    } else {
        empty = false;
    }
    return empty;
}

bool_t audio_queue_isfull(BUFQ *q)
{
    bool_t full = false;

    if (queue_next(q->tail) == q->head) {
        full = true;
    } else {
        full = false;
    }
    return full;
}

bool_t audio_queue_getnum(BUFQ *q)
{
    bool_t num = 0;

    if (q->tail >= q->head) {
        num = q->tail - q->head;
    } else {
        num = (QUEUE_SIZE + q->tail) - q->head;
    }
    return num;
}

bool_t audio_queue_push(BUFQ *q, uint32_t *pval_l, uint32_t *pval_r)
{
    bool_t pushed = false;

    if (queue_next(q->tail) == q->head) {
        pushed = false;
    } else {
        q->buf_l[q->tail] = *pval_l;
        q->buf_r[q->tail] = *pval_r;
        q->tail = queue_next(q->tail);
        pushed = true;
    }
    return pushed;
}

bool_t audio_queue_pop(BUFQ *q, uint32_t *pval_l, uint32_t *pval_r)
{
    bool_t poped = false;

    if (q->head == q->tail) {
        poped = false;
    } else {
        *pval_l = q->buf_l[q->head];
        *pval_r = q->buf_r[q->head];
        q->head = queue_next(q->head);
        poped = true;
    }
    return poped;
}
