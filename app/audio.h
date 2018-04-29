#ifndef AUDIO_H
#define AUDIO_H

#define QUEUE_SIZE 512
#define BUFFERING_SIZE 128

#ifndef TOPPERS_MACRO_ONLY

extern void audio_open(void);
extern void audio_close(void);
extern bool_t audio_read_data(uint32_t *pbuf_l, uint32_t *pbuf_r);
extern void audio_read_rdy_cb(void);
extern bool_t audio_write_data(uint32_t *pbuf_l, uint32_t *pbuf_r,
        uint32_t *pbuf_ls, uint32_t *pbuf_rs);
extern void audio_write_rdy_cb(void);

#endif /* TOPPERS_MACRO_ONLY */

#endif /* AUDIO_H */
