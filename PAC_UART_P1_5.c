//                MSP430FR2433
//             -----------------
//         /|\|                 |
//          | |                 |
//          --|RST              |
//            |                 |
//            |                 |
//            |     P1.4/UCA0TXD|---->
//            |                 |
//            |     P1.5/UCA0RXD|<----
//            |                 |
//            |                 |
//            |                 |
//
//******************************************************************************
#include <msp430.h>

const int true = 1;
const int false = 0;
char RXData;
const int size = 10;
char RXBuffer[size] = "0000000000";
unsigned int j = 0;
int f_buffer_lleno = 0;
int f_salto_de_linea = 0;
int f_timer_finished = 0;
unsigned int g = 0;
unsigned int r = 0;
const int limitt = 8; //Declaramos varible que es el limite de tiempo, está en segundos, el valor de es limitt = t-1, donde t son los segundos a esperar
int cont = 0;
int main(void){

    WDTCTL = WDTPW | WDTHOLD;                 // Detenemos el timer del perro guardian

    PM5CTL0 &= ~LOCKLPM5;                     // Deshabilitamos la alta impedencia en los GPIO

    // Configuración de timer A0
    TA0CTL |= TASSEL_1 | MC_1;   //Seleccionamos la fuente de reloj SMCLK (1MHz) y seleccionamos Up mode (cuenta hasta el valor de TA0CCR0
    TA0CCR0 = 32768;            //Le damos un valor pequeño al registro de captura comparación.
    //TA0CCTL0 |= CCIE;           // Habilitamos las interrupciones del timer A

    // Configuración de los pines para la comunicación UART
    P1SEL0 |= BIT4 | BIT5;                    // Habilitamos la función secundaria de los pines p1.4 y p1.5

    // Configuración de la comunicación UART
    UCA0CTLW0 |= UCSSEL__SMCLK;               // Seleccionamos 1 bit de stop y la fuente de reloj SMCLK
    UCA0BR0 = 8;                              // Damos el preescalador de la fuente de reloj
                                              // se divide el valor de los Hz del reloj entre el valor de los baudios por segundo deseados.
    UCA0MCTLW = 0xD600;                       // Porción fraccional de N (Preescalador de la fuente de reloj).

    UCA0CTLW0 &= ~UCSWRST;                    // Habilitamos la comunicación UART
    UCA0IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt

    while (true){
        //Desbordamiento del buffer
        if(f_buffer_lleno == true){
            TA0CCTL0 &= ~CCIE;                 // Deshabilitamos las interrupciones del timer A
            // Transmitimos los carácteres dentro del buffer
            for(g=0;g<size;g++){
                while(!(UCA0IFG&UCTXIFG));
                UCA0TXBUF = RXBuffer[g];
            }
            j = 0;                                 // Reiniciamos el índice
            f_buffer_lleno = false;                // Ponemos en cero la bandera de desboramiento de buffer
            // Borramos lo que esté en el buffer
            //for(r=0;r<size;r++){
            //    RXBuffer[r] = '0';
            //}
        }
        // Detección del salto de línea
        if(f_salto_de_linea == true){
            TA0CCTL0 &= ~CCIE;                 // Deshabilitamos las interrupciones del timer A
            // Transmitimos los carácteres dentro del buffer
            for(g=0;g<j;g++){
                while(!(UCA0IFG&UCTXIFG));
                UCA0TXBUF = RXBuffer[g];
            }
            j = 0;                                 // Reiniciamos el índice
            f_salto_de_linea = false;              // Ponemos en cero la bandera de detección de salto de línea
            // Borramos lo que esté en el buffer
            //for(r=0;r<size;r++){
            //    RXBuffer[r] = '0';
            //}
        }
        // Desbordamiento timer
        if(f_timer_finished == true){
            TA0CCTL0 &= ~CCIE;                 // Deshabilitamos las interrupciones del timer A
            // Transmitimos los carácteres dentro del buffer
            for(g=0;g<j;g++){
                while(!(UCA0IFG&UCTXIFG));
                UCA0TXBUF = RXBuffer[g];
            }
            j = 0;                                 // Reiniciamos el índice
            f_timer_finished = false;              // Ponemos en cero la bandera de detección de salto de línea
            // Borramos lo que esté en el buffer
            //for(r=0;r<size;r++){
            //    RXBuffer[r] = '0';
            //}
        }
        __bis_SR_register(LPM0_bits|GIE);     // Enter LPM0
        __no_operation();                     // For debugger
    }
}

// Rutina de interrupción del timer A0
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A (void){
    cont++;
    if(cont == limitt){
        f_timer_finished = true;
    }
    __bic_SR_register_on_exit(LPM0_bits); // Exit LPM0 on reti
}

// Rutina de interrupción eUSCI_A en modo UART
#pragma vector = USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void) {
   if(UCA0IFG & UCRXIFG){                // La bandera UCRXIFG, del registo UCA0IFG, se pone en alto cuando se recibe un carácter completo
       RXBuffer[j] = UCA0RXBUF;          // Guardamos en un buffer lo que esté en el buffer de recepción del UART
       TA0CCTL0 |= CCIE;                 // Habilitamos las interrupciones del timer A
       cont = 0;
       j++;                              // Aumentamos en 1 el índice del buffer
       if(j == size){
           f_buffer_lleno = true;        // Se pone en alto la bandera de desbordamiento de buffer
       }
       if(UCA0RXBUF == 10){
           f_salto_de_linea = true;      // Se pone en alto la bandera de dectección de salto de línea
       }
   }
   __bic_SR_register_on_exit(LPM0_bits); // Exit LPM0 on reti
 }
