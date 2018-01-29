/*
 * delay.h
 *
 *  Created on: 08.01.2018
 *      Author: Hannes
 */

#ifndef DELAY_H_
#define DELAY_H_

namespace delay{
	inline void cycles(uint32_t t){do{asm("NOP");}while(--t > 0);}
	inline void ms(uint32_t t){cycles(16000*t);} //Needs proper implementation
}



#endif /* DELAY_H_ */
