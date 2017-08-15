#include "Ventilation.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <util/atomic.h>

volatile uint8_t Ventilation::statusGroup1 = StatusOfGroup::STOP;
volatile uint8_t Ventilation::statusGroup2 = StatusOfGroup::STOP;
volatile bool Ventilation::was = false;
volatile uint16_t Ventilation::timerOpen1 = 0;
volatile uint16_t Ventilation::timerClose1 = 0;
volatile uint16_t Ventilation::timerOpen2 = 0;
volatile uint16_t Ventilation::timerClose2 = 0;
volatile uint16_t Ventilation::timerTrevoga = 0;

Ventilation::Ventilation()
{
	statusGroup1 = false;
	statusGroup2 = false;
	was = false;
}

void Ventilation::initPorts()
{	
	//порты анализа кнопок вентиляции на вход
	VENT_DDR &= ~((1 << OPEN_1)|(1 << CLOSE_1)|(1 << OPEN_2)|(1 << CLOSE_2));
	
	//подтяжка портов анализа тревожных кнопок на +5V
	TREV_PORT |= (1 << TREVOGA_1);
	
	//порты анализа тревожных кнопок на вход
	TREV_DDR &= ~(1 << TREVOGA_1);
	
	//порт сигнала тревоги на выход
	TREVOGA_OUT_DDR |= (1 << TREVOGA_SIGNAL);
	TREVOGA_OUT_PORT |= (1 << TREVOGA_SIGNAL);
	
	//порты управления транзисторами на выход
	TRANS_DDR |= (1 << TRANS_1_OPEN)|(1 << TRANS_1_CLOSE)|(1 << TRANS_2_OPEN)|(1 << TRANS_2_CLOSE);
	TRANS_PORT &= ~((1 << TRANS_1_OPEN)|(1 << TRANS_1_CLOSE)|(1 << TRANS_2_OPEN)|(1 << TRANS_2_CLOSE));
}

void Ventilation::initTimer2_analysis()
{
	//частота вызова f = 14 745 600 / 1024 / 256 = 56.25Гц
	TCCR2B |= (1 << CS22)|(1 << CS21)|(1 << CS20); //предделитель 1024
	TIMSK2 |= (1 << TOIE2); //разрешить прерывание по переполнению
	
}

void Ventilation::Timer2_analysis()
{
	//Анализ вентиляционных кнопок
	if(((TREV_PIN & (1 << TREVOGA_1)) == (0 << TREVOGA_1)))
	{
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
		{
			timerTrevoga = 0;
			if(was == true)
			{
				TREVOGA_OUT_PORT |= (1 << TREVOGA_SIGNAL);
				TRANS_PORT &= ~(1 << TRANS_1_CLOSE);
				TRANS_PORT &= ~(1 << TRANS_1_OPEN);
				TRANS_PORT &= ~(1 << TRANS_2_CLOSE);
				TRANS_PORT &= ~(1 << TRANS_2_OPEN);			
				statusGroup1 = StatusOfGroup::STOP;
				statusGroup2 = StatusOfGroup::STOP;
				was = false;
			}
		}
		
		//Анализ вентиляционных кнопок группы 1
		if(((VENT_PIN & (1 << OPEN_1)) == (1 << OPEN_1)) && ((VENT_PIN & (1 << CLOSE_1)) == (0 << CLOSE_1)))
		{
			timerOpen1++;
			if(timerOpen1 >= 30)
			{
				statusGroup1 = StatusOfGroup::OPEN;
				timerOpen1 = 0;
			}
		}
	
		else if(((VENT_PIN & (1 << OPEN_1)) == (0 << OPEN_1)) && ((VENT_PIN & (1 << CLOSE_1)) == (1 << CLOSE_1)))
		{
			timerClose1++;
			if(timerClose1 >= 30)
			{
				statusGroup1 = StatusOfGroup::CLOSE;
				timerClose1 = 0;	
			}	
		}
	
		else if(((VENT_PIN & (1 << OPEN_1)) == (0 << OPEN_1)) && ((VENT_PIN & (1 << CLOSE_1)) == (0 << CLOSE_1)))
		{
			timerOpen1 = 0;
			timerClose1 = 0;
			statusGroup1 = StatusOfGroup::STOP;
		}
	
		else if(((VENT_PIN & (1 << OPEN_1)) == (1 << OPEN_1)) && ((VENT_PIN & (1 << CLOSE_1)) == (1 << CLOSE_1)))
		{
			timerOpen1 = 0;
			timerClose1 = 0;
			statusGroup1 = StatusOfGroup::STOP;
		}	
	
		//Анализ вентиляционных кнопок группы 2
		if(((VENT_PIN & (1 << OPEN_2)) == (1 << OPEN_2)) && ((VENT_PIN & (1 << CLOSE_2)) == (0 << CLOSE_2)))
		{
			timerOpen2++;
			if(timerOpen2 >= 30)
			{
				statusGroup2 = StatusOfGroup::OPEN;
				timerOpen2 = 0;
			}
		}
	
		else if(((VENT_PIN & (1 << OPEN_2)) == (0 << OPEN_2)) && ((VENT_PIN & (1 << CLOSE_2)) == (1 << CLOSE_2)))
		{
			timerClose2++;
			if(timerClose2 >= 30)
			{
				statusGroup2 = StatusOfGroup::CLOSE;
				timerClose2 = 0;
			}
		}
	
		else if(((VENT_PIN & (1 << OPEN_2)) == (0 << OPEN_2)) && ((VENT_PIN & (1 << CLOSE_2)) == (0 << CLOSE_2)))
		{
			timerOpen2 = 0;
			timerClose2 = 0;
			statusGroup2 = StatusOfGroup::STOP;
		}
	
		else if(((VENT_PIN & (1 << OPEN_2)) == (1 << OPEN_2)) && ((VENT_PIN & (1 << CLOSE_2)) == (1 << CLOSE_2)))
		{
			timerOpen2 = 0;
			timerClose2 = 0;
			statusGroup2 = StatusOfGroup::STOP;
		}	
	}
	
	//Анализ тревожных кнопок
	else if((TREV_PIN & (1 << TREVOGA_1)) == (1 << TREVOGA_1))
	{
		was = true;
		timerTrevoga++;
		if(timerTrevoga >= 60)
		{
			statusGroup1 = StatusOfGroup::TREVOGA;
			statusGroup2 = StatusOfGroup::TREVOGA;
			timerOpen1 = 0;
			timerClose1 = 0;
			timerOpen2 = 0;
			timerClose2 = 0;
			timerTrevoga = 0;
		}
	}
	
	else
	{
		statusGroup1 = StatusOfGroup::STOP;
		statusGroup2 = StatusOfGroup::STOP;
	}
}

uint8_t Ventilation::getStatusOfGroup(uint8_t numberOfGroup)
{
	switch(numberOfGroup)
	{
		case 1:
		return statusGroup1;
		break;
		
		case 2:
		return statusGroup2;
		break;
		
		default:
		return 0;
	}
}

void Ventilation::setGroup(uint8_t numberOfGroup, uint8_t status)
{
	switch(numberOfGroup)
	{
		case 1:
		if(status == StatusOfGroup::OPEN)
		{
			TRANS_PORT &= ~(1 << TRANS_1_CLOSE);
			TRANS_PORT &= ~(1 << TRANS_1_OPEN);
			_delay_ms(500);
			TRANS_PORT &= ~(1 << TRANS_1_CLOSE);
			TRANS_PORT |= (1 << TRANS_1_OPEN);	
		}
		else if(status == StatusOfGroup::CLOSE)
		{
			TRANS_PORT &= ~(1 << TRANS_1_CLOSE);
			TRANS_PORT &= ~(1 << TRANS_1_OPEN);
			_delay_ms(500);
			TRANS_PORT &= ~(1 << TRANS_1_OPEN);
			TRANS_PORT |= (1 << TRANS_1_CLOSE);			
		}
		else
		{
			TRANS_PORT &= ~(1 << TRANS_1_CLOSE);
			TRANS_PORT &= ~(1 << TRANS_1_OPEN);			
		}
		break;
		
		case 2:
		if(status == StatusOfGroup::OPEN)
		{
			TRANS_PORT &= ~(1 << TRANS_2_CLOSE);
			TRANS_PORT &= ~(1 << TRANS_2_OPEN);
			_delay_ms(500);
			TRANS_PORT &= ~(1 << TRANS_2_CLOSE);
			TRANS_PORT |= (1 << TRANS_2_OPEN);
		}
		else if(status == StatusOfGroup::CLOSE)
		{
			TRANS_PORT &= ~(1 << TRANS_2_CLOSE);
			TRANS_PORT &= ~(1 << TRANS_2_OPEN);
			_delay_ms(500);
			TRANS_PORT &= ~(1 << TRANS_2_OPEN);
			TRANS_PORT |= (1 << TRANS_2_CLOSE);
		}
		else
		{
			TRANS_PORT &= ~(1 << TRANS_2_CLOSE);
			TRANS_PORT &= ~(1 << TRANS_2_OPEN);	
		}
		break;
		
		case 12:
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
		{
			if(status == StatusOfGroup::TREVOGA)
			{
				TREVOGA_OUT_PORT |= (1 << TREVOGA_SIGNAL);	
				TRANS_PORT &= ~(1 << TRANS_1_CLOSE);
				TRANS_PORT &= ~(1 << TRANS_1_OPEN);			
				TRANS_PORT &= ~(1 << TRANS_2_CLOSE);
				TRANS_PORT &= ~(1 << TRANS_2_OPEN);	
				_delay_ms(1000);
				TREVOGA_OUT_PORT &= ~(1 << TREVOGA_SIGNAL);	
				TRANS_PORT &= ~(1 << TRANS_1_CLOSE);
				TRANS_PORT |= (1 << TRANS_1_OPEN);	
				TRANS_PORT &= ~(1 << TRANS_2_CLOSE);
				TRANS_PORT |= (1 << TRANS_2_OPEN);						
			}
		}
		break;
		
		default:
		TRANS_PORT &= ~(1 << TRANS_1_CLOSE);
		TRANS_PORT &= ~(1 << TRANS_1_OPEN);		
		TRANS_PORT &= ~(1 << TRANS_2_CLOSE);
		TRANS_PORT &= ~(1 << TRANS_2_OPEN);			
	}
}