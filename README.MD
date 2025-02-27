# Projeto Final da Residência Tecnológica EMBARCATECH
## Jogo de Resposta Rápida com BitdogLab

Aluno: Inacio Oliveira Borges

Link para o projeto em execuçao: [link](https://youtu.be/tb0csUjDXew)

## Introdução
Este documento descreve o projeto final da residência tecnológica em sistemas embarcados EMBARCATECH, um jogo de resposta rápida implementado no BitdogLab. O jogo testa a agilidade do jogador em identificar e responder a letras exibidas em uma matriz de LEDs 5x5.

Este projeto explora o uso de diversas ferramentas de programação de microcontroladores, incluindo:
- **Timers**: Para medir o tempo de resposta do jogador com precisão.
- **Interrupções**: Para detectar o pressionamento dos botões de forma rápida e eficiente, sem bloquear o fluxo principal do programa.
- **Geração de números aleatórios**: Para gerar as letras A e B de forma imprevisível.
- **Manipulação de matrizes**: Para controlar a exibição das letras na matriz de LEDs.
- **Comunicação I2C**: Para manipulação do display OLED.
- **Funções de entrada/saída (GPIO)**: Para controlar os pinos da matriz de LEDs e ler os botões.
- **Lógica de controle de fluxo**: Para gerenciar o estado do jogo e a sequência de eventos.

A combinação dessas ferramentas permite criar um jogo interativo e responsivo, demonstrando a versatilidade dos microcontroladores.

## Escopo do Projeto
### Função do Sistema Embarcado
O sistema embarcado proposto, implementado no Raspberry Pi Pico W através do BitdogLab, consiste em um jogo interativo de resposta rápida que pode ser adaptado para objetivos de treinamento cognitivo para pessoas com dificuldades de aprendizado ou necessidades especiais. O objetivo é testar e aprimorar a agilidade do jogador, utilizando a matriz de LEDs 5x5 e dois botões. O sistema irá:
- **Gerar letras aleatórias**: As letras "A" ou "B" serão selecionadas de forma aleatória e imprevisível, utilizando as funções de geração de números aleatórios do Pico SDK.
- **Exibir letras em matriz de LEDs**: A letra selecionada será exibida em uma matriz de LEDs 5x5, controlada através dos pinos GPIO do Raspberry Pi Pico W.
- **Detectar entradas do jogador**: O sistema irá monitorar dois botões conectados aos pinos GPIO do Raspberry Pi Pico W, utilizando interrupções para garantir a detecção rápida e eficiente dos pressionamentos.
- **Medir tempo de resposta**: O tempo decorrido entre a exibição da letra e o pressionamento do botão será medido com precisão, utilizando os timers do Raspberry Pi Pico W.
- **Verificar resposta**: O sistema irá comparar a entrada do jogador com a letra exibida, determinando se a resposta está correta ou incorreta.
- **Exibir tempo de resposta**: O tempo de resposta do jogador será exibido no display OLED conectado através do barramento I2C.
- **Fornecer feedback**: O sistema irá indicar se a resposta está correta ou incorreta.

### Paralelo com Projeto Inclusivo (Treinamento Cognitivo)
Este projeto pode ser adaptado para servir como ferramenta de treinamento cognitivo para pessoas com dificuldades de aprendizado ou necessidades especiais. As adaptações podem incluir:
- **Níveis de dificuldade ajustáveis**: Permitir que o usuário escolha a velocidade de exibição das letras e o tempo limite para resposta.
- **Feedback visual e auditivo**: Utilizar cores, padrões e sons para tornar o jogo mais acessível e engajador.
- **Modos de jogo personalizados**: Oferecer diferentes modos de jogo, como sequências de letras, reconhecimento de padrões ou jogos de memória.
- **Interface simplificada**: Utilizar botões grandes e de fácil acesso, além de uma interface visual clara e intuitiva.
- **Registros de progresso**: Armazenar dados sobre o desempenho do usuário para acompanhar seu progresso e ajustar o treinamento.

## Componentes de Hardware
- **BitdogLab**: Plataforma de desenvolvimento com microcontrolador integrado.
- **Matriz de LEDs 5x5**: Para exibição das letras "A" e "B".
- **Dois botões**: Para entrada de dados do jogador.

```c
#define OUT_PIN 7 // GPIO conectado a matriz de LEDs endereçáveis
#define I2C_PORT i2c1 // Definição da porta I2C para o display OLED
#define I2C_SDA 14 // Pino serial do barramento I2C
#define I2C_SCL 15 // Pino serial do barramento I2C
#define endereco 0x3C // Endereço serial do display OLED
#define BOTAO_A 5 // Botão A conectado ao GPIO 5
#define BOTAO_B 6 // Botão B conectado ao GPIO 6
```

## Software
- **Linguagem de programação**: C.
- **Ambiente de desenvolvimento**: VsCode com Pico SDK.

### Bibliotecas
- Bibliotecas específicas do BitdogLab para controle da matriz de LEDs, leitura de botões e medição de tempo.
- Biblioteca para geração de números aleatórios.

### Variáveis
```c
int resposta; // Resposta do jogador
int Letra; // Letra gerada automaticamente (índice da matriz de letras)
double Letras[3][25]; // Matriz que armazena as letras para serem exibidas.
double letras[3][25] = {
    {0.0, 0.0, 1.0, 0.0, 0.0,
     0.0, 1.0, 0.0, 1.0, 0.0,
     1.0, 1.0, 1.0, 1.0, 1.0,
     1.0, 0.0, 0.0, 0.0, 1.0,
     1.0, 0.0, 0.0, 0.0, 1.0}, // A

    {1.0, 1.0, 1.0, 0.0, 0.0,
     1.0, 0.0, 0.0, 1.0, 0.0,
     1.0, 1.0, 1.0, 0.0, 0.0,
     1.0, 0.0, 0.0, 1.0, 0.0,
     1.0, 1.0, 1.0, 0.0, 0.0}, // B

    {0.0, 0.0, 0.0, 0.0, 0.0,
     0.0, 0.0, 0.0, 0.0, 0.0,
     0.0, 0.0, 0.0, 0.0, 0.0,
     0.0, 0.0, 0.0, 0.0, 0.0,
     0.0, 0.0, 0.0, 0.0, 0.0}  // apagado
};

uint64_t tempo_inicio; // Tempo de início (contado a partir da exibição da letra na matriz de LED)
uint64_t tempo_fim;
uint64_t tempo_espera; // Tempo gerado aleatoriamente para variar o tempo de espera antes da exibição da letra na matriz de LEDs.
uint64_t tempo_resposta; // Tempo entre a exibição da letra e a resposta do jogador.
