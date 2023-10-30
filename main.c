#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include "nokia5110.h"
#include <stdio.h>

#define TIMER_CLK F_CPU / 1024
#define IRQ_FREQ 1.5

void start_screen(void);
void print_dino(void);
void print_jumping_dino(void);
void print_ducking_dino(void);
void print_floor(void);
void print_score(void);
void print_tree(int x);
void print_arrow(int x);
void end_screen(void);
void reset_game(void);
void detect_coll(void);
void choose_dino_to_print(void);
void verify_obj_to_print(void);
void choose_next_obj_to_print(void);
void comeca_jogo(void);

// tam altura = 0x47
// tam largura = 0x83

volatile int jogoRolando = 0;
volatile int dinoJump = 0;
volatile int dinoDuck = 0;
volatile int tempoAtual = 0;
volatile int colisao = 0;
volatile int posicaoFlecha = 69;
volatile int posicaoArvore = 76;
volatile int contJump = 1;
volatile int flechaNaTela = 0;
volatile int arvoreNaTela = 0;

uint8_t dinoUp[] =
    {
        0b0100000,
        0b1110000,
        0b0111000,
        0b1111110,
        0b0000110}; // largura = 5  altura = 7

uint8_t dinoDown[] =
    {
        0b0100000,
        0b1110000,
        0b0110000,
        0b1111000,
        0b0011000}; // largura = 5  altura = 8

uint8_t tree[] =
    {
        0b0011000,
        0b0011100,
        0b1111100,
        0b0011100,
        0b0011000}; // largura = 5  altura = 7

uint8_t arrow[] =
    {
        0b000100,
        0b1100010,
        0b0111111,
        0b1100010,
        0b0000100}; // largura = 7  altura = 5

void start_screen(void)
{
    nokia_lcd_clear();
    nokia_lcd_set_cursor(3, 0);
    nokia_lcd_write_string("Dinosaur Game", 1, 0);
    nokia_lcd_set_cursor(5, 12);
    nokia_lcd_write_string("Press Start!", 1, 0);
    print_dino();
    nokia_lcd_set_cursor(20, 24);
    nokia_lcd_write_string("W - JUMP", 1, 0);
    nokia_lcd_set_cursor(20, 36);
    nokia_lcd_write_string("S - DUCK", 1, 0);
    print_floor();
    nokia_lcd_render();
}

void print_dino(void)
{
    nokia_lcd_custom(1, dinoUp);
    nokia_lcd_set_cursor(0, 48);
    nokia_lcd_write_string("\001", 2, 0);
}

void print_jumping_dino(void)
{
    nokia_lcd_custom(1, dinoUp);
    nokia_lcd_set_cursor(1, 18);
    nokia_lcd_write_string("\001", 2, 0);
}

void print_ducking_dino(void)
{
    nokia_lcd_custom(1, dinoDown);
    nokia_lcd_set_cursor(1, 48);
    nokia_lcd_write_string("\001", 2, 0);
}

void print_floor(void)
{
    nokia_lcd_drawline(0, 47, 83, 47);
}

void print_score(void)
{
    nokia_lcd_set_cursor(0, 0);
    char score[30];
    sprintf(score, "Score: %d", tempoAtual);
    nokia_lcd_write_string(score, 1, 0);
    nokia_lcd_drawline(0, 9, 83, 9);
}

void print_tree(int x)
{
    if (x <= 0)
    {
        posicaoArvore = 69;
        arvoreNaTela = !arvoreNaTela;
    }

    nokia_lcd_custom(1, tree);
    nokia_lcd_set_cursor(x, 32);
    nokia_lcd_write_string("\001", 2, 0);
}

void print_arrow(int x)
{
    if (x <= 0)
    {
        posicaoFlecha = 69;
        flechaNaTela = !flechaNaTela;
    }

    nokia_lcd_custom(1, arrow);
    nokia_lcd_set_cursor(x, 32);
    nokia_lcd_write_string("\001", 1, 1);
}
void end_screen(void)
{
    nokia_lcd_clear();
    nokia_lcd_set_cursor(3, 0);
    nokia_lcd_write_string("GAME OVER!", 1, 0);
    nokia_lcd_set_cursor(1, 12);
    char score[30];
    sprintf(score, "Score: %d", tempoAtual);
    nokia_lcd_write_string(score, 1, 0);
    print_floor();
    nokia_lcd_render();
    _delay_ms(4000);

    nokia_lcd_clear();
    nokia_lcd_set_cursor(3, 0);
    nokia_lcd_write_string("HIGH SCORES:", 1, 0);
    nokia_lcd_drawline(0, 9, 83, 9);
    nokia_lcd_set_cursor(1, 12);

    if (tempoAtual <= 16)
    {
        nokia_lcd_set_cursor(1, 12);
        nokia_lcd_write_string("Lo: 84", 1, 0);
        nokia_lcd_set_cursor(1, 24);
        nokia_lcd_write_string("Rei: 32", 1, 0);
        nokia_lcd_set_cursor(1, 36);
        nokia_lcd_write_string("Mat: 16", 1, 0);
        nokia_lcd_render();
        _delay_ms(4000);
    }

    else if (tempoAtual > 16 && tempoAtual <= 32)
    {
        nokia_lcd_set_cursor(1, 12);
        nokia_lcd_write_string("Lo: 84", 1, 0);
        nokia_lcd_set_cursor(1, 24);
        nokia_lcd_write_string("Rei: 32", 1, 0);
        nokia_lcd_set_cursor(1, 36);
        sprintf(score, "You: %d", tempoAtual);
        nokia_lcd_write_string(score, 1, 0);
        nokia_lcd_render();
        _delay_ms(4000);
    }

    else if (tempoAtual > 32 && tempoAtual <= 84)
    {
        nokia_lcd_set_cursor(1, 12);
        nokia_lcd_write_string("Lo: 84", 1, 0);
        nokia_lcd_set_cursor(1, 24);
        sprintf(score, "You: %d", tempoAtual);
        nokia_lcd_write_string(score, 1, 0);
        nokia_lcd_set_cursor(1, 36);
        nokia_lcd_write_string("Rei: 32", 1, 0);
        nokia_lcd_render();
        _delay_ms(4000);
    }

    else if (tempoAtual > 84)
    {
        nokia_lcd_set_cursor(1, 12);
        sprintf(score, "You: %d", tempoAtual);
        nokia_lcd_write_string(score, 1, 0);
        nokia_lcd_set_cursor(1, 24);
        nokia_lcd_write_string("Lo: 84", 1, 0);
        nokia_lcd_set_cursor(1, 36);
        nokia_lcd_write_string("Rei: 32", 1, 0);
        nokia_lcd_render();
        _delay_ms(4000);
    }

    reset_game();
}

void reset_game(void)
{

    flechaNaTela = 0;
    arvoreNaTela = 0;
    posicaoArvore = 76;
    posicaoFlecha = 69;
    colisao = 0;
    tempoAtual = 0;
    jogoRolando = !jogoRolando;
}

void detect_coll(void)
{
    if (posicaoArvore <= 10)
    {
        if (dinoJump == 0)
        {
            colisao = 1;
            cli();
        }
    }

    if (posicaoFlecha <= 10)
    {
        if (dinoDuck == 0)
        {
            colisao = 1;
            cli();
        }
    }
}

void choose_dino_to_print(void)
{
    if ((PIND & (1 << PD2)) | (dinoJump))
    { // faz o pulo
        nokia_lcd_clear();
        print_score();
        dinoDuck = 0;
        print_jumping_dino();
        dinoJump = 1;
    }

    if ((PIND & (1 << PD3)) | (dinoDuck))
    { // faz o agacho
        nokia_lcd_clear();
        print_score();
        dinoJump = 0;
        print_ducking_dino();
        dinoDuck = 1;
    }

    if (!(PIND & (1 << PD3)))
        dinoDuck = 0;

    if (!(dinoJump) & !(dinoDuck))
        print_dino();
}

void verify_obj_to_print(void)
{

    if (flechaNaTela)
    {
        if (tempoAtual <= 10)
        {
            print_arrow(posicaoFlecha); // posicao inicial da flecha
            posicaoFlecha--;
        }
        else if (tempoAtual >= 10 && tempoAtual <= 30)
        {
            print_arrow(posicaoFlecha); // posicao inicial da flecha
            posicaoFlecha = posicaoFlecha - 2;
        }
        else if (tempoAtual >= 35 && tempoAtual <= 60)
        {
            print_arrow(posicaoFlecha); // posicao inicial da flecha
            posicaoFlecha = posicaoFlecha - 2;
        }
        else if (tempoAtual >= 60 && tempoAtual < 70)
        {
            print_arrow(posicaoFlecha); // posicao inicial da flecha
            posicaoFlecha = posicaoFlecha - 3;
        }
        else
        {
            print_arrow(posicaoFlecha); // posicao inicial da flecha
            posicaoFlecha = posicaoFlecha - 4;
        }
    }

    if (arvoreNaTela)
    {
        if (tempoAtual <= 10)
        {
            print_tree(posicaoArvore); // posicao inicial da arvore
            posicaoArvore--;
        }
        else if (tempoAtual >= 10 && tempoAtual <= 30)
        {
            print_tree(posicaoArvore); // posicao inicial da arvore
            posicaoArvore = posicaoArvore - 2;
        }
        else if (tempoAtual >= 30 && tempoAtual <= 60)
        {
            print_tree(posicaoArvore); // posicao inicial da arvore
            posicaoArvore = posicaoArvore - 3;
        }
        else if (tempoAtual > 60 && tempoAtual < 80)
        {
            print_tree(posicaoArvore - 2); // posicao inicial da arvore
            posicaoArvore = posicaoArvore - 3;
        }
        else
        {
            print_tree(posicaoArvore); // posicao inicial da arvore
            posicaoArvore = posicaoArvore - 4;
        }
    }
}

void choose_next_obj_to_print(void)
{
    if (tempoAtual % 4 == 0)
    {
        if (tempoAtual == 8 || tempoAtual == 28 || tempoAtual == 44)
            flechaNaTela = 0;
        else 
            flechaNaTela = 1;
    }
    if (tempoAtual % 9 == 0)
    {
        if (tempoAtual == 36 || tempoAtual == 72)
            arvoreNaTela = 0;
        else 
            arvoreNaTela = 1;
    }
}

void comeca_jogo(void)
{
    nokia_lcd_clear();
    sei();

    while (jogoRolando)
    {
        nokia_lcd_clear();
        print_score();

        choose_dino_to_print();

        verify_obj_to_print();

        print_floor();
        nokia_lcd_render();

        detect_coll();

        if (colisao)
        {
            break;
        }

        choose_next_obj_to_print();
    }
    end_screen();
}

ISR(TIMER1_COMPA_vect)
{
    // aqui marca o tempo
    if (jogoRolando)
        tempoAtual++;

    // aqui decide quando o dino volta pra posição normal
    if (dinoJump == 1)
    {
        // contJump--;
        contJump = 0;
    }

    if (contJump <= 0)
    {
        dinoJump = 0;
        contJump = 1;
    }
}

int main(void)
{
    // desabilita interrupcoes
    cli();
    // resseta contadores para TIMER1
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;
    // seta o registrador output compare
    OCR1A = (TIMER_CLK / IRQ_FREQ) - 1;
    // liga modo CTC
    TCCR1B |= (1 << WGM12);
    // seta CS10 e CS12 para prescaler 1024
    TCCR1B |= (1 << CS12) | (1 << CS10);
    // habilita mascara do timer1
    TIMSK1 |= (1 << OCIE1A);

    DDRD &= ~(1 << PD1) | ~(1 << PD2) | ~(1 << PD3); // entradas
    PORTD |= (1 << PD1) | (1 << PD2) | (1 << PD3);   // desabilita pull-up

    nokia_lcd_init();
    start_screen();

    while (1)
    {
        if (PIND & (1 << PD1))
            if (!(jogoRolando))
            {
                jogoRolando = 1;
                comeca_jogo();
            }
    }
}
