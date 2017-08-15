#ifndef UPS_H
#define UPS_H

#define KEY_DDR DDRA
#define KEY_PORT PORTA
#define PWR_OUT 7
#define CHARGE_OUT 6
#define VCC_SHITD_OUT 5

#define MAIN_DETECT_IN 3
#define V_ADC_IN 2

class UPS
{	
	private:
		static volatile uint8_t MAIN_DETECT;
		static volatile uint8_t V_ADC;
		uint8_t VCC_SHITD;
		uint8_t PWR;
		uint8_t CHARGE;
		
	public:
		UPS();
		void initPorts(void);
		void initADC_analysis(void);
		static void ADC_analysis(void);
		uint8_t get_MAIN_DETECT(void);
		uint8_t get_V_ADC(void);
		void set_VCC_SHITD(uint8_t status);
		void set_PWR(uint8_t status);
		void set_CHARGE(uint8_t status);
};

#endif