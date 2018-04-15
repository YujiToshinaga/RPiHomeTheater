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

#define queue_next(n) (((n) + 1) % 512)

typedef struct buf_queue {
    uint32_t buf_l[512];
    uint32_t buf_r[512];
    int head;
    int tail;
} BUFQ;

static BUFQ inbufq;
static BUFQ outbufq;

void audio_queue_init(BUFQ *q);
int audio_queue_isempty(BUFQ *q);
int audio_queue_isfull(BUFQ *q);
int audio_queue_push(BUFQ *q, uint32_t *pval_l, uint32_t *pval_r);
int audio_queue_pop(BUFQ *q, uint32_t *pval_l, uint32_t *pval_r);

void audio_init() {
    static int init_flag = 0;

    if (init_flag == 0) {
//        ini_sem(AUDIO_READ_SEM);
//        ini_sem(AUDIO_WRITE_SEM);
        gpio_init();
        i2c_init(I2C_MSTR1, 0x1a);
        i2s_init(I2S_SLAV, 48000, 32);
        wm8731_init(WM8731_MSTR, 48000, 32);
        wm8713_active();
        init_flag = 1;
    }
}

void audio_read(uint32_t *pbuf_l, uint32_t *pbuf_r)
{
    uint32_t l, r;
    int i, j;

    syslog(LOG_NOTICE, "audio_read call");
    for (i = 0; i < 128; i++) {
        for ( ; ; ) {
//            wai_sem(AUDIO_READ_SEM);
            if (audio_queue_isempty(&inbufq) != 1) {
                break;
            }
//            sig_sem(AUDIO_READ_SEM);
            syslog(LOG_NOTICE, "h");
        }

        syslog(LOG_NOTICE, "audio_read");

//        wai_sem(AUDIO_READ_SEM);
        audio_queue_pop(&inbufq, &l, &r);
//        sig_sem(AUDIO_READ_SEM);

        *pbuf_l = l;
        *pbuf_r = r;
        pbuf_l++;
        pbuf_r++;
    }
}

void audio_write(uint32_t *pbuf_l, uint32_t *pbuf_r)
{
    uint32_t l, r;
    int i;

    for (i = 0; i < 128; i++) {
        while (audio_queue_isfull(&outbufq) == 1);
        l = *pbuf_l;
        r = *pbuf_r;
        audio_queue_push(&outbufq, &l, &r);
        pbuf_l++;
        pbuf_r++;
    }
}

void audio_read_task(intptr_t exinf)
{
    uint32_t l, r;
    int count;
    int ret;

    tslp_tsk(3000);

	syslog_msk_log(LOG_UPTO(LOG_INFO), LOG_UPTO(LOG_EMERG));
	serial_opn_por(TASK_PORTID_G_SYSLOG);
	serial_ctl_por(TASK_PORTID_G_SYSLOG,
			(IOCTL_CRLF | IOCTL_FCSND | IOCTL_FCRCV));
	syslog(LOG_NOTICE, "prc %d : audio_read_task start", (int)exinf);

//    wai_sem(AUDIO_READ_SEM);
    audio_queue_init(&inbufq);
//    sig_sem(AUDIO_READ_SEM);

    count = 0;
    for ( ; ; ) {
        i2s_read(&l, &r);
        if ((count % 48000) == 0) {
//            syslog(LOG_NOTICE, "%08x", l);
        }

//        wai_sem(AUDIO_READ_SEM);
        ret = audio_queue_push(&inbufq, &l, &r);
        if (ret == 0) {
//            audio_queue_init(&inbufq);
        }
//        sig_sem(AUDIO_READ_SEM);

        count++;
    }
}

void audio_write_task(intptr_t exinf)
{
    uint32_t l, r;
    int count;
    int ret;

    tslp_tsk(3000);

	syslog_msk_log(LOG_UPTO(LOG_INFO), LOG_UPTO(LOG_EMERG));
	serial_opn_por(TASK_PORTID_G_SYSLOG);
	serial_ctl_por(TASK_PORTID_G_SYSLOG,
			(IOCTL_CRLF | IOCTL_FCSND | IOCTL_FCRCV));
	syslog(LOG_NOTICE, "prc %d : audio_write_task start", (int)exinf);

    audio_queue_init(&outbufq);

    count = 0;
    for ( ; ; ) {
        ret = audio_queue_pop(&outbufq, &l, &r);
        if ((count % 48000) == 0) {
//            syslog(LOG_NOTICE, "%08x", l);
        }
        if (ret == 0) {
//            audio_queue_init(&outbufq);
        }
        i2s_write(&l, &r);
        count++;
    }
}

void audio_queue_init(BUFQ *q)
{
    q->head = 0;
    q->tail = 0;
}

int audio_queue_isempty(BUFQ *q)
{
    int ret = 0;

    if (q->head == q->tail) {
        ret = 1;
    } else {
        ret = 0;
    }
    return ret;
}

int audio_queue_isfull(BUFQ *q)
{
    int ret = 0;

    if (queue_next(q->tail) == q->head) {
        ret = 1;
    } else {
        ret = 0;
    }
    return ret;
}

int audio_queue_push(BUFQ *q, uint32_t *pval_l, uint32_t *pval_r)
{
    int ret = 0;

    if (queue_next(q->tail) == q->head) {
        ret = 0;
    } else {
        q->buf_l[q->tail] = *pval_l;
        q->buf_r[q->tail] = *pval_r;
        q->tail = queue_next(q->tail);
        ret = 1;
    }
    return ret;
}

int audio_queue_pop(BUFQ *q, uint32_t *pval_l, uint32_t *pval_r)
{
    int ret = 0;

    if (q->head == q->tail) {
        ret = 0;
    } else {
        *pval_l = q->buf_l[q->head];
        *pval_r = q->buf_r[q->head];
        q->head = queue_next(q->head);
        ret = 1;
    }
    return ret;
}
