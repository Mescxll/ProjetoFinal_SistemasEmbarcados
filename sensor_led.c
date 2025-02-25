#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

#define LED_VM 13 // LED vermelho
#define LED_AZ 12 // LED azul
#define LED_VD 11 // LED verde

const uint16_t WRAP_PERIOD = 4000;
const float PWM_DIVISER = 4.0;
uint16_t led_level = 200; 

// Função para configurar o módulo PWM
void pwm_setup(uint LED) {
    gpio_set_function(LED, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(LED);
    pwm_set_clkdiv(slice, PWM_DIVISER);
    pwm_set_wrap(slice, WRAP_PERIOD);
    pwm_set_gpio_level(LED, 0);
    pwm_set_enabled(slice, true);
}

// Níveis de intensidade dos LEDs
int valores[] = {4000, 2800, 1200, 1200, 2800, 4000};
int led[] = {11, 13};
// Crítico (vermelho 100%)
// Perigo (vermelho 70%)
// Perigo Potencial (vermelho 30%)
// Alerta (verde 30%)
// Oscilando (verde 70%)
// Ideal (verde 100%)

void inicializar_leds() {
    gpio_init(LED_VM);
    gpio_set_dir(LED_VM, GPIO_OUT);
    gpio_init(LED_AZ);
    gpio_set_dir(LED_AZ, GPIO_OUT);
    gpio_init(LED_VD);
    gpio_set_dir(LED_VD, GPIO_OUT);
}
