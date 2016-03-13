/*
 * Copyright (C) 2016 Tolga Ceylan
 *
 * CopyPolicy: Released under the terms of the GNU GPL v3.0.
 */
/* 16Mhz AtMega32u4 */
#define F_CPU 16000000

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/twi.h>
#include <util/delay.h>
#include <util/atomic.h>
#include <avr/sleep.h>
#include <string.h>
#include "TWI_slave.h"

// ENCODER 1 B  B7
// ENCODER 1 A  B6
// ENCODER 2 B  B5
// ENCODER 2 A  B4
// LED          B0

#define LED_ON   (0xba)
#define LED_OFF  (0xab)
#define RESET_COUNTER (0xaa)
#define RESET_ENC0 (0xb0)
#define RESET_ENC1 (0xb1)

#define TWI_SLAVE_ADDRESS (0x20)
#define ENC0 (0)
#define ENC1 (1)
#define ISR_COUNT (2)
#define CMD_COUNT (3)
#define RECV_BUF_SIZE 1

/* rotary encoder values */
volatile uint32_t encoders[4];

/* recv buffer for I2C */
uint8_t recvBuffer[RECV_BUF_SIZE];

static inline void handle_data(uint8_t *msg, uint8_t msg_size)
{
	if (msg_size == 0)
		return;

	if (msg[0] == LED_ON) {
		PORTB |= (1 << PORTB0);
	}
	else if (msg[0] == LED_OFF) {
		PORTB &= ~(1 << PORTB0);
	}
	else if (msg[0] == RESET_COUNTER) {
		encoders[ISR_COUNT] = 0;
	}
	else if (msg[0] == RESET_ENC0) {
		encoders[ENC0] = 0;
	}
	else if (msg[0] == RESET_ENC1) {
		encoders[ENC1] = 0;
	}

	++encoders[CMD_COUNT];
}

static inline void reset_data(uint8_t *msg, uint8_t msg_size)
{
	memset(msg, 0, msg_size);
}

static inline void check_handle_recv(void)
{
	// Check if the last operation was a reception
	if (TWI_statusReg.RxDataInBuf &&
		TWI_Get_Data_From_Transceiver(recvBuffer, RECV_BUF_SIZE)) {

		handle_data(recvBuffer, RECV_BUF_SIZE);
		reset_data(recvBuffer, RECV_BUF_SIZE);
	}
}

static inline void check_handle_send(void)
{
	TWI_Start_Transceiver_With_Data((uint8_t *)encoders, sizeof(encoders));
}

ISR(PCINT0_vect)
{
	static uint8_t lastEncoded[2];

	uint8_t encoded[2] = { 0x00, 0x00 };
	uint8_t sum[2] = { 0x00, 0x00 };
	uint8_t values = PINB;

	encoded[0] = (values & (1 << PINB4)) ? 0x02 : 0x00;
	encoded[0] |= (values & (1 << PINB5)) ? 0x01 : 0x00;
	encoded[1] = (values & (1 << PINB6)) ? 0x02 : 0x00;
	encoded[1] |= (values & (1 << PINB7)) ? 0x01 : 0x00;

	sum[0] = (lastEncoded[0] << 2) | encoded[0];
	sum[1] = (lastEncoded[1] << 2) | encoded[1];

	if (sum[0] == 0b1101 || sum[0] == 0b0100 || sum[0] == 0b0010 || sum[0] == 0b1011)
		++encoders[ENC0];
  	if (sum[0] == 0b1110 || sum[0] == 0b0111 || sum[0] == 0b0001 || sum[0] == 0b1000)
  		--encoders[ENC0];

	if (sum[1] == 0b1101 || sum[1] == 0b0100 || sum[1] == 0b0010 || sum[1] == 0b1011)
		++encoders[ENC1];
  	if (sum[1] == 0b1110 || sum[1] == 0b0111 || sum[1] == 0b0001 || sum[1] == 0b1000)
  		--encoders[ENC1];

  	lastEncoded[0] = encoded[0];
  	lastEncoded[1] = encoded[1];
		
	// this is our simple counter / debugger
	++encoders[ISR_COUNT];
}

void initialize(void)
{
	// enable all required ports
	DDRB |= (1 << DDB4) | (1 << DDB5) | (1 << DDB6) | (1 << DDB7);

	// don't forget our LED
	DDRB |= (1 << DDB0);

	//turn on pullups
	PORTB |= (1 << DDB4) | (1 << DDB5) | (1 << DDB6) | (1 << DDB7);

  	//enable encoder pins interrupt sources
  	PCMSK0 |= (1 << PCINT7) | (1 << PCINT6) | (1 << PCINT5) | (1 << PCINT4);  

	// enable pin change int
	PCICR |= (1 << PCIE0);

	reset_data(recvBuffer, RECV_BUF_SIZE);
  
	TWI_Slave_Initialise(TWI_SLAVE_ADDRESS << TWI_ADR_BITS); 
                       
	sei();

	TWI_Start_Transceiver();
}

int main(void)
{
	initialize();

	for(;;)
	{
		if (!TWI_Transceiver_Busy()) {
			check_handle_recv();
			check_handle_send();
		}
	}

	return 0;
}
