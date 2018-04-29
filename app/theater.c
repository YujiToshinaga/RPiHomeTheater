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

//uint_t const task1_id[TNUM_PRCID] = {TASK1_1, TASK1_2, TASK1_3, TASK1_4};
//uint_t const cychdr_id[TNUM_PRCID] = {CYCHDR_1, CYCHDR_2, CYCHDR_3, CYCHDR_4};

void cyclic_handler(intptr_t exinf)
{
//	iact_tsk(task1_id[(int)exinf - 1]);
}

void task1(intptr_t exinf)
{
//	static int cnt[TNUM_PRCID] = {0};
//
//	syslog(LOG_NOTICE, "prc %d : TASK1 %d", (int)exinf, cnt[(int)exinf - 1]);
//	cnt[(int)exinf - 1]++;
}

void main_task(intptr_t exinf)
{
    uint32_t inbuf_l[128], inbuf_r[128];
    uint32_t outbuf_l[128], outbuf_r[128];
    uint32_t outbuf_ls[128], outbuf_rs[128];
    int count;
    int i;

//	syslog_msk_log(LOG_UPTO(LOG_INFO), LOG_UPTO(LOG_EMERG));
	syslog_msk_log(0, LOG_UPTO(LOG_INFO));
	serial_opn_por(TASK_PORTID_G_SYSLOG);
	serial_ctl_por(TASK_PORTID_G_SYSLOG,
			(IOCTL_CRLF | IOCTL_FCSND | IOCTL_FCRCV));

	syslog(LOG_NOTICE, "prc %d : Sample program starts", (int)exinf);

//	sta_cyc(cychdr_id[(int)exinf - 1]);
//	slp_tsk();

	tslp_tsk(1000);

	syslog(LOG_EMERG, "initialize IO");

    gpio_init();
    i2c_init(I2C_MSTR1, 0x1a);
    i2s_init(I2S_SLAV, 48000, 32);
    pwm_init(48000, 32);
    wm8731_init(WM8731_MSTR, 48000, 32);

	tslp_tsk(1000);

	syslog(LOG_EMERG, "open");

    audio_open();

	syslog(LOG_EMERG, "opened");

	tslp_tsk(1000);

    if ((int)exinf == 1) {
        count = 0;
        for ( ; ; ) {
            audio_read_data(inbuf_l, inbuf_r);
            for (i = 0; i < 128; i++) {
                outbuf_l[i] = inbuf_l[i];
                outbuf_r[i] = inbuf_r[i];
                outbuf_ls[i] = inbuf_l[i];
                outbuf_rs[i] = inbuf_r[i];
            }
            audio_write_data(outbuf_l, outbuf_r, outbuf_ls, outbuf_rs);

//            if ((count % 48000) == 0) {
//                syslog(LOG_NOTICE, "%08x", inbuf_l[0]);
//            }
            count = count + 128;
        }
    }

	for ( ; ; );

	syslog(LOG_NOTICE, "prc %d : Sample program ends", (int)exinf);
	ext_ker();
	assert(0);
}

