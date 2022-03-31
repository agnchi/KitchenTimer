
#include <avr/io.h>
#include <avr/interrupt.h>
#define B1_pressed !(PINC & (1<<PINC1))
#define B2_pressed !(PINC & (1<<PINC2))


unsigned char numbers[10]={0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F};

struct digits
{
	unsigned char d1;
	unsigned char d2;
	unsigned char d3;
	unsigned char d4;
};

char start=0;
struct digits ourdigits={0,0,0,0};
struct digits dispdigits;
char pressed;
char ring_var=0;

void timer1Setup()
{
	// CTC mode - clear timer on compare
	//1024 prescaler
	//compared value: 15625 to get 1s
	
	TCCR1B=(1<<WGM12); //sets mode
	OCR1A=15625; //actually 2 registers holding lower and higher nibbles
	TIMSK1=(1<<OCIE1A); //enable compare A match interrupt
	TCCR1B |= (1<<CS10) |(1<<CS12); //set prescaler and start the timer
}
void ext_interruptSetup()
{

	PCMSK1=(1<<PCINT9)|(1<<PCINT10); //portC pin1
	PCICR=(1<<PCIE1); //enable interrupt on port c

}


void displaySetup()
{
	DDRB=15;
	DDRD=255;
	PORTB=0b00000000;
	PORTD=~0x3F;
}
void wait(unsigned int delay)
{
	volatile unsigned int i; //volatile prevents from i optimization
	for(i=0; i<delay; i++); //empty loop as delay
}
void display(struct digits *numbers_to_display)
{

	for(int i=0;i<10;i++)
	{
		PORTB=0b00001110;
		PORTD=~(numbers_to_display->d1);
		wait(5000);
		
		PORTB=0b00001101;
		PORTD=~(numbers_to_display->d2);
		wait(5000);
		
		PORTB=0b00001011;
		PORTD=~(numbers_to_display->d3);
		wait(5000);
		
		PORTB=0b00000111;
		PORTD=~(numbers_to_display->d4);
		wait(5000);
	}
	
}
struct digits encode(struct digits *numbers_to_encode)
{
	struct digits copydigits=(*numbers_to_encode);
	
	for(int i=0;i<10;i++)
	{
		if(numbers_to_encode->d1 ==i)
		{
			copydigits.d1=numbers[i];
		}
		if(numbers_to_encode->d2 ==i)
		{
			copydigits.d2=numbers[i];
		}
		if(numbers_to_encode->d3 ==i)
		{
			copydigits.d3=numbers[i];
		}
		if(numbers_to_encode->d4 ==i)
		{
			copydigits.d4=numbers[i];
		}
	}
	return copydigits;
}
struct digits countdown(struct digits* dig)
{
	struct digits digitsout=(*dig);
	if((dig->d4)>0)
	{
		(digitsout.d4)--;
	}
	else
	{
		(digitsout.d4)=9;
		if((dig->d3)>0)
		{
			(digitsout.d3)--;
		}
		else
		{
			(digitsout.d3)=5;
			if((dig->d2)>0)
			{
				(digitsout.d2)--;
			}
			else
			{
				(digitsout.d2)=9;
				if((dig->d1)>0)
				{
					(digitsout.d1)--;
				}
				else
				{
					(digitsout.d1)=5;
				}
			}
		}
	}
	
	return digitsout;
}
struct digits countup(struct digits* dig)
{
	struct digits digitsout=(*dig);
	
	if((dig->d4)<9)
	{
		(digitsout.d4)++;
	}
	else
	{
		(digitsout.d4)=0;
		if((dig->d3)<5)
		{
			(digitsout.d3)++;
		}
		else
		{
			(digitsout.d3)=0;
			if((dig->d2)<9)
			{
				(digitsout.d2)++;
			}
			else
			{
				(digitsout.d2)=0;
				if((dig->d1)<5)
				{
					(digitsout.d1)++;
				}
				else
				{
					(digitsout.d1)=0;
					
				}
			}
		}
	}
	return digitsout;
}
void ring()
{
	DDRB |=(1<<PORTB5);
	char ring_time=5;
	while(ring_time>0)
	{
		PORTB |=(0<<PORTB5);
		ring_time--;
	}
	PORTB |=(1<<PORTB5);
}

int main(void)
{
	timer1Setup();
	ext_interruptSetup();
	displaySetup();
	sei(); //enable interrupts globally
	dispdigits=encode(&ourdigits);
	display(&dispdigits);

	while(1)
	{
		if(pressed==0)
		{
			display(&dispdigits);
		}
		if(ring_var==1)
		{
			ring();
			ring_var=0;
		}
	}
	
}
ISR(PCINT1_vect)
{
	if(B1_pressed){
		pressed=1;
		ourdigits=countup(&ourdigits);
		dispdigits=encode(&ourdigits);
		display(&dispdigits);
		pressed=0;
	}
	if(B2_pressed)
	{
		start=1;
	}
}

ISR(TIMER1_COMPA_vect)
{
	if(start==1)
	{
		pressed=1;
		display(&dispdigits);
		ourdigits=countdown(&ourdigits);
		dispdigits=encode(&ourdigits);
		pressed=0;
		if(ourdigits.d1==0 && ourdigits.d2==0 && ourdigits.d3==0 && ourdigits.d4==0 )
		{
			start=0;
			pressed=0;
			display(&dispdigits);
			ring_var=1;
		}
	}
}