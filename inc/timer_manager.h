/*
 * timer_manager.h
 *
 *  Created on: 24.01.2018
 *      Author: Hannes
 */

#ifndef TIMER_MANAGER_H_
#define TIMER_MANAGER_H_

#include <array>
#include <utility>

#include "interrupt_manager.h"

template<class Func_t>
class TimerManager {
public:
	using time_t = unsigned int;
	enum{ size = 128 };

	TimerManager() {
		task_t t;
		t.used = false;
		t.time = 0;
		t.func = Func_t();
		t.resetVal = 0;
		tasks_.fill(t);

		InterruptDispatcher::getInstance().register_systick_callback([this](){
			tick();
		});
		SysTick_Config(16000000/1000);
	}

	template<class Callable>
	bool run_in_ms(Callable c, time_t ms) {
		IntLock l;

		for (auto& p : tasks_) {
			if (!p.used) {
				p.time = ms;
				p.func.assignHandler(c);
				p.used = true;
				p.resetVal = 0;
				return true;
			}
		}
		return false;
	}
	template<class Callable>
		bool rec_run_in_ms(Callable c, time_t ms) {
			IntLock l;

			for (auto& p : tasks_) {
				if (!p.used) {
					p.time = ms;
					p.func.assignHandler(c);
					p.used = true;
					p.resetVal = ms;
					return true;
				}
			}
			return false;
		}

	//Has to be called each ms
	void tick() {
		for (auto& p : tasks_) {
			if (p.used) {
				p.time -= 1;
				if (p.time == 0) {
					p.func();
					if(p.resetVal){
						p.time = p.resetVal;
					}else{
						p.used = false;
					}
				}
			}
		}
	}
private:
	template<class T_t, class F_t>
	struct Task {
		volatile bool used;
		volatile T_t resetVal;
		volatile T_t time;
		F_t func;
	};
	using task_t = Task<time_t, Func_t>;
	std::array<task_t, size> tasks_;
};

#endif /* TIMER_MANAGER_H_ */
