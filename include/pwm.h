#ifndef PWM_H
#define PWM_H

#include "define.h"

typedef struct
{
    uint8_t timer;
    uint8_t channel;
    uint8_t duty;
} PWM_Config;

void pwm_init(PWM_Config *pwm);
void pwm_setDuty(PWM_Config *pwm, uint8_t duty);
void pwm_start(PWM_Config *pwm);
void pwm_stop(PWM_Config *pwm);

#endif
