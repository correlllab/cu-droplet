#define F_CPU 8000000

#include <avr/io.h>
#include <util/delay.h>

#define BAUD_RATE 4800
#define CHARGE_DELAY 25000 //in ms
#define SIGNAL_DELAY 4000 //in ms
#define HIGH 1
#define LOW 0

inline void set_tx(uint8_t VAL)
{
	if(VAL) PORTB |= (1 << PB4);
	else    PORTB &= ~(1 << PB4);
}

inline void set_ctrl(uint8_t VAL)
{
	if(VAL) PORTB |= (1 << PB3);
	else    PORTB &= ~(1 << PB3);
}

void send_char(uint8_t ch)
{
	set_tx(LOW); _delay_us(1000000/BAUD_RATE); //start bit
	for(uint8_t i=0;i<8;i++)
	{
		set_tx(ch&(0x1<<i));
		_delay_us(1000000/BAUD_RATE);
	}
	set_tx(HIGH); _delay_us(1000000/BAUD_RATE); //stop bit
}

int main(void)
{
	DDRB |= (1 << DDB3); 	// Configure port3 (control) as an output pin
	DDRB |= (1 << DDB4);	// Configure port4 (serial tx) as an output pin.
	set_tx(HIGH);
	set_ctrl(LOW);
	
	while(1)
	{
		set_ctrl(HIGH);
		send_char('s');
		_delay_ms(SIGNAL_DELAY);
		set_ctrl(LOW);
		_delay_ms(CHARGE_DELAY);
	}
}


