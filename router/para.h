/*
	Copyright (c) <2017> <Zhipeng Zhao, Carnegie Mellon University>

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

/*
	Parameters for router configuration
*/
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include <math.h>
#include <stdint.h>
#include <cstdlib>
#include <ap_int.h>
#include "ap_shift_reg.h"

//Number of receiving endpoints: LOG2NODE affects the bit-width of dest_t field in flit struct. 
#define NODE 2
#define LOG2NODE 1

//Port number:
#define N 4
#define M 2
#define LOG2N 2
#define LOG2M 1

//Router Type: IQ, VC, VOQ (Default:IQ)
#define IQ

//Number of VCs: 2-8
#define VC_NUM 2
#define LOG2VC 1

//Flow Control Type: PEEK, CREDIT (Default:PEEK; Only PEEK when VOQ)
#ifdef VOQ
	#define PEEK//unchangeble
#else
	#define PEEK
#endif

//Flit Data Width: 8-1024
#define DATA_WIDTH 32

//Flit Buffer Depth: 4-64
#define FIFO_DEPTH 8
//log2 fifo_depth(when FIFO_DEPTH is power 2, need to plus one bit)
#define LOG2_FIFO_DEPTH 4

//Allocator: SEPIF, SEPOF (Default:SEPIF)
#define SEPIF

//Arbiter: STATIC, ROUNDROBIN (Default:ROUNDROBIN)
#define ROUNDROBIN

//Virtual Link: VL, NO_VL (Default:NO_VL)
//#define NO_VL

//Pipeline: PIPE_CORE, PIPE_ALLOC, PIPE_LINK (Default:NO_PIPE)
//#define NO_PIPE

//Buffer_num in one port
#ifdef VC
	#define B VC_NUM
#else
	#ifdef VOQ
		#define B M
	#else
		#define B 1
	#endif//VOQ
#endif//VC

typedef ap_uint<N> in_port_t;
typedef ap_uint<LOG2N> in_port_num_t;
typedef ap_uint<M> out_port_t;
typedef ap_uint<LOG2M> out_port_num_t;
typedef ap_uint<LOG2_FIFO_DEPTH> fifo_cnt_t;
typedef ap_uint<LOG2NODE> dest_t;
typedef ap_uint<VC_NUM> vc_t;
typedef ap_uint<B> b_t;

//UG902 P161: the first element of the struct is aligned on the LSB of the vector
//and the final element of the struct is aligned with the MSB of the vector.
struct flit_t {
	ap_uint<DATA_WIDTH> data;
	ap_uint<LOG2VC> vc;
	ap_uint<LOG2NODE>  dest;
	ap_uint<1> tail;
	ap_uint<1> valid;
};

struct RT_flit_t {
	out_port_num_t port_num;
	flit_t flit;
};
