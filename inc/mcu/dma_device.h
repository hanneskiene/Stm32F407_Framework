/*
 * dma_device.h
 *
 *  Created on: 21.01.2018
 *      Author: Hannes
 */

#ifndef MCU_DMA_DEVICE_H_
#define MCU_DMA_DEVICE_H_

#include "stdint.h"
#include "stm32f407xx.h"
#include "../util/staticfunction.h"
#include "../interrupt_manager.h"
#include <functional>
/*
 ADD INTERRUPT SUPPORT or clear flags manually; OTHERWISE DMA WILL ONLY RUN ONCE!
 */
namespace mcu {

template<class EventQueue_t, class Func_t>
class DmaDevice {
public:
	//
	// Public Constant Data
	//
	enum Instance {
		dma1 = DMA1_BASE, dma2 = DMA2_BASE
	};
	enum Stream {
		stream0 = 0,
		stream1 = 1,
		stream2 = 2,
		stream3 = 3,
		stream4 = 4,
		stream5 = 5,
		stream6 = 6,
		stream7 = 7,
		stream8 = 8
	};
	enum Channel {
		channel0 = 0,
		channel1 = 1,
		channel2 = 2,
		channel3 = 3,
		channel4 = 4,
		channel5 = 5,
		channel6 = 6,
		channel7 = 7,
		channel8 = 8
	};

	using dma_t = volatile DMA_TypeDef* const; //Type that points to DMA Memory Structure
	using str_t = volatile DMA_Stream_TypeDef* const; //Type that points to DMA Stream Memory Structure
	using addr_t = uint32_t;
	using size_t = uint32_t;

	//
	// Public Functions
	//
	//Constructor: Device must be initialized with proper Values
	explicit DmaDevice(const EventQueue_t& queue, const Instance inst,
			const Stream stream_nr, const Channel channel_nr,
			const addr_t peripheral_address = 0x0, const bool memoryIncrement =
					true) :
			eQueue_(queue), dma_(reinterpret_cast<dma_t>(inst)), stream_(
					reinterpret_cast<str_t>(inst + 0x10 + (0x18 * stream_nr))), stream_nr(
					stream_nr), channel_nr_(channel_nr)
	{
		enableClock();
		stream_->PAR = peripheral_address;
		stream_->CR |= (channel_nr << DMA_SxCR_CHSEL_Pos) //Channel Select
		| (0x01 << DMA_SxCR_DIR_Pos) //Transfer Direction
				| DMA_SxCR_TCIE; //Transfer Complete Interrupt
		if (memoryIncrement) {
			stream_->CR |= DMA_SxCR_MINC; //Enable Memory Increment
		}
		enableInterrupt();
	}

	void onInterrupt() {
		clearFlags();
		callback_tfc_();
	}

	//Sets the Transfer Complete Callback Function
	//!!Needs to be determined whether to call it in Interrupt Ctx or post it to eQueue
	template<class Callable>
	inline void setTransferCompleteCallback(Callable&& c) {
		callback_tfc_.assignHandler(std::forward<Callable>(c));
	}

	//Sets the Buffer Adress and Counter without starting the DMA
	inline void setBuffer(addr_t buffer, size_t count) const {
		stream_->CR &= ~(DMA_SxCR_EN); //Disable
		stream_->M0AR = buffer; //Source Address
		stream_->NDTR = count; //Amount of data to be transfered
	}

	inline void setPeripheralAddress(const addr_t per) const {
		stream_->PAR = per;
	}

	//Starts the Dma with new Buffer Address and Element Count
	inline void start(const addr_t buffer, const size_t count) const {
		stream_->CR &= ~(DMA_SxCR_EN); //Disable
		stream_->M0AR = buffer; //Source Address
		stream_->NDTR = count; //Amount of data to be transfered

		stream_->CR |= DMA_SxCR_EN;
	}

	//Starts the DMA with the Buffer and Count of the last Operation
	inline void start() const {
		stream_->CR |= DMA_SxCR_EN;
	}

private:
	//
	//Private Member
	//
	const EventQueue_t& eQueue_; //Ref to EventQueue
	dma_t dma_; //Pointer to DMA in Hardware
	str_t stream_; //Pointer to DMA Stream in Hardware
	const int stream_nr; //DMA Stream Nr
	const int channel_nr_; // DMA Stream Channel Nr

	Func_t callback_tfc_; //Callback on Transfer Complete
	//
	//Private Constant Data
	//
	const unsigned int DMA1_interrupt_nr[7] = { 11, 12, 13, 14, 15, 16, 17 }; //IRQ Numbers for DMA1
	const unsigned int DMA2_interrupt_nr[8] = { 56, 57, 58, 59, 60, 68, 69, 70 }; //IRQ Numbers for DMA2
	//
	//Private Helper Functions
	//

	//Enables the DMA Clock
	inline void enableClock() const {
		//Dma Clock
		switch ((addr_t) dma_) {
		case DMA1_BASE:
			RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
			break;
		case DMA2_BASE:
			RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;
			break;
		}
	}

	//Enables Interrupt for the DMA Stream
	inline void enableInterrupt() {
		switch ((addr_t) dma_) {
		case DMA1_BASE:
			NVIC_EnableIRQ((IRQn_Type) DMA1_interrupt_nr[stream_nr]);
			InterruptDispatcher::getInstance().register_callback(
								[this]() {onInterrupt();},
								(IRQn_Type) DMA1_interrupt_nr[stream_nr]);

			break;
		case DMA2_BASE:
			NVIC_EnableIRQ((IRQn_Type) DMA2_interrupt_nr[stream_nr]);
			InterruptDispatcher::getInstance().register_callback(
					[this]() {onInterrupt();},
					(IRQn_Type) DMA2_interrupt_nr[stream_nr]);
			break;
		}

	}

	//Clears the Interrupt Flags, example for Stream3 only!!!
	inline void clearFlags() {
		DMA2->LIFCR = DMA_LIFCR_CFEIF3 | DMA_LIFCR_CDMEIF3 | DMA_LIFCR_CHTIF3
				| DMA_LIFCR_CTCIF3 | DMA_LIFCR_CTEIF3;
	}

};
}

#endif /* MCU_DMA_DEVICE_H_ */
