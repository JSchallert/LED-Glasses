/*
 * ATTiny84ATest.c
 *
 * Created: 5/12/2021 8:58:31 PM
 * Author : Jonathan Schallert
 * jpschallert@gmail.com
 */ 

#define F_CPU 8000000
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>

/*----------------------MACROS-------------------------*/
#define LED_ON		PORTA |= (1<<DDA0)
#define LED_OFF		PORTA &= ~(1<<DDA0)
#define LED_TOGGLE	PINA |= (1<<PINA0)
#define PWM10K		800

enum {UP,DOWN};

/*---------------------FUNCTIONS-----------------------*/
void PWMIncreaseBrightness(void);
void PWMDecreaseBrightness(void);
void PWMBrightDim(void);
void timerFrequency(uint8_t freq);
void PWM_Init(float duty);
void milliS_timer(uint8_t millis);

ISR(TIM1_COMPA_vect){
	LED_ON;
}

ISR(TIM1_COMPB_vect){
	LED_OFF;
}

ISR(TIM0_COMPA_vect){
	PWMBrightDim();
}

void PWMIncreaseBrightness(void){
	uint16_t period = OCR1A;
	uint16_t duty = OCR1B;
	if(duty<period)
	{
		duty++;
	}
	else
	{
		duty=0;
	}
	OCR1B = duty;
}

void PWMDecreaseBrightness(void){
	uint16_t period = OCR1A;
	uint16_t duty = OCR1B;
	if(duty>0)
	{
		duty--;
	}
	else
	{
		duty=0;
	}
	OCR1B = duty;
}

void PWMBrightDim(void){
	uint16_t duty = OCR1B;
	uint16_t period = OCR1A;
	static uint8_t direction;

	switch(direction)
	{
		case UP:
			if(++duty == (period-1))
			direction=DOWN;
			break;
		case DOWN:
			if(--duty==2)
			direction=UP;
			break;
	}
	OCR1B = duty;
}

void timerFrequency(uint8_t freq){
	TCCR1B |= (1<<CS12) | (1<<WGM12); //Set prescaler to 256, set WGM to CTC@OCR1A
	TIMSK1 |= (1<<OCIE1A); //Enable overflow interrupt
	OCR1A = (F_CPU/(freq*2*256-1));
}

void PWM_Init(float duty){
	TCCR1B |= (1<<CS10) | (1<<WGM12); //Set prescale to 0
	TIMSK1 |= (1<<OCIE1A) | (1<<OCIE1B); //Enable overflow interrupt
	OCR1A = PWM10K;
	OCR1B = (duty * PWM10K); 
}

void milliS_timer(uint8_t millis){
	TCCR0A |= (1<<WGM01);				//Set to CTC mode
	TCCR0B |= (1<<CS02) | (1<<CS00);	//Set prescale to 1024
	OCR0A = millis*3.90625-1;
	TIMSK0 |= (1<<OCIE1A);
};

int main(void)
{
	//Initialize A0 as OUTPUT, A1 and A2 as INPUTS
	DDRA |= (1<<DDA0);
	DDRA &= ~(1<<DDA1);
	//timerFrequency(4);
	milliS_timer(2);
	PWM_Init(0.125);
	sei();
	
	while (1){
	}
}

/*-----EXAMPLE: Sleep mode, Pin Change Interrupt Enable----------*/
/*
//Interrupt: check A1, inverse output to A0
ISR(PCINT0_vect)
{
	if(SWITCH_PRESSED){
		LED_OFF;
		}else{
		LED_ON;
	}
}

int main(void)
{
	//Initialize A0 as OUTPUT, A1 and A2 as INPUTS
	DDRA |= (1<<DDA0);
	DDRA &= ~((1<<DDA1) | (1<<DDA2));
	PORTA |= (1<<PORTA2);		//Pullup to make interrupt easier
	
	PCMSK0 |= (1<<PCINT2);		//Mask set for PCINT2, on A2
	GIMSK |= (1<<PCIE0);		//Enable Interrupt Bank 0
	MCUCR |= (1<<ISC01);		//Interrupt happens on pin LOW
	
	sei();						//Set global interrupts to ENABLE

	while (1)
	{
		//Go into sleep mode
		sleep_mode();
		//wait for an interrupt
	}
}
*/