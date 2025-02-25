//Projeto Final da Embarca Tech por Maria Eduarda Campos - versão provisória

#include "matriz.c"
#include "sensor_led.c"
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/i2c.h"
#include "inc/ssd1306.h"
#include "inc/font.h"

#define JOYSTICK_X 27 //Joystick eixo X
#define JOYSTICK_Y 26 //Joystick eixo Y
#define BOTAO_JOY 22 //Botão do Joystick
#define BOTAO_A 5 //Botão A

#define I2C_PORT i2c1 //Porta do I2C
#define I2C_SDA 14 //Display SDA
#define I2C_SCL 15 //Display SCL

#define endereco 0x3C 

//Váriaveis PWM
const float DIVIDER_PWM = 256.f; 
const uint16_t PERIOD = 4096;

static volatile uint32_t ultimo_tempo = 0; //Armazena o último tempo absoluto 
static volatile bool estado_leds = 1; //Indica o estado dos LEDs

//Protótipo da função callback
static void gpio_irq_handler(uint gpio, uint32_t events);


//Lê a posição do Joystick
uint16_t leitura_joystick(uint adc){
    adc_select_input(adc);
    return adc_read();
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
        ssd1306_draw_string(&display, "Vibracoes", 10, 40);
    } else {
        ssd1306_draw_string(&display, "Sem vibracoes", 10, 40);
    }
    if (y_valor < (2048 - margem) || y_valor > (2048 + margem)) {
        ssd1306_draw_string(&display, "Solo fofo", 10, 15);
    } else {
        ssd1306_draw_string(&display, "Solo compacto", 10, 15);
    }

    // Envia os dados para o display apenas uma vez no final
    ssd1306_send_data(&display);

    sleep_ms(100); // Delay
}

    return 0;
}

//Função de Callback
void gpio_irq_handler(uint botao, uint32_t eventos){
    //Armazena o tempo absoluto do sistema
    uint32_t tempo_real = to_us_since_boot(get_absolute_time());

    //Debouncing
    if (tempo_real - ultimo_tempo > 200000) { //Verifica se passou 200ms desde o último evento
        if (botao == BOTAO_A) { //Caso botão A
            estado_leds = !estado_leds; //Muda a váriavel de estado dos LEDs
            if (!estado_leds) { //Desliga os LEDs
               
            }
        } else if (botao == BOTAO_JOY) { //Caso botão do joystick
           
        }
        ultimo_tempo = tempo_real; //Atualiza o tempo
    }
}