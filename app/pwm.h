#ifndef PWM_H
#define PWM_H

extern void pwm_init(int sr, int bit);
extern void pwm_write(uint32_t* val_l, uint32_t* val_r);
extern void pwm_write_16(int16_t* val_l, int16_t* val_r);

#endif /* PWM_H */
