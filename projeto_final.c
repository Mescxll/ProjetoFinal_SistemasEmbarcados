//Projeto Final da Embarca Tech por Maria Eduarda Campos - versão provisória

//Bibliotecas -----------------------------------------------------------------------------------------------------------

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/i2c.h"
#include "inc/ssd1306.h"
#include "inc/font.h"
#include "matrizes.c"
#include "projeto_final.pio.h"
#include "hardware/pio.h"

//Váriaveis ------------------------------------------------------------------------------------------------------------

#define JOYSTICK_X 27 //Joystick eixo X
#define JOYSTICK_Y 26 //Joystick eixo Y
#define BOTAO_A 5 //Botão A
#define BOTAO_B 6 //Botão B
#define BUZZER_A 21 //Buzzer
#define LED_VM 13 // LED vermelho
#define LED_VD 11 // LED verde
#define LEDS 25 //Número de LEDs
#define PINO_LEDS 7 //Pino da Matriz de LEDs

#define I2C_PORT i2c1 //Porta do I2C
#define I2C_SDA 14 //Display SDA
#define I2C_SCL 15 //Display SCL
#define endereco 0x3C 

ssd1306_t display;  //Display

static volatile uint numero = -1; //Indica a figura mostrada na Matriz
static volatile uint a = 1; //Váriavel de incremento
static volatile uint32_t ultimo_tempo = 0; //Armazena o último tempo absoluto 
static volatile bool estado_leds = 0; //Indica o estado dos LEDs
int nivel_atual = 0; //Armazena o LED atual

//Sm e outras variaveis definidas como estáticas para não estourar o limite de Sm por PIO
static PIO pio = pio0;
static uint sm;
static uint offset;

//Variáveis que definem a cor usada nos LEDs da Matriz
double r = 0.0, g = 0.0, b = 1.0;

//Protótipo da função callback
static void gpio_irq_handler(uint gpio, uint32_t events);

//Váriaveis PWM
const uint16_t WRAP_PERIOD = 4000;
const float PWM_DIVISER = 4.0;
uint16_t level = 200; 

//Níveis de Umidade

// 5 Crítica -> coluna 4
// 4 Alta -> coluna 3
// 3 Potencial -> coluna 2
// 2 Oscilando -> coluna 1
// 1 Ideal -> coluna 0

//Níveis de Temperatura

// 6 Crítica (vermelho 100%) -> 4000
// 5 Alta (vermelho 60%) -> 2400
// 4 Potencial (vermelho 10%) -> 400
// 3 Subindo (verde 10%) -> 400
// 2 Oscilando (verde 60%) -> 2400
// 1 Ideal (verde 100%) -> 4000

// Definição dos níveis
int valores_VM[] = {4000, 2400, 400, 0, 0, 0};  // Intensidade do LED vermelho
int valores_VD[] = {0, 0, 0, 400, 2400, 4000};  // Intensidade do LED verde
int nivel = 5;

//Funções --------------------------------------------------------------------------------------------------------------------

//Função para configurar o módulo PWM
void configurar_pwm(uint pino) {
    gpio_set_function(pino, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(pino);
    pwm_set_clkdiv(slice, PWM_DIVISER);
    pwm_set_wrap(slice, WRAP_PERIOD);
    pwm_set_gpio_level(pino, 0);
    pwm_set_enabled(slice, true);
}

//Função para ler a posição do Joystick
uint16_t leitura_joystick(uint adc){
    adc_select_input(adc);
    return adc_read();
}

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

void reiniciar_display(){
    ssd1306_fill(&display, false); //Limpa o Display
    ssd1306_rect(&display, 0, 0, 128, 64, true, false); //Desenha Borda 
}

//Função para ativar o Buzzer com uma frequência específica
void ativar_buzzer(uint pino, uint16_t frequencia) {
    reiniciar_display();
    //Emite mensagem de Alerta
    ssd1306_draw_string(&display, "Alerta", 28, 15); 
    ssd1306_draw_string(&display, "Anomalias      Detectadas", 20, 25); 
    ssd1306_send_data(&display); //Envia os dados para o display

    uint slice = pwm_gpio_to_slice_num(pino);
    uint16_t level = WRAP_PERIOD / 2; 
    pwm_set_clkdiv(slice, PWM_DIVISER);
    pwm_set_wrap(slice, WRAP_PERIOD);
    pwm_set_gpio_level(pino, level);
    pwm_set_enabled(slice, true);
    
    sleep_ms(500); //Atraso para manter o som
    pwm_set_gpio_level(pino, 0);//Desativa o PWM
}

//Função para desativar o Buzzer
void parar_buzzer() {
    gpio_put(BUZZER_A, 0);
}

//Função para fazer as inicializações necessárias
void inicializar(){
    //Inicializa as bibliotecas de entrada e saída padrão
    stdio_init_all();

    //Inicializa o Joystick
    adc_init();
    adc_gpio_init(JOYSTICK_X);
    adc_gpio_init(JOYSTICK_Y);
    adc_select_input(0);

    //Inicializa os LEDs Vermelho e Verde, define seus pinos como saída e os configura como PWM
    gpio_init(LED_VM);
    gpio_set_dir(LED_VM, GPIO_OUT);
    gpio_init(LED_VD);
    gpio_set_dir(LED_VD, GPIO_OUT);
    configurar_pwm(LED_VM);
    configurar_pwm(LED_VD);

    //Inicializa o Botão A, define como entrada e põe em nível alto enquanto não pressionado
    gpio_init(BOTAO_A);
    gpio_set_dir(BOTAO_A, GPIO_IN);
    gpio_pull_up(BOTAO_A);

    //Inicializa o Botão B, define como entrada e põe em nível alto enquanto não pressionado
    gpio_init(BOTAO_B);
    gpio_set_dir(BOTAO_B, GPIO_IN);
    gpio_pull_up(BOTAO_B);

    //Inicializa o Buzzer e configura como PWM
    gpio_init(BUZZER_A);
    gpio_set_dir(BUZZER_A, GPIO_OUT);
    configurar_pwm(BUZZER_A);

    //Inicializa o I2C na porta i2c1 em 400Hz
    i2c_init(I2C_PORT, 400 * 1000);
}

//Função principal ------------------------------------------------------------------------------------------------------------------

int main(){
    //Chama a função de inicialização e configura o PIO
    inicializar();
    configurar_pio();
    
    //Limpa a Matriz
    ligarMatriz(r, g, b, figura_zeros);

    //Configura o I2C
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    //Configura o display
    ssd1306_init(&display, 128, 64, false, endereco, I2C_PORT); 
    ssd1306_config(&display); 
    reiniciar_display();


    //Chama a função de callback para os Botões A e B
    gpio_set_irq_enabled_with_callback(BOTAO_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(BOTAO_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    // Loop Infinito While
    while (true) {       
        // Lê os valores dos eixos X e Y
        uint16_t y_valor = leitura_joystick(0);
        uint16_t x_valor = leitura_joystick(1);

        reiniciar_display();
       
        // Define a faixa central do JoyStick
        int16_t margem = 600;
        
        //Define um intervalo onde o centro do eixo X indica âusencia de Vibrações no solo e as suas extremidades indicam presença de Vibrações
        if (x_valor < (2048 - margem) || x_valor > (2048 + margem)) {
            ssd1306_draw_string(&display, "Vibracoes", 10, 20);
        } else {
            ssd1306_draw_string(&display, "Sem vibracoes", 10, 20);
        }

        //Define um intervalo onde o centro do eixo Y indica que o solo está Compacto e as suas extremidades indicam pouca Compactividade
        if (y_valor < (2048 - margem) || y_valor > (2048 + margem)) {
            ssd1306_draw_string(&display, "Solo fofo", 10, 32);
        } else {
            ssd1306_draw_string(&display, "Solo compacto", 10, 32);
        }

        if((y_valor < (2048 - margem) || y_valor > (2048 + margem)) && (x_valor < (2048 - margem) || x_valor > (2048 + margem))){
            ativar_buzzer(BUZZER_A, 2000);
        }else{
            parar_buzzer();
        }

        //Define, de acordo a representação da Matriz, qual o nível de umidade do solo
        switch (numero) {
            case 0: // Ideal              
                parar_buzzer();
                ssd1306_draw_string(&display, "Umidade         Ideal", 25, 45);
                break;
            case 1: // Oscilando 
                ssd1306_draw_string(&display, "Umidade       Oscilando", 25, 45);
                break;          
            case 2: //Potencial 
                ssd1306_draw_string(&display, "Umidade       Potencial",25, 45);
                break;
            case 3: // Alta
                ssd1306_draw_string(&display, "Umidade         Alta", 25, 45);
                break;
            case 4: //Crítica
                ssd1306_draw_string(&display, "Umidade        Critica", 25, 45);              
                ativar_buzzer(BUZZER_A, 2000); // Ativa o buzzer com frequência de 2000 Hz
                break;
            default:
                ssd1306_draw_string(&display, "Pressione B     para umidade", 15, 45);
                break;
        }

        //Define, de acordo a representação do LED, qual a temperatura do solo
        switch (nivel_atual) {
            case 0:
                ssd1306_draw_string(&display, "Pressione A      para clima", 15, 1);
                break;
            case 1: //Ideal (Verde 100%)
                parar_buzzer();
                ssd1306_draw_string(&display, "Temperatura        Ideal", 15, 1);
                break;
            case 2: // Oscilando (Verde 60%)
                ssd1306_draw_string(&display, "Temperatura      Oscilando", 15, 1);
                break;
            case 3: //Subindo (Verde 10%)
                ssd1306_draw_string(&display, "Temperatura        Subindo", 15, 1);
                break;
            case 4://Potencial (Vermelho 10%)
                ssd1306_draw_string(&display, "Temperatura      Potencial",15, 1);
                break;
            case 5:
                //Alta (Vermelho 60%)
                ssd1306_draw_string(&display, "Temperatura         Alta", 15, 1);
                break;
            case 6: //Crítica (Vermelho 100%)
                ssd1306_draw_string(&display, "Temperatura       Critica", 15, 1);              
                ativar_buzzer(BUZZER_A, 2000); // Ativa o buzzer com frequência de 1000 Hz (1 kHz)
                break;
            default:
                //Caso nenhum nível seja válido
                ssd1306_draw_string(&display, "Temperatura     Desconhecida", 15, 1);
                break;
        }
        
        //Envia os dados para o display
        ssd1306_send_data(&display);

        // Delay
        sleep_ms(50); 
    }
    return 0;
}

//Função de Callback --------------------------------------------------------------------------------------------------------------

void gpio_irq_handler(uint botao, uint32_t eventos) {
    uint32_t tempo_real = to_us_since_boot(get_absolute_time()); //Obtém o tempo real
    
    //Debouncing
    if (tempo_real - ultimo_tempo > 200000) { //Verifica se passou 200ms desde o último evento
        
        if (botao == BOTAO_A) { // Botão A pressionado
           
            estado_leds = !estado_leds;  // Alterna o estado dos leds
            if (estado_leds) { 
                if (nivel_atual == 0) { //Verde 100%
                    pwm_set_gpio_level(LED_VD, valores_VD[nivel]);
                    pwm_set_gpio_level(LED_VM, valores_VM[nivel]);
                    nivel_atual = 1;
                    nivel--;
                } else if (nivel_atual == 1) { //Verde 60%
                    pwm_set_gpio_level(LED_VD, valores_VD[nivel]);
                    pwm_set_gpio_level(LED_VM, valores_VM[nivel]);
                    nivel_atual = 2;
                    nivel--;
                } else if (nivel_atual == 2) { //Verde 10%
                    pwm_set_gpio_level(LED_VD, valores_VD[nivel]);
                    pwm_set_gpio_level(LED_VM, valores_VM[nivel]);
                    nivel_atual = 3;
                    nivel--;
                } else if (nivel_atual == 3) { //Vermelho 10%
                    pwm_set_gpio_level(LED_VD, valores_VD[nivel]);                    
                    pwm_set_gpio_level(LED_VM, valores_VM[nivel]);
                    nivel_atual = 4;
                    nivel--;
                } else if (nivel_atual == 4) { //Vermelho 60%
                    pwm_set_gpio_level(LED_VM, valores_VM[nivel]);
                    pwm_set_gpio_level(LED_VD, valores_VD[nivel]);
                    nivel_atual = 5;
                    nivel--;
                } else if (nivel_atual == 5) { //Vermelho 100%
                    pwm_set_gpio_level(LED_VM, valores_VM[nivel]);
                    pwm_set_gpio_level(LED_VD, valores_VD[nivel]);
                    nivel_atual = 6;
                } else if (nivel_atual == 6) { //Volta para Verde 100%
                    nivel = 5;
                    pwm_set_gpio_level(LED_VM, valores_VM[nivel]);
                    pwm_set_gpio_level(LED_VD, valores_VD[nivel]);
                    nivel_atual = 1;                  
                }               
            }
        } 

        if (botao == BOTAO_B) { // Botão B pressionado 
            numero = (numero + 1) % 5;
            ligarMatriz(r, g, b, figuras[numero]); //Mostra as figuras do vetor na Matriz (de 0 a 4) cada vez que pressionado o Botão          
            a++;
        }
        // Atualiza o tempo do último acionamento
        ultimo_tempo = tempo_real;      
    }   
}