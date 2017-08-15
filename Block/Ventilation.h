#ifndef Ventilation_H
#define Ventilation_H

//Кнопки вентиляции
#define VENT_DDR DDRA
#define VENT_PIN PINA
#define OPEN_1 3
#define CLOSE_1 4
#define OPEN_2 1
#define CLOSE_2 2

//Управление транзисторами
#define TRANS_DDR DDRC
#define TRANS_PORT PORTC
#define TRANS_1_OPEN 0
#define TRANS_1_CLOSE 1
#define TRANS_2_OPEN 2
#define TRANS_2_CLOSE 3

//Кнопка тревоги
#define TREV_DDR DDRE
#define TREV_PIN PINE
#define TREV_PORT PORTE
#define TREVOGA_1 5

//Сигнал тревоги
#define TREVOGA_OUT_DDR DDRB
#define TREVOGA_OUT_PORT PORTB
#define TREVOGA_SIGNAL 6

class Ventilation
{
	private:
		static volatile uint8_t statusGroup1;
		static volatile uint8_t statusGroup2;
		static volatile bool was;
		static volatile uint16_t timerOpen1;
		static volatile uint16_t timerClose1;
		static volatile uint16_t timerOpen2;
		static volatile uint16_t timerClose2;
		static volatile uint16_t timerTrevoga;
		
	public:
		Ventilation();
		void initPorts(void);
		void initTimer2_analysis(void);
		static void Timer2_analysis(void);
		static uint8_t getStatusOfGroup(uint8_t numberOfGroup);
		void setGroup(uint8_t numberOfGroup, uint8_t status);
};

#endif