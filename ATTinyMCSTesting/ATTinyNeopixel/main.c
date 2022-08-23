/*
 * ATTinyNeopixel.c
 *
 * Created: 5/18/2021 2:56:41 AM
 * Author : Jonathan Schallert
 * jpschallert@gmail.com
 */ 

//TODO:
// Add speed adjust potentiometer, code to make it work
// Add brightness adjust potentiometer, code to make it work
// Add support for 90-100 pixels, 3535s ideally

#define F_CPU 8000000UL
#define NUM_LEDS 12

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "Light_WS2812/light_ws2812.h"

volatile long long millSec;

//Interrupt for generating millis value, every ~1 sec
ISR(TIM0_COMPA_vect){
	millSec++;
	TCNT0 = 130; // Clear timer0 counter
}


struct cRGB led[12];
const int max_fastSin_input = 63;

const unsigned char sin_lookup[63] = 
{137, 150, 162, 174, 185, 196, 206, 215, 223, 230, 236, 242, 245, 248,
 250, 250, 249, 247, 243, 239, 233, 226, 218, 209, 200, 189, 178, 167,
 155, 143, 130, 118, 105, 93, 81, 70, 59, 49, 39, 30, 23, 16, 10, 6, 3,
 1, 0, 0, 2, 5, 9, 15, 21, 28, 37, 46, 56, 67, 78, 90, 102, 115, 127 };
 
inline unsigned int fastSin( float input ){
	unsigned int approximation = ((unsigned int)(input * 10) % max_fastSin_input);
	return sin_lookup[ approximation ];
}

inline unsigned int fastSinPhaseDelay( float input , float phaseAdjust){
	unsigned int approximation = (unsigned int)(input * 10 + phaseAdjust*64) % max_fastSin_input;
	return sin_lookup[ approximation ];
}

void timer0_Init() {
	// Clear timer0 counter
	TCNT0 = 130;  //255-125=130
	TIMSK0 = (1 << OCIE0A);		// Enable interrupt for timer0 overflow
	TCCR0B = (1<<CS00)|(1<<CS01);	// Prescaler to 64, start timer
}

int main(void)
{
	timer0_Init();
	millSec = 0;  // init the millisecond counter
	for(int i = NUM_LEDS; i>0; i--){
		led[i-1].r=0;led[i-1].g=0;led[i-1].b=0;
	}
	sei();	
	
	
    while (1) 
    {
			long currTime = millSec;
			for(int i = NUM_LEDS; i>0; i--){
				float fracLEDs = (float)i/(float)NUM_LEDS;
				led[i-1].r=fastSinPhaseDelay( currTime * 0.01 , fracLEDs);
				led[i-1].g=0;
				led[i-1].b=0;
			}
			_delay_ms(5);
			ws2812_sendarray((uint8_t *)led,36);
    }
}

