/*
 * Irq_Ctrl.cpp
 *
 *  Created on: 24.01.2018
 *      Author: Hannes
 */

#include "../inc/interrupt_manager.h"
#include "../inc/mcu/pin_device.h"

InterruptDispatcher interrupt_dispatcher;

void Default_Handler() {

	for (int i = 0; i < 81; i++) {
		volatile bool active = NVIC_GetActive((IRQn_Type) i);
		if (active) {
				InterruptDispatcher::getInstance().dispatch(i);
				return;
		}
	}
	mcu::Pin red(mcu::Pin::portE, 7, mcu::Pin::output);
	while(true){
		red.tgl();
	}
}

void SysTick_Handler(){
	InterruptDispatcher::getInstance().onSysTick();
}
void NMI_Handler(){
	while(true);
}
void HardFault_Handler(){
	while(true);
}
void MemManage_Handler(){
	while(true);
}
void BusFault_Handler(){
	while(true);
}
void UsageFault_Handler(){
	while(true);
}
void SVC_Handler(){
	while(true);
}
void DebugMon_Handler(){
	while(true);
}
void PendSV_Handler(){
	while(true);
}


