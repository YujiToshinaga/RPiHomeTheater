#include <kernel.h>
#include <t_syslog.h>
#include <t_stdlib.h>
#include <sil.h>
#include "syssvc/serial.h"
#include "syssvc/syslog.h"
#include "kernel_cfg.h"

#include "theater.h"
#include "gpio.h"
#include "i2c.h"
#include "i2s.h"
#include "wm8731.h"
#include "pwm.h"
#include "audio.h"

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
bool_t audio_queue_isleastone(BUFQ *q);
bool_t audio_queue_isfull(BUFQ *q);
bool_t audio_queue_isspace(BUFQ *q);
int audio_queue_getnum(BUFQ *q);
bool_t audio_queue_push(BUFQ *q, uint32_t *pval_l, uint32_t *pval_r);
bool_t audio_queue_pop(BUFQ *q, uint32_t *pval_l, uint32_t *pval_r);

void audio_open(void) {
    ini_sem(AUDIO_READ_SEM);
    ini_sem(AUDIO_WRITE_SEM);

    wm8713_active();
}

void audio_close(void) {
    ini_sem(AUDIO_READ_SEM);
    ini_sem(AUDIO_WRITE_SEM);

    i2s_rcv_int_dis();
    i2s_snd_int_dis();

    wm8713_inactive();
}

bool_t audio_read_data(uint32_t *pbuf_l, uint32_t *pbuf_r)
{
    uint32_t l, r;
    bool_t read = false;
    int i;

    while ((audio_queue_isspace(&inbufq) == true)
            && (i2s_rcv_isrdy() == true)) {
        i2s_rcv_data(&l, &r);
        audio_queue_push(&inbufq, &l, &r);
    }

    if (audio_queue_getnum(&inbufq) >= BUFFERING_SIZE) {
        i2s_rcv_int_dis();
    } else {
        i2s_rcv_int_ena(audio_read_rdy_cb);
        wai_sem(AUDIO_READ_SEM);
    }

    if (audio_queue_getnum(&inbufq) >= BUFFERING_SIZE) {
        for (i = 0; i < BUFFERING_SIZE; i++) {
            audio_queue_pop(&inbufq, pbuf_l, pbuf_r);
            pbuf_l++;
            pbuf_r++;
        }
        read = true;
    } else {
        read = false;
    }

    return read;
}

void audio_read_rdy_cb(void)
{
    uint32_t l, r;

    while ((audio_queue_isspace(&inbufq) == true)
            && (i2s_rcv_isrdy() == true)) {
        i2s_rcv_data(&l, &r);
        audio_queue_push(&inbufq, &l, &r);
    }

    if (audio_queue_getnum(&inbufq) >= BUFFERING_SIZE) {
        i2s_rcv_int_dis();
        isig_sem(AUDIO_READ_SEM);
    }
}

bool_t audio_write_data(uint32_t *pbuf_l, uint32_t *pbuf_r)
{
    uint32_t l, r;
    int written = false;
    int i;

//	syslog(LOG_EMERG, "audio_write_data");

    while ((audio_queue_isleastone(&outbufq) == true)
            && (i2s_snd_isrdy() == true)) {
        audio_queue_pop(&outbufq, &l, &r);
        i2s_snd_data(&l, &r);
        pwm_snd_data(&l, &r);
    }

    if (audio_queue_getnum(&outbufq) < (QUEUE_SIZE - BUFFERING_SIZE)) {
        i2s_snd_int_dis();
    } else {
        i2s_snd_int_ena(audio_write_rdy_cb);
        wai_sem(AUDIO_WRITE_SEM);
    }

    if (audio_queue_getnum(&outbufq) < (QUEUE_SIZE - BUFFERING_SIZE)) {
        for (i = 0; i < BUFFERING_SIZE; i++) {
            audio_queue_push(&outbufq, pbuf_l, pbuf_r);
            pbuf_l++;
            pbuf_r++;
        }
        written = true;
    } else {
        i2s_snd_int_dis();
        written = false;
    }

    return written;
}

void audio_write_rdy_cb(void)
{
    uint32_t l, r;

    while ((audio_queue_isleastone(&outbufq) == true)
            && (i2s_snd_isrdy() == true)) {
        audio_queue_pop(&outbufq, &l, &r);
        i2s_snd_data(&l, &r);
        pwm_snd_data(&l, &r);
    }

    if (audio_queue_getnum(&outbufq) < (QUEUE_SIZE - BUFFERING_SIZE)) {
        i2s_snd_int_ena(audio_write_rdy_cb);
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

bool_t audio_queue_isleastone(BUFQ *q)
{
    bool_t empty = false;

    if (q->head != q->tail) {
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

bool_t audio_queue_isspace(BUFQ *q)
{
    bool_t full = false;

    if (queue_next(q->tail) != q->head) {
        full = true;
    } else {
        full = false;
    }
    return full;
}

int audio_queue_getnum(BUFQ *q)
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
