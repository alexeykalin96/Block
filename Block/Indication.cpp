#include "Indication.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

uint8_t Indication::currentSound = SOUND_NO;
uint8_t Indication::freq = 0;
uint16_t Indication::timerSound = 0;

Indication::Indication()
{
	currentSound = 0;
	freq = 0;
}

void Indication::initPorts(void)
{
	//Порт звуковой сигнализации на выход
	SOUND_DDR |= (1 << SOUND_OUT);
	SOUND_PORT &= ~(1 << SOUND_OUT);
	
	//Порт световой сигнализации на выход
	LED_DDR |= (1 << LED_OUT);
	LED_PORT &= ~(1 << LED_OUT);
	
	//Порты индикации UPS на выход
	INDICATION_DDR |= (1 << RED_OUT)|(1 << GREEN_OUT)|(1 << YELLOW_OUT);
	INDICATION_PORT &= ~((1 << RED_OUT)|(1 << GREEN_OUT)|(1 << YELLOW_OUT));
}

void Indication::Timer1_PWM_Generator(void)
{
	switch(currentSound)
	{
		case SOUND_UPS:
		if(timerSound == 0)
		{
			OCR0A = 60;
			OCR0B = OCR0A/2;
			SOUND_DDR |= (1 << SOUND_OUT);
			LED_PORT |= (1 << LED_OUT);
			timerSound++;
		}
		else if((timerSound > 0) && (timerSound < 300))
		{
			timerSound++;
		}		
		else if((timerSound >= 300) && (timerSound < 2000))
		{
			SOUND_DDR &= ~(1 << SOUND_OUT);
			SOUND_PORT &= ~(1 << SOUND_OUT);
			LED_PORT &= ~(1 << LED_OUT);
			timerSound++;
		}
		else if(timerSound >= 2000)
		{
			timerSound = 0;
		}
		break;
		
		
		
		case SOUND_TREVOGA:
		if(freq == 0)
		{
			OCR0A = 30; OCR0B = OCR0A/2; freq = 1;
			SOUND_DDR |= (1 << SOUND_OUT);
			LED_PORT |= (1 << LED_OUT);
		}
		else if(freq == 1)
		{
			OCR0A--; OCR0B = OCR0A/2;
			if(OCR2A <= 10)
			{
				freq = 2;
			}
		}
		else if(freq == 2)
		{
			OCR0A++; OCR0B = OCR0A/2;
			if(OCR0A >= 60)
			{
				LED_PORT &= ~(1 << LED_OUT);
			}
			if(OCR0A >= 100)
			{
				freq = 3;
			}
		}
		else
		{
			freq = 0;
		}
		break;
		
		default:
		OCR0A = 0;
	}	
}

void Indication::soundOn(uint8_t signal)
{
	//Выбор звука
	currentSound = signal;
	//Настройка частоты ШИМ
	TCCR0A |= (1 << COM0B1)|(1 << WGM01)|(1 << WGM00);
	TCCR0B |= (1 << WGM02)|(1 << CS02);
	//Настройка ЧИМ
	TCCR1B |= (1 << CS10);
	TIMSK1 |= (1 << TOIE1);	
}

void Indication::soundOff(void)
{
	//Отключение ШИМ
	TCCR0A &= ~((1 << COM0B1)|(1 << WGM01)|(1 << WGM00));
	TCCR0B &= ~((1 << WGM02)|(1 << CS02));
	//Отключение ЧИМ
	TCCR1B &= ~(1 << CS10);
	TIMSK1 &= ~(1 << TOIE1);
	//Сброс значений
	OCR0A = 0;
	OCR0B = 0;
	SOUND_PORT &= ~(1 << SOUND_OUT);
	LED_PORT &= ~(1 << LED_OUT);	
	currentSound = SOUND_NO;
	timerSound = 0;
}

uint8_t Indication::getCurrentSound(void)
{
	return currentSound;
}

void Indication::set_RED(uint8_t status)
{
	switch(status)
	{
		case 1:
		INDICATION_PORT |= (1 << RED_OUT);
		break;
		
		default:
		INDICATION_PORT &= ~(1 << RED_OUT);
	}
}

void Indication::set_GREEN(uint8_t status)
{
	switch(status)
	{
		case 1:
		INDICATION_PORT |= (1 << GREEN_OUT);
		break;
		
		default:
		INDICATION_PORT &= ~(1 << GREEN_OUT);
	}
}

void Indication::set_YELLOW(uint8_t status)
{
	switch(status)
	{
		case 1:
		INDICATION_PORT |= (1 << YELLOW_OUT);
		break;
		
		default:
		INDICATION_PORT &= ~(1 << YELLOW_OUT);
	}
}
