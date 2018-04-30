#ifndef EFF_H
#define EFF_H

#ifndef TOPPERS_MACRO_ONLY

extern void eff(uint32_t *pinbuf_l, uint32_t *pinbuf_r,
        uint32_t *poutbuf_l, uint32_t *poutbuf_r,
        uint32_t *poutbuf_ls, uint32_t *poutbuf_rs);
extern void eff_init(void);

#endif /* TOPPERS_MACRO_ONLY */

#endif /* EFF_H */
