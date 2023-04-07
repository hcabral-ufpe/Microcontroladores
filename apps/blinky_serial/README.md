# Um aplicativo para piscar um led
Este programa é um exemplo do uso das funcionalidades de temporizador, comunicação serial e portas digitais que desenvolvemos na disciplina de Microcontroladores - 2022.2 da UFPE.

Ele pisca o led da placa do Arduino Nano (pino PB5) em diferentes frequências. O usuário pode, através da interface de comunicação serial, aumentar (enviando o caractere '+') ou diminuir (enviando '-') a frequência de piscagem do led.  Além disso, ele pode desligar o led enviando o dígito 0.

O led pode ser controlado também por 3 push buttons conectados aos pinos PD0, PD1 e PD2. Esta funcionalidade é deixada como exercício para você.
