#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "inc/ssd1306.h"
#include "inc/font.h"
#include "math.h"
#include <string.h>   // para usar strlen(), tamanho do vetor

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C
#define A0 28
#define E50_CONTADOR 50
double e24_val;
bool cor = true;
ssd1306_t ssd;
char Rx_display[6];
char E24Rx_display[6];
int Rc = 10000; // resistor conhecido
int leitura_adc=0, Rx=0, media=0, amostras=10000; // Rx= resistor desconhecido

double valor_comercial_mais_proximo(double valor); // valor comercial E24

char *cores[10] = { // vetor para informar as faixas do resistor
    "preto",  // 0
    "marrom", // 1
    "vermelho",//2
    "laranja", //3
    "amarelo", //4
    "verde",   //5
    "azul",    //6
    "violeta", //7
    "cinza",   //8
    "branco"   //9
};

int main()
{
    stdio_init_all();

    adc_init(); // inicializa o conversor adc
    adc_gpio_init(A0);
    adc_select_input(2);

    i2c_init(I2C_PORT, 400000); //inicializa o display oled
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_send_data(&ssd);


    while (true) {
        adc_select_input(2);
        leitura_adc = adc_read();
        int soma=0;
    
        for (int i=0; i < amostras; i++) { // calcula a media de pontos de amostras
            leitura_adc = adc_read();
            Rx = (leitura_adc*Rc) / (4096 - leitura_adc); // calcula o valor do resistor
            soma+=Rx;
        }
    
        media = soma/amostras;
        sprintf(Rx_display, "%d",media); // converte media para caractere, para por no display
    
        ssd1306_fill(&ssd, !cor);
        ssd1306_draw_string(&ssd,"ohm", 100, 0);
        ssd1306_draw_string(&ssd,"Resis", 0, 0);
        ssd1306_draw_string(&ssd, Rx_display, 46, 0);
    
        e24_val = valor_comercial_mais_proximo(media);
        sprintf(E24Rx_display, "%.0f",e24_val); // converte e24_val para caractere, para por no display, referente aos resistores e24
    
        ssd1306_draw_string(&ssd,"E24", 0, 20);
        ssd1306_draw_string(&ssd,"ohm", 100, 20);
        ssd1306_draw_string(&ssd, E24Rx_display, 46, 20);

        int tam_vetor = strlen(E24Rx_display);
        int multiplicador = tam_vetor -2; // informa qual numero multiplicador para desenhar sua cor correspondente
    
        ssd1306_draw_string(&ssd, cores[E24Rx_display[0] - '0'], 37, 35); // desenha no display oled os nomes referentes aos numeros das faixas
        ssd1306_draw_string(&ssd, cores[E24Rx_display[1] - '0'], 37, 45);
        ssd1306_draw_string(&ssd, cores[multiplicador], 37, 55);
        ssd1306_send_data(&ssd);
    
        //primeiro_digito = printf ("%c\n", E24Rx_display[0]);
        //segundo_digito = printf ("%c\n", E24Rx_display[1]); -- debug    
    
        sleep_ms(200);
        }
    }

//Função para encontrar o valor comercial mais próximo  
double valor_comercial_mais_proximo(double valor) {  
     //Array com os valores da série E24  
    double valores_e24[E50_CONTADOR] = {  
        1000.00, 10000.00, 100000.00, 1100.00, 11000.00, 
        1200.00, 12000.00, 1300.00, 13000.00, 
        1500.00, 15000.00, 1600.00, 16000.00, 
        1800.00, 18000.00, 2200.00, 22000.00,
        2400.00, 24000.00, 2700.00, 27000.00,   
        3300.00, 33000.00, 3600.00, 36000.00, 
        3900.00, 39000.00, 4700.00, 47000.00,
        510.00, 5100.00, 51000.00,
        560.00, 5600.00, 56000.00, 
        620.00, 6200.00, 62000.00,
        680.00, 6800.00, 68000.00,   
        750.00, 7500.00, 75000.00, 
        820.00, 8200.00, 82000.00,
        910.00, 9100.00, 91000.00    
	};  

    double valor_proximo = valores_e24[0];  //Inicializa com o primeiro valor da E24  
    double menor_diferenca = fabs(valor_proximo - valor);  //Calcula a diferença inicial em valor absoluto 

    for (int i = 1; i < E50_CONTADOR; i++) {  
        double diferenca_atual = fabs(valores_e24[i] - valor);  
        if (diferenca_atual < menor_diferenca) {  
            menor_diferenca = diferenca_atual;  //Atualiza a menor diferença  
            valor_proximo = valores_e24[i];  //Atualiza o valor mais próximo  
        }  
    }  

    return valor_proximo;  
}  
