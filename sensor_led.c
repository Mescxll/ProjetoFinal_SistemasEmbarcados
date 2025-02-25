#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

#define LED_VM 13 //LED vermelho
#define LED_AZ 12 //LED azul
#define LED_VD 11 //LED verde

void inicializar_leds(){
    //Inicializa o LED vermelho e define como saída
    gpio_init(LED_VM);
    gpio_set_dir(LED_VM, GPIO_OUT);

    //Inicializa o LED azul e define como saída
    gpio_init(LED_AZ);
    gpio_set_dir(LED_AZ, GPIO_OUT);

    //Inicializa o LED verde e define como saída
    gpio_init(LED_VD);
    gpio_set_dir(LED_VD, GPIO_OUT);
}

void monitorar_temperaturas(){
    //critico vermelho 100%
    //perigo veremlho 70%
    //perigo potencial vermelho 30%
    //alerta verde 30%
    //oscilando verde 70%
    //ideal verde 100%
}