/*
 * main.cpp
 *
 *  Created on: 21.01.2018
 *      Author: Hannes
 */
//#include "../inc/event_queue.h"
#include "../inc/event_queue_prio.h"
#include "../inc/interrupt_manager.h"

#include "../inc/mcu/spi_device.h"
#include "../inc/mcu/dma_device.h"
#include "../inc/mcu/pin_device.h"
#include "../inc/timer_manager.h"
#include "../inc/oled.h"
#include "../inc/util/staticfunction.h"

#include <functional>
#include "../inc/gfx.h"

// NO statics that require a constructor in functions, if needed compile with ?-fno_threadsafe_statics
//Requires the Guideline Support Library by Microsoft (add to eclipse include paths)

int main() {

	__disable_irq();

	//Callback function Type
	using Func_t = StaticF<void(), 20>;
	//Pin alias
	using Pin_t = mcu::Pin;


	//
	//Module Initialization Code
	//

	//TimerManager Setup
	using TimerManager_t = TimerManager<Func_t>;
	TimerManager_t timer_manager;

	using EventQueue_t = EventQueuePrio<TimerManager_t, Func_t>;
	EventQueue_t event_queue(timer_manager);


	//DMA Setup
	using DmaDSpi_t = mcu::DmaDevice<EventQueue_t, Func_t>;
	DmaDSpi_t dma_spi(event_queue, DmaDSpi_t::dma2, DmaDSpi_t::stream3,
			DmaDSpi_t::channel3);

	//Spi Setup
	using SPI_t = mcu::SpiDevice<EventQueue_t, DmaDSpi_t, Pin_t, Func_t>;
	SPI_t spi_d(event_queue, dma_spi, SPI_t::spi1, SPI_t::fast,
			Pin_t(Pin_t::portA, 5, Pin_t::alternate, Pin_t::spi_1_2),
			Pin_t(Pin_t::portA, 7, Pin_t::alternate, Pin_t::spi_1_2), SPI_t::master,
			SPI_t::mode_dma);

	//Oled Setup
	using Oled_t = Oled<SPI_t, Pin_t>;
	Oled_t oled_d(spi_d, Pin_t(Pin_t::portA, 3, Pin_t::output),
			Pin_t(Pin_t::portA, 6, Pin_t::output),
			Pin_t(Pin_t::portA, 4, Pin_t::output));

	__enable_irq();

	//
	//Application Code
	//

	oled_d.init();//Oled init needs working interrupts

	Gfx<Oled_t> screen(oled_d);
	screen.clear();
	screen << "Boot complete ";
	screen.display();

	event_queue.rec_post_in_ms([&]() {
		static int z = 0;
		z += 1;
		screen.clear();
		screen << "Calls: " << z << "\n";
		screen.rect(Point(70, 0), (z/10)<70?z/10:z/100, 5);
		screen << "Value 1: " << z/2*3+4 << "\n";
		screen << "Value 2: " << z/4*2+z/7 << "\n";
		screen << "Value 3: " << z*z/7+z*5/50 << "\n";
		screen << "Value 4: " << -z*5+z/7 << "\n";
		screen << "Status: " << ((z%5 < 3) ? "OK" : "Still OK :(") << "\n";
		screen.display();
	}, 1000ms, 100);


	// Indicates running
	Pin_t green(Pin_t::portE, 9, Pin_t::output);
	event_queue.rec_post_in_ms([&]() {
		green.tgl();
	}, 250ms, 0);

//	screen.clear();
//	for(int i = 0; i < 10; i++){
//	event_queue.rec_post_in_ms([&, i](){screen << " P:" << i ; screen.display();}, 1000, i);
//	}

	event_queue.run(); //Should never exit
	while (true) {
	}
	return 0;
}

//Needed, reason unknown
void operator delete(void*, unsigned int) {
}
//Needed to disable Exception code generation
namespace std {
void terminate(){
	while(true);
}
void __throw_out_of_range_fmt(char const*, ...) {
	while (true)
		;
}
}
//Clock initialization
extern "C" int SystemInit() {

	return 0;
}
/*
 extern "C" int _init()
 {

 return 0;
 }
 */

//If one of these symbols is missing,
//disable stdlib and override function
//that is calling said symbol
//*
extern "C" {
void _exit() {
}
void _kill() {
}
void _getpid() {
}
void _write() {
}
void _close() {
}
void _fstat() {
}
void _isatty() {
}
void _lseek() {
}
void _read() {
}
void _sbrk() {
}

int __cxa_guard_acquire(int* z) {
	while (true)
		;
}
void __cxa_guard_release(int* z) {
}
void __cxa_pure_virtual() {
	while (true)
		;
}
}
//*/

