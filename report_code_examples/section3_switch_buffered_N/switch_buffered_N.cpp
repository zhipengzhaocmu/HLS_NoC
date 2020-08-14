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

void switch_buffered_N(VData I[N],
		 VData *Odd, VData *Even,
		 bool acpt[N]) {
#pragma HLS INTERFACE ap_none port=acpt
#pragma HLS INTERFACE ap_none port=Even
#pragma HLS INTERFACE ap_none port=Odd
#pragma HLS INTERFACE ap_ctrl_none port=return
#pragma HLS ARRAY_PARTITION variable=acpt complete dim=1
#pragma HLS ARRAY_PARTITION variable=I complete dim=1
#pragma HLS LATENCY max=0

  static FIFO<int> F[N];
  bool okX[N];

  VData frontX[N];

  for(int i=0;i<N;i++){
#pragma HLS UNROLL
	  frontX[i].v=!F[i].empty();
	  frontX[i].d=F[i].front();
  }

  switch_2stage_N
    (frontX, Odd, Even, okX);

  for(int i=0;i<N;i++){
#pragma HLS UNROLL
	  if (!F[i].full() && I[i].v) { acpt[i]=true; }
	  else acpt[i]=false;

	  //update the buffer by providing input_data + read_req + write_req
	  F[i].next(I[i].d,okX[i],I[i].v);
  }
}
