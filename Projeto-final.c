#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "inc/ssd1306.h"
#include "inc/font.h"
#include <stdio.h>
#include <math.h>
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/adc.h"
#include "pico/bootrom.h"

#include "main.pio.h"

// número de LEDs
#define NUM_PIXELS 25
// pino de saída
#define OUT_PIN 7

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C

#define BOTAO_A 5
#define BOTAO_B 6

ssd1306_t ssd; // Inicializa a estrutura do display

int resposta;
int Letra;
uint64_t tempo_inicio;
uint64_t tempo_fim;
uint64_t tempo_espera;
uint64_t tempo_resposta;

// letras na matriz de leds
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
     0.0, 0.0, 0.0, 0.0, 0.0} // apagado
};

bool debounce()
{
  static uint32_t ultimo_tempo_botao = 0;
  uint32_t tempo_atual = to_ms_since_boot(get_absolute_time());

  if (tempo_atual - ultimo_tempo_botao < 200)
  { // 200ms de debounce
    return false;
  }

  ultimo_tempo_botao = tempo_atual;
  return true;
}

// rotina para definição da intensidade de cores do led
uint32_t matrix_rgb(double r, double g, double b)
{
  unsigned char R, G, B;
  R = r * 30;
  G = g * 30;
  B = b * 30;
  return (G << 24) | (R << 16) | (B << 8);
}

void gpio_callback(uint gpio, uint32_t events)
{
  gpio_acknowledge_irq(gpio, events); // Garante que a interrupção foi reconhecida

  if (gpio == BOTAO_A)
  {
    printf("botao A pressionado \n");
    resposta = 0;
  }
  else if (gpio == BOTAO_B)
  {
    printf("botao B pressionado \n");
    resposta = 1;
  }
  else
    resposta = -1;
}

int main()
{
  Letra = 2;

  PIO pio = pio0;
  bool ok;
  uint16_t i;
  uint32_t valor_led;
  double r = 0.0, b = 0.0, g = 0.0;

  // configurações da PIO
  uint offset = pio_add_program(pio, &main_program);
  uint sm = pio_claim_unused_sm(pio, true);
  main_program_init(pio, sm, offset, OUT_PIN);

  stdio_init_all(); // Inicializa comunicação USB CDC para monitor serial

  // Inicialização dos GPIOs
  gpio_init(BOTAO_A);
  gpio_init(BOTAO_B);

  // Configuração da direção dos GPIOs
  gpio_set_dir(BOTAO_A, GPIO_IN);
  gpio_set_dir(BOTAO_B, GPIO_IN);

  // Configuração do pull-up para o botão
  gpio_pull_up(BOTAO_A);
  gpio_pull_up(BOTAO_B);

  // Configuração da interrupção do botão (detecta borda de descida)
  gpio_set_irq_enabled_with_callback(BOTAO_A,
                                     GPIO_IRQ_EDGE_FALL,
                                     true,
                                     &gpio_callback);

  gpio_set_irq_enabled_with_callback(BOTAO_B,
                                     GPIO_IRQ_EDGE_FALL,
                                     true,
                                     &gpio_callback);

  // I2C Initialisation. Using it at 400Khz.
  i2c_init(I2C_PORT, 400 * 1000);

  gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
  gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
  gpio_pull_up(I2C_SDA);                     // Pull up the data line
  gpio_pull_up(I2C_SCL);                     // Pull up the clock line

  ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
  ssd1306_config(&ssd);                                         // Configura o display
  ssd1306_send_data(&ssd);                                      // Envia os dados para o display

  // Limpa o display. O display inicia com todos os pixels apagados.
  ssd1306_fill(&ssd, false);
  ssd1306_send_data(&ssd);

  bool cor = true;

  for (int i = 0; i < 10; i++)
  {
    for (int j = 0; j < NUM_PIXELS; j++)
    {
      valor_led = matrix_rgb(b, r = letras[Letra][24 - j], g); // Usa o padrão atual
      pio_sm_put_blocking(pio, sm, valor_led);
    }
  }

  while (true)
  {
    resposta = -1;
    printf("Jogo de Resposta Rápida iniciado, aguarde a letra aparecer e precione no botao correspondente.");

    ssd1306_fill(&ssd, false);                                                                        // Limpa o display
    ssd1306_draw_string(&ssd, "aguarde a letra aparecer e precione no botao correspondente.", 0, 32); // Desenha uma string no OLED
    ssd1306_send_data(&ssd);                                                                          // Atualiza o display

    Letra = rand() % 2;

    // gera um tempo de espera aleatorio entre 1s e 10s
    tempo_espera = (rand() % (10000 - 1000 + 1)) + 1000;
    printf("Tempo de espera aleatório: %d ms\n", tempo_espera);

    sleep_ms(tempo_espera);

    for (int i = 0; i < 10; i++)
    {
      for (int j = 0; j < NUM_PIXELS; j++)
      {
        valor_led = matrix_rgb(b, r = letras[Letra][24 - j], g);
        pio_sm_put_blocking(pio, sm, valor_led);
      }
    }
    tempo_inicio = time_us_64();

    while (resposta == -1)
    {
    }
    // Finalizar medição de tempo
    tempo_fim = time_us_64();
    tempo_resposta = tempo_fim - tempo_inicio;

    if (resposta == Letra)
    {
      printf("parabens, seu tempo foi de %d ms", tempo_resposta);

      ssd1306_fill(&ssd, false);                                                              // Limpa o display
      ssd1306_draw_string(&ssd, ("parabens, seu tempo foi de %d ms", tempo_resposta), 0, 32); // Desenha uma string no OLED
      ssd1306_send_data(&ssd);                                                                // Atualiza o display
    }
    else
    {
      printf("botao errado, tente outra vez");
      ssd1306_fill(&ssd, false);                                                              // Limpa o display
      ssd1306_draw_string(&ssd, "botao errado, tente outra vez", 0, 32); // Desenha uma string no OLED
      ssd1306_send_data(&ssd);                                                                // Atualiza o display
    
    }
    sleep_ms(20000);
  }
}