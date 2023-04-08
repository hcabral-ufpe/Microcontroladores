#include <avr/io.h>
#include <avr/interrupt.h>
#include "uart.h"
#include "circular_buffer.h"


/*
 * Definição dos tipos privados relacionados com o módulo serial
 */

/* Estrutura para encapsular os registradores referentes ao módulo
   serial de modo a permitir um fácil acesso a ele pelas funções
   envolvidas */
typedef struct {
    volatile uint8_t ucsra;
    volatile uint8_t ucsrb;
    volatile uint8_t ucsrc;
    volatile uint8_t reservado1;
    volatile uint8_t ubrrl;
    volatile uint8_t ubrrh;
    volatile uint8_t udr;
} Uart_Regs_t;

/* Estrutura encapsulando o que é necessário para implementar o módulo
   serial com buffers */
struct Uart_Struct {
    Uart_Regs_t* regs;
    circular_buffer_t rx_buf;
    circular_buffer_t tx_buf;
    uart_rx_cb_t rx_cb;
    uart_tx_cb_t tx_cb;
};

/*
 * Definição da variável relativa aos módulos de hardware de
 * comunicação serial (o ATmega328p só tem um, e portanto só há uma
 * variável abaixo)
 */
Uart_t UART1_var;
Uart_t* UARTD1 = &UART1_var;

/*
 * Inicializa as variáveis acima para deixá-las prontas para o uso das
 * outras funções. Esta função deve ser chamada no início do programa.
 */
void uart_init(Uart_t* drv) {
    UART1_var.regs = (Uart_Regs_t *) &UCSR0A;
    UART1_var.rx_cb = 0;
    UART1_var.tx_cb = 0;
    cb_init(&UARTD1->rx_buf);
    cb_init(&UARTD1->tx_buf);

    /* PARA A SERIAL, NÃO HÁ MAIS NADA A IMPLEMENTAR AQUI */
}

/*
 * Configura e ativa o transmissor e/ou o receptor (1 ativa, 0 mantém
 * como está). A configuração consiste dos parâmetros de comunicação
 * (paridade, número de bits de dados, número de bits de stop e
 * velocidade de comunicação). Como esta implementação do módulo
 * serial usa interrupções, esta função deve habilitá-las também.
 */
void uart_start(Uart_t* drv, Uart_Config_t* cfg,
                uint8_t transmitter_on, uint8_t receiver_on) {
    /* DESENVOLVA SEU CÓDIGO AQUI */
}

/*
 * Desativa o transmissor e/ou o receptor (1 desativa, 0 mantém como
 * está). Como esta implementação do módulo serial usa interrupções,
 * esta função deve desabilitá-las também.
 *
 * Esta função só retorna quando o registrador paralelo-serial do transmissor se esvazia, o que, dependendo da velocidade de transmissão, pode ser um longo tempo para a CPU.
 */
void uart_stop(Uart_t* drv, uint8_t transmitter_off, uint8_t receiver_off) {
    /* DESENVOLVA SEU CÓDIGO AQUI */
    /* Lembre-se de só retornar depois da flag TXC0 for setada
       indicando que o último byte foi transmitido */
}

/*
 * Esta função escreve um caractere no buffer de transmissão se tiver
 * espaço e retorna. Se o buffer estiver cheio, espera liberar espaço.
 */
void uart_writechar(Uart_t* drv, uint8_t ch) {
    while (is_cb_full(&drv->tx_buf))
        ;
    /* DESENVOLVA SEU CÓDIGO AQUI */

    /* No final, habilitamos a interrupção de buffer de dados vazio
       para que possamos saber quando transmitir o próximo byte */
    drv->regs->ucsrb |= (1 << UDRIE0);
}

/*
 * Esta função escreve "len" bytes armazenados em "buf" no buffer de
 * transmissão do módulo para serem transmitidos um a um pelo canal de
 * comunicação serial.
 *
 * Se a quantidade "len" é maior do que o espaço livre do buffer de
 * transmissão, a função trava à espera do esvaziamento do buffer.
 * Assim, a função só retorna após copiar o último byte de "buf" no
 * buffer de transmissão (observe que a função provavelmente retornará
 * antes do último byte ser efetivamente transmitido).
 */
uint8_t uart_write(Uart_t* drv, uint8_t* buf, uint8_t len) {
    /* DESENVOLVA SEU CÓDIGO AQUI */

    /* No final, habilitamos a interrupção de buffer de dados vazio
       para que possamos saber quando transmitir o próximo byte */
    drv->regs->ucsrb |= (1 << UDRIE0);
}

/*
 * Esta função lê exatamente um byte da fila de bytes recebido e o
 * retorna. Se a fila estiver vazia (nenhum byte foi recebido desde a
 * última leitura), a função deve retornar -1.
 */
uint16_t uart_read(Uart_t* drv) {
    /* DESENVOLVA SEU CÓDIGO AQUI */
}

/*
 * Esta função lê "len" bytes recebidos pelo módulo da serial e
 * armazena-os na memória apontada por "buf". A função deve retornar o
 * número de bytes lidos, o que pode ser menor do que "len".
 *
 * A função deve transferir os "len" bytes do buffer de recepção para
 * a memória apontada por "buf", deixando quaisquer bytes restantes no
 * buffer de recepção.  Se a quantidade de bytes no buffer de recepção
 * for menos do que "len", a função deve copiar os bytes recebidos
 * deixando o buffer de recepção vazio.
 */
uint8_t uart_read_bytes(Uart_t* drv, uint8_t* buf, uint8_t len) {
    /* DESENVOLVA SEU CÓDIGO AQUI */
}

/*
 * Esta função retorna quantos bytes já foram recebidos e podem ser
 * lidos. Basicamente é quantos bytes tem no buffer circular de
 * recepção.
 */
uint8_t uart_available(Uart_t* drv) {
    /* DESENVOLVA SEU CÓDIGO AQUI */
    /* Use as funções relativas ao buffer circular */
}

/*
 * Esta função retorna quantos bytes podem ser escritos sem que a
 * função write() trave.  Basicamente é quantos bytes ainda cabem no
 * buffer de transmissão antes dele ficar completamente cheio.
 */
uint8_t uart_available_for_write(Uart_t* drv) {
    /* DESENVOLVA SEU CÓDIGO AQUI */
    /* Use as funções relativas ao buffer circular */
}

/*
 * Função de interrupção para o evento de byte disponível.  Se o
 * ponteiro para a função de callback for não-nulo, a função é chamada
 * e o dado recebido é passado para ela. Se o ponteiro for nulo e o
 * buffer de recepção não estiver cheio, o byte recebido é colocado no
 * buffer. Caso contrário, o byte é descartado
 */
ISR(USART_RX_vect) {
    uint8_t data = UDR0;
    uint8_t errors = UCSR0A & ((1 << FE0) | (1 << DOR0) |
                               (1 << UPE0));

    if (UART1_var.rx_cb)
        UART1_var.rx_cb(UARTD1, data, errors);
    else
        if (!errors && !is_cb_full(&UART1_var.rx_buf))
            cb_push(&UART1_var.rx_buf, data);

    /* NÃO HÁ MAIS NADA A IMPLEMENTAR AQUI */
}

/*
 * Função de interrupção para o evento do registrador UDR vazio. A
 * busca pelo byte a ser transmitido é feita primeiro através da
 * função de callback de transmissão. Caso o ponteiro para esta função
 * seja nulo ou se a função retornar -1, verifica-se se o buffer
 * circular de recepção tem dado.  Se tiver, recupera-se um byte e
 * transmite-se. Caso tudo falhe, a interrupção é desativada.
 */
ISR(USART_UDRE_vect) {
    uint16_t data = -1;

    if (UART1_var.tx_cb) {
        data = UART1_var.tx_cb(UARTD1);
        if (data == -1)
            data = cb_pop(&UARTD1->tx_buf);
    } else
        data = cb_pop(&UARTD1->tx_buf);

    if (data != -1)
        UDR0 = data;
    else
        /* Se não há mais nada a ser transmitido, desativamos a
           interrupção */
        UCSR0B &= ~(1 << UDRIE0);

    /* NÃO HÁ MAIS NADA A IMPLEMENTAR AQUI */
}
