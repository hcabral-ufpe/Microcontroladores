# Diretório de aplicativos
Esse diretório tem o objetivo de organizar os aplicativos que fazem uso da infraestrutura que estamos desenvolvendo na disciplina de Microcontroladores da UFPE.

Para compilar, entre em seu diretório e rode o comando

```bash
avr-gcc -mmcu=atmega328p -O1 -I ../../src main_serial.c ../../src/*.c -o main.bin
```
Caso queira usar os arquivos pré-compilados fornecidos, use o comando

```bash
avr-gcc -mmcu=atmega328p -O1 -I ../../src main_serial.c ../../src/gpio.c ../../src/circular_buffer.c ../../bin/*.o -o main.bin
```

Para gravação do binário gerado no microcontrolador, siga as instruções contidas no diretório src.
