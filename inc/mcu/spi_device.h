/*
 * spi_device.h
 *
 *  Created on: 21.01.2018
 *      Author: Hannes
 */

#ifndef MCU_SPI_DEVICE_H_
#define MCU_SPI_DEVICE_H_

#include "stdint.h"
#include "stm32f407xx.h"

namespace mcu {
template<class EventQueue_t, class Dma_t, class Pin_t, class Func_t>
class SpiDevice {
public:
	//
	// Public Constant Data
	//
	enum Instance_t {
		spi1 = SPI1_BASE, spi2 = SPI2_BASE
	};

	enum Speed_t {
		fast = 0x0, medium = 0x4, slow = 0x7
	};

	enum MorS_t {
		slave = 0, master = 1
	};

	enum Mode_t {
		mode_polling = 0, mode_interrupt = 1, mode_dma = 2
	};

	using spi_t = volatile SPI_TypeDef* const; //Type that points to DMA Memory Structure
	using addr_t = uint32_t;
	using size_t = uint32_t;
	//
	// Public Functions
	//
	//Constructor: Device must be initialized with proper Values
	explicit SpiDevice(const EventQueue_t& queue, Dma_t& dma,
			const Instance_t inst, const Speed_t speed, const Pin_t mo, const Pin_t mi, const MorS_t m_ors =
					master, const Mode_t dat_mode = mode_polling) :
			eQueue_(queue), dma_(dma), spi_(reinterpret_cast<spi_t>(inst)), mosi_(mo), miso_(mi), mode_(
					dat_mode), busy_(false) {
		enableClock();
		//Configure SPI
		spi_->CR1 |= ((speed & 0x7) << SPI_CR1_BR_Pos);
		spi_->CR1 |= SPI_CR1_SSI; //Slave Select Internal
		spi_->CR1 |= SPI_CR1_SSM; //Slave Select
		if (m_ors == master) {
			spi_->CR1 |= SPI_CR1_MSTR; //Mode Master
		}
		if (mode_ == mode_dma) { //Enable DMA Requests
			spi_->CR2 |= SPI_CR2_TXDMAEN;
			spi_->CR2 |= SPI_CR2_RXDMAEN;

			dma_.setPeripheralAddress((addr_t)&spi_->DR);
			dma_.setTransferCompleteCallback( //Setup the dma to call tfc handler
			[this](){onTransferComplete();}
			);
		}

		enable(); //Leave out if Spi should not be enabled by default
	}

	//Enables the SPI
	inline void enable() const{
		spi_->CR1 |= SPI_CR1_SPE; //Enable
	}
	//Disables the SPI
	inline void disable() const{
		while ((SPI1->SR & SPI_SR_BSY) != 0)
			; //Wait till not busy, blocks!!!
		spi_->CR1 &= ~SPI_CR1_SPE; //Disable
	}

	inline bool getBusy() const{
		return busy_;
	}

	//Sends single byte
	inline void send(uint8_t byte) {
		while(busy_);
		enable();
		spi_->DR = byte; // Write byte to send
		while (!(spi_->SR & SPI_SR_TXE))
			; //While Tx Not Empty
	}

	//Sends buffer based on internal mode
	void send(addr_t buffer, size_t size);

	//Sets the Transfer Complete Callback Function
	//!!Needs to be determined whether to call it in Interrupt Ctx or post it to eQueue
	template<class Callable>
	inline void setTransferCompleteCallback(Callable c) {
		while(busy_);
		callback_tfc_.assignHandler(std::forward<Callable>(c));
	}

	void onTransferComplete() {
		while ((spi_->SR & SPI_SR_BSY) != 0)
					; //Wait till not busy, blocks!!!
		callback_tfc_();
		busy_ = false;
	}

private:
	//
	//Private Member
	//
	const EventQueue_t& eQueue_; //Ref to EventQueue
	Dma_t& dma_;

	spi_t spi_; //Pointer to SPI in Hardware

	const Pin_t mosi_;
	const Pin_t miso_;

	Mode_t mode_;

	Func_t callback_tfc_; //Callback on Transfer Complete
	volatile bool busy_;

	//
	//Private Helper Functions
	//

	//Enables the DMA Clock
	inline void enableClock() const {
		//SPI Clock
		switch ((addr_t) spi_) {
		case spi1:
			RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
			break;
		case spi2:
			RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
			break;
		}
	}

	//Enables Interrupt
	inline void enableInterrupt() const {
		switch ((addr_t) spi_) {
		case spi1:
			NVIC_EnableIRQ(SPI1_IRQn);
			break;
		case spi2:
			NVIC_EnableIRQ(SPI2_IRQn);
			break;
		}
	}

	//Clears the Interrupt Flags
	inline void clearFlags() {
		switch ((addr_t) spi_) {
		case spi1:
			while(true);//implement if needed
			break;
		case spi2:
			while(true);//implement if needed
			break;
		}
	}

};

template<class EventQueue_t, class Dma_t, class Pin_t, class Func_t>
void SpiDevice<EventQueue_t, Dma_t, Pin_t, Func_t>::
send(addr_t buffer, size_t size){
		while(busy_);
		enable();
		busy_ = true;
		switch (mode_) {
		case mode_polling: {
			auto* ptr = (volatile uint8_t*)buffer;
			for (unsigned int i = 0; i < size; i++) {
				spi_->DR = *ptr; // Write byte to send
				ptr++;
				while (!(spi_->SR & SPI_SR_TXE))
					; //While Tx Not Empty
			}
			while ((spi_->SR & SPI_SR_BSY) != 0)
								; //Wait till not busy, blocks!!!
			callback_tfc_();
			busy_ = false;
		}
			break;
		case mode_interrupt:
			break;
		case mode_dma:
			dma_.start((addr_t) buffer, size);
			break;
		}
	}

}	// !Namespace MCU
#endif /* MCU_SPI_DEVICE_H_ */
