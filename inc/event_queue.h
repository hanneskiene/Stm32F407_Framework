/*
 * event_queue.h
 *
 *  Created on: 21.01.2018
 *      Author: Hannes
 */

#ifndef EVENT_QUEUE_H_
#define EVENT_QUEUE_H_

#error Use EventQueuePrio

#include "mcu/stm32f407xx.h"
#include <array>
#include <functional>
#include <chrono>
#include "interrupt_manager.h"

#include "mcu/pin_device.h"

template<class TimerManager, class Func_t>
class EventQueue {
	enum{ size = 200 };
public:

	using time_t = std::chrono::milliseconds;

	EventQueue(TimerManager& tm) :
			tm_(tm), head_(0), tail_(0) {
	}
	void run() {
		using Pin_t = mcu::Pin;
		Pin_t led(Pin_t::portE, 8, Pin_t::output);
		while (true) {
			//Handle posted functions until empty
			while (tail_ != head_) {
				//Call Function
				functions_.at(tail_)();
				tail_++;
				if(tail_ >= size){
					tail_ = 0;
				}
			}
			//Sleep
			led.set();
			__WFI();
			led.clr();
		}
	}

	template<class Callable>
	void post(Callable&& c) {
		IntLock l; //Disables interrupts for the scope of the function
		functions_.at(head_).assignHandler(std::forward<Callable>(c)); // Store Function Call
		head_++;
		if(head_ >= size){
			head_ = 0;
		}
	}

	template<class Callable>
	void post_in_ms(Callable&& c, time_t ms) {
		//Call through Timer:
		IntLock l;
		tm_.run_in_ms([this, c]() {
			post(c);
		}, ms.count());
	}

	template<class Callable>
		void rec_post_in_ms(Callable&& c, time_t ms) {
			//Call through Timer:
		IntLock l;
			tm_.rec_run_in_ms([this, c]() {
				post(c);
			}, ms.count());
		}
private:
//Tasks to run
	std::array<Func_t, size> functions_;
	TimerManager& tm_;
//Current Task Count
	volatile uint32_t head_;
	volatile uint32_t tail_;

};

#endif /* EVENT_QUEUE_H_ */
