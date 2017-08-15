#include "UPS.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <util/atomic.h>

volatile int m;
volatile int v;
volatile uint8_t UPS::MAIN_DETECT = 0;
volatile uint8_t UPS::V_ADC = 0;

UPS::UPS()
{
	MAIN_DETECT = 0;
}

void UPS::initPorts(void)
{
	KEY_DDR |= (1 << PWR_OUT)|(1 << CHARGE_OUT)|(1 << VCC_SHITD_OUT);
	KEY_PORT &= ~((1 << PWR_OUT)|(1 << CHARGE_OUT)|(1 << VCC_SHITD_OUT));
}

void UPS::initADC_analysis(void)
{
	//АЦП с опорным напряжением AVCC в 8-битном режиме
	//Первоначально настроено на ADC3 - MAIN_DETECT
	ADMUX = (0 << REFS1)|(1 << REFS0)|(1 << ADLAR)|(0 << MUX4)|(0 << MUX3)|(0 << MUX2)|(1 << MUX1)|(1 << MUX0);
	ADCSRB = (0 << 3);
	ADCSRA = (1 << ADEN)|(1 << ADSC)|(1 << ADIE)|(1 << ADPS2)|(1 << ADPS1)|(1 << ADPS0);
}

void UPS::ADC_analysis(void)
{
	if((ADMUX & (1 << MUX0)) == 1)
	{
		MAIN_DETECT = ADCH;
		m = MAIN_DETECT;
		ADMUX &= ~(1 << MUX0);	
	}
	else if(((ADMUX & (1 << MUX0)) == 0))
	{
		V_ADC = ADCH;
		v = V_ADC;		
		ADMUX |= (1 << MUX0);
	}
	ADCSRA |= (1 << ADSC);//запускаем очередное преобразование 
}

uint8_t UPS::get_MAIN_DETECT(void)
{
	return MAIN_DETECT;
}

uint8_t UPS::get_V_ADC(void)
{
	return V_ADC;
}

void UPS::set_VCC_SHITD(uint8_t status)
{
	switch(status)
	{
		case 1:
		KEY_PORT |= (1 << VCC_SHITD_OUT);
		break;
		
		default:
		KEY_PORT &= ~(1 << VCC_SHITD_OUT);
	}
}

void UPS::set_PWR(uint8_t status)
{
	switch(status)
	{
		case 1:
		KEY_PORT |= (1 << PWR_OUT);
		break;
		
		default:
		KEY_PORT &= ~(1 << PWR_OUT);
	}	
}

void UPS::set_CHARGE(uint8_t status)
{
	switch(status)
	{
		case 1:
		KEY_PORT |= (1 << CHARGE_OUT);
		break;
		
		default:
		KEY_PORT &= ~(1 << CHARGE_OUT);
	}	
}






