/*
 * oled.h
 *
 *  Created on: 24.01.2018
 *      Author: Hannes
 */

#ifndef OLED_H_
#define OLED_H_

#include "delay.h"

template<class Spi_t, class Pin_t>
class Oled {
public:

	using buf_t = std::array<uint8_t, 1024>;

	Oled(Spi_t& spi, const Pin_t cs, const Pin_t dat, const Pin_t res) :
			spi_(spi), chipSelect_(cs), datSelect_(dat), res_(res) {

	}

	buf_t& getBuffer() const {
		return buf_;
	}

	void init();

	inline void drawPoint(const unsigned int x, const unsigned int y) {
		if ((x > 127) || (y > 63)) {
			return;
		}
		buf_[x + (y / 8) * 128] |= (1 << (y % 8));
	}

	inline void clear() {
		buf_.fill(0);
	}
	void display() {
		spi_.setTransferCompleteCallback([this]() {deselect();});
		select();
		mode_data();
		spi_.send((uint32_t) &buf_, 1024);
	}
	inline void select() const {
		chipSelect_.clr();
	}
	inline void deselect() const {
		chipSelect_.set();
	}
	inline void mode_data() const {
		datSelect_.set();
	}
	inline void mode_cmd() const {
		datSelect_.clr();
	}
private:
	Spi_t& spi_;
	const Pin_t chipSelect_;
	const Pin_t datSelect_;
	const Pin_t res_;

	buf_t buf_;
};

template<class Spi_t, class Pin_t>
void Oled<Spi_t, Pin_t>::init() {

		spi_.setTransferCompleteCallback([this]() {deselect();});

		select();
		mode_cmd();

		res_.set();
		delay::ms(1);
		res_.clr();
		delay::ms(1);
		res_.set();
		delay::ms(1);

		uint8_t conf[25] = { 0xAE, 0xD5, 0x80, 0xA8, 0x3F, 0xD3, 0x00, 0x40,
				0xA1, 0xC8, 0xDA, (0x1 << 1) | (0x1 << 4) | (0x0 << 5), 0xD9,
				0xF1, 0x8D, 0x14, 0x20, 0x00, 0xAF, 0x21, 0x00, 0x7F, 0x22, 0x00, 0x07 };
		//copy buffer in image buffer
		for (uint32_t i = 0; i < 25; i++) {
			buf_[i] = conf[i];
		}
		//send the imagebuffer
		spi_.send((uint32_t) &buf_, 25);
		delay::ms(1);
	}

#endif /* OLED_H_ */
