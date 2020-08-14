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

/* In this example, we want to implement a memory block with
 * one asynchronous read port and one synchronous write port.
 * However, Vivado-HLS can only result in FF array or LUT-RAM
 * with synchronous read port.
 */
#include <ap_int.h>

void ram(ap_uint<3> raddr, int *rdata, ap_uint<3> waddr, int wdata){
#pragma HLS INTERFACE ap_ctrl_none port=return
#pragma HLS LATENCY max=0
	static int X[8];

//Fully partition X will lead to FF array
//#pragma HLS ARRAY_PARTITION variable=X complete dim=1

//Map to LURAM will lead to synchronous read port.
//#pragma HLS RESOURCE variable=X core=RAM_2P

	*rdata=X[raddr];
	X[waddr]=wdata;
}

