/* 
 * File:   main.c
 * Author: jrich & mwainwright
 *
 * Created on June 1, 2022, 15:34 PM
 * PIC12F1840
 * MPLAB X IDE v5.45
 * XC8 (v2.32)
 * 
 * The code for the Radical22 is available on GitHub under the CC license 
 * Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0). 
 * It can also be download from the Dirty Electronics and Noise Technology 
 * websites.
 */

//Pin IO
//1 = +5
//2 = RA5/CCP1 (PWM) - DDS output
//3 = RA4/AN3 (Phaseshift) - ADCPhaseShift
//4 = MCLR/VPP
//5 = RA2 using external interrupt flag - tactile/prog select
//6 = RA1/AN1/ICSPCLK - readADC(1)
//7 = RA0/AN0/ICSPDAT - LED test/readADC(0)
//8 = GND  

// CONFIG1
#pragma config FOSC = INTOSC    // Oscillator Selection->INTOSC oscillator: I/O function on CLKIN pin
#pragma config WDTE = OFF    // Watchdog Timer Enable->WDT disabled
#pragma config PWRTE = OFF    // Power-up Timer Enable->PWRT disabled
#pragma config MCLRE = ON    // MCLR Pin Function Select->MCLR/VPP pin function is MCLR
#pragma config CP = OFF    // Flash Program Memory Code Protection->Program memory code protection is disabled
#pragma config CPD = OFF    // Data Memory Code Protection->Data memory code protection is disabled
#pragma config BOREN = ON    // Brown-out Reset Enable->Brown-out Reset enabled
#pragma config CLKOUTEN = OFF    // Clock Out Enable->CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin
#pragma config IESO = ON    // Internal/External Switchover->Internal/External Switchover mode is enabled
#pragma config FCMEN = ON    // Fail-Safe Clock Monitor Enable->Fail-Safe Clock Monitor is enabled

// CONFIG2
#pragma config WRT = OFF    // Flash Memory Self-Write Protection->Write protection off
#pragma config PLLEN = ON    // PLL Enable->4x PLL enabled
#pragma config STVREN = ON    // Stack Overflow/Underflow Reset Enable->Stack Overflow or Underflow will cause a Reset
#pragma config BORV = LO    // Brown-out Reset Voltage Selection->Brown-out Reset Voltage (Vbor), low trip point selected.
#pragma config LVP = ON    // Low-Voltage Programming Enable->Low-voltage programming enabled


////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <xc.h>

#define _XTAL_FREQ 32000000
//////////////////////////////////////////////////////////////////////////////

unsigned char ctr = 1; // counter for presets/algos - we can add more counters
unsigned char ctr2 = 1; // counter for presets/algos

//counter step variables (not all used)
unsigned char step;
unsigned char step1;
unsigned char step2;
unsigned char step3;
unsigned char step4;


const unsigned char sine[256] = {// sine wave 8 bit resolution scaled to 90% max val
    131, 132, 135, 137, 140, 143, 146, 149, 152, 155, 157, 160, 163, 166, 168, 171,
    174, 176, 179, 181, 184, 186, 189, 191, 194, 196, 198, 200, 202, 205, 207, 209,
    211, 212, 214, 216, 218, 219, 221, 223, 224, 226, 227, 228, 229, 231, 232, 233,
    234, 234, 235, 236, 237, 237, 238, 238, 239, 239, 239, 239, 240, 240, 240, 239,
    239, 239, 239, 238, 238, 237, 237, 236, 236, 235, 234, 233, 232, 231, 230, 229,
    227, 226, 225, 223, 222, 220, 219, 217, 216, 214, 212, 210, 208, 206, 204, 202,
    200, 198, 196, 194, 192, 189, 187, 185, 182, 180, 177, 175, 173, 170, 167, 165,
    162, 160, 157, 154, 152, 149, 146, 144, 141, 138, 135, 133, 130, 127, 124, 122,
    119, 116, 113, 111, 108, 105, 103, 100, 97, 95, 92, 89, 87, 84, 82, 79,
    77, 74, 72, 69, 67, 64, 62, 60, 58, 55, 53, 51, 49, 47, 45, 43,
    41, 39, 37, 36, 34, 32, 31, 29, 28, 26, 25, 24, 22, 21, 20, 19,
    18, 17, 16, 15, 15, 14, 13, 13, 12, 12, 12, 11, 11, 11, 11, 11,
    11, 11, 11, 12, 12, 12, 13, 14, 14, 15, 16, 16, 17, 18, 19, 21,
    22, 23, 24, 26, 27, 29, 30, 32, 33, 35, 37, 39, 41, 43, 45, 47,
    49, 51, 53, 56, 58, 60, 63, 65, 68, 70, 73, 75, 78, 81, 83, 86,
    89, 92, 94, 97, 100, 103, 106, 108, 111, 114, 117, 120, 123, 126, 129, 130
};

const unsigned char myarray[256] = {// myarray 8 bit resolution. 
    //Microcomputer music is always in-flux, noisy and may be overwritten. Adjusted ASCII to decimal conversion
    217, 211, 217, 214, 208, 202, 252, 249, 252, 234, 240, 243, 234, 243, 234, 226, 223, 211, 199, 205,
    197, 194, 211, 220, 211, 199, 194, 194, 199, 205, 211, 223, 223, 255, 249, 243, 243, 249, 246,
    243, 243, 234, 214, 197, 199, 214, 197, 208, 197, 191, 199, 191, 156, 104, 168, 217, 205, 199,
    208, 220, 211, 205, 197, 199, 208, 202, 182, 121, 60, 0, 17, 37, 104, 168, 217, 208, 194, 199,
    211, 223, 252, 249, 252, 249, 249, 243, 249, 249, 252, 234, 234, 240, 243, 234, 228, 240, 240,
    234, 231, 223, 252, 249, 249, 252, 252, 249, 243, 240, 237, 231, 228, 240, 243, 249, 243, 249,
    252, 249, 246, 240, 234, 223, 226, 223, 226, 226, 214, 197, 194, 199, 208, 194, 211, 220, 226,
    228, 231, 220, 231, 226, 223, 217, 205, 211, 228, 246, 246, 246, 246, 237, 231, 243, 234, 214,
    223, 234, 231, 234, 214, 223, 255, 249, 246, 246, 249, 246, 237, 240, 240, 231, 243, 249, 252,
    249, 249, 243, 240, 240, 246, 243, 249, 249, 249, 252, 249, 246, 249, 252, 252, 234, 243, 249,
    249, 252, 249, 243, 249, 249, 249, 246, 237, 226, 223, 234, 240, 246, 240, 237, 231, 226, 228,
    226, 234, 243, 249, 243, 234, 214, 223, 165, 104, 168, 231, 237, 226, 182, 136, 78, 20, 37, 104,
    168, 231, 237, 226, 214, 208, 194, 199, 202, 208, 217, 217, 199, 202, 182, 121, 60, 0, 17, 37,
    104, 168, 234, 234, 228
};

////////////////////////////////Global variable here////////////////////////////////////////
long PhaseAccum; //phase accumulator generates the cycle rate for lookup table
//loading thereby changing frequency. The MSbyte is used to provide the byte address
//of the look up table value to be used.
long PhaseShift; //value added to PhaseAccum every PWM cycle. This makes the waveform
//lookup faster or slower which changes frequency.
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////PIC Config routine here/////////////////////////////////////

void Init_Main() {
    //PIC12F1840 specific config

    OPTION_REG = 0b11000000; // weak pull-up disabled // interrupt on rising edge of INT pin
    OSCCON = 0b11110000; // 8MHz clk //32Mhz pll
    TRISA = 0b00011111; // RA0 = input; RA1 = input; RA2 = input; RA3 = input; RA4 = input; RA5 = out (PWM/DDS)
    //TRISA = 0b00011110; // RA0 = output LED (pin 7); RA1 = input; RA2 = input; RA3 = input; RA4 = input; RA5 = out (PWM/DDS)
    ANSELA = 0b00001010; // select AN1 (pin 6) and AN3 (pin 3) as A2D inputs // ANSELA select as analogue or digital // PhaseShift and Touch
    PORTAbits.RA0 = 1; // LED test
    T2CON = 0b00000100; // TMR2 ON, postscale 1:1, prescale 1:1
    PR2 = (0x50); // sets PWM rate to approx 98.5KHz with 32Mhz internal oscillator
    CCP1CON = 0b00001111; // CCP1 ON, and set to simple PWM mode (pin 2)
    PhaseShift = 0x00FFFFFF; // 16777215 - frequency values loaded into
    APFCONbits.CCP1SEL = 1; // RA2 = 0 RA5 = 1 //alternative pin function //tactile select
    ADCON0 = 0b00001101; // configure ADC // channel = AN3 // see also readADC
    ADCON1 = 0b00100000; // configure ADC //  1 = right justification (bit 7); 0 = left just. // And clock division
    INTCON = 0b10010000; // config interrupt - GIE and INTE
    //INTCON = 0b10010010; // config interrupt - GIE and INTE // INTCONbits.INTF = 1; may need this setting if select doesn't toggle correctly
}
//tactile switch @ RA2 (pin 5) not used

void __interrupt() ISR(void) {
    if (INTCONbits.INTF == 1) { //using external interrupt flag @ RA2
        INTCONbits.INTF = 0; //clear flag
    }
}

///////////////////////////////ADC functions//////////////////////////////////////

// NOTE justification ADCON1 needs to be right just for 10-bit?
// See algos below for use of e.g. AN1 readADC(1) channel 1 (pin 6); AN0 readADC(0) channel 0 (pin 7)

unsigned int readADC(unsigned char channel) {
    unsigned int AN_Val;
    // Right justification (10 bit value)
    ADCON1bits.ADFM = 1;

    // Select the A/D channel
    ADCON0bits.CHS = channel;

    // Turn on the ADC module
    ADCON0bits.ADON = 1;

    // Acquisition time delay
    __delay_us(5); // sampling time // can be c. 5 us - acquisition time

    // Start the conversion
    ADCON0bits.GO_nDONE = 1;

    // Wait for the conversion to finish
    while (ADCON0bits.GO_nDONE) {
    }
    //AN_Val = ((ADRESH << 8) + ADRESL); // returns 10-bit // NOTE justification ADCON1!
    AN_Val = ADRES; //trick? as above
    //AN_Val = ADRESH; //returns 8-bit

    return AN_Val; // int ADC(unsigned char channel)
}

void ADCPhaseShift() {
    // Left justification @ 16-bit (65535)?
    ADCON1bits.ADFM = 0;

    // Select the A/D channel
    ADCON0bits.CHS = 3;

    // Turn on the ADC module
    ADCON0bits.ADON = 1;

    // Acquisition time delay
    __delay_us(5); // sampling time // can be c. 5 us - acquisition time

    // Start the conversion
    ADCON0bits.GO_nDONE = 1;

    // Wait for the conversion to finish
    while (ADCON0bits.GO_nDONE) {
    }
    ((char *) &PhaseShift)[1] = ADRESL; //load ADRESL into PhaseShift
    ((char *) &PhaseShift)[2] = ADRESH; //load ADRESH into PhaseShift
}

////////////////////////////main program loop here/////////////////////////////////

void main() {
    Init_Main(); //config part      

    while (1) { //alway do this 
        ADCPhaseShift(); //read ADC here to get value into PhaseShift to change freq

        while (!PIR1bits.TMR2IF); // wait for TMR2 cycle to restart //  nested loop
        //some step variables (not all used) - we can use these to move through waveform at different speeds
        step ^= ADRESL;
        //step1 += 
        step2 += 1;
        step3 += 3;
        step4 += 6;


        ////// 4 different waveform generators 'cores' ///////   
        int ctr2 = (readADC(0) % 128) >> 5;

        if (ctr2 == 0) { //nosy
            CCPR1L = (sine[((char *) &PhaseAccum)[3]]) + step + 1; // * 2; // load MSbits 7-2 duty cycle value into CCPRIL
            CCP1CON ^= ((sine[step]) & 0x03) << 4; // load in bits 1-0 into 5 and 4 of CCP1CON
        }

        if (ctr2 == 1) { //sawww
            CCPR1L = ((char *) &PhaseAccum)[3] >> 2; // load MSbits 7-2 duty cycle value into CCPRIL               
            CCP1CON ^= ((char *) &PhaseAccum)[3] & 0x03 << 4; // load in bits 1-0 into 5 and 4 of CCP1CON // 0x03 = 3
        }

        if (ctr2 == 2) { //vinegar
            CCPR1L = (sine[ ((char *) &PhaseAccum) [3] & readADC(1)]) >> 2; // load MSbits 7-2 duty cycle value into CCPRIL              
            CCP1CON ^= ((sine[((char *) &PhaseAccum)[3]]) & 0x03) << 4; // load in bits 1-0 into 5 and 4 of CCP1CON // 0x03 = 3 
        }

        if (ctr2 == 3) { //myarray
            CCPR1L = (myarray[((char *) &PhaseAccum)[3]]) >> 2; // load MSbits 7-2 duty cycle value into CCPRIL
            CCP1CON ^= ((myarray[((char *) &PhaseAccum)[3]]) & 0x03) << 4; // load in bits 1-0 into 5 and 4 of CCP1CON
        }


        ////// 8 algorithms //////
        int ctr = readADC(0) >> 7;

        //stuff
        if (ctr == 0) {
            PhaseAccum = PhaseAccum + ((PhaseShift * sine[readADC(1)]) + 1);
        }
        //whistle FM
        if (ctr == 1) {
            PhaseAccum = PhaseAccum + (((PhaseShift << readADC(1)) * sine[step2]) + 1);
            //step2 += 1; //see variables
        }
        //brocken
        if (ctr == 2) {
            PhaseAccum = ((PhaseShift * sine[ADRESL]) + PhaseShift + readADC(1));
        }
        //starling
        if (ctr == 3) {
            PhaseAccum = PhaseAccum % (PhaseAccum + step3) + (((PhaseShift * sine[step3 >> readADC(1)])));
            //step3 += 3;
        }
        //sine
        if (ctr == 4) {// brackets added!
            PhaseAccum = PhaseAccum + ((PhaseShift << 6) + 1);
        }
        //brownouts
        if (ctr == 5) {
            PhaseAccum = ((PhaseShift * sine[ADRESL]) + PhaseShift)*2232;
        }
        //speed-dial
        if (ctr == 6) {
            PhaseAccum = PhaseAccum + (PhaseShift << (myarray[readADC(1)] >> 5));
            PhaseShift ^= readADC(1);
        }
        //strange memory effect
        if (ctr == 7) {
            ADRESL = readADC(1) * PhaseShift;
            PhaseAccum = PhaseAccum + ((PhaseShift * (sine[readADC(1)*6])));
        }
    }
}