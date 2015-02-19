/* 
 * File:   thedudesrhythm.c
 * Author: Dev
 *this program uses an analog signal from an encoder to infer shaft position
 * on a DC motor which is driven by an H bridge chip
 * This program could be improved by saving the position of the shaft in non-
 * volatile memory so it does not have to be calibrated every time
 * Created on February 7, 2015, 8:46 PM
 */

#include <xc.h>
#pragma config FOSC=INTRCIO, WDTE=OFF, PWRTE=OFF, MCLRE=ON, CP=OFF, CPD=OFF, \
BOREN=OFF, IESO=OFF, FCMEN=OFF
#define _XTAL_FREQ 4000000

int READING;       //variable to store Analogue value
int COUNT=0;       //variable to store Analogue signal changes


void main(void)
{
    TRISA=0xFF; //set all PORTs as inputs
    TRISB=0xFF;
    TRISC=0xFF;

    ANSEL = 0x00; //disable all analog ports
    ANSELH = 0x00;

    TRISAbits.TRISA0=1; //set A0/AN0 as an input
    ANSELbits.ANS0=1;   //set A0/AN0 to Analogue mode

    TRISC=0x00;         //set PORTC as all outputs

     ////////////////
    // ADC0 Setup //
    ///////////////

    ADCON0bits.ADFM = 1; //ADC result is right justified
    ADCON0bits.VCFG = 0; //Vdd is the +ve reference
    ADCON1bits.ADCS = 0b001; //Fosc/8 is the conversion clock
                            //This is selected because the conversion
                            //clock period (Tad) must be greater than 1.5us.
                            //With a Fosc of 4MHz, Fosc/8 results in a Tad
                            //of 2us.
    ADCON0bits.ADON = 1; //Turn on the ADC

     ///////////////
    //main loop///
    /////////////

    while(1)
    {
                                    //Gearbox motor shaft turning CW
        PORTCbits.RC3=1;            //set enable bit high
        PORTCbits.RC4=1;            //set 1A high
        PORTCbits.RC5=0;            //set 2A  low
        
        checkReading(); //this will only happen after 10 m0rev/for executions

        for (int loopCount = 0; loopCount < 10; loopCount++) // this will loop through the servo functions 10 times
        {                                                       //to compensate for the 200 ms delay to poll the encoder
            if(COUNT>=5)                //after a count of 5 rotate the servo one way
            {                    
                // PORTCbits.RC0=1;
                // __delay_ms(10);
                // PORTCbits.RC0=0;
                // __delay_ms(10);
                m0rev();
            }
            else
            {
                // PORTCbits.RC0=1;        //otherwise rotate the other way
                // __delay_ms(1);
                // PORTCbits.RC0=0;
                // __delay_ms(19);
                m0for();
            }
        }
    }
}

int m0for()
{
    PORTCbits.RC0 = 1;
    __delay_ms(1);
    PORTCbits.RC0 = 0;
    __delay_ms(19);
}

int m0rev()
{
    PORTCbits.RC0 = 1;
    __delay_ms(10);
    PORTCbits.RC0 = 0;
    __delay_ms(10);
}

int checkReading() //this function is only called after the for-loop completes, which equals 200 ms of delay
{
    __delay_us(5); //Wait the acquisition time (about 5us).
    ADCON0bits.CHS = 0; //select analog input, AN0
    ADCON0bits.GO = 1; //start the conversion
    while(ADCON0bits.GO==1){}; //wait for the conversion to end
    READING = ADRESL+(ADRESH*256); /*combine the 10 bits and store in
                                     a variable*/
    
    if(READING>=800)          //when the analog output signal of encoder is greater than
    {                         // 800 or when the infrared chip reads a blank space on the wheel
        COUNT++;              // the chip counts 1
        // __delay_ms(200);      //delay for 200 microseconds so the chip doesn't poll constantly
        if (COUNT==10)        //10 counts is one full rotation of the wheel
        {
            COUNT=0;          //reset the count after 10
            PORTC=0b00000100;
        }
    }
}