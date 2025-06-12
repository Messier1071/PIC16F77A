/*
 * File:   main.c
 * Author: ianma
 *
 * Created on 24 April 2025, 15:26
 */


#include <xc.h>
#include <pic16f877a.h>
#include <stdio.h>
#define _XTAL_FREQ 4000000

#define ADDI PORTBbits.RB1
#define SUBI PORTBbits.RB2

#define T1H 0x0B
#define T1L 0xDC 

#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = ON       // Power-up Timer Enable bit (PWRT enabled)
#pragma config BOREN = ON     // Brown-out Reset Enable bit (BOR disabled)
#pragma config LVP = OFF        // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3 is digital I/O, HV on MCLR must be used for programming)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit

//*** define pinos referentes a interface com LCD
#define RS RD2
#define EN RD3
#define D4 RD4
#define D5 RD5
#define D6 RD6
#define D7 RD7

//**** inclui a biblioteca do LCD escolhido
#include "Lib/lcd.h"

char Hours = 0;
char Minutes = 0;
char Seconds = 0;

char state = 0;

void main(void) {
    char Line1[20]; //vari?vel para o fun??o sprintf

    char Line2[10]; //vari?vel para o fun??o sprintf

    TRISD = 0x00; //configura PORTD como sa?da. Local onde deve estar o LCD
    TRISB = 0xff; // IO port b

    Lcd_Init(); //necess?rio para o LCD iniciar

    OPTION_REGbits.nRBPU = 0; //portb PullUp
    OPTION_REGbits.INTEDG = 0; //interrupt on port0 rising edge 0 -> 1
    OPTION_REGbits.T0CS = 1; //TMR0 Clock Source Select bit 
    OPTION_REGbits.T0SE = 1; // TMR0 Source Edge Select bit
    OPTION_REGbits.PSA = 1; //Prescaler Assignment bit       1 = Prescaler is assigned to the WDT  0 = Prescaler is assigned to the Timer0 module  
    OPTION_REGbits.PS0 = 0; // Prescaler Rate Select bits
    OPTION_REGbits.PS1 = 0;
    OPTION_REGbits.PS2 = 0;

    INTCONbits.GIE = 1; // global interrupt enable     
    INTCONbits.PEIE = 1; // Peripheral Interrupt Enable bit   
    INTCONbits.TMR0IE = 0; // TMR0 Overflow Interrupt Enable bit     
    INTCONbits.INTE = 1; // RB0/INT External Interrupt Enable bit
    INTCONbits.RBIE = 0; // RB Port Change Interrupt Enable bit     
    INTCONbits.TMR0IF = 0; // TMR0 Overflow Interrupt Flag bit    (must be cleared in software)


    INTCONbits.INTF = 0; // RB0/INT External Interrupt Flag bit (must be cleared in software)
    INTCONbits.RBIF = 0; // RB Port Change Interrupt Flag bit   (must be cleared in software)
    PIE1bits.TMR1IE = 1; //Habilita int do timer 1

    T1CONbits.TMR1CS = 0;
    T1CONbits.T1CKPS0 = 1; //bit pra configurar pre-escaler, nesta caso 1:8
    T1CONbits.T1CKPS1 = 1;

    //1khz / 8



    TMR1L = T1L; //carga do valor inicial no contador (65536-62500)
    TMR1H = T1H; //3036. Quando estourar contou 62500, passou 0,5s   


    //todo pedir p professor re-explicar

    T1CONbits.TMR1ON = 1; //Liga o timer





    while (1) {
        Lcd_Clear(); //limpa LCD
        if (state == 0) T1CONbits.TMR1ON = 1;
        switch (state) {

            case 0:
                sprintf(Line1, "%i:%i:%i", (int) Hours, (int) Minutes, (int) Seconds); //Armazena em buffer o conte?do da vari?vel f formatado com duas casas
                Lcd_Set_Cursor(1, 1);
                Lcd_Write_String(Line1); //Escreve o conte?do de buffer no LCD
                __delay_ms(500);
                break;
            case 1:
                T1CONbits.TMR1ON = 0;
                sprintf(Line1, "%i:%i:%i", (int) Hours, (int) Minutes, (int) Seconds); //Armazena em buffer o conte?do da vari?vel f formatado com duas casas
                Lcd_Set_Cursor(1, 1);
                Lcd_Write_String(Line1); //Escreve o conte?do de buffer no LCD
                Lcd_Set_Cursor(2, 1);
                Lcd_Write_String("Editing Minutes");
                if (ADDI == 0) {
                    Minutes++;
                }
                if (SUBI == 0) {
                    Minutes--;
                }
                

                __delay_ms(150);

                break;
            case 2:
                T1CONbits.TMR1ON = 0;
                sprintf(Line1, "%i:%i:%i", (int) Hours, (int) Minutes, (int) Seconds); //Armazena em buffer o conte?do da vari?vel f formatado com duas casas
                Lcd_Set_Cursor(1, 1);
                Lcd_Write_String(Line1); //Escreve o conte?do de buffer no LCD
                Lcd_Set_Cursor(2, 1);
                Lcd_Write_String("Editing Hours");
                if (ADDI == 0) {
                    Hours++;
                }
                if (SUBI == 0) {
                    Hours--;
                }
                
                __delay_ms(150);
                break;
        }
        if (Minutes > 59) {
            Hours++;
            Minutes = 0;
        }
        if (Hours > 23) {
            Hours = 0;
        }
        
    }

    return;
}

void __interrupt() TrataInt(void)
 {
    if (INTF == 1) {
        INTCONbits.INTF = 0;
        state++;
        if (state > 2) {
            state = 0;
        }
    }
    if (TMR1IF) //foi a interrup??o de estouro do timer1 que chamou a int?
    {
        PIR1bits.TMR1IF = 0; //reseta o flag da interrup??o
        TMR1L = T1L; //carga do valor inicial no contador (65536-62500)
        TMR1H = T1H; //3036. Quando estourar contou 62500, passou 0,5s   


        Seconds++;
        if (Seconds > 59) {
            Minutes++;
            Seconds = 0;
        }
        if (Minutes > 59) {
            Hours++;
            Minutes = 0;
        }
        if (Hours > 23) {
            Hours = 0;
        }

        //comandos pra tratar a interrup??o

    }
    return;
}
