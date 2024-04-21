#include <msp430.h> 
//                MSP430FR2433
//             -----------------
//         /|\|                 |
//          | |                 |
//          --|RST              |
//            |                 |
//            |                 |
//            |     P1.4/UCA0TXD|---->
//            |                 |   |
//            |     P1.5/UCA0RXD|<----
//            |                 |
//            |                 |
//            |                 |
char TXData;

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	PM5CTL0 &= ~LOCKLPM5;                     // Disable the GPIO power-on default high-impedance mode
	                                          // to activate previously configured port settings
	P1DIR |= BIT0;                            // Ponemos el pin p1.0 como salida (Led rojo)
	P1OUT &= ~BIT0;                           // Apagamos el Led rojo

	TXData = 'G';                             // Le damos valor a la variable TXData

	// Configuración de timer A0
	TA0CTL |= TASSEL_2 | MC_1;   //Seleccionamos la fuente de reloj SMCLK (1MHz) y seleccionamos Up mode (cuenta hasta el valor de TA0CCR0
	TA0CCR0 = 3277;            //Le damos un valor pequeño al registro de captura comparación.
	TA0CCTL0 |= CCIE;           // Habilitamos las interrupciones del timer A

	// Configure UART pins
	P1SEL0 |= BIT4 | BIT5;

	// Configure UART
	UCA0CTLW0 |= UCSWRST;                   // Put eUSCI in reset

	// Configuración de la comunicación UART
	UCA0CTLW0 |= UCSSEL__SMCLK;             // Seleccionamos 1 bit de stop y la fuente de reloj SMCLK
	UCA0CTLW0 &= ~UCSPB;
	UCA0BR0 = 104;                            // Damos el preescalador de la fuente de reloj
	                                        // se divide el valor de los Hz del reloj entre el valor de los baudios por segundo deseados.
	UCA0MCTLW = 0x1100;                     // Porción fraccional de N (Preescalador de la fuente de reloj).
	
	UCA0CTLW0 &= ~UCSWRST;                  // Habilitamos la comunicación UART
	UCA0IE |= UCRXIE;                       // Enable USCI_A0 RX interrupt

	__bis_SR_register(LPM0_bits|GIE);     // Enter LPM0
	__no_operation();                     // For debugger
	return 0;
}

//Rutina de interrupción del timer A0
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A (void){
    UCA0TXBUF = TXData;
    //UCA0TXBUF = 33;         // Envíamos el mensaje por comunicación UART.
    P1OUT ^= BIT0;                          // Toggleamos el led rojo.
}
