//Projeto Final da Embarca Tech por Maria Eduarda Campos - versão provisória

#include "matriz.c"
#include "sensor_led.c"
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/i2c.h"
#include "inc/ssd1306.h"
#include "inc/font.h"
#include "matrizes.c"

#define JOYSTICK_X 27 //Joystick eixo X
#define JOYSTICK_Y 26 //Joystick eixo Y
#define BOTAO_JOY 22 //Botão do Joystick
#define BOTAO_A 5 //Botão A
#define BOTAO_B 6 //Botão B
#define BUZZER_A 21

#define I2C_PORT i2c1 //Porta do I2C
#define I2C_SDA 14 //Display SDA
#define I2C_SCL 15 //Display SCL

#define endereco 0x3C 

static volatile uint32_t ultimo_tempo = 0; //Armazena o último tempo absoluto 
static volatile bool estado_leds = 0; //Indica o estado dos LEDs
int level_atual = 0; // Mantém o nível atual dos LEDs
int valor = 0;

//Protótipo da função callback
static void gpio_irq_handler(uint gpio, uint32_t events);

//Lê a posição do Joystick
uint16_t leitura_joystick(uint adc){
    adc_select_input(adc);
    return adc_read();
}

// Função para ativar o buzzer com uma frequência específica
void ativar_buzzer(uint pino, uint16_t frequency) {
    uint slice = pwm_gpio_to_slice_num(pino);
    uint16_t level = WRAP_PERIOD / 2; // Definindo um duty cycle de 50%
    
    // Configurar a frequência do PWM
    pwm_set_clkdiv(slice, PWM_DIVISER);
    pwm_set_wrap(slice, WRAP_PERIOD);
    
    // Definindo o nível do PWM
    pwm_set_gpio_level(pino, level);
    
    // Ativar o PWM
    pwm_set_enabled(slice, true);
    
    // Atraso para manter o som por um tempo (ajustável)
    sleep_ms(500);
    
    // Desativar o PWM
    pwm_set_gpio_level(pino, 0);
}

void parar_buzzer() {
    gpio_put(BUZZER_A, 0);
}

void inicializar(){
    stdio_init_all();

    //Inicializa o Joystick
    adc_init();
    adc_gpio_init(JOYSTICK_X);
    adc_gpio_init(JOYSTICK_Y);
    adc_select_input(0);

    //Inicializa o Botão A, define como entrada e põe em nível alto enquanto não pressionado
    gpio_init(BOTAO_A);
    gpio_set_dir(BOTAO_A, GPIO_IN);
    gpio_pull_up(BOTAO_A);

    //Inicializa o Buzzer
    gpio_init(BUZZER_A);
    gpio_set_dir(BUZZER_A, GPIO_OUT);
    pwm_setup(BUZZER_A);
    
    //Inicializa o Botão B, define como entrada e põe em nível alto enquanto não pressionado
    gpio_init(BOTAO_B);
    gpio_set_dir(BOTAO_B, GPIO_IN);
    gpio_pull_up(BOTAO_B);

    //Inicializa o Botão do Joystick, define como entrada e põe em nível alto enquanto não pressionado
    gpio_init(BOTAO_JOY);
    gpio_set_dir(BOTAO_JOY, GPIO_IN);
    gpio_pull_up(BOTAO_JOY);
    
    //Chama a função de callback
    gpio_set_irq_enabled_with_callback(BOTAO_JOY, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(BOTAO_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    //Inicializa o I2C na porta i2c1 em 400Hz
    i2c_init(I2C_PORT, 400 * 1000);
}

//Função principal
int main(){
    inicializar();
    inicializar_leds();
    //Configura o I2C
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_t display; //Declara o Dysplay
    ssd1306_init(&display, 128, 64, false, endereco, I2C_PORT); //Inicializa
    ssd1306_config(&display); //Configura
    ssd1306_fill(&display, false); //Limpa o display
    ssd1306_send_data(&display); //Envia os dados do display

    //Váriavel para indicar o ativamento de um pixel
    bool cor = true;
    pwm_setup(LED_VM);
    pwm_setup(LED_VD);
    uint up_down = 1; //variável para controlar se o nível do LED aumenta ou diminui


    // Loop Infinito
    while (true) {
        // Lê os valores dos eixos X e Y
        uint16_t y_valor = leitura_joystick(0);
        uint16_t x_valor = leitura_joystick(1);

        //int16_t x_iluminacao = abs(x_valor - 1893) * 16 / 1893;
        //int16_t y_iluminacao = abs(y_valor - 2099) * 16 / 2099;

        // Limpa a tela e desenha a borda
        ssd1306_fill(&display, false); 
        ssd1306_rect(&display, 0, 0, 128, 64, true, false); 

        //Configura a posição do quadrado
        //uint8_t x_pos = x_valor * 57 / 4096;
        //uint8_t y_pos = y_valor * 121 / 4096;

        // Define a faixa central sem vibração (por exemplo, ±200 de margem)
        int16_t margem = 500;
        
        if (x_valor < (2048 - margem) || x_valor > (2048 + margem)) {
            ssd1306_draw_string(&display, "Vibracoes", 10, 48);
        } else {
            ssd1306_draw_string(&display, "Sem vibracoes", 10, 48);
        }
        if (y_valor < (2048 - margem) || y_valor > (2048 + margem)) {
            ssd1306_draw_string(&display, "Solo fofo", 10, 30);
        } else {
            ssd1306_draw_string(&display, "Solo compacto", 10, 30);
        }

        switch (level_atual) {
            case 0:
                ssd1306_draw_string(&display, "Pressione A   para ver clima", 15, 5);
                break;
            case 1:
                // Ideal (Verde 100%)
                parar_buzzer();
                ssd1306_draw_string(&display, "Temperatura        Ideal", 15, 5);
                break;
            case 2:
                // Oscilando (Verde 70%)
                ssd1306_draw_string(&display, "Temperatura      Oscilando", 15, 5);
                break;
            case 3:
                // Alerta (Verde 30%)
                ssd1306_draw_string(&display, "Temperatura        Caindo", 15, 5);
                break;
            case 4:
                // Perigo Potencial (Vermelho 30%)
                ssd1306_draw_string(&display, "Temperatura      Potencial",15, 5);
                break;
            case 5:
                // Perigo (Vermelho 70%)
                ssd1306_draw_string(&display, "Temperatura         Alta", 15, 5);
                break;
            case 6:
                ssd1306_draw_string(&display, "Temperatura       Critica", 15, 5);              
                ativar_buzzer(BUZZER_A, 2000); // Ativa o buzzer com frequência de 1000 Hz (1 kHz)
                break;
            default:
                // Caso nenhum nível seja válido
                ssd1306_draw_string(&display, "Temperatura     Desconhecida", 15, 5);
                break;
        }
        
        // Envia os dados para o display apenas uma vez no final
        ssd1306_send_data(&display);

        sleep_ms(100); // Delay
    }
    return 0;
}

//Função de Callback
void gpio_irq_handler(uint botao, uint32_t eventos) {
    uint32_t tempo_real = to_us_since_boot(get_absolute_time());

    // Debounce: Verifica se passaram pelo menos 200ms desde o último acionamento
    if (tempo_real - ultimo_tempo > 200000) {
        if (botao == BOTAO_A) { // Botão A pressionado
            estado_leds = !estado_leds; // Alterna o estado
            if (estado_leds) { // Estado ativo
                if (level_atual == 0) {
                    pwm_set_gpio_level(LED_VD, valores_VD[nivel]);
                    pwm_set_gpio_level(LED_VM, valores_VM[nivel]);
                    level_atual = 1;
                    nivel--;
                } else if (level_atual == 1) {
                    pwm_set_gpio_level(LED_VD, valores_VD[nivel]);
                    pwm_set_gpio_level(LED_VM, valores_VM[nivel]);
                    level_atual = 2;
                    nivel--;
                } else if (level_atual == 2) {
                    pwm_set_gpio_level(LED_VD, valores_VD[nivel]);
                    pwm_set_gpio_level(LED_VM, valores_VM[nivel]);
                    level_atual = 3;
                    nivel--;
                } else if (level_atual == 3) {
                    pwm_set_gpio_level(LED_VD, valores_VD[nivel]);                    
                    pwm_set_gpio_level(LED_VM, valores_VM[nivel]);
                    level_atual = 4;
                    nivel--;
                } else if (level_atual == 4) { // LED_VM em 400
                    pwm_set_gpio_level(LED_VM, valores_VM[nivel]);
                    pwm_set_gpio_level(LED_VD, valores_VD[nivel]);
                    level_atual = 5;
                    nivel--;
                } else if (level_atual == 5) { // LED_VM em 2400
                    pwm_set_gpio_level(LED_VM, valores_VM[nivel]);
                    pwm_set_gpio_level(LED_VD, valores_VD[nivel]);
                    level_atual = 6;
                } else if (level_atual == 6) { // LED_VM em 4000, volta para LED_VD
                    nivel = 5;
                    pwm_set_gpio_level(LED_VM, valores_VM[nivel]);
                    pwm_set_gpio_level(LED_VD, valores_VD[nivel]);
                    level_atual = 1;                  
                }               
            }
        } else if (botao == BOTAO_JOY) { 
            // Lógica para botão do joystick (se necessário)
        } else if (botao == BOTAO_B) { 
            // Lógica para botão B (se necessário)
        }
        // Atualiza o tempo do último acionamento
        ultimo_tempo = tempo_real;
    }
}