/********************************************************************

    Programa-exemplo para a disciplina de Microcontroladores da
    Universidade Federal de Pernambuco

    Copyright (C) 2023  Hermano A. Cabral

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.

 *******************************************************************/

#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "timer.h"
#include "uart.h"
#include "gpio.h"


/********************************************************************
 **
 ** INTRODUÇÃO
 ** ==========
 **
 ** Este programa é um exemplo do uso das funcionalidades de
 ** temporizador, comunicação serial e portas digitais que
 ** desenvolvemos na disciplina de Microcontroladores - 2022.2 da
 ** UFPE.
 **
 ** Ele pisca o led da placa do Arduino Nano (pino PB5) em diferentes
 ** frequências. O usuário pode, através da interface de comunicação
 ** serial, aumentar (enviando o caractere '+') ou diminuir (enviando
 ** '-') a frequência de piscagem do led.  Além disso, ele pode
 ** desligar o led enviando o dígito 0.
 **
 ** O led pode ser controlado também por 3 push buttons conectados aos
 ** pinos PD0, PD1 e PD2. Esta funcionalidade é deixada como exercício
 ** para você.
 **
 *******************************************************************/

/********************************************************************
 **
 ** ESTRUTURA DO CÓDIGO
 ** ===================
 **
 ** O código abaixo está estruturado em 5 partes: a primeira para
 ** definição de constantes e variáveis globais, a segunda para as
 ** funcionalidades do temporizador, a terceira para a serial, a
 ** quarta para os pinos digitais, e a última para a função main().
 **
 ** O programa faz uso das funções de callback para implementar a
 ** lógica relativa à comunicação serial e ao temporizador, liberando
 ** a CPU para lidar com os pinos digitais de entrada. Observe que se
 ** usássemos a funcionalidade de interrupção externa, a CPU seria
 ** liberada de monitorar os pinos de entrada também, podendo se
 ** dedicar a outras coisas ou ser colocada em um modo de redução de
 ** consumo de energia.
 **
 *******************************************************************/


/********************************************************************
 ** Definição de constantes e variáveis globais
 *******************************************************************/
#define MSK_LED 0X20
#define MSK_INPUT 0x07
#define MAX_PERIOD 8000
#define MIN_PERIOD 1000
#define STATE_LED_ON 1
#define STATE_LED_OFF 0
#define INPUT_PORT GPIOD4
#define OUTPUT_PORT GPIOD2

/* Contador usado pelo callback do temporizador para estender o seu
   período */
static int16_t g_ctr;

/* Variável para guardar o período desejável para piscar o led */
static int16_t g_set_period;

/* Índice para a posição atual no array de períodos */
static int16_t g_idx;

/* Variável para indicar o estado do LED */
static uint8_t g_state_led;

/* Array para armazenar os possíveis períodos. Observe que armazenamos
   o array na memória flash ao invés da memória RAM através do código
   PROGMEM */
static const int16_t g_possible_periods[] PROGMEM = {
    16000, 8000, 5333, 4000, 2666, 2000, 1600};

/* Constante definidndo o tamanho do arra acima */
#define NBR_PERIODS sizeof(g_possible_periods)/sizeof(g_possible_periods[0])



/********************************************************************
 ** Definição das funcionalidades relativas ao temporizador
 *******************************************************************/
/*
 * Callback chamado quando há um overflow do temporizador. 
 *
 * Com este callback podemos implementar períodos de contagem maiores
 * do que o período do temporizador em si.
 */
void timer_cb(GPT_t* drv) {
    if (--g_ctr <= 0) {
	gpio_toggle_group(OUTPUT_PORT, MSK_LED);
	g_ctr = g_set_period;
    }
}

/* 
 * Função de inicialização do temporizador
 *
 * Observe que inicializamos as variáveis usadas no callback também.
 */
void init_timer() {
    GPT_Config cfg = {MODE_NORMAL, DIVISOR_8, 0xFF};
    gpt_init();
    gpt_start(GPTD1, &cfg);
    g_idx = 0;
    /* Observe que para ler o valor desta variável que está armazenada
       na memória flash temos que usar a função pgm_read_word */
    g_set_period = (int16_t) pgm_read_word(g_possible_periods);
    g_state_led = STATE_LED_ON;
    gpt_start_notification(GPTD1, timer_cb, 0);
}



/********************************************************************
 ** Definição das funcionalidades relativas à serial
 *******************************************************************/
/* 
 * Callback para tratar dados vindo da serial
 *
 *
 */
void uart_rx_cb(Uart_t* drv, uint8_t ch) {
    if (ch == '+') {
        if (g_state_led == STATE_LED_OFF) {
            g_state_led = STATE_LED_ON;
            gpio_set_group(OUTPUT_PORT, MSK_LED);
            gpt_start_notification(GPTD1, timer_cb, 0);
        }
	if (g_idx < NBR_PERIODS - 1)
            g_ctr = g_set_period = (int16_t) pgm_read_word(g_possible_periods + ++g_idx);
    } else if (ch == '-') {
        if (g_state_led == STATE_LED_OFF) {
            g_state_led = STATE_LED_ON;
            gpio_set_group(OUTPUT_PORT, MSK_LED);
            gpt_start_notification(GPTD1, timer_cb, 0);
        }
	if (g_idx > 0)
            g_ctr = g_set_period = (int16_t) pgm_read_word(g_possible_periods + --g_idx);
    } else if (ch == '0') {
        gpt_stop_notification(GPTD1);
        gpio_clear_group(OUTPUT_PORT, MSK_LED);
        g_state_led = STATE_LED_OFF;
    } else if ((ch == '\x0a') || (ch == '\x0d')) {
        return;
    } else {
        uart_writechar(UARTD1, '?');
        uart_writechar(UARTD1, '\n');
        return;
    }

    uart_writechar(UARTD1, 'O');
    uart_writechar(UARTD1, 'K');
    uart_writechar(UARTD1, '\n');
}

/* 
 * Função de inicialização do temporizador
 *
 * Função para inicialização do que será usado: pinos de entrada e
 * saída, comunicação serial e temporizador. Além da inicialização dos
 * módulos de hardware, inicializamos também as variáveis usadas nas
 * suas funcionalidades.
 */
void init_uart() {
    Uart_Config_t uart_cfg = {115200, 0, 8, 1, uart_rx_cb, 0};

    uart_init(UARTD1);
    uart_start(UARTD1, &uart_cfg, 1, 1);
}



/********************************************************************
 ** Definição das funcionalidades relativas aos pinos digitais
 *******************************************************************/
void init_gpio() {
    /* Inicialização do pino PB5 como saída (led da placa) e dos pinos
       PD0, PD1 e PD2 como entrada. Observe que o pino de saída PB5
       está sendo inicializado como um grupo de 1 bit como
       demonstração das funcionalidades de grupo de pinos. Por ser
       apenas um pino, ele poderia ser inicializado de forma mais
       simples com as funcionalidade de GPIO para um único pino. */
    GPIO_mode mode[PORT_WIDTH];

    mode[5] = GPIO_OUT;
    gpio_set_group_mode(OUTPUT_PORT, MSK_LED, mode);

    mode[0] = GPIO_IN_PULLUP;
    mode[1] = GPIO_IN_PULLUP;
    mode[2] = GPIO_IN_PULLUP;

    gpio_set_group_mode(INPUT_PORT, MSK_INPUT, mode);
}



/********************************************************************
 ** Definição da função main
 *******************************************************************/
/*
 * Função para implementar um atraso no tempo usando a CPU.
 *
 *  A duração do atraso é em milissegundos e é dado pelo parâmetro da
 *  função.
 */
void delay_ms(uint16_t ms) {
    uint16_t counter;
    uint16_t i;

    for (i=ms; i>0; i--) {
	counter = 3500;
        while (counter-- > 0)
            asm("nop");
    }
}

/*
 * Função main
 */
int main() {
    /* Inicialização e habilitação das interrupções */
    sei();
    init_uart();
    init_timer();
    init_gpio();

    /* Uso da CPU para monitorar as 3 portas de entrada usadas como
       exemplo. ESTA FUNCIONALIDADE ESTÁ INCOMPLETA. Caso queira
       usá-la, você deve completar o código. */
    uint8_t old_port_value = gpio_read_port(INPUT_PORT);
    uint8_t port_value;
    uint8_t port_change;

    while (1) {
        port_value = gpio_read_port(INPUT_PORT);
        if (old_port_value == port_value)
            continue;
        port_change = port_value ^ old_port_value;
        old_port_value = port_value;

        /* PARA COMPLETAR */

        /* Possível lógica para implementar o debouncing de uma chave,
           onde, após a detecção de uma mudança acima, esperamos 1
           décimo de segundo para reler a porta */
        delay_ms(100);
    }
}
