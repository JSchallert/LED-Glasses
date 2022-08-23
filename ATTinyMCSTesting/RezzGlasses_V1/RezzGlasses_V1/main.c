/*
 * RezzGlassesV1
 * Author : Jonathan Schallert
 * jpschallert@gmail.com
 */ 

//TODO:
// Add speed adjust potentiometer, code to make it work
// Add brightness adjust potentiometer, code to make it work

// Add in sequences; struct of functions and times, 
// set currTime to 0 and parse struct to play songs

// Add user inputs
 
#define F_CPU 8000000UL

#define NUM_LEDS 120

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "Light_WS2812/light_ws2812.h"

volatile long long millSec;

//Interrupt for generating millis value, every ~1 sec
ISR(TIMER0_COMPA_vect){
	millSec++;
	TCNT0 = 130; // Clear timer0 counter
}

// Topology
char ring[4] = {28,16,12,4};
	
struct cRGB led[NUM_LEDS];
float *RGB;
const int max_fastSin_input = 64;
unsigned char RGBSpeed = 30;

struct color {
	float R;
	float G;
	float B;
};

struct color white	 = {.R=1,.G=1,.B=1};
struct color red	 = {.R=1,.G=0,.B=0};
struct color green	 = {.R=0,.G=1,.B=0};
struct color blue	 = {.R=0,.G=0,.B=1};
struct color yellow	 = {.R=1,.G=1,.B=0};
struct color cyan	 = {.R=0,.G=1,.B=1};
struct color magenta = {.R=1,.G=0,.B=1};

const unsigned char sin_lookup[64] =
{128,165,202,239,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,239,202,165,128,90,53,16,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,16,53,90};

const unsigned char sin_full_lookup[64] =
{128,140,152,165,176,188,198,208,218,226,234,240,245,250,253,254,255,254,253,250,245,240,234,226,218,208,198,188,176,165,152,140,128,115,103,90,79,67,57,47,37,29,21,15,10,5,2,1,0,1,2,5,10,15,21,29,37,47,57,67,79,90,103,115};

inline unsigned int fastSin( float input ){
	unsigned int approximation = ((unsigned int)(input * 10) % max_fastSin_input);
	return sin_full_lookup[ approximation ];
}

inline unsigned int lookup( float input , float phaseAdjust, const unsigned char lookupData[]){
	unsigned int approximation = (unsigned int)(input * 10 + phaseAdjust*64) % max_fastSin_input;
	return lookupData[ approximation ];
}

// Gets array of RGB values based on time and speed seed
float *getRGB(long currTime, unsigned char RGBSpeed) {
	static float RGB[3];
	float colorNum = 255-((currTime%(256*RGBSpeed))/RGBSpeed);
	  if(colorNum < 85) {
		  RGB[0]= (255 - colorNum * 3)/255;
		  RGB[1]= 0;
		  RGB[2]= colorNum * 3/255;
		  return RGB;
	  }
	  if(colorNum < 170) {
		  colorNum -= 85;
		  RGB[0] = 0;
		  RGB[1] = colorNum * 3/255;
		  RGB[2] = (255 - colorNum * 3)/255;
		  return RGB;
	  }
	  colorNum -= 170;
	  RGB[0] = colorNum * 3/255;
	  RGB[1] = (255 - colorNum * 3)/255;
	  RGB[2] = 0;
	  return RGB;
}

//Use this function as the dummy function to get all the abstracted shit working
//TODO: add rgb toggle

void spiralStatic(long currTime, struct color RGBInput, float fadeSpeed, char isPrimary, char hasFade, char direction){
	char OFFSET = 0 + 60 * (!isPrimary);
	char RGBVal = 0;
	float fracLEDs;
	float FadeVal;
	if(direction){
		for (int j = 0; j<sizeof(ring);++j){
			for(int i = ring[j]; i>0; i--){
				if(hasFade){
					 FadeVal = (float)fastSin( currTime * fadeSpeed)/255; //Default = 0.015
				}else{
					FadeVal = 1;
				}
				if(j==3){
					fracLEDs = (3+j+(float)i*2)/(float)ring[j];
					RGBVal = lookup( -currTime * 0.015 , fracLEDs, sin_full_lookup);
					}else{
					fracLEDs = (j*2.75+(float)i*2)/(float)ring[j];
					RGBVal = lookup( -currTime * 0.015 , fracLEDs, sin_lookup);
				}
				led[i-1+OFFSET].r=(int)(RGBInput.R * RGBVal * FadeVal);
				led[i-1+OFFSET].g=(int)(RGBInput.G * RGBVal * FadeVal);
				led[i-1+OFFSET].b=(int)(RGBInput.B * RGBVal * FadeVal);
			}
			OFFSET += ring[j];
			}
	}else{
		for (int j = 0; j<sizeof(ring);++j){
			for(int i = ring[j]; i>0; i--){
				float FadeVal = (float)fastSin( currTime * fadeSpeed)/255; //Default = 0.015
				if(j==3){
					fracLEDs = (3+j+(float)i*2)/(float)ring[j];
					RGBVal = lookup( currTime * 0.015 , fracLEDs, sin_full_lookup);
					}else{
					fracLEDs = (j*2.75+(float)i*2)/(float)ring[j];
					RGBVal = lookup( currTime * 0.015 , fracLEDs, sin_lookup);
				}
				led[i-1+OFFSET].r=(int)(RGBInput.R * RGBVal * FadeVal);
				led[i-1+OFFSET].g=(int)(RGBInput.G * RGBVal * FadeVal);
				led[i-1+OFFSET].b=(int)(RGBInput.B * RGBVal * FadeVal);
			}
			OFFSET += ring[j];
			}
	}
}



void spiralCross(long currTime, float fadeSpeed, char isPrimary, char hasFade, char direction){
	char OFFSET = 0 + 60 * (!isPrimary);
	char RGBVal = 0;
	float fracLEDs;
	if(direction){
		for (int j = 0; j<sizeof(ring);++j){
			for(int i = ring[j]; i>0; i--){
				if(j==3){
					fracLEDs = (3+j+(float)i*2)/(float)ring[j];
					RGBVal = lookup( currTime * 0.015 , fracLEDs, sin_full_lookup);
					}else if(j==1){
					fracLEDs = (j*2.75+(float)i*2)/(float)ring[j];
					RGBVal = lookup( currTime * 0.015 , fracLEDs, sin_lookup);
					}else{
					fracLEDs = (j+(float)i*2)/(float)ring[j];
					RGBVal = lookup( -currTime * 0.015 , fracLEDs, sin_lookup);
				}
				led[i-1+OFFSET].r=(int)(*(RGB) * RGBVal);
				led[i-1+OFFSET].g=(int)(*(RGB+1) * RGBVal);
				led[i-1+OFFSET].b=(int)(*(RGB+2) * RGBVal);
			}
			OFFSET += ring[j];
		}
	}else{
		for (int j = 0; j<sizeof(ring);++j){
			for(int i = ring[j]; i>0; i--){
				if(j==3){
					fracLEDs = (3+j+(float)i*2)/(float)ring[j];
					RGBVal = lookup( -currTime * 0.015 , fracLEDs, sin_full_lookup);
					}else if(j==1){
					fracLEDs = (j*2.75+(float)i*2)/(float)ring[j];
					RGBVal = lookup( -currTime * 0.015 , fracLEDs, sin_lookup);
					}else{
					fracLEDs = (j+(float)i*2)/(float)ring[j];
					RGBVal = lookup( currTime * 0.015 , fracLEDs, sin_lookup);
				}
				led[i-1+OFFSET].r=(int)(*(RGB) * RGBVal);
				led[i-1+OFFSET].g=(int)(*(RGB+1) * RGBVal);
				led[i-1+OFFSET].b=(int)(*(RGB+2) * RGBVal);
			}
			OFFSET += ring[j];
		}
	}
}

void pulse(long currTime, int color){
	char RGBVal = 0;
	for(int i = NUM_LEDS; i>0; i--){
		RGBVal = lookup( -currTime * 0.015 , 0, sin_full_lookup);
		led[i-1].r=RGBVal;
		led[i-1].g=0;
		led[i-1].b=0;
	}
}

void rainbowCycle(long currTime, float *RGB){
	//char RGBVal = 0;
	for(int i = NUM_LEDS; i>0; i--){
		//RGBVal = lookup( -currTime * 0.015 , 0, sin_full_lookup);
		led[i-1].r= (int)(*(RGB) * 255);
		led[i-1].g= (int)(*(RGB+1) * 255);
		led[i-1].b= (int)(*(RGB+2) * 255);
	}
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
	CLKPR = (1<<CLKPCE);
	millSec = 0;  // init the millisecond counter
	for(int i = NUM_LEDS; i>0; i--){
		led[i-1].r=0;led[i-1].g=0;led[i-1].b=0;
	}
	sei();	
	
	
    while (1) 
    {
			long currTime = millSec;
			RGB = getRGB(currTime, RGBSpeed); //Get RGB as a pointer to a char array
			
			if(currTime<500){
				spiralCross(currTime,0.01, 1, 0, 1);
				spiralCross(currTime,0.01, 0, 0, 0);
			}else if(currTime<1800){
				spiralStatic(currTime, magenta, 0.01, 1, 1, 1);
				spiralStatic(currTime, blue, 0.01, 0, 1, 0);
			}else if(currTime<3000){
				spiralStatic(currTime, blue, 0.01, 1, 1, 0);
				spiralStatic(currTime, magenta, 0.01, 0, 1, 1);
			}else if (currTime<3800){
				spiralStatic(currTime, cyan, 0.011, 1, 1, 1);
				spiralStatic(currTime, cyan, 0.011, 0, 1, 0);
			}else{
				spiralCross(currTime,0.01, 1, 0, 0);
				spiralCross(currTime,0.01, 0, 0, 1);
			}
			_delay_ms(5);
			ws2812_sendarray((uint8_t *)led,NUM_LEDS*3);
    }
}

