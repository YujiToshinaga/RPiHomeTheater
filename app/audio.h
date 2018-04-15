#include "target_test.h"

#define AUDIO_PRIORITY	5

#ifndef AUDIO_STACK_SIZE
#define	AUDIO_STACK_SIZE		4096
#endif /* AUDIO_STACK_SIZE */

#ifndef TOPPERS_MACRO_ONLY

extern void audio_init();
extern void audio_read(uint32_t *pbuf_l, uint32_t *pbuf_r);
extern void audio_write(uint32_t *pbuf_l, uint32_t *pbuf_r);
extern void audio_read_task(intptr_t exinf);
extern void audio_write_task(intptr_t exinf);

#endif /* TOPPERS_MACRO_ONLY */
