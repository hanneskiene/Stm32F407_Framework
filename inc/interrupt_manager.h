/*
 * irq_ctrl.h
 *
 *  Created on: 24.01.2018
 *      Author: Hannes
 */

#ifndef INTERRUPT_MANAGER_H_
#define INTERRUPT_MANAGER_H_

#include "mcu/stm32f407xx.h"
#include "util/staticfunction.h"
#include <array>

//
//{IrqLock l; ... ... ...} disables Interrupts during the scope of l and restores them to their previous state
//
class IntLock {
public:
	IntLock() {
		prev_disabled_ = __get_PRIMASK();
		__disable_irq();
	}
	~IntLock() {
		if(!prev_disabled_){
			__enable_irq();
		}
	}
private:
	volatile bool prev_disabled_;
};

extern"C"{
void Default_Handler();
void SysTick_Handler();
//Not needed
void NMI_Handler();
void HardFault_Handler();
void MemManage_Handler();
void BusFault_Handler();
void UsageFault_Handler();
void SVC_Handler();
void DebugMon_Handler();
void PendSV_Handler();
}


class InterruptDispatcher;
extern InterruptDispatcher interrupt_dispatcher;

class InterruptDispatcher{
public:

	using Func_t = StaticF<void()>;

	static InterruptDispatcher& getInstance(){
		return interrupt_dispatcher;
	}

	void dispatch(unsigned int i){
		(functions_.at((int)i))();
	}
	void onSysTick(){
		systick_callback_();
	}

	template<class Callable>
	void register_callback(Callable c, IRQn_Type num){
		functions_.at((unsigned int) num) = Func_t(c);
	}
	template<class Callable>
		void register_systick_callback(Callable c){
			systick_callback_ = Func_t(c);
		}

	InterruptDispatcher(){}
private:
	std::array<Func_t, 90> functions_;
	Func_t systick_callback_;
};

#endif /* INTERRUPT_MANAGER_H_ */
