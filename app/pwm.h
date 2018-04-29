#ifndef PWM_H
#define PWM_H

#ifndef TOPPERS_MACRO_ONLY

extern void pwm_init(int sr, int bit);
extern bool_t pwm_snd_isrdy(void);
extern void pwm_snd_data(uint32_t* val_l, uint32_t* val_r);

#endif /* TOPPERS_MACRO_ONLY */

#endif /* PWM_H */
