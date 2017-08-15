/*
 * Block.cpp
 *
 * Created: 07.07.2017 12:03:43
 *  Author: user
 */ 

#define F_CPU 14745600UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>
#include "Constants.h"
#include "Ventilation.h"
#include "Ventilation.cpp"
#include "Indication.h"
#include "Indication.cpp"
#include "UPS.h"
#include "UPS.cpp"

#define NO_UPS_MODE 1
#define YES_UPS_MODE 2 

int main(void)
{
	_delay_ms(1000);
	
//==Проверка наличия UPS=======================================================	
	UPS ups;
	Indication indication;
	ups.initPorts();
	ups.initADC_analysis();
	indication.initPorts();
	
	sei();
	
	_delay_ms(1000);
	
	uint8_t mode = 0;
	uint8_t current_MAIN_DETECT = ups.get_MAIN_DETECT();
	uint8_t current_V_ADC = ups.get_V_ADC();
	uint32_t powerOffTime = 0;
	
	//Аккумуляторы обнаружены
	if(current_V_ADC >= UPSVoltages::BATTERY_START)
	{
		ups.set_VCC_SHITD(1);//Включение аккумуляторов
		_delay_ms(1000);
		ups.set_PWR(1);//Включение нагрузки
		_delay_ms(500);
		indication.set_RED(1);
		_delay_ms(500);
		indication.set_RED(0);
		indication.set_YELLOW(1);
		_delay_ms(500);
		indication.set_YELLOW(0);
		indication.set_GREEN(1);
		_delay_ms(500);
		indication.set_GREEN(0);
		mode = YES_UPS_MODE;
	}
	//Аккумуляторы не обнаружены
	else if(current_V_ADC < UPSVoltages::BATTERY_START)
	{
		_delay_ms(1000);
		ups.set_PWR(1);//Включение нагрузки
		_delay_ms(500);
		indication.set_GREEN(1);
		_delay_ms(500);
		indication.set_GREEN(0);
		mode = NO_UPS_MODE;
	}
	cli();
//===============================================================================	
	
	//Объявление объектов
	Ventilation ventilation;
	
	//Инициализация портов
	ventilation.initPorts();
	
	ventilation.initTimer2_analysis();
	uint8_t currentStatusOfGroup[2] = {StatusOfGroup::STOP, StatusOfGroup::STOP};
	
	sei();
	
	_delay_ms(1000);
	
    while(1)
    {
		switch(mode)
		{
			//Аккумуляторы не обнаружены, работа без UPS, индикации нет
			case NO_UPS_MODE:
			indication.set_GREEN(1);
			//======Работа с вентиляционными кнопками и кнопками тревоги==========================
			for(uint8_t i = 0; i < 2; i++)
			{
				if(currentStatusOfGroup[i] != ventilation.getStatusOfGroup(i+1))
				{
					if(currentStatusOfGroup[i] == StatusOfGroup::TREVOGA)
					{
						indication.soundOff();
					}
					currentStatusOfGroup[i] = ventilation.getStatusOfGroup(i+1);
					switch(currentStatusOfGroup[i])
					{
						case StatusOfGroup::OPEN:
						ventilation.setGroup(i+1, StatusOfGroup::OPEN);
						break;
						
						case StatusOfGroup::CLOSE:
						ventilation.setGroup(i+1, StatusOfGroup::CLOSE);
						break;
						
						case StatusOfGroup::STOP:
						ventilation.setGroup(i+1, StatusOfGroup::STOP);
						break;
						
						case StatusOfGroup::TREVOGA:
						if(i == 0)
						{
							ventilation.setGroup(12, StatusOfGroup::TREVOGA);
							indication.soundOn(SOUND_TREVOGA);
						}
						break;
						
						default:
						ventilation.setGroup(i+1, StatusOfGroup::STOP);
					}
				}
			}
			break;
			//========================================================================================
			
			
			
			//Аккумуляторы обнаружены, работа с UPS, индикация есть
			case YES_UPS_MODE:
			
			//==Работа от сети: работа с вентиляционными кнопками и кнопками тревоги (полные функции), подзарядка аккумуляторов============
			current_MAIN_DETECT = ups.get_MAIN_DETECT();
			if((current_MAIN_DETECT > UPSVoltages::CIRCUIT_LOW) && (current_MAIN_DETECT < UPSVoltages::CIRCUIT_HIGH))
			{
				indication.set_GREEN(1);
				indication.set_YELLOW(0);
				//======Работа с вентиляционными кнопками и кнопками тревоги (полные функции)==========================
				for(uint8_t i = 0; i < 2; i++)
				{
					if(currentStatusOfGroup[i] != ventilation.getStatusOfGroup(i+1))
					{
						if(currentStatusOfGroup[i] == StatusOfGroup::TREVOGA)
						{
							indication.soundOff();
						}
						currentStatusOfGroup[i] = ventilation.getStatusOfGroup(i+1);
						switch(currentStatusOfGroup[i])
						{
							case StatusOfGroup::OPEN:
							ventilation.setGroup(i+1, StatusOfGroup::OPEN);
							break;
							
							case StatusOfGroup::CLOSE:
							ventilation.setGroup(i+1, StatusOfGroup::CLOSE);
							break;
							
							case StatusOfGroup::STOP:
							ventilation.setGroup(i+1, StatusOfGroup::STOP);
							break;
							
							case StatusOfGroup::TREVOGA:
							if(i == 0)
							{
								ventilation.setGroup(12, StatusOfGroup::TREVOGA);
								indication.soundOn(SOUND_TREVOGA);
							}
							break;
							
							default:
							ventilation.setGroup(i+1, StatusOfGroup::STOP);
						}
					}
				}
				
				//==Подзарядка аккумуляторов===============================================
					current_V_ADC = ups.get_V_ADC();
					if((current_V_ADC > UPSVoltages::BATTERY_CHARGED) && (current_V_ADC < UPSVoltages::BATTERY_HIGH))
					{
						indication.set_RED(0);
						ups.set_CHARGE(0);
					}
					else if(current_V_ADC <= UPSVoltages::BATTERY_LOW)
					{
						indication.set_RED(1);
						ups.set_CHARGE(1);
					}
					if(current_V_ADC > UPSVoltages::BATTERY_LOW)
					{
						indication.set_RED(0);
					}
					if((indication.getCurrentSound() != SOUND_TREVOGA) && (indication.getCurrentSound() == SOUND_UPS))
					{
						indication.soundOff();
					}
			}
			//====================================================================================
			
			
			
			//==Работа от аккумуляторов: работа с вентиляционными кнопками и кнопками тревоги (ограниченные функции), отключение при глубоком разряде========
			else if(current_MAIN_DETECT <= UPSVoltages::CIRCUIT_LOW)
			{
				indication.set_GREEN(0);
				indication.set_YELLOW(1);
				ups.set_CHARGE(0);
				//======Работа с вентиляционными кнопками и кнопками тревоги (ограниченные функции)==========================
				for(uint8_t i = 0; i < 2; i++)
				{
					if(currentStatusOfGroup[i] != ventilation.getStatusOfGroup(i+1))
					{
						if(currentStatusOfGroup[i] == StatusOfGroup::TREVOGA)
						{
							indication.soundOff();
						}
						currentStatusOfGroup[i] = ventilation.getStatusOfGroup(i+1);
						switch(currentStatusOfGroup[i])
						{
							case StatusOfGroup::CLOSE:
							ventilation.setGroup(i+1, StatusOfGroup::CLOSE);
							break;
							
							case StatusOfGroup::STOP:
							ventilation.setGroup(i+1, StatusOfGroup::STOP);
							break;
							
							case StatusOfGroup::TREVOGA:
							if(i == 0)
							{
								ventilation.setGroup(12, StatusOfGroup::TREVOGA);
								indication.soundOn(SOUND_TREVOGA);
							}
							break;
							
							default:
							ventilation.setGroup(i+1, StatusOfGroup::STOP);
						}
					}
				}
				
				//==Отключение при глубоком разряде==================================================
					current_V_ADC = ups.get_V_ADC();
					if(current_V_ADC <= UPSVoltages::BATTERY_LOW)
					{
						indication.set_RED(1);
						if((indication.getCurrentSound() != SOUND_TREVOGA) && (indication.getCurrentSound() != SOUND_UPS))
						{
							indication.soundOn(SOUND_UPS);
						}
					}
					else if(current_V_ADC > UPSVoltages::BATTERY_LOW)
					{
						indication.set_RED(0);
						if((indication.getCurrentSound() != SOUND_TREVOGA) && (indication.getCurrentSound() == SOUND_UPS))
						{
							indication.soundOff();
						}
					}
					
					if(current_V_ADC <= UPSVoltages::BATTERY_VERY_LOW)
					{
						powerOffTime++;
						if(powerOffTime > 100000)
						{
							ups.set_PWR(0);
							ups.set_VCC_SHITD(0);
							indication.set_YELLOW(0);
							indication.set_RED(0);
							indication.soundOff();
							_delay_ms(10000);
						}
					}
					else if(current_V_ADC > UPSVoltages::BATTERY_VERY_LOW)
					{
						powerOffTime = 0;
					}
			}
			break;
			
			default:
			
			break;
		}
		
    }
}

ISR(TIMER1_OVF_vect, ISR_NOBLOCK)
{
	Indication::Timer1_PWM_Generator();
}

ISR(TIMER2_OVF_vect, ISR_NOBLOCK)
{
	Ventilation::Timer2_analysis();
}

ISR(ADC_vect)
{
	UPS::ADC_analysis();
}
