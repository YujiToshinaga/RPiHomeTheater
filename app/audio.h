#include "target_test.h"

#define AUDIO_PRIORITY	5

#ifndef AUDIO_STACK_SIZE
#define	AUDIO_STACK_SIZE		4096
#endif /* AUDIO_STACK_SIZE */

#ifndef TOPPERS_MACRO_ONLY

extern void audio_open(void);
extern void audio_close(void);
extern void audio_read_data(uint32_t *pbuf_l, uint32_t *pbuf_r);
extern void audio_read_rdy_cb(void);
extern void audio_write_data(uint32_t *pbuf_l, uint32_t *pbuf_r);
extern void audio_write_rdy_cb(void);

#endif /* TOPPERS_MACRO_ONLY */
