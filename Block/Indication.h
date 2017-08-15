#ifndef Indication_H
#define Indication_H

//Звуковая сигнализация
#define SOUND_DDR DDRG
#define SOUND_PORT PORTG
#define SOUND_OUT 5

//Звуки
#define SOUND_NO 0
#define SOUND_UPS 1
#define SOUND_TREVOGA 2

//Световая сигнализация
#define LED_DDR DDRE
#define LED_PORT PORTE
#define LED_OUT 7

//Индикация UPS
#define INDICATION_DDR DDRB
#define INDICATION_PORT PORTB
#define RED_OUT 0
#define GREEN_OUT 1
#define YELLOW_OUT 2

class Indication
{
	private:
		static uint8_t currentSound;
		static uint8_t freq;
		static uint16_t timerSound;
	
	public:
		Indication();
		void initPorts(void);
		void soundOn(uint8_t signal);
		void soundOff(void);
		static void Timer1_PWM_Generator(void);
		static uint8_t getCurrentSound(void);
		void set_RED(uint8_t status);
		void set_GREEN(uint8_t status);
		void set_YELLOW(uint8_t status);
};

#endif