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

#include "foo_class.h"

void foo(int I1, int I2, int *O){
/*
 * These two INTERFACE pragmas are commented to pass the co-simulation.
 * If you do not want the ap_ctrl and ovld signals in generated verilog code,
 * you can uncomment these pragmas.
*/
//#pragma HLS INTERFACE ap_none port=O
//#pragma HLS INTERFACE ap_ctrl_none port=return
#pragma HLS LATENCY max=0

	static int L=0;

	*O=I1*L;
	L=I2+L;
}

template<int num> void foo_temp(int I1, int I2, int *O){
#pragma HLS LATENCY max=0
	static int L=0;

	*O=I1*L;
	L=I2+L;
};

void fxn_reuse_try(int I, int *O){
	int tmp;

	foo(I,I,&tmp);
	foo(tmp,tmp,O);
}

void fxn_reuse_correct(int I, int *O){
#pragma HLS LATENCY max=0
	int tmp;

	foo_temp<0>(I,I,&tmp);
	foo_temp<1>(tmp,tmp,O);
}

void fxn_ordering_try(int I, int *O){
	int tmp1;
	int tmp2;

	foo_temp<0>(I,tmp2,&tmp1);
	foo_temp<1>(tmp1,tmp1,&tmp2);
	*O=tmp2;
}

void top_ordering(int I, int *O){
#pragma HLS LATENCY max=0
//#pragma HLS INTERFACE ap_none port=O
//#pragma HLS INTERFACE ap_ctrl_none port=return

	static foo_class foo1,foo2;

	int tmp1;
	int tmp2;

	//combinational-query behaviors
	foo1.query(I,&tmp1);
	foo2.query(tmp1,&tmp2);
	*O=tmp2;

	//state-update behaviors
	foo1.update(tmp2);
	foo2.update(tmp1);
}
