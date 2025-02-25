//Bibliotecas
#include "pico/stdlib.h"
#include "projeto_final.pio.h"
#include "hardware/pio.h"

#define LEDS 25 //Número de LEDs
#define PINO_LEDS 7 //Pino da Matriz de LEDs

//Sm e outras variaveis definidas como estáticas para não estourar o limite de Sm por PIO
static PIO pio = pio0;
static uint sm;
static uint offset;

//Variáveis que definem a cor usada e sua intensidade -> 1 = 100%
double r = 0.0, g = 1.0, b = 0.0;


//Função para configurar o PIO
void configurar_pio() {
    sm = pio_claim_unused_sm(pio, true);
    offset = pio_add_program(pio, &projeto_final_program);
    projeto_final_program_init(pio, sm, offset, PINO_LEDS);
}

//Função para definir a intensidade dos leds
uint32_t definirLeds(double intVermelho, double intVerde, double intAzul){
    unsigned char vermelho, verde, azul;

    vermelho = intVermelho*255;
    verde = intVerde*255;
    azul = intAzul*255;

    return (verde << 24) | (vermelho << 16) | (azul << 8);
}
  
//Função para fazer com que a matriz seja ativada, em uma cor especificada
void ligarMatriz(double r, double g, double b, double *desenho){
uint32_t valorLed;
for (int16_t i = 0; i < LEDS; i++) { 
    if(r != 0.0){
        valorLed = definirLeds(desenho[24-i], g, b);           
    }else if(g != 0.0){
        valorLed = definirLeds(r, desenho[24-i], b);    
    }else if(b != 0.0){
        valorLed = definirLeds(r, g, desenho[24-i]);           
    }
    pio_sm_put_blocking(pio, sm, valorLed);
}
}