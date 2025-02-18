#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/timer.h"
#include "hardware/clocks.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"
#include "lib/ssd1306.h"
#include "lib/font.h"


// Definições dos pinos do LED RGB
#define LED_VERDE   11  // LED Verde (Digital)
#define LED_AZUL    12  // LED Azul (PWM)
#define LED_VERMELHO 13 // LED Vermelho (PWM)

// Configuração dos botões
#define BOTAO_A  5        // Botão principal para ativação/desativação do LED
#define BOTAO_JOYSTICK  22 // Botão embutido no joystick

// Configuração dos eixos do joystick (Entradas analógicas)
#define JOYSTICK_Y  26  // Leitura do eixo Y
#define JOYSTICK_X  27  // Leitura do eixo X

// Ajustes de sensibilidade do joystick
#define LIMITE_SUPERIOR 2197 // Ponto máximo de desvio do joystick
#define LIMITE_INFERIOR 1897 // Ponto mínimo de desvio do joystick

// Configuração do PWM para controle de brilho dos LEDs
#define PWM_WRAP 4095  // Valor máximo para PWM 
#define PWM_DIV  1.0   // Fator de divisão da frequência do PWM

// Configuração do display OLED via I2C
#define I2C_CANAL i2c1
#define I2C_PINO_SDA 14
#define I2C_PINO_SCL 15
#define ENDERECO_DISPLAY 0x3C

// Controle de tempo para debounce dos botões
static volatile uint32_t tempo_ultimo_press_a = 0;
static volatile uint32_t tempo_ultimo_press_jstk = 0;

// Flags de controle
static volatile bool estado_led = true;  // Indica se o LED está ligado ou desligado
static volatile uint8_t contador_bordas = 0; // Contador para alternar bordas no display
static volatile bool borda_externa = false, borda_media = false; // Controle de bordas do display

// Configuração dos LEDs (um digital e dois PWM)
void configurar_leds() {
    uint slice_pwm;

    // Configuração do LED Verde como saída digital
    gpio_init(LED_VERDE);
    gpio_set_dir(LED_VERDE, GPIO_OUT);
    gpio_put(LED_VERDE, 0); // Inicialmente desligado

    // Configuração do LED Azul como saída PWM
    gpio_set_function(LED_AZUL, GPIO_FUNC_PWM);
    slice_pwm = pwm_gpio_to_slice_num(LED_AZUL);
    pwm_set_clkdiv(slice_pwm, PWM_DIV);
    pwm_set_wrap(slice_pwm, PWM_WRAP);
    pwm_set_gpio_level(LED_AZUL, 0); // Inicialmente desligado
    pwm_set_enabled(slice_pwm, true);

    // Configuração do LED Vermelho como saída PWM
    gpio_set_function(LED_VERMELHO, GPIO_FUNC_PWM);
    slice_pwm = pwm_gpio_to_slice_num(LED_VERMELHO);
    pwm_set_clkdiv(slice_pwm, PWM_DIV);
    pwm_set_wrap(slice_pwm, PWM_WRAP);
    pwm_set_gpio_level(LED_VERMELHO, 0); // Inicialmente desligado
    pwm_set_enabled(slice_pwm, true);
}
// Configuração do joystick (botão e leitura dos eixos analógicos)
void configurar_joystick() {
    gpio_init(BOTAO_JOYSTICK);
    gpio_set_dir(BOTAO_JOYSTICK, GPIO_IN);
    gpio_pull_up(BOTAO_JOYSTICK);

    adc_init(); // Inicializa o ADC para leitura dos eixos analógicos
    adc_gpio_init(JOYSTICK_X);
    adc_gpio_init(JOYSTICK_Y);
}


// Retorna o valor do eixo Y do joystick (0 a 4095)
uint16_t ler_eixo_y() {
    adc_select_input(0);
    return adc_read();
}

// Retorna o valor do eixo X do joystick (0 a 4095)
uint16_t ler_eixo_x() {
    adc_select_input(1);
    return adc_read();
}


//onfiguração do display OLED via I2C
void configurar_display(ssd1306_t *display) {
    i2c_init(I2C_CANAL, 400 * 1000);
    gpio_set_function(I2C_PINO_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_PINO_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_PINO_SDA);
    gpio_pull_up(I2C_PINO_SCL);

    ssd1306_init(display, WIDTH, HEIGHT, false, ENDERECO_DISPLAY, I2C_CANAL);
    ssd1306_config(display);
    ssd1306_send_data(display);
    ssd1306_fill(display, false); //inicializa com tela limpa
    ssd1306_send_data(display);
}
// Configuração do botão principal (BOTAO_A) com pull-up interno
void configurar_botao() {
    gpio_init(BOTAO_A);
    gpio_set_dir(BOTAO_A, GPIO_IN);
    gpio_pull_up(BOTAO_A);
}

// Interrupção acionada quando um dos botões for pressionado
void tratar_interrupcao_botao(uint gpio, uint32_t eventos) {
    uint32_t tempo_atual = to_ms_since_boot(get_absolute_time());

    // Alterna o LED quando o botão A for pressionado (com debounce)
    if ((gpio == BOTAO_A) && (tempo_atual - tempo_ultimo_press_a > 200)) {
        tempo_ultimo_press_a = tempo_atual;
        estado_led = !estado_led;
    }
    // Alterna as bordas do display e o LED verde quando o botão do joystick for pressionado
    if ((gpio == BOTAO_JOYSTICK) && (tempo_atual - tempo_ultimo_press_jstk > 200)) {
        tempo_ultimo_press_jstk = tempo_atual;
        gpio_put(LED_VERDE, !gpio_get(LED_VERDE)); // Inverte o estado do LED Verde
        // Alterna entre três estados de borda no display OLED
        if (contador_bordas == 0) {
            contador_bordas++;
            borda_externa = true;
        } else if (contador_bordas == 1) {
            contador_bordas++;
            borda_media = true;
        } else {
            contador_bordas = 0;
            borda_externa = false;
            borda_media = false;
        }
    }
}

int main() {
    uint16_t eixo_x, eixo_y;
    uint8_t pos_x = 3, pos_y = 3;
    ssd1306_t tela;
    uint32_t tempo_atual, tempo_ultimo_debug = 0;

    // Inicializações do sistema
    stdio_init_all();
    configurar_botao();
    configurar_leds();
    configurar_display(&tela);
    configurar_joystick();

    // Configuração de interrupções dos botões
    gpio_set_irq_enabled_with_callback(BOTAO_A, GPIO_IRQ_EDGE_FALL, true, &tratar_interrupcao_botao);
    gpio_set_irq_enabled_with_callback(BOTAO_JOYSTICK, GPIO_IRQ_EDGE_FALL, true, &tratar_interrupcao_botao);

    while (true) {
        sleep_ms(20);
        // Leitura dos valores do joystick
        eixo_x = ler_eixo_x();
        eixo_y = ler_eixo_y();

        //controle dos LEDs baseado na posição do joystick
        if (estado_led) {
            if (eixo_x >= LIMITE_SUPERIOR) {
                pwm_set_gpio_level(LED_VERMELHO, (eixo_x - 2047) * 2);
            } else if (eixo_x <= LIMITE_INFERIOR) {
                pwm_set_gpio_level(LED_VERMELHO, (2047 - eixo_x) * 2);
            } else {
                pwm_set_gpio_level(LED_VERMELHO, 0);
            }

            if (eixo_y >= LIMITE_SUPERIOR) {
                pwm_set_gpio_level(LED_AZUL, (eixo_y - 2047) * 2);
            } else if (eixo_y <= LIMITE_INFERIOR) {
                pwm_set_gpio_level(LED_AZUL, (2047 - eixo_y) * 2);
            } else {
                pwm_set_gpio_level(LED_AZUL, 0);
            }
        } else {
            pwm_set_gpio_level(LED_VERMELHO, 0);
            pwm_set_gpio_level(LED_AZUL, 0);
        }

        ssd1306_rect(&tela, pos_y, pos_x, 8, 8, false, true);
         // Atualiza a posição do quadrado no display
        pos_y = 53 - ((eixo_y / 4095.0) * 50);
        pos_x = ((eixo_x / 4095.0) * 114) + 3;

        ssd1306_rect(&tela, 0, 0, 128, 64, borda_externa, false);
        ssd1306_rect(&tela, 1, 1, 126, 62, borda_media, false);
        ssd1306_rect(&tela, 2, 2, 124, 60, true, false);
        ssd1306_rect(&tela, pos_y, pos_x, 8, 8, true, true);
        ssd1306_send_data(&tela);

        tempo_atual = to_ms_since_boot(get_absolute_time());
        if ((tempo_atual - tempo_ultimo_debug) >= 200) {
            tempo_ultimo_debug = tempo_atual;
            printf("Posição do eixo X: %u Posição do eixo Y: %u\n", pos_x, pos_y);
        }
    }

    return 0;
}
