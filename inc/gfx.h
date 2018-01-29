/*
 * graphics.h
 *
 *  Created on: 26.01.2018
 *      Author: Hannes
 */

#ifndef GFX_H_
#define GFX_H_

#include "font.h"
#include <cmath>

#include <gsl/string_span>

//
//Output Type must expose public: 	drawPoint(unsigned x, unsigned y)
//									clear() //Clear the buffer
//									display() //Transmit buffer to device
//									unsigned getWidth()
//									unsigned getHeight()
//

	struct Point{
		explicit Point(int pX, int pY): x(pX), y(pY){}
		int x;
		int y;
	};

template<class output_t>
class Gfx {
public:
	enum {colWidth = 6, lineHeight = 10};
	enum {sizeX = 128, sizeY = 64}; //Add as private Members?

	class Cursor{
	public:
		Cursor():x(0), y(0){}
		Cursor(int line, int col):x(col*colWidth), y(line*lineHeight){}

		inline void set(int line, int col){
			x = col*colWidth;
			y = line*lineHeight;
		}
		inline void setPoint(Point p){
					x = p.x;
					y = p.y;
				}
		inline void newLine(){
			x = 0;
			y += lineHeight;
		}

		Cursor& operator++(int inc){
			x += colWidth;
			if(x >= sizeX){
				x = 0;
				y += lineHeight;
			}
			return *this;
		}
		bool operator==(const Cursor& other){
					if(x == other.x && y == other.y){
						return true;
					}else{
						return false;
					}
				}

		inline void reset(){x = 0; y = 0;}

		int x;
		int y;
	};

	Gfx(output_t& o) :
			out_(o), cursor_() {
	}

	Gfx& operator<<(const char c[]) {
		text(gsl::ensure_z(c), cursor_);
		return *this;
	}

	Gfx& operator<<(int n) {
		char c[12]; //10+2 digits max
		text(gsl::ensure_z(intToText(n, c)), cursor_);
		return *this;
	}

	//returns 0 terminated string beginning at location c
	char* intToText(int n, char* c);

	void writeChar(const char c, const Cursor& cur);

	void text(gsl::cstring_span<> s, Cursor& cur);

	void rect(const Point p, const int w, const int h);

	inline void display() {
		out_.display();
	}
	inline void clear() {
		out_.clear();
		cursor_.reset();
	}

private:
	output_t& out_;
	Cursor cursor_;
};

template<class output_t>
char* Gfx<output_t>::intToText(int n, char* c){
		if(n == 0){c[0] = '0'; c[1] = '\0'; return c;} //Number = 0
		bool neg = false;
		unsigned int num;
		if (n > 0) {
			c[0] = '0';
			num = n;
		} else {
			neg = true;
			//Abs
			const int mask = n >> 31;
			num = ((n + mask) ^ mask);
		}
		//Calculate Digits
		int s = 10;
		while ((s > 0)) {
			c[s] = (num % 10) + '0';
			num /= 10;
			s--;
		}
		c[11] = '\0';
		//Remove leading Zeros
		unsigned int i = 1;
		while (c[i] == '0'){
			i++;
		}
		//Add sign
		if(neg){
			c[i-1] = '-'; //Stores Sign
			i--;
		}
		return &c[i];
	}

template<class output_t>
void Gfx<output_t>::writeChar(const char c, const Cursor& cur) {
		for (uint8_t i = 0; i < 5; i++) {
			uint8_t line = font[c * 5 + i];
			for (uint8_t j = 0; j < 8; j++) {
				if (line & 0x1) {
					out_.drawPoint(cur.x + i, cur.y + j);
				}
				line >>= 1;
			}
		}
	}

template<class output_t>
void Gfx<output_t>::text(gsl::cstring_span<> s,  Cursor& cur) {
		//Check whether text is supposed to be at current gfx cursor
		bool in_line = false;
		if (cur == cursor_) {
			in_line = true;
		}
		for(const auto& c: s){
			if(c == '\n'){
				cur.newLine();
			}else{
				writeChar(c, cur);
				cur++;
			}
		}
		//Update gfx cursor
		if (in_line) {
			cursor_ = cur;
		}
	}

template<class output_t>
void Gfx<output_t>::rect(const Point p, const int w, const int h) {
		for (int i = 0; i <= h; i++) {
			out_.drawPoint(p.x, p.y + i);
			out_.drawPoint(p.x + w, p.y + i);
		}
		for (int i = 0; i <= w; i++) {
			out_.drawPoint(p.x + i, p.y);
			out_.drawPoint(p.x + i, p.y + h);
		}
	}

#endif /* GFX_H_ */
