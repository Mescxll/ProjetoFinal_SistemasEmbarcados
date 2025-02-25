#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

#define LED 13 //Pino do LED vermelho


void ativar_led_pwm(){
    gpio_put(LED, 1); //Atraso estratégico
}