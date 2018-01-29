/*
 * event_queue_prio.h
 *
 *  Created on: 27.01.2018
 *      Author: Hannes
 */

#ifndef EVENT_QUEUE_PRIO_H_
#define EVENT_QUEUE_PRIO_H_

#include "mcu/stm32f407xx.h"
#include <array>
#include <functional>
#include <chrono>
#include "interrupt_manager.h"
#include "mcu/pin_device.h"

//
//Add Pin_t as Template, pass Pin as Signal Device for Sleep mode
//

using namespace std::chrono_literals;

template<class TimerManager, class Func_t>
class EventQueuePrio {
	enum{ size = 200 };
public:

	using time_t = std::chrono::milliseconds;

	EventQueuePrio(TimerManager& tm) :
			tm_(tm), head_(0), tail_(0) {
	}

	void run() {
		using Pin_t = mcu::Pin;
		Pin_t led(Pin_t::portE, 8, Pin_t::output);
		asm("NOP");
		while (true) {
			//Handle posted functions until empty
			while (tail_ != head_) {
				uint8_t highestPrio = tasks_.at(tail_).prio;
				auto it = tail_;
				auto highest = tail_;
				while(it != head_){
					if(tasks_.at(it).prio > highestPrio){
						highestPrio = tasks_.at(it).prio;
						highest = it;
					}
					it++;
					if(it >= size) {it = 0;}
				}
				//Higher priority found?
				if(highestPrio > tasks_.at(tail_).prio){
					//Switch highest with mine
					auto temp = tasks_.at(tail_);
					tasks_.at(tail_) = tasks_.at(highest);
					tasks_.at(highest) = temp;
				}
				tasks_.at(tail_).f();
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
		void post(Callable&& c, uint8_t prio = 0) {
			IntLock l; //Disables interrupts for the scope of the function
			tasks_.at(head_).f.assignHandler(std::forward<Callable>(c)); // Store Function Call
			tasks_.at(head_).prio = prio;
			head_++;
			if(head_ >= size){
				head_ = 0;
			}
		}

	template<class Callable>
	void post_in_ms(Callable&& c, time_t ms, uint8_t prio = 0) {
		//Call through Timer:
		IntLock l;
		tm_.run_in_ms([this, c, prio]() {
			post(c, prio);
		}, ms.count());
	}

	template<class Callable>
			void rec_post_in_ms(Callable c, time_t ms, uint8_t prio = 0) {
				//Call through Timer:
			IntLock l;
				tm_.rec_run_in_ms([this, c, prio]() {
					post(c, prio);
				}, ms.count());
			}
private:
	struct Task_t{
		uint8_t prio;
		Func_t f;
	};
//Tasks to run
	std::array<Task_t, size> tasks_;
	TimerManager& tm_;
//Current Task Count
	volatile uint32_t head_;
	volatile uint32_t tail_;

};



#endif /* EVENT_QUEUE_PRIO_H_ */
