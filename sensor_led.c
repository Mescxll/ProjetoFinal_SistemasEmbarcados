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

// Definição dos níveis
int valores_VM[] = {4000, 2400, 400, 0, 0, 0};  // Intensidade do LED vermelho
int valores_VD[] = {0, 0, 0, 400, 2400, 4000};  // Intensidade do LED verde
int nivel = 5; 

// 6 Crítico (vermelho 100%)
// 5 Perigo (vermelho 70%)
// 4 Perigo Potencial (vermelho 30%)
// 3 Alerta (verde 30%)
// 2 Oscilando (verde 70%)
// 1 Ideal (verde 100%)

void inicializar_leds() {
    gpio_init(LED_VM);
    gpio_set_dir(LED_VM, GPIO_OUT);
    gpio_init(LED_AZ);
    gpio_set_dir(LED_AZ, GPIO_OUT);
    gpio_init(LED_VD);
    gpio_set_dir(LED_VD, GPIO_OUT);
}
