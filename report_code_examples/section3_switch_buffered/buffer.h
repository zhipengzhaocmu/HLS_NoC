/*
	Copyright (c) <2017> <Zhipeng Zhao and James C. Hoe, Carnegie Mellon university>

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/

#include <stdio.h>
#include <ap_int.h>
#include "ap_shift_reg.h"

#define FIFO_DEPTH 8
//log2 fifo_depth(when FIFO_DEPTH is power 2, we need plus one bit)
#define LOG2_FIFO_DEPTH 4
typedef ap_uint<LOG2_FIFO_DEPTH> fifo_cnt_t;

template <typename T>
class FIFO{
public:
	ap_shift_reg <T, FIFO_DEPTH> Sreg;
	fifo_cnt_t cnt;

	bool empty(){
		return (cnt == 0);
	}

	bool full(){
		return (cnt == FIFO_DEPTH);
	}

	T front(){
#pragma HLS INLINE
		int temp_cnt;
		if(empty())
			temp_cnt = 1;
		else
			temp_cnt = cnt;

		return Sreg.read(temp_cnt-1);
	}

	void next(T in, bool read_req, bool write_req){
#pragma HLS INLINE
		int temp_cnt;
		bool read_grant,write_grant;

		read_grant = !empty() && read_req;
		write_grant = !full() && write_req;

		if(full() && write_req){
			printf("WARNING: Write full!\n");
		}

		if(empty())
			temp_cnt = 1;
		else
			temp_cnt = cnt;

		if(write_grant)
			Sreg.shift(in, temp_cnt-1);

		//update cnt
		if(read_grant && !write_grant){
			cnt = cnt -1;
		}
		else if(write_grant && !read_grant){
			cnt = cnt + 1;
		}
	}
};
