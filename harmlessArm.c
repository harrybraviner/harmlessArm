/* Code copyright Harry Braviner March 2015

harmlessArm.c

Code to perform PWM for servo control.
Servo positions are passed to the ATmega over UART

*/

#define F_CPU 16000000
#define BUFF_LEN 5

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <util/atomic.h>

volatile uint16_t servoAsetting, servoBsetting;
// N.B.: these two don't need to be volatile, since there
// is NO code which changes them which can be interrupted
char RXBuffer[BUFF_LEN];
uint8_t bufferCnt;

ISR(USART_RX_vect){
	char recvByte = UDR0;

	if (recvByte != '\n'){
		// Write into the buffer, if there's space left
		if(bufferCnt < BUFF_LEN){
			RXBuffer[bufferCnt] = recvByte;
			bufferCnt++;
		} else if (bufferCnt == BUFF_LEN){
			bufferCnt++;	// Record that we've received too many characters
		}
	} else {
		//OCR1A = (0xFFFF - OCR1A);
		// This code runs if recvByte == '\n'
		// Check that we have received exactly 5 characters
		if(bufferCnt != BUFF_LEN){
			bufferCnt = 0;
		} else {
			uint8_t valid = 1;	// Used to check if a valid command was sent
			uint8_t servoAB = 0;	// Used to record which servo the setting is for
			uint16_t setting = 0;	// Used to record the numerical value of the setting
			
			if (RXBuffer[0] == 'A'){ servoAB = 0; }
			else if (RXBuffer[0] == 'B') { servoAB = 1; }
			else { valid = 0; }
			
			for(int i=1; i<BUFF_LEN && valid == 1; i++){
				if ( ('0' <= RXBuffer[i]) && (RXBuffer[i] <= '9') ){
					setting += ( (RXBuffer[i] - '0') << (4*(BUFF_LEN-1-i)) );
				} else if ( ('a' <= RXBuffer[i]) && (RXBuffer[i] <= 'f') ){
					setting += ( (RXBuffer[i] - 'a' + 10) << (4*(BUFF_LEN-1-i)) );
				} else if ( ('A' <= RXBuffer[i]) && (RXBuffer[i] <= 'F') ){
					setting += ( (RXBuffer[i] - 'A' + 10) << (4*(BUFF_LEN-1-i)) );
				} else { valid = 0; }
			}

			// If we've received a valid packet, update the servo settings
			if (valid == 1) {
				if(servoAB == 0){
					OCR1A = setting;
				} else if (servoAB == 1){
					OCR1B = setting;
				}
			}

			// Set the counter back to the start of the buffer
			bufferCnt = 0;
		}
	}
}

ISR(BADISR_vect){
	// This block is here to avoid a reset if an
	// interrupt is triggered for which no handler is installed
}

void timer1Setup();
void UARTSetup();

int main(){
	// Set up UART
	UARTSetup();

	// Set the servo positions to zero initially
	servoAsetting = 0; servoBsetting = 0;

	// Set PB1(OC1A) and PB2(OC1B) to be a outputs
	DDRB |= (1<<DDB1)|(1<<DDB2);

	// Set up the Timer 1 to PWM OC1A and OC1B
	timer1Setup();

	// Enable global interrupts
	sei();


	// Set OC1A and OC1B low
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		OCR1A = 0x0000; OCR1B = 0x0000;
	}

	for(;;) {}
}

void timer1Setup(){
	TCCR1A = 0;
	TCCR1A |= (1<<COM1A1)|(1<<COM1B1);	// Set at BOTTOM, clear on match
	TCCR1A |= (1<<WGM11);								// Fast PWM, TOP = ICR1
	TCCR1B = 0;
	TCCR1B |= (1<<WGM12)|(1<<WGM13);		// Fast PWM, TOP = ICR1
	TCCR1B |= (0<<CS12)|(0<<CS11)|(1<<CS10);	// Clock / 1
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		ICR1 = 0xFFFF;										// Set the PWM resolution to 16 bit
	}
	TIMSK1 = 0;													// Disable interrupts from Timer 1
}

void UARTSetup(){
	// Enable USART0 in asynchronous, 8n1 mode, 9600 baud
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		// Disable the power-reduction bit for USART0
		PRR &= ~(1<<PRUSART0);
		// Normal speed, multi-processor communication mode disabled
		UCSR0A &= ~((1<<U2X0)|(1<<MPCM0));
		// Enable RX and RX Complete Interrupt
		UCSR0B |= (1<<RXCIE0)|(1<<RXEN0);
		// Disable TX and TX Complete Interrupt
		UCSR0B &= ~((1<<TXCIE0)|(1<<TXEN0));
		// Disable the Data Register Empty Interrupt
		UCSR0B &= ~(1<<UDRIE0);
		// Set the character size to 8-bit (UCSZ0 = 3 = 0b011)
		UCSR0B &= ~(1<<UCSZ02);
		UCSR0C |= (1<<UCSZ01)|(1<<UCSZ00);
		// Asynchronous USART
		UCSR0C &= ~((1<<UMSEL01)|(1<<UMSEL00));
		// No parity check, single stop bit
		UCSR0C &= ~((1<<UPM01)|(1<<UPM00)|(USBS0));
		// Set the baud rate to 9600
		UBRR0 = 103;	// See table 20-6, page 200 of at ATmega328p manual
		// Clear the receive buffer
		for(int i=0; i<5; i++) { RXBuffer[i] = '0'; };
		bufferCnt = 0;
	}
}
