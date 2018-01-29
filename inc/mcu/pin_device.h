/*
 * pin_device.h
 *
 *  Created on: 24.01.2018
 *      Author: Hannes
 */

#ifndef MCU_PIN_DEVICE_H_
#define MCU_PIN_DEVICE_H_

#include "stm32f407xx.h"

namespace mcu {

class Pin {
public:
	//
	// Public Constant Data
	//
	enum Port {
		portA = GPIOA_BASE,
		portB = GPIOB_BASE,
		portC = GPIOC_BASE,
		portD = GPIOD_BASE,
		portE = GPIOE_BASE,
		portF = GPIOF_BASE
	};

	enum Direction {
		input = 0, output = 1, alternate = 2, analog = 3
	};

	enum Afsel {
		gpio = 0,
		tim_1_2 = 1,
		tim_3_4_5 = 2,
		tim_8_9_10_11 = 3,
		i2c_1_2 = 4,
		spi_1_2 = 5,
		spi_3 = 6,
		uart_1_2_3 = 7,
		uart_4_5_6 = 8,
		can_tim_12_13_14 = 9,
		usb = 10,
		eth = 11,
		fsmc_sdio_usbotg = 12,
		dcmi = 13,
		event = 15
	};

	enum Pupd {
		none = 0, up = 1, down = 2, updown = 3
	};

	using port_t = volatile GPIO_TypeDef* const; //Points to Hardware
	using addr_t = uint32_t;
	using size_t = uint32_t;

	//
	// Public Functions
	//
	//Constructor: Device must be initialized with proper Values
	explicit Pin(const Port p, const uint32_t number, const Direction dir =
			input, const Afsel af = gpio, const Pupd pull = none) :
			port_(reinterpret_cast<port_t>(p)), nr_(number) {
		enableClock(); //Has to be first!
		port_->MODER |= (dir << (nr_ * 2)); //Set Pin Mode
		setAlternateFunction(af);
		port_->PUPDR |= (pull << (nr_ * 2)); //Set Pull Up/Down
	}

	inline void set() const{
		port_->BSRR = (0x1 << nr_);
	}

	inline void clr() const{
		port_->BSRR = (0x1 << nr_ << 16);
	}

	inline void tgl() const{
		if( port_->ODR & (0x1 << nr_))
			clr();
		else
			set();
	}

	inline bool get(){
		return ((port_->IDR >> nr_) & 0x1);
	}

private:
	port_t port_;
	const uint32_t nr_;

	//Needs to be called first!
	inline void enableClock() const{
		unsigned int p = 0; //Find Port Index
		switch ((addr_t) port_) {
		case portA:
			p = 0;
			break;
		case portB:
			p = 1;
			break;
		case portC:
			p = 2;
			break;
		case portD:
			p = 3;
			break;
		case portE:
			p = 4;
			break;
		case portF:
			p = 5;
			break;
		}
		RCC->AHB1ENR |= 0x1 << p; // Enable Correct Clock
	}

	inline void setAlternateFunction(const Afsel af) const{
		//Set AFSEL
		if (nr_ < 8)
			port_->AFR[0] |= (af << (nr_ * 4));
		else
			port_->AFR[1] |= (af << (nr_ * 4));
	}
}
;

} //!MCU
#endif /* MCU_PIN_DEVICE_H_ */
